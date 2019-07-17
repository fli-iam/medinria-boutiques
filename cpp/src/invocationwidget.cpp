#include <QtWidgets>
#include "invocationwidget.h"

InvocationWidget::InvocationWidget(QWidget *parent, SearchToolsWidget *searchToolsWidget) : QWidget(parent), searchToolsWidget(searchToolsWidget)
{
    this->layout = new QVBoxLayout();

    this->invocationGUIWidget = new InvocationGUIWidget(this, this->searchToolsWidget);
    this->openInvocationButton = new QPushButton("Open invocation file");

    this->invocationEditor = new QTextEdit();
    this->invocationEditor->setMinimumHeight(150);
    this->saveInvocationButton = new QPushButton("Save invocation file");

    this->layout->addWidget(this->openInvocationButton);
    this->layout->addWidget(this->invocationGUIWidget);
    this->layout->addWidget(this->invocationEditor);
    this->layout->addWidget(this->saveInvocationButton);

    connect(this->openInvocationButton, &QPushButton::clicked, this, &InvocationWidget::openInvocationFile);
    connect(this->saveInvocationButton, &QPushButton::clicked, this, &InvocationWidget::saveInvocationFile);
    connect(this->invocationGUIWidget, &InvocationGUIWidget::invocationChanged, this, &InvocationWidget::invocationChanged);

    this->setLayout(this->layout);
}

void InvocationWidget::generateInvocationFile()
{
    const SearchResult* tool = this->searchToolsWidget->getSelectedTool();
    if(tool == nullptr)
    {
        return;
    }

    QProcess bosh;
    QStringList args({"example"});
    if(this->invocationGUIWidget->generateCompleteInvocation())
    {
        args.append("--complete");
    }
    args.append(tool->id.c_str());
    bosh.start(BOSH_PATH, args);
    if (!bosh.waitForFinished()) {
        return;
    }

    QByteArray result = bosh.readAll();
    this->invocationJSON = QJsonDocument::fromJson(result).object();
    this->invocationEditor->setText(QString::fromUtf8(result));
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
    this->invocationGUIWidget->parseDescriptor(&this->invocationJSON);
}

void InvocationWidget::toolDeselected()
{
    this->hide();
}
