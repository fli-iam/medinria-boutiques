#include <iostream>
#include <string>
#include <regex>
#include <QtWidgets>
#include "searchtoolswidget.h"

#define BOUTIQUES_CACHE_PATH "/.cache/boutiques"
#define DATABASE_NAME "all-descriptors.json"

SearchToolsWidget::SearchToolsWidget(QWidget *parent) : QWidget(parent), toolDatabaseUpdated(false)
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
    this->createSearchView();

    this->infoLabel = new QLabel("Tool info");
    this->infoLabel->hide();
    this->info = new QTextEdit();
    this->info->setMinimumHeight(300);
    this->info->setReadOnly(true);
    this->info->hide();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(this->searchGroupBox);
    layout->addWidget(this->loadingLabel);
//    layout->addWidget(this->table);
    layout->addWidget(this->searchView);
    layout->addWidget(this->infoLabel);
    layout->addWidget(this->info);
    this->setLayout(layout);

    connect(this->button, &QPushButton::clicked, this, &SearchToolsWidget::searchBoutiquesTools);

    connect(this->searchLineEdit, &QLineEdit::textChanged, this, &SearchToolsWidget::searchChanged);
    connect(this->searchLineEdit, &QLineEdit::returnPressed, this, &SearchToolsWidget::searchBoutiquesTools);

    this->createProcesses();
    this->loadToolDatabase();
    this->downloadToolDatabase();
}

ToolDescription *SearchToolsWidget::getSelectedTool()
{
//    int currentRow = this->table->currentRow();
    int currentRow = this->proxyToolModel->mapToSource(this->searchView->currentIndex()).row();
    int currentToolIndex = this->toolModel->index(currentRow, 0).data(Qt::UserRole).toInt();
    return currentToolIndex >= 0 && currentToolIndex < int(this->searchResults.size()) ? &this->searchResults[static_cast<unsigned int>(currentToolIndex)] : nullptr;
}

void SearchToolsWidget::createSearchView()
{
    this->proxyToolModel = new QSortFilterProxyModel();
    this->proxyToolModel->setFilterKeyColumn(-1);
    this->proxyToolModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    this->searchView = new QTreeView();
    this->searchView->setRootIsDecorated(false);
    this->searchView->setAlternatingRowColors(true);
    this->searchView->setSortingEnabled(true);

    this->searchView->setMinimumHeight(150);
    this->searchView->setUniformRowHeights(true);
//    void QAbstractItemView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)

    this->toolModel = new QStandardItemModel(0, 4, this);

    this->toolModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Id"));
    this->toolModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Title"));
    this->toolModel->setHeaderData(2, Qt::Horizontal, QObject::tr("Description"));
    this->toolModel->setHeaderData(3, Qt::Horizontal, QObject::tr("Downloads"));

    this->proxyToolModel->setSourceModel(this->toolModel);
    this->searchView->setModel(this->proxyToolModel);

    QHeaderView *viewHeader = this->searchView->header();
    viewHeader->setDefaultSectionSize(100);
    viewHeader->setSectionResizeMode(0, QHeaderView::Interactive);
    viewHeader->setSectionResizeMode(1, QHeaderView::Interactive);
    viewHeader->setSectionResizeMode(2, QHeaderView::Stretch);
    viewHeader->setSectionResizeMode(3, QHeaderView::Fixed);
    viewHeader->setStretchLastSection(false);

    this->searchView->setColumnWidth(0, 50);
    this->searchView->setColumnWidth(1, 250);
    this->searchView->setColumnWidth(3, 80);

    this->searchView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(this->searchView->selectionModel(), &QItemSelectionModel::currentChanged, this, &SearchToolsWidget::selectionChanged);
//    this->searchView->hide();

//    this->table = new QTableWidget();
//    this->table->setMinimumHeight(150);
//    this->table->setRowCount(0);
//    this->table->setColumnCount(4);
//    this->table->move(0, 0);
//    this->table->setSelectionBehavior(QAbstractItemView::SelectRows);
//    this->table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    this->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    this->table->setHorizontalHeaderLabels({ "ID", "Title", "Description", "Downloads" });
//    connect(this->table, &QTableWidget::itemSelectionChanged, this, &SearchToolsWidget::selectionChanged);
//    this->table->hide();
}

void SearchToolsWidget::createProcesses()
{
    this->searchProcess = new QProcess(this);
    connect(this->searchProcess, &QProcess::errorOccurred, this, &SearchToolsWidget::errorOccurred);
    connect(this->searchProcess, &QProcess::started, this, &SearchToolsWidget::searchProcessStarted);
    connect(this->searchProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::searchProcessFinished);

    this->pprintProcess = new QProcess(this);
    connect(this->pprintProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::pprintProcessFinished);

    this->toolDatabaseProcess = new QProcess(this);
}

