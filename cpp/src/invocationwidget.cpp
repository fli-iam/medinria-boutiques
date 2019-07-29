#include <QtWidgets>
#include "invocationwidget.h"

InvocationWidget::InvocationWidget(QWidget *parent, SearchToolsWidget *searchToolsWidget, AbstractFileHandler *FileHandler) :
    QWidget(parent),
    searchToolsWidget(searchToolsWidget)
{
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

QStringList InvocationWidget::setAndGetAbsoluteDirectories()
{
    QStringList directories;
    QString invocationString = this->invocationEditor->toPlainText();
    QJsonObject invocationJsonAbsolutePath = QJsonDocument::fromJson(invocationString.toUtf8()).object();

    this->invocationGUIWidget->populateDirectories(invocationJsonAbsolutePath, directories);

    QJsonDocument document(invocationJsonAbsolutePath);
    QByteArray output = document.toJson(QJsonDocument::Indented);
    this->invocationEditor->setText(output);
    return directories;
}

void InvocationWidget::generateInvocationFile()
{
    const SearchResult* tool = this->searchToolsWidget->getSelectedTool();
    if(tool == nullptr)
    {
        return;
    }

    QStringList args({"example"});
    if(this->invocationGUIWidget->generateCompleteInvocation())
    {
        args.append("--complete");
    }
    args.append(tool->id.c_str());
    this->generateInvocationProcess->kill();
    this->generateInvocationProcess->start(BOSH_PATH, args);
}

void InvocationWidget::generateInvocationProcessFinished()
{
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
