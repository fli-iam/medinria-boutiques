#include <iostream>
#include <regex>
#include <QtWidgets>
#include "executionwidget.h"

ExecutionWidget::ExecutionWidget(QWidget *parent, SearchToolsWidget *searchToolsWidget, InvocationWidget *invocationWidget) :
    QWidget(parent), searchToolsWidget(searchToolsWidget), invocationWidget(invocationWidget)
{
    connect(this->invocationWidget->invocationGUIWidget, &InvocationGUIWidget::invocationChanged, this, &ExecutionWidget::invocationChanged);
    this->layout = new QVBoxLayout();

    this->generatedCommandLabel = new QLabel("Generated command:");
    this->generatedCommand = new QTextEdit();
    this->executeButton = new QPushButton("Execute tool");
    this->cancelButton = new QPushButton("Cancel execution");
    this->cancelButton->hide();

    this->output = new QTextEdit();
    this->output->setMinimumHeight(300);
    this->output->setReadOnly(true);

    this->layout->addWidget(this->generatedCommandLabel);
    this->layout->addWidget(this->generatedCommand);
    this->layout->addWidget(this->executeButton);
    this->layout->addWidget(this->cancelButton);
    this->layout->addWidget(this->output);

    connect(this->executeButton, &QPushButton::clicked, this, &ExecutionWidget::executeTool);
    connect(this->cancelButton, &QPushButton::clicked, this, &ExecutionWidget::cancelExecution);

    this->executionProcess = new QProcess(this);
    connect(this->executionProcess, &QProcess::errorOccurred, this, &ExecutionWidget::errorOccurred);
    connect(this->executionProcess, &QProcess::readyRead, this, &ExecutionWidget::dataReady);
    connect(this->executionProcess, &QProcess::started, this, &ExecutionWidget::executionProcessStarted);
    connect(this->executionProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ExecutionWidget::executionProcessFinished);

    this->simulationProcess = new QProcess(this);
    connect(this->simulationProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ExecutionWidget::simulationProcessFinished);

    this->setLayout(this->layout);
}

QString ExecutionWidget::getTemporaryInvocationFile()
{
    QDir temp = QDir::tempPath();
    temp.filePath("invocation.json");

    QString temporaryInvocationFilePath = temp.absoluteFilePath("invocation.json");
    QFile file(temporaryInvocationFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return "";

    file.write(this->invocationWidget->invocationEditor->toPlainText().toUtf8());
    return temporaryInvocationFilePath;
}

void ExecutionWidget::invocationChanged()
{
    if(this->simulationProcess->state() != QProcess::NotRunning)
    {
        this->simulationProcess->kill();
        QTimer::singleShot(100, this, &ExecutionWidget::invocationChanged);
        return;
    }

    ToolDescription *tool = this->searchToolsWidget->getSelectedTool();

    if(tool == nullptr)
    {
        return;
    }
    QString temporaryInvocationFilePath = this->getTemporaryInvocationFile();

    this->simulationProcess->start(BoutiquesPaths::Python(), {BoutiquesPaths::Bosh(), "exec", "simulate", "-i", temporaryInvocationFilePath, tool->id});
}

void ExecutionWidget::simulationProcessFinished()
{
    QString output = QString::fromUtf8(this->simulationProcess->readAll());
    this->generatedCommand->setText(output);
}

void ExecutionWidget::executeTool()
{

    if(this->executionProcess->state() != QProcess::NotRunning)
    {
        this->executionProcess->kill();
        QTimer::singleShot(100, this, &ExecutionWidget::executeTool);
        return;
    }

    ToolDescription *tool = this->searchToolsWidget->getSelectedTool();

    if(tool == nullptr)
    {
        return;
    }
    QString currentPath = QDir::currentPath();

    QStringList directories;
    this->invocationWidget->setAndGetAbsoluteDirectories(directories);

    QString temporaryInvocationFilePath = this->getTemporaryInvocationFile();

    QStringList args({BoutiquesPaths::Bosh(), "exec", "launch", "-s", tool->id, temporaryInvocationFilePath});

    for(const QString &directory: directories)
    {
        args.push_back("-v");
        args.push_back(directory + ":" + directory);
    }

    this->executionProcess->start(BoutiquesPaths::Python(), args);
    this->output->clear();

    QDir::setCurrent(currentPath);
}

void ExecutionWidget::cancelExecution()
{
    this->executionProcess->kill();
    this->executeButton->show();
    this->cancelButton->show();
}

void ExecutionWidget::print(const QString& text)
{
    QTextCursor cursor = this->output->textCursor();
    cursor.movePosition(cursor.End);
    cursor.insertText(text);
    this->output->ensureCursorVisible();
}

void ExecutionWidget::executionProcessStarted()
{
    this->print("Process started...\n\n");
    this->executeButton->hide();
    this->cancelButton->show();
}

void ExecutionWidget::executionProcessFinished()
{
    this->print("Process finished.");
    this->executeButton->show();
    this->cancelButton->hide();

    emit success(this->invocationWidget->invocationGUIWidget->getOutputFileName());
}

void ExecutionWidget::dataReady()
{
    QString output = QString::fromUtf8(this->executionProcess->readAll());

    regex e("\x1b\[[0-9;]*[mGKF]");

    string outputClean = regex_replace(output.toStdString(), e, "");
    this->print(QString::fromStdString(outputClean));
}

void ExecutionWidget::errorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error)
    this->print("An error occured during the process execution.\n\n");
}

void ExecutionWidget::toolSelected()
{
    this->show();
}

void ExecutionWidget::toolDeselected()
{
    this->hide();
}
