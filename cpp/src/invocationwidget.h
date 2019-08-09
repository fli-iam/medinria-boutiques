#ifndef INVOCATIONWIDGET_H
#define INVOCATIONWIDGET_H

#include <QWidget>
#include "configuration.h"
#include "searchtoolswidget.h"
#include "invocationguiwidget.h"
#include "abstractfilehandler.h"

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
    explicit InvocationWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr, AbstractFileHandler *FileHandler = nullptr);
    void setAndGetAbsoluteDirectories(QStringList &directories);

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
