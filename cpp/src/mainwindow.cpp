#include <iostream>
#include <QtWidgets>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->checkBoutiquesInstallation();

    this->toolBoxWidget = new ToolBoxWidget(this);
    this->searchToolsWidget = new SearchToolsWidget(this);
    this->invocationWidget = new InvocationWidget(this, this->searchToolsWidget, this->toolBoxWidget->fileHandler);
    this->executionWidget = new ExecutionWidget(this, this->searchToolsWidget, this->invocationWidget);

    this->invocationWidget->hide();
    this->executionWidget->hide();

    connect(this->searchToolsWidget, &SearchToolsWidget::toolSelected, this->invocationWidget, &InvocationWidget::toolSelected);
    connect(this->searchToolsWidget, &SearchToolsWidget::toolSelected, this->executionWidget, &ExecutionWidget::toolSelected);
    connect(this->searchToolsWidget, &SearchToolsWidget::toolDeselected, this->invocationWidget, &InvocationWidget::toolDeselected);
    connect(this->searchToolsWidget, &SearchToolsWidget::toolDeselected, this->executionWidget, &ExecutionWidget::toolDeselected);

    QVBoxLayout *scrollAreaLayout = new QVBoxLayout();
    scrollAreaLayout->addWidget(this->toolBoxWidget);
    scrollAreaLayout->addWidget(this->searchToolsWidget);
    scrollAreaLayout->addWidget(this->invocationWidget);
    scrollAreaLayout->addWidget(this->executionWidget);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
    QWidget *scrollAreaWidget = new QWidget();
    scrollAreaWidget->setLayout(scrollAreaLayout);
    scrollArea->setWidget(scrollAreaWidget);

    this->setCentralWidget(scrollArea);
}

MainWindow::~MainWindow()
{

}

void MainWindow::checkBoutiquesInstallation()
{
    QFile file(BoutiquesPaths::Settings());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        file.close();
        this->installBoutiques();
    } else {

        QJsonDocument jsonDocument(QJsonDocument::fromJson(file.readAll()));

        QJsonObject settings = jsonDocument.object();
        if(!settings.contains("installed") || !settings["installed"].toBool())
        {
            file.close();
            this->installBoutiques(&settings);
        }
    }
}

void MainWindow::setBoutiquesInstalled(QJsonObject *settings)
{
    QFile file(BoutiquesPaths::Settings());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if(settings != nullptr)
        {
            (*settings)["installed"] = true;
            QJsonDocument settingsDocument(*settings);
            file.write(settingsDocument.toJson());
        }
        else
        {
            QJsonObject settings;
            settings["installed"] = true;
            QJsonDocument settingsDocument(settings);
            file.write(settingsDocument.toJson());
        }
    }
}

bool MainWindow::isPythonWorking(const QString &version)
{
    QProcess pythonProcess(this);
    pythonProcess.start(BoutiquesPaths::Python(), {"--version"});
    pythonProcess.waitForFinished();
    QString output = QString::fromUtf8(pythonProcess.readAllStandardOutput());
    QString error = QString::fromUtf8(pythonProcess.readAllStandardError());
    QString pythonVersion = "Python " + version;
    return output.contains(pythonVersion) || error.contains(pythonVersion);
}

bool MainWindow::isDockerWorking()
{
    QProcess dockerProcess(this);
    dockerProcess.start(BoutiquesPaths::Docker(), {"--version"});
    dockerProcess.waitForFinished();
    QString output = QString::fromUtf8(dockerProcess.readAllStandardOutput());
    QString error = QString::fromUtf8(dockerProcess.readAllStandardError());
    QString dockerVersion = "Docker version";
    return output.contains(dockerVersion) || error.contains(dockerVersion);
}

void MainWindow::installBoutiques(QJsonObject *settings)
{
    bool pythonAndDockerAreWorking = true;

    if (QSysInfo::productType() == "winrt" || QSysInfo::productType() == "windows") {

        if(!this->isPythonWorking("3"))
        {
            // Install visual studio redistributable

            QProcess installVisualStudioRedistributableProcess(this);
            installVisualStudioRedistributableProcess.start(BoutiquesPaths::VCRedis(), {"\\q"});

            if (!installVisualStudioRedistributableProcess.waitForFinished())
            {
                pythonAndDockerAreWorking = false;
                QMessageBox::critical(this, "Could not install Microsoft Visual C++ Redistributable for Visual Studio", "Error while installing Microsoft Visual C++ Redistributable for Visual Studio.\nThis software is required to run python3 and boutiques under windows.\n\nTry to install it manually.");
            }
            else if(!this->isPythonWorking("3"))
            {
                pythonAndDockerAreWorking = false;
                QMessageBox::critical(this, "Python is not working", "Python.exe (" + BoutiquesPaths::Python() + ") is not working.\n\nYou need a working python3 version at this location to run boutiques tools.");
            }
        }

    } else {

        if(!this->isPythonWorking())
        {
            pythonAndDockerAreWorking = false;
            QMessageBox::critical(this, "Could not run Python", "Error while testing Python.\nInstall python 2.7 or 3 to run boutiques tools.");
        }
    }

    if (!this->isDockerWorking())
    {
        pythonAndDockerAreWorking = false;
        QMessageBox::critical(this, "Could not run Docker", "Error while testing Docker.\nInstall Docker or Singularity to run boutiques tools.");
    }

    if(pythonAndDockerAreWorking)
    {
        this->setBoutiquesInstalled(settings);
    }
}
