#ifndef TOOLBOXWIDGET_H
#define TOOLBOXWIDGET_H

#include <QWidget>
#include <QMimeData>
#include "invocationguiwidget.h"
#include "abstractfilehandler.h"

#define BOUTIQUE_GUI_STANDALONE

QT_BEGIN_NAMESPACE
class medDataIndex;
class medAbstractData;
QT_END_NAMESPACE

class ToolBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBoxWidget(QWidget *parent = nullptr);

    AbstractFileHandler *fileHandler;
signals:

public slots:
};

#endif // TOOLBOXWIDGET_H
