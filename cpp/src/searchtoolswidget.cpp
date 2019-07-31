#include <iostream>
#include <string>
#include <regex>
#include <QtWidgets>
#include "searchtoolswidget.h"

#define BOUTIQUES_CACHE_PATH "/.cache/boutiques"
#define DATABASE_NAME "all-descriptors.json"

SearchToolsWidget::SearchToolsWidget(QWidget *parent) : QWidget(parent), mustCreateToolDatabase(false)
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
    this->info->setMinimumHeight(300);
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

    this->createProcesses();
}

SearchResult *SearchToolsWidget::getSelectedTool()
{
    int currentRow = this->table->currentRow();
    return currentRow >= 0 && currentRow < int(this->searchResults.size()) ? &this->searchResults[static_cast<unsigned int>(currentRow)] : nullptr;
}

void SearchToolsWidget::createTable()
{
    this->table = new QTableWidget();
    this->table->setMinimumHeight(150);
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

void SearchToolsWidget::createProcesses()
{
    this->searchProcess = new QProcess(this);
    connect(this->searchProcess, &QProcess::errorOccurred, this, &SearchToolsWidget::errorOccurred);
    connect(this->searchProcess, &QProcess::started, this, &SearchToolsWidget::searchProcessStarted);
    connect(this->searchProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::searchProcessFinished);

    this->pprintProcess = new QProcess(this);
    connect(this->pprintProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::pprintProcessFinished);

    this->pullProcess = new QProcess(this);
    connect(this->pullProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::pullProcessFinished);
}

void SearchToolsWidget::downloadTools()
{
    this->mustCreateToolDatabase = true;
    this->searchProcess->kill();
    this->searchProcess->start(BOSH_PATH, {"search", "-m 1000"});
}

void SearchToolsWidget::selectionChanged()
{
    SearchResult *tool = this->getSelectedTool();
    if(tool == nullptr) {
        return;
    }

    this->pprintProcess->kill();
    this->pprintProcess->start(BOSH_PATH, {"pprint", tool->id.c_str()});
    this->loadingLabel->setText("Getting tool help...");
}

void SearchToolsWidget::pprintProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QByteArray result = this->pprintProcess->readAll();
    this->info->setText(QString::fromUtf8(result));

    this->infoLabel->show();
    this->info->show();
    emit toolSelected();
}

void SearchToolsWidget::searchBoutiquesTools()
{
    QString searchQuery = this->searchLineEdit->text();

    if(!this->descriptors.isEmpty())
    {
        searchResults.clear();
        for(int i=0 ; i<this->descriptors.size() ; i++)
        {
            const QJsonObject &descriptor = this->descriptors.at(i).toObject();
            const QString &name = descriptor["name"].toString();
            const QString &description= descriptor["description"].toString();
            if(name.contains(searchQuery, Qt::CaseInsensitive) || description.contains(searchQuery, Qt::CaseInsensitive))
            {
                this->searchResults.emplace_back();
                SearchResult& searchResult = this->searchResults.back();
                searchResult.title = name.toStdString();
                searchResult.description = description.toStdString();
            }
        }
        return;
    }
    this->loadingLabel->show();
    this->table->hide();
    this->infoLabel->hide();
    this->info->hide();


    this->searchProcess->kill();
    this->searchProcess->start(BOSH_PATH, {"search", "-m 50", searchQuery});

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

void SearchToolsWidget::searchProcessStarted()
{
    this->loadingLabel->setText("Searching for tools...");
}

void SearchToolsWidget::searchProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    if(exitStatus == QProcess::ExitStatus::NormalExit && this->mustCreateToolDatabase)
    {
        this->createToolDatabase();
        return;
    }

    this->loadingLabel->hide();
    this->table->show();

    QString output = QString::fromUtf8(this->searchProcess->readAll());

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

void SearchToolsWidget::createToolDatabase()
{

    QString output = QString::fromUtf8(this->searchProcess->readAll());

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
        return;
    }

    line = lines[1];
    string::size_type idIndex = line.find("ID");
    string::size_type titleIndex = line.find("TITLE");
    string::size_type downloadsIndex = line.find("DOWNLOADS");
    this->table->setRowCount(int(lines.size() - 2));

    searchResults.clear();
    this->zenodoIdsAndDownloads.clear();
    for(unsigned int i=2 ; i<lines.size() ; i++){
        string line = lines[i];

        const QString &zenodoId = QString::fromStdString(trim(line.substr(idIndex, titleIndex - idIndex)));
        try
        {
            int nDownloads = stoi(trim(line.substr(downloadsIndex, line.size() - 1)));
            this->zenodoIdsAndDownloads.push_back(pair<QString, int>(zenodoId, nDownloads));
        } catch (const invalid_argument& ia)
        {
            Q_UNUSED(ia)
            continue;
        }

    }

//    this->zenodoIdsAndDownloads.insert(0, "pull");
//    this->pullProcess->kill();
//    this->pullProcess->start(BOSH_PATH, this->zenodoIdsAndDownloads);

}

void SearchToolsWidget::pullProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QDir cacheDirectory(QDir::homePath() + BOUTIQUES_CACHE_PATH);

    QJsonArray descriptors;
    for(const pair<QString, int> &zeonodIdAndDownload : this->zenodoIdsAndDownloads)
    {
        QString zenodoId = zeonodIdAndDownload.first;
        int zenodoDownloads = zeonodIdAndDownload.second;
        QString descriptorFileName = zenodoId.replace(QChar('.'), QChar('-')) + ".json";
        QFile file(cacheDirectory.absoluteFilePath(descriptorFileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, "Could not open descriptor file", "Error while opening descriptor file (" + descriptorFileName + ") from ~" + BOUTIQUES_CACHE_PATH);
            return;
        }

        QJsonDocument descriptorDocument(QJsonDocument::fromJson(file.readAll()));
        QJsonObject descriptorObject = descriptorDocument.object();
        descriptorObject["id"] = zenodoId;
        descriptorObject["downloads"] = zenodoDownloads;
        descriptors.push_back(descriptorObject);
    }

    QFile descriptorsFile(cacheDirectory.absoluteFilePath(DATABASE_NAME));
    if (!descriptorsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Could not open descriptors file", "Error while opening descriptors file.");
        return;
    }
    QJsonDocument descriptorsDocument(descriptors);
    descriptorsFile.write(descriptorsDocument.toJson());
}

void SearchToolsWidget::loadToolDatabase()
{
    QDir cacheDirectory(QDir::homePath() + BOUTIQUES_CACHE_PATH);

    QFile file(cacheDirectory.absoluteFilePath(DATABASE_NAME));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Could not open descriptor file", "Error while opening descriptors file.");
        return;
    }
    QJsonDocument descriptorsDocument(QJsonDocument::fromJson(file.readAll()));
    this->descriptors = descriptorsDocument.array();
}
