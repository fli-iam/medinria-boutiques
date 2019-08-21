#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "configuration.h"
#include "searchtoolswidget.h"
#include "invocationwidget.h"
#include "executionwidget.h"
#include "toolboxwidget.h"
#include "installer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    SearchToolsWidget *searchToolsWidget;
    InvocationWidget *invocationWidget;
    ExecutionWidget *executionWidget;
    ToolBoxWidget *toolBoxWidget;           // Simulates the medBoutiquesToolBox widget in medInria medBoutiques plugin
                                            // It is only used for compatibility with medInria
                                            // and to simulates the draggable inputs (drag-and-drop on input parameters)
    QWidget *centralWidget;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
};

#endif // MAINWINDOW_H
