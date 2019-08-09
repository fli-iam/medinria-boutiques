#include <iostream>
#include <QtWidgets>
#include "mainwindow.h"

#define VCREDIS BOUTIQUES_DIRECTORY "vc_redist.x86.exe"
#define WINPYTHON BOUTIQUES_DIRECTORY "WinPython32-3.7.1.0Zero.exe"
#define PYTHON "python"
#define PIP "pip"
#define DOCKER "docker"

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
    QFile file(BOUTIQUES_GUI_SETTINGS_PATH);
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
    QFile file(BOUTIQUES_GUI_SETTINGS_PATH);
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

void MainWindow::installBoutiques(QJsonObject *settings)
{
    if (QSysInfo::productType() == "winrt" || QSysInfo::productType() == "windows") {
        QProcess installVisualStudioRedistributableProcess(this);
        installVisualStudioRedistributableProcess.start(VCREDIS, {"\\q"});

        if (!installVisualStudioRedistributableProcess.waitForFinished())
        {
            QMessageBox::critical(this, "Could not install Microsoft Visual C++ Redistributable for Visual Studio", "Error while installing Microsoft Visual C++ Redistributable for Visual Studio.\nThis software is required to run python3 and boutiques under windows.\n\nTry to install it manually.");
        }

        QProcess winPythonProcess(this);
        winPythonProcess.start(WINPYTHON, {"/VERYSILENT", "/DIR=" + QDir::toNativeSeparators(QDir::currentPath() + "/python")});

        if (!winPythonProcess.waitForFinished() || winPythonProcess.exitCode() == 2)
        {
            QMessageBox::critical(this, "Could not install pip", "Error while installing pip.\nThis software is required to install boutiques.\n\nTry to install it manually.");
        }

        QProcess pipProcess(this);
        pipProcess.start(PIP, {"install", "boutiques"});

        if (!pipProcess.waitForFinished())
        {
            QMessageBox::critical(this, "Could not install boutiques", "Error while installing boutiques.\nThis software is required, try to install it manually.");
        }
        else
        {
            QProcess dockerProcess(this);
            dockerProcess.start(DOCKER, {"-v"});

            if (!dockerProcess.waitForFinished())
            {
                QMessageBox::warning(this, "Could not run Docker", "Error while testing Docker.\nInstall Docker or Singularity to run boutiques tools.");
            }
            else
            {
                this->setBoutiquesInstalled(settings);
            }
        }

    } else {

        QProcess pipProcess(this);

        pipProcess.start(PIP, {"-v"});

        if (!pipProcess.waitForFinished())
        {
            QProcess pythonProcess(this);
            pythonProcess.start(PYTHON, {BOUTIQUES_DIRECTORY "get-pip.py"});

            if (!pythonProcess.waitForFinished() || pythonProcess.exitCode() == 2)
            {
                QByteArray result = pythonProcess.readAll();
                QMessageBox::critical(this, "Could not install pip", "Error while installing pip. This software is required to install boutiques. Try to install it manually.");
            }
        }

        pipProcess.start(PIP, {"install", "boutiques"});

        if (!pipProcess.waitForFinished())
        {
            QMessageBox::critical(this, "Could not install boutiques", "Error while installing boutiques. This software is required, try to install it manually.");
        }

        pipProcess.start("bosh");
        if (!pipProcess.waitForFinished())
        {
            QMessageBox::critical(this, "Could not run boutiques", "Error while testing boutiques. This software is required, try to install it manually.");
        }
        else
        {
            QProcess dockerProcess(this);
            dockerProcess.start(DOCKER, {"-v"});

            if (!dockerProcess.waitForFinished())
            {
                QMessageBox::warning(this, "Could not run Docker", "Error while testing Docker.\nInstall Docker or Singularity to run boutiques tools.");
            }
            else
            {
                this->setBoutiquesInstalled(settings);
            }
        }
    }
}