void SearchToolsWidget::downloadToolDatabase()
{
    connect(this->toolDatabaseProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::createToolDatabase);
    this->toolDatabaseProcess->start(BOSH_PATH, {"search", "-m 1000"});
}

void SearchToolsWidget::searchChanged()
{
    QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::RegExp);
    Qt::CaseSensitivity caseSensitivity = Qt::CaseInsensitive;

    QRegExp regExp(this->searchLineEdit->text(), caseSensitivity, syntax);
    this->proxyToolModel->setFilterRegExp(regExp);
}

void SearchToolsWidget::selectionChanged()
{
    ToolDescription *tool = this->getSelectedTool();
    if(tool == nullptr) {
        return;
    }

    this->pprintProcess->kill();
    this->pprintProcess->start(BOSH_PATH, {"pprint", tool->id});
    this->loadingLabel->setText("Getting tool help...");
}

void SearchToolsWidget::pprintProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

//    QByteArray result = this->pprintProcess->readAll();
    QByteArray result = this->pprintProcess->readAllStandardOutput();
    this->info->setText(QString::fromUtf8(result));

    this->infoLabel->show();
    this->info->show();
    emit toolSelected();
}

void SearchToolsWidget::searchBoutiquesTools()
{
    this->searchProcess->kill();

    if(this->toolDatabaseUpdated)
    {
        this->loadToolDatabase();
        this->toolDatabaseUpdated = false;
    }

    emit toolDeselected();

    if(!this->descriptors.isEmpty())
    {
        // If descriptors are not empty: the tool database was loaded, all tools are added in the model and filtered while typing: do nothing when user hits enter.
        return;
    }

    this->loadingLabel->show();
//    this->table->hide();
//    this->searchView->hide();
    this->infoLabel->hide();
    this->info->hide();

    QString searchQuery = this->searchLineEdit->text();
    this->searchProcess->start(BOSH_PATH, {"search", "-m 50", searchQuery});
    this->loadingLabel->setText("Search launched...");
    this->searchResults.clear();
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

QStringList SearchToolsWidget::readSearchResults(QString &output)
{
    QString outputClean = output.replace(QRegExp("\x1b\[[0-9;]*[mGKF]"), "");

    QTextStream outputSream(&outputClean);
    QStringList lines;
    while (!outputSream.atEnd())
    {
       lines.push_back(outputSream.readLine());
    }
    return lines;
}

void SearchToolsWidget::parseSearchResults(const QStringList &lines, vector<ToolDescription> &searchResults)
{
    QString line = lines[1];
    int idIndex = line.indexOf("ID");
    int titleIndex = line.indexOf("TITLE");
    int descriptionIndex = line.indexOf("DESCRIPTION");
    int nDownloadsIndex = line.indexOf("DOWNLOADS");

    for(int i=2 ; i<lines.size() ; i++){
        QString line = lines[i];

        searchResults.emplace_back();
        ToolDescription& searchResult = searchResults.back();
        searchResult.id = line.mid(idIndex, titleIndex - idIndex).trimmed();
        searchResult.title = line.mid(titleIndex, descriptionIndex - titleIndex).trimmed();
        searchResult.description = line.mid(descriptionIndex, nDownloadsIndex - descriptionIndex).trimmed();
        searchResult.nDownloads = line.mid(nDownloadsIndex, line.size() - 1).trimmed().toInt();
    }
}

void SearchToolsWidget::addSearchResult(const ToolDescription &toolDescription, const unsigned int toolDescriptionIndex)
{
    this->toolModel->insertRow(0);
    this->toolModel->setData(this->toolModel->index(0, 0), toolDescription.id);
    this->toolModel->setData(this->toolModel->index(0, 1), toolDescription.title);
    this->toolModel->setData(this->toolModel->index(0, 2), toolDescription.description);
    this->toolModel->setData(this->toolModel->index(0, 3), toolDescription.nDownloads);
    this->toolModel->setData(this->toolModel->index(0, 0), QSize(50, 50), Qt::SizeHintRole);
    this->toolModel->setData(this->toolModel->index(0, 0), toolDescriptionIndex, Qt::UserRole);
}

void SearchToolsWidget::displaySearchResults()
{
    this->loadingLabel->hide();
    this->searchView->show();

//    this->toolModel->clear();
    this->toolModel->removeRows(0, this->toolModel->rowCount());
//    this->table->show();
//    this->table->setRowCount(static_cast<int>(this->searchResults.size()));
    for(unsigned int i=0 ; i<this->searchResults.size() ; i++){
        const ToolDescription &searchResult = this->searchResults[i];
        this->addSearchResult(searchResult, i);
//        this->table->setItem(static_cast<int>(i), 0, new QTableWidgetItem(searchResult.id));
//        this->table->setItem(static_cast<int>(i), 1, new QTableWidgetItem(searchResult.title));
//        this->table->setItem(static_cast<int>(i), 2, new QTableWidgetItem(searchResult.description));
//        this->table->setItem(static_cast<int>(i), 3, new QTableWidgetItem(searchResult.nDownloads));
    }
//    this->searchView->update();
//    this->searchView->setModel(this->toolModel);
//    emit this->toolModel->dataChanged(this->toolModel->index(0, 0), this->toolModel->index(this->toolModel->rowCount() - 1, this->toolModel->columnCount() - 1));
}

void SearchToolsWidget::searchProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)
    this->loadingLabel->hide();

    QString output = QString::fromUtf8(this->searchProcess->readAll());
    QStringList lines = this->readSearchResults(output);

    if(lines.size() < 2)
    {
        if(output.contains("Error"))
        {
            QMessageBox::warning(this, "Error while searching in Zenodo database.", "An error occured while searching the zenodo database.");
        }
        return;
    }

    searchResults.clear();

    this->parseSearchResults(lines, this->searchResults);
    this->displaySearchResults();
}

