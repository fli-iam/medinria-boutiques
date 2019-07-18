#ifndef INVOCATIONGUIWIDGET_H
#define INVOCATIONGUIWIDGET_H

#include <map>
#include <functional>
#include <QWidget>
#include <QJsonObject>
#include "searchtoolswidget.h"

QT_BEGIN_NAMESPACE
class QScrollArea;
class QJsonObject;
class QDoubleSpinBox;
class QRadioButton;
class QComboBox;
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
};

class InvocationGUIWidget : public QWidget
{
    Q_OBJECT
private:
    SearchToolsWidget *searchToolsWidget;
    QVBoxLayout *layout;
    QScrollArea *scrollArea;
    QWidget *group;
    QGroupBox *optionalInputGroup;
    QJsonObject *invocationJSON;
    QJsonObject *completeInvocationJSON;
    map<string, InputObject> idToInputObject;
    vector<GroupObject> groupObjects;
    bool ignoreSignals;

public:
    explicit InvocationGUIWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr);
    void parseDescriptor(QJsonObject *invocationJSON);
    bool generateCompleteInvocation();

private:
    bool inputGroupIsMutuallyExclusive(const string &inputId);
    pair<QGroupBox *, QVBoxLayout *> createGroupAndLayout(const string &name);
    void valueChanged(const string &inputId);
    void removeMutuallyExclusiveParameters(const string &inputId);
    QJsonArray stringToArray(const string &string);

signals:
    void invocationChanged();

public slots:
private slots:
    void optionalGroupChanged(bool on);
    void mutuallyExclusiveGroupChanged(GroupObject *groupObject, int itemIndex);
};

#endif // INVOCATIONGUIWIDGET_H
