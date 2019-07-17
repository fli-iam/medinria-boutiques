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
    this->output->setReadOnly(true);

    this->layout->addWidget(this->generatedCommandLabel);
    this->layout->addWidget(this->generatedCommand);
    this->layout->addWidget(this->executeButton);
    this->layout->addWidget(this->cancelButton);
    this->layout->addWidget(this->output);

    connect(this->executeButton, &QPushButton::clicked, this, &ExecutionWidget::executeTool);
    connect(this->executeButton, &QPushButton::clicked, this, &ExecutionWidget::cancelExecution);

    this->process = new QProcess(this);
    connect(this->process, &QProcess::readyRead, this, &ExecutionWidget::dataReady);
    connect(this->process, &QProcess::started, this, &ExecutionWidget::processStarted);
    connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ExecutionWidget::processFinished);

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
    SearchResult *tool = this->searchToolsWidget->getSelectedTool();

    if(tool == nullptr)
    {
        return;
    }
    QString temporaryInvocationFilePath = this->getTemporaryInvocationFile();

    QProcess bosh;
    bosh.start(BOSH_PATH, {"exec", "simulate", "-i", temporaryInvocationFilePath.toStdString().c_str(), tool->id.c_str()});
    if (!bosh.waitForFinished())
    {
        return;
    }

    QString output = QString::fromUtf8(bosh.readAll());
    this->generatedCommand->setText(output);
}

void ExecutionWidget::executeTool()
{
    SearchResult *tool = this->searchToolsWidget->getSelectedTool();

    if(tool == nullptr)
    {
        return;
    }

    QString temporaryInvocationFilePath = this->getTemporaryInvocationFile();

    this->process->start(BOSH_PATH, {"exec", "launch", "-s", tool->id.c_str(), temporaryInvocationFilePath.toStdString().c_str()});
    this->output->clear();
}

void ExecutionWidget::cancelExecution()
{
    this->process->kill();
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

void ExecutionWidget::processStarted()
{
    this->print("Process started...\n\n");
    this->executeButton->hide();
    this->cancelButton->show();
}

void ExecutionWidget::processFinished()
{
    this->print("Process finished.");
    this->executeButton->show();
    this->cancelButton->hide();
}

void ExecutionWidget::dataReady()
{
    QString output = QString::fromUtf8(this->process->readAll());

    regex e("\x1b\[[0-9;]*[mGKF]");

    string outputClean = regex_replace(output.toStdString(), e, "");
    this->print(QString::fromStdString(outputClean));
}

void ExecutionWidget::toolSelected()
{
    this->show();
}

void ExecutionWidget::toolDeselected()
{
    this->hide();
}