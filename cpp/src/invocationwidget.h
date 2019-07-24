#ifndef INVOCATIONWIDGET_H
#define INVOCATIONWIDGET_H

#include <QWidget>
#include "searchtoolswidget.h"
#include "invocationguiwidget.h"

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class InvocationWidget : public QWidget
{
    Q_OBJECT
public:
    InvocationGUIWidget *invocationGUIWidget;
    QTextEdit *invocationEditor;

private:
    SearchToolsWidget *searchToolsWidget;
    QVBoxLayout *layout;
    QPushButton *openInvocationButton;
    QPushButton *saveInvocationButton;
    QJsonObject invocationJSON;
    QProcess *generateInvocationProcess;

public:
    explicit InvocationWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr);
    QStringList getFilePaths();

private:
    void generateInvocationFile();

signals:

public slots:
    void generateInvocationProcessFinished();
    void openInvocationFile();
    void saveInvocationFile();
    void invocationChanged();
    void toolSelected();
    void toolDeselected();
};

#endif // INVOCATIONWIDGET_H