void SearchToolsWidget::createToolDatabase(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    if(exitStatus != QProcess::ExitStatus::NormalExit)
    {
        return;
    }

    QString output = QString::fromUtf8(this->toolDatabaseProcess->readAll());
    QStringList lines = this->readSearchResults(output);

    if(lines.size() < 2)
    {
        return;
    }

    this->allTools.clear();
    this->parseSearchResults(lines, this->allTools);

    QStringList args;
    args.push_back("pull");
    for(unsigned int i=0 ; i<this->allTools.size() ; i++){
        const ToolDescription &tool = this->allTools[i];
        args.push_back(tool.id);
    }
    disconnect(this->toolDatabaseProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::createToolDatabase);
    connect(this->toolDatabaseProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &SearchToolsWidget::pullProcessFinished);
    this->toolDatabaseProcess->start(BOSH_PATH, args);
}

void SearchToolsWidget::pullProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(exitStatus)

    QDir cacheDirectory(QDir::homePath() + BOUTIQUES_CACHE_PATH);

    QJsonArray descriptors;
    for(const ToolDescription &toolDescription: this->allTools)
    {
        QString id = QString::fromStdString(toolDescription.id.toStdString()); // deep copy the string
        QString descriptorFileName = id.replace(QChar('.'), QChar('-')) + ".json";
        QFile file(cacheDirectory.absoluteFilePath(descriptorFileName));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, "Could not open descriptor file", "Error while opening descriptor file (" + descriptorFileName + ") from ~" + BOUTIQUES_CACHE_PATH);
            return;
        }

        QJsonDocument descriptorDocument(QJsonDocument::fromJson(file.readAll()));
        QJsonObject descriptorObject = descriptorDocument.object();
        descriptorObject["id"] = toolDescription.id;
        descriptorObject["nDownloads"] = toolDescription.nDownloads;
        descriptors.push_back(descriptorObject);
    }

    QFile descriptorsFile(cacheDirectory.absoluteFilePath(DATABASE_NAME));
    if (!descriptorsFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Could not open descriptors file", "Error while opening descriptors file.");
        return;
    }
    QJsonDocument descriptorsDocument(descriptors);
    descriptorsFile.write(descriptorsDocument.toJson());
    this->toolDatabaseUpdated = true;
}

void SearchToolsWidget::loadToolDatabase()
{
    QDir cacheDirectory(QDir::homePath() + BOUTIQUES_CACHE_PATH);

    QFile file(cacheDirectory.absoluteFilePath(DATABASE_NAME));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    QJsonDocument descriptorsDocument(QJsonDocument::fromJson(file.readAll()));
    this->descriptors = descriptorsDocument.array();

    searchResults.clear();
    for(int i=0 ; i<this->descriptors.size() ; i++)
    {
        const QJsonObject &descriptor = this->descriptors.at(i).toObject();
        this->searchResults.emplace_back();
        ToolDescription& searchResult = this->searchResults.back();
        searchResult.title = descriptor["name"].toString();
        searchResult.description = descriptor["description"].toString();
        searchResult.id = descriptor["id"].toString();
        searchResult.nDownloads = descriptor["nDownloads"].toInt();

    }
    this->displaySearchResults();
}
