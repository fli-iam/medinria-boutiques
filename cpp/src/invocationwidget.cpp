#include <iostream>
#include <QtWidgets>
#include "invocationwidget.h"

InvocationWidget::InvocationWidget(QWidget *parent, SearchToolsWidget *searchToolsWidget, AbstractFileHandler *FileHandler) :
    QWidget(parent),
    searchToolsWidget(searchToolsWidget)
{
    // Create the invocation GUI:
    //  - an open invocation file button
    //  - the automatically generated invocation gui
    //  - the invocation editor which will be automatically updated by the above invocation gui, but which can be edited by the user (this is the invocation data which will be used in the end)
    //  - a save invocation button
    this->layout = new QVBoxLayout();

    this->invocationGUIWidget = new InvocationGUIWidget(this, this->searchToolsWidget, FileHandler);
    this->openInvocationButton = new QPushButton("Open invocation file");

    this->invocationEditor = new QTextEdit();
    this->invocationEditor->setMinimumHeight(300);
    this->saveInvocationButton = new QPushButton("Save invocation file");

    this->layout->addWidget(this->openInvocationButton);
    this->layout->addWidget(this->invocationGUIWidget);
    this->layout->addWidget(this->invocationEditor);
    this->layout->addWidget(this->saveInvocationButton);

    connect(this->openInvocationButton, &QPushButton::clicked, this, &InvocationWidget::openInvocationFile);
    connect(this->saveInvocationButton, &QPushButton::clicked, this, &InvocationWidget::saveInvocationFile);
    connect(this->invocationGUIWidget, &InvocationGUIWidget::invocationChanged, this, &InvocationWidget::invocationChanged);

    this->setLayout(this->layout);

    this->generateInvocationProcess = new QProcess(this);
    connect(this->generateInvocationProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &InvocationWidget::generateInvocationProcessFinished);
}

void InvocationWidget::setAndGetAbsoluteDirectories(QStringList &directories)
{
    // Parse the invocationEditor to get the final invocation values (the values from the invocation GUI might have been modified by the user)
    QString invocationString = this->invocationEditor->toPlainText();
    QJsonObject invocationJsonAbsolutePath = QJsonDocument::fromJson(invocationString.toUtf8()).object();

    // Populate directories, set all paths to absolute paths, and update the invocation editor accordingly
    this->invocationGUIWidget->populateDirectoriesAndSetOutputFileName(invocationJsonAbsolutePath, directories);

    QJsonDocument document(invocationJsonAbsolutePath);
    QByteArray output = document.toJson(QJsonDocument::Indented);
    this->invocationEditor->setText(output);
}

void InvocationWidget::generateInvocationFile()
{
    // If the invocation generation is running: kill the process and retry generateInvocationFile()
    if(this->generateInvocationProcess->state() != QProcess::NotRunning)
    {
        this->generateInvocationProcess->kill();
        QTimer::singleShot(100, this, &InvocationWidget::generateInvocationFile);
        return;
    }

    const ToolDescription* tool = this->searchToolsWidget->getSelectedTool();
    if(tool == nullptr)
    {
        return;
    }

    // Generate invocation:
    //  - if there are some optional parameters: generate complete invocation:           bosh example --complete toolID
    //  - otherwise: generate minimal invocation:                                        bosh example toolID
    QStringList args({BoutiquesPaths::Bosh(), "example"});
    if(this->invocationGUIWidget->mustGenerateCompleteInvocation())
    {
        args.append("--complete");
    }
    args.append(tool->id);
    this->generateInvocationProcess->start(BoutiquesPaths::Python(), args);
}

void InvocationWidget::generateInvocationProcessFinished()
{
    // Read invocation process results, update the invocation editor, and update the invocation GUI widget with the new values
    QByteArray result = this->generateInvocationProcess->readAll();
    this->invocationJSON = QJsonDocument::fromJson(result).object();
    this->invocationEditor->setText(QString::fromUtf8(result));
    this->invocationGUIWidget->parseDescriptor(&this->invocationJSON);
}

void InvocationWidget::openInvocationFile()
{
    const QString& name = QFileDialog::getOpenFileName(this, "Open invocation file");

    QFile file(name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    QByteArray text = file.readAll();
    this->invocationEditor->setText(text);
}

void InvocationWidget::invocationChanged()
{
    QJsonDocument document(this->invocationJSON);
    QByteArray output = document.toJson(QJsonDocument::Indented);
    this->invocationEditor->setText(output);
}

void InvocationWidget::saveInvocationFile()
{
    const QString& name = QFileDialog::getSaveFileName(this, "Save invocation file");
    QFile file(name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    file.write(this->invocationEditor->toPlainText().toUtf8());
}

void InvocationWidget::toolSelected()
{
    this->show();
    this->generateInvocationFile();
}

void InvocationWidget::toolDeselected()
{
    this->hide();
}
