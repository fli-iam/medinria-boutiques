#include "dropwidget.h"

DropWidget::DropWidget(QWidget *parent) : QWidget(parent)
{
    this->setAcceptDrops(true);
}

void DropWidget::dragEnterEvent(QDragEnterEvent *event)
{
    emit dragEnter(event);
}

void DropWidget::dragMoveEvent(QDragMoveEvent *event)
{
    emit dragMove(event);
}

void DropWidget::dropEvent(QDropEvent *event)
{
    emit drop(event);
}
