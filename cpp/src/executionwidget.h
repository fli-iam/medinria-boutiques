#ifndef EXECUTIONWIDGET_H
#define EXECUTIONWIDGET_H

#include <QWidget>
#include "configuration.h"
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

private:
    QLayout *layout;
    QLabel *generatedCommandLabel;
    QTextEdit *generatedCommand;
    QTextEdit *output;                  // Read only text field to display the output of the boutiques tool process
    QPushButton *executeButton;
    QPushButton *cancelButton;
    QProcess *executionProcess;         // The execution process executes the tool: "bosh exec launch -s toolID temporaryInvocationFilePath"
    QProcess *simulationProcess;        // The simulation process gives the generated command that will be given to the boutiques tool (from the invocation file)
                                        //      "bosh simulate -i invocation.json toolID"

public:
    explicit ExecutionWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr, InvocationWidget *invocationWidget = nullptr);

private:
    // Writes the invocation (content of the text editor of the invocation widget) to a temporary file and return the path of this file
    QString getTemporaryInvocationFile();
    // Print the given text into the output text edit widget
    void print(const QString& text);

signals:
    void success(const QString outputFileName);

public slots:
    // On invocation changed: start the simulation process to display the generated command
    void invocationChanged();
    void executeTool();
    void cancelExecution();
    void errorOccurred(QProcess::ProcessError error);
    // Write output data in the output text field as soon as it is ready from the execution output
    void dataReady();
    void executionProcessStarted();
    void executionProcessFinished();
    void simulationProcessFinished();

    // Show or hide this widget when a boutiques tool is selected / deselected
    void toolSelected();
    void toolDeselected();
};

#endif // EXECUTIONWIDGET_H
