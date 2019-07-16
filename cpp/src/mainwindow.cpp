#include <QtWidgets>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->searchToolsWidget = new SearchToolsWidget(this);
    this->invocationWidget = new InvocationWidget(this, this->searchToolsWidget);
    this->executionWidget = new ExecutionWidget(this, this->searchToolsWidget, this->invocationWidget);

    this->invocationWidget->hide();
    this->executionWidget->hide();

    connect(this->searchToolsWidget, &SearchToolsWidget::toolSelected, this->invocationWidget, &InvocationWidget::toolSelected);
    connect(this->searchToolsWidget, &SearchToolsWidget::toolSelected, this->executionWidget, &ExecutionWidget::toolSelected);
    connect(this->searchToolsWidget, &SearchToolsWidget::toolDeselected, this->invocationWidget, &InvocationWidget::toolDeselected);
    connect(this->searchToolsWidget, &SearchToolsWidget::toolDeselected, this->executionWidget, &ExecutionWidget::toolDeselected);

    this->layout = new QVBoxLayout();
    this->layout->addWidget(this->searchToolsWidget);
    this->layout->addWidget(this->invocationWidget);
    this->layout->addWidget(this->executionWidget);
    this->setLayout(this->layout);
}

MainWindow::~MainWindow()
{

}
