#ifndef TOOLBOXWIDGET_H
#define TOOLBOXWIDGET_H

#include <QWidget>
#include <QMimeData>
#include "configuration.h"
#include "invocationguiwidget.h"
#include "abstractfilehandler.h"

QT_BEGIN_NAMESPACE
class medDataIndex;
class medAbstractData;
QT_END_NAMESPACE

// ToolboxWidget simulates the root widget in mediaInria boutique plugin

class ToolBoxWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBoxWidget(QWidget *parent = nullptr);
    virtual ~ToolBoxWidget();

    AbstractFileHandler *fileHandler;
signals:

public slots:
};

#endif // TOOLBOXWIDGET_H
