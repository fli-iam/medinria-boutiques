#include <QtWidgets>
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
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
