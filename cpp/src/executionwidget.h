#ifndef EXECUTIONWIDGET_H
#define EXECUTIONWIDGET_H

#include <QWidget>
#include "searchtoolswidget.h"
#include "invocationwidget.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class ExecutionWidget : public QWidget
{
    Q_OBJECT
public:
    SearchToolsWidget *searchToolsWidget;
    InvocationWidget *invocationWidget;
    QLayout *layout;
    QLabel *generatedCommandLabel;
    QTextEdit *generatedCommand;
    QTextEdit *output;
    QPushButton *executeButton;
    QPushButton *cancelButton;
    QProcess *executionProcess;
    QProcess *simulationProcess;

public:
    explicit ExecutionWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr, InvocationWidget *invocationWidget = nullptr);

private:
    QString getTemporaryInvocationFile();
    void print(const QString& text);

signals:

public slots:
    void invocationChanged();
    void executeTool();
    void cancelExecution();
    void errorOccurred(QProcess::ProcessError error);
    void dataReady();
    void executionProcessStarted();
    void executionProcessFinished();
    void simulationProcessFinished();
    void toolSelected();
    void toolDeselected();
};

#endif // EXECUTIONWIDGET_H
