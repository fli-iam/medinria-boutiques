#include "toolboxwidget.h"
#include "filehandler.h"
#include <QtWidgets>

ToolBoxWidget::ToolBoxWidget(QWidget *parent) : QWidget(parent), fileHandler(new FileHandler())
{
#ifdef TEST_DRAGGABLE_INPUTS
    QListWidget *listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    listWidget->setDragEnabled(true);
    listWidget->addItem("Draggable item 1");
    listWidget->addItem("Draggable item 2");
    listWidget->addItem("Draggable item 3");
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(listWidget);
    this->setLayout(layout);
#endif
}
