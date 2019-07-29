#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "searchtoolswidget.h"
#include "invocationwidget.h"
#include "executionwidget.h"
#include "toolboxwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    SearchToolsWidget *searchToolsWidget;
    InvocationWidget *invocationWidget;
    ExecutionWidget *executionWidget;
    ToolBoxWidget *toolBoxWidget;
    QWidget *centralWidget;
//    QLayout *layout;

private:

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
};

#endif // MAINWINDOW_H
