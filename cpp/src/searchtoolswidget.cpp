#include <iostream>
#include <string>
#include <regex>
#include <QtWidgets>
#include "searchtoolswidget.h"


SearchToolsWidget::SearchToolsWidget(QWidget *parent) : QWidget(parent)
{
    this->searchLineEdit = new QLineEdit();
    this->searchLineEdit->setPlaceholderText("Search a tool in Boutiques...");
    this->button = new QPushButton("Search");

//    Search input
    this->searchGroupBox = new QGroupBox();
    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(this->searchLineEdit);
    searchLayout->addWidget(this->button);
    this->searchGroupBox->setLayout(searchLayout);

    this->loadingLabel = new QLabel("Loading...");
    this->loadingLabel->hide();
    this->createTable();

    this->infoLabel = new QLabel("Tool info");
    this->infoLabel->hide();
    this->info = new QTextEdit();
    this->info->setReadOnly(true);
    this->info->hide();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(this->searchGroupBox);
    layout->addWidget(this->loadingLabel);
    layout->addWidget(this->table);
    layout->addWidget(this->infoLabel);
    layout->addWidget(this->info);
    this->setLayout(layout);

    connect(this->button, &QPushButton::clicked, this, &SearchToolsWidget::searchBoutiquesTools);
    connect(this->searchLineEdit, &QLineEdit::returnPressed, this, &SearchToolsWidget::searchBoutiquesTools);

    this->createProcess();
}

SearchResult *SearchToolsWidget::getSelectedTool()
{
    int currentRow = this->table->currentRow();
    return currentRow >= 0 && currentRow < int(this->searchResults.size()) ? &this->searchResults[static_cast<unsigned int>(currentRow)] : nullptr;
}

void SearchToolsWidget::createTable()
{
    this->table = new QTableWidget();
    this->table->setRowCount(0);
    this->table->setColumnCount(4);
    this->table->move(0, 0);
    this->table->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->table->setHorizontalHeaderLabels({ "ID", "Title", "Description", "Downloads" });
    connect(this->table, &QTableWidget::itemSelectionChanged, this, &SearchToolsWidget::selectionChanged);
    this->table->hide();
}

void SearchToolsWidget::createProcess()
{
    this->process = new QProcess(this);
    connect(this->process, &QProcess::errorOccurred, this, &SearchToolsWidget::errorOccurred);
    connect(this->process, &QProcess::started, this, &SearchToolsWidget::processStarted);
    connect(this->process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::processFinished);
}

void SearchToolsWidget::selectionChanged()
{
    SearchResult *tool = this->getSelectedTool();
    if(tool == nullptr) {
        return;
    }

    QProcess bosh;
    bosh.start(BOSH_PATH, {"pprint", tool->id.c_str()});
    if (!bosh.waitForFinished()) {
        return;
    }

    QByteArray result = bosh.readAll();
    this->info->setText(QString::fromUtf8(result));

    this->infoLabel->show();
    this->info->show();
    emit toolSelected();
}

void SearchToolsWidget::searchBoutiquesTools()
{
    this->loadingLabel->show();
    this->table->hide();
    this->infoLabel->hide();
    this->info->hide();

    QString searchQuery = this->searchLineEdit->text();

    this->process->kill();
    this->process->start(BOSH_PATH, {"search", "-m 50", searchQuery});

    this->loadingLabel->setText("Search launched...");

    emit toolDeselected();

    this->searchResults.clear();
}

// trim from start (in place)
static inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) { return !isspace(ch); }));
}

// trim from end (in place)
static inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](int ch) { return !isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
static inline string trim(string s) {
    ltrim(s);
    rtrim(s);
    return s;
}

void SearchToolsWidget::errorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    this->loadingLabel->setText("An error occurred while searching the tool.");
}

void SearchToolsWidget::processStarted()
{
    this->loadingLabel->setText("Searching for tools...");
}

void SearchToolsWidget::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    this->loadingLabel->hide();
    this->table->show();

    QString output = QString::fromUtf8(this->process->readAll());

    regex e("\x1b\[[0-9;]*[mGKF]");

    string outputClean = regex_replace(output.toStdString(), e, "");

    stringstream outputSream(outputClean);
    string line;
    vector<string> lines;
    while (getline(outputSream, line))
    {
        lines.push_back(line);
    }
    if(lines.size() < 2)
    {
        if(output.contains("Error"))
        {
            QMessageBox messageBox;
            messageBox.setText("Error while searching tools in Zenodo database.");
            messageBox.setInformativeText("Do you want to open fake search results?");
            messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::No);
            messageBox.setDefaultButton(QMessageBox::No);
            int returnCode = messageBox.exec();
            if(returnCode == QMessageBox::Ok)
            {
                QDir dataDirectory("../../data/");
                QString fakeResultFileName = dataDirectory.absoluteFilePath("fakeSearchResult.txt");
                QFile fakeResultFile(fakeResultFileName);
                if (fakeResultFile.open(QIODevice::ReadOnly | QIODevice::Text))
                {
                    stringstream fakeOutputSream(fakeResultFile.readAll().toStdString());
                    lines.clear();
                    while (getline(fakeOutputSream, line))
                    {
                        lines.push_back(line);
                    }
                    if(lines.size() < 2)
                    {
                        QMessageBox::critical(this, "Error reading fake search result file", "Not result found.");
                        return;
                    }
                }
                else
                {
                    QMessageBox::critical(this, "Error reading fake search result file", "File not found.");
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
    }

    line = lines[1];
    string::size_type idIndex = line.find("ID");
    string::size_type titleIndex = line.find("TITLE");
    string::size_type descriptionIndex = line.find("DESCRIPTION");
    string::size_type downloadsIndex = line.find("DOWNLOADS");
    this->table->setRowCount(int(lines.size() - 2));

    searchResults.clear();
    for(unsigned int i=2 ; i<lines.size() ; i++){
        string line = lines[i];

        this->searchResults.emplace_back();
        SearchResult& searchResult = this->searchResults.back();
        searchResult.id = trim(line.substr(idIndex, titleIndex - idIndex));
        searchResult.title = trim(line.substr(titleIndex, descriptionIndex - titleIndex));
        searchResult.description = trim(line.substr(descriptionIndex, downloadsIndex - descriptionIndex));
        try
        {
            searchResult.downloads = stoi(trim(line.substr(downloadsIndex, line.size() - 1)));
        } catch (const invalid_argument& ia)
        {
            Q_UNUSED(ia)
            continue;
        }

        this->table->setItem(int(i-2), 0, new QTableWidgetItem(QString::fromStdString(searchResult.id)));
        this->table->setItem(int(i-2), 1, new QTableWidgetItem(QString::fromStdString(searchResult.title)));
        this->table->setItem(int(i-2), 2, new QTableWidgetItem(QString::fromStdString(searchResult.description)));
        this->table->setItem(int(i-2), 3, new QTableWidgetItem(QString::fromStdString(to_string(searchResult.downloads))));

    }
}
