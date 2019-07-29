#ifndef DROPWIDGET_H
#define DROPWIDGET_H

#include <QWidget>

class DropWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DropWidget(QWidget *parent = nullptr);

signals:
    void dragEnter(QDragEnterEvent *event);
    void dragMove(QDragMoveEvent *event);
    void drop(QDropEvent *event);

public slots:
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // DROPWIDGET_H
