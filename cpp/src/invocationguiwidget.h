#ifndef INVOCATIONGUIWIDGET_H
#define INVOCATIONGUIWIDGET_H

#include <map>
#include <functional>
#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
#include "searchtoolswidget.h"
#include "abstractfilehandler.h"
#include "dropwidget.h"

QT_BEGIN_NAMESPACE
class QScrollArea;
class QJsonObject;
class QDoubleSpinBox;
class QRadioButton;
class QComboBox;
class QTimer;
QT_END_NAMESPACE

struct GroupObject {
    QJsonObject description;
    QLayout *layout = nullptr;
    QGroupBox *groupBox = nullptr;
    QComboBox *comboBox = nullptr;
    bool optional;
};

struct InputObject {
    QJsonObject description;
    QWidget *widget = nullptr;
    GroupObject *group = nullptr;
    std::function<QJsonValue()> getValue;
    InputObject(const QJsonObject &description): description(description) {}
};

class InvocationGUIWidget : public QWidget
{
    Q_OBJECT
private:
    SearchToolsWidget *searchToolsWidget;
    AbstractFileHandler *fileHandler;
    QVBoxLayout *layout;
    QGroupBox *selectCurrentDirectoryGroupBox;
    QLineEdit *selectCurrentDirectoryLineEdit;
    QScrollArea *scrollArea;
    QWidget *group;
    QGroupBox *optionalInputGroup;
    QTimer *emitInvocationChangedTimer;
    QJsonObject *invocationJSON;
    QJsonObject *completeInvocationJSON;
    QJsonArray outputFiles;
    map<QString, InputObject> idToInputObject;
    vector<GroupObject> groupObjects;
    bool ignoreSignals;

public:
    explicit InvocationGUIWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr, AbstractFileHandler *toolBox = nullptr);
    ~InvocationGUIWidget();
    void parseDescriptor(QJsonObject *invocationJSON);
    bool generateCompleteInvocation();
    // Check all input and output paths. Change [CURRENT INPUT] to actual temporary file created for this purpose in QDir::tempPath().
    // If one or more relative paths: open a dialog for the user to select the root directory ; relative paths will be converted to absolute path from this directory
    // Return the list of used directories to mount them (bosh -v option), to make them accessible to docker
    void populateDirectories(QJsonObject &invocationJSON, QStringList &directories);

private:
    bool inputGroupIsMutuallyExclusive(const QString &inputId);
    pair<QGroupBox *, QVBoxLayout *> createGroupAndLayout(const QString &name);
    void valueChanged(const QString &inputId);
    void removeMutuallyExclusiveParameters(const QString &inputId);
    QJsonArray stringToArray(const QString &string);
    QWidget *createUnsetGroup(const QString &inputId, QWidget *widget);
    void askChangeCurrentDirectory();
    void populateAbsolutePath(const QJsonValue &fileNameValue, QStringList &directories, bool &hasChangedCurrentDirectory);
    void populateInputDirectories(const QJsonObject &invocationJSON, QStringList &directories, bool &hasChangedCurrentDirectory);
    void populateOutputDirectories(const QJsonObject &invocationJSON, QStringList &directories, bool &hasChangedCurrentDirectory);

    void createSelectCurrentDirectoryGUI();

signals:
    void invocationChanged();

public slots:
private slots:
    void optionalGroupChanged(bool on);
    void mutuallyExclusiveGroupChanged(GroupObject *groupObject, int itemIndex);
    void emitInvocationChanged();
};

#endif // INVOCATIONGUIWIDGET_H
