#ifndef INVOCATIONGUIWIDGET_H
#define INVOCATIONGUIWIDGET_H

#include <map>
#include <functional>
#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
#include "configuration.h"
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

// Input parameters can be grouped together in Boutiques
// This data structure stores the description, layout and widgets of an input group
struct GroupObject {
    QJsonObject description;
    QLayout *layout = nullptr;
    QGroupBox *groupBox = nullptr;
    QComboBox *comboBox = nullptr;
    bool optional;
};

// The data structure to store the data (Json description, widgets and the getValue() callback) of an input parameter
struct InputObject {
    QJsonObject description;
    QWidget *widget = nullptr;
    GroupObject *group = nullptr;
    std::function<QJsonValue()> getValue;                                       // A function called to get the value of the parameter (eg. returns the text of a QLineEdit enclosed in brackets for Lists, the value of a QSpinBox for Numbers, etc.)
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
    QGroupBox *optionalInputGroup;                  // Input parameters are either required or optional. Optional inputs are gathered under the optionalInputGroup
    QTimer *emitInvocationChangedTimer;             // When an input parameter changes, a signal is emitted a few milliseconds later (so that the invocation file updates when the user changes the input, but not synchronically to avoid blocking the interface) ; the emitInvocationChangedTimer timer enables this delay
    QJsonObject *invocationJSON;                    // The Json object describing the invocation of the tool
    QJsonObject *completeInvocationJSON;            // When the user unchecks the optional parameters: remove all parameters but store all values to be able to restore them later (when re-checking optional parameters)
    QJsonArray outputFiles;                         // A list of output files to manage output file paths (docker needs to mount the directories of input and output files)
    map<QString, InputObject> idToInputObject;      // Input ID to input object
    vector<GroupObject> groupObjects;
    QString selectedOutputId;                       // The selected output id used to set the output file name when executing the process
    QString outputFileName;

public:
    explicit InvocationGUIWidget(QWidget *parent = nullptr, SearchToolsWidget *searchToolsWidget = nullptr, AbstractFileHandler *toolBox = nullptr);
    ~InvocationGUIWidget();
    // Parse the tool descriptor and generate a corresponding GUI from the input parameters, input groups, and output-files templates
    void parseDescriptor(QJsonObject *invocationJSON);
    bool mustGenerateCompleteInvocation();
    // Check all input and output paths.
    // If one or more relative paths:
    //  - make them absolute by using the current directory defined in the GUI (see createSelectCurrentDirectoryGUI) at the root directory,
    //  - or by asking the user to select the root directory with a file dialog
    // Return the list of used directories to mount them (bosh -v option), to make them accessible to docker.
    // Also set the output file name which will be automatically opened in medInria once the process is over
    // Note: Changes the current path to the home path or the user defined current directory
    void populateDirectoriesAndSetOutputFileName(QJsonObject &invocationJSON, QStringList &directories);
    // Returns the output file name which will be automatically opened in medInria once the process is over
    QString getOutputFileName();

private:
    // True if the input belongs to a mutually exclusive group
    bool inputGroupIsMutuallyExclusive(const QString &inputId);
    // Create a group box and its layout
    pair<QGroupBox *, QVBoxLayout *> createGroupAndLayout(const QString &name);
    // Called when a value of the invocation is changed
    // Update invocationJSON accordingly and emit the invocationChanged() signal
    void valueChanged(const QString &inputId);
    // Converts a QString (a list of comma seperated values) to a QJsonArray by adding enclosing brackets: [string]
    QJsonArray stringToArray(const QString &string);
    // Create an unset group (a widget with a horizontal layout, which will contain the parameters widget, and an "Unset" push button)
    // The "Unset" button is added to all optional parameters, to unset them
    QWidget *createUnsetGroup(const QString &inputId, QWidget *widget);
    // If the user entered a relative path but did not specify a working directory: BoutiquesGUI asks to change the current directory
    // (select the directory of reference for the relative paths)
    void askChangeCurrentDirectory();
    // Check if the given path is relative or not, if it is relative: askChangeCurrentDirectory() ; then add the absolute path to directories
    void populateAbsolutePath(const QJsonValue &fileNameValue, QStringList &directories, bool &hasChangedCurrentDirectory);
    // Add all absolute file paths of the File inputs (parameters of type "File") to the directories list; converts relative paths to absolute with populateAbsolutePath()
    void populateInputDirectories(const QJsonObject &invocationJSON, QStringList &directories, bool &hasChangedCurrentDirectory);
    // Add all absolute file paths of the outputs (from the "output-files" attribute of the descriptors, and their "template-path" value)
    void populateOutputDirectoriesAndSetOutputFileName(const QJsonObject &invocationJSON, QStringList &directories, bool &hasChangedCurrentDirectory);
    // Create the GUI to select the current working directory (from which relative paths will be defined)
    void createSelectCurrentDirectoryGUI();
    // Set or unset the output file (which will be automatically opened in medInria)
    // If the parameter can be associated with multiple output files, the user is asked to selected the output he wants to consider
    void toggleOutputFile(QPushButton *toggleOutputPushButton, const QString &inputId);

    // Create input GUI
    // Create input groups
    // For each group:
    // - create the GroupObject (containing the widget, layout and if it is optional)
    // - create a QGroupBox and its layout
    // - check if group is optional (group is optional if one of its member is optional)
    // - add the group widget to the layout accordingly (main inputs or optional inputs)
    void createInputGroups(const QJsonObject &descriptor, vector<pair<QGroupBox *, QVBoxLayout *> > &destinationLayouts, QVBoxLayout *mainLayout, QVBoxLayout *optionalLayout);
    // If input is a list of files: a "Select files" push button opens a dialog to select multiple files
    void createInputFiles(QWidget *widget, QLayout *layout, QLineEdit *lineEdit, const QString &inputName);
    // Populate idToInputObject from the descriptor inputs
    void populateIdToInputObject(const QJsonObject &descriptor);
    // Mutually exclusive groups are controlled with a comboBox (drop down menu) containing the group parameters (the user must choose one parameter among the group members)
    void createMutuallyExclusiveGroup(GroupObject *groupObject, QLayout *parentLayout, const QString &inputName, const QString &inputId, const QJsonValue &inputValue);
    // Create an input field (QLineEdit) to define a list of values (files, strings or numbers)
    void createInputList(InputObject &inputObject, QWidget *widget, QLayout *layout, const QString &inputId, const QString &inputName, const QString &inputType, const QJsonValue &inputValue);
    // If the parameter has "value-choices": create a combo box to be able to select one of those choices
    void createInputChoices(InputObject &inputObject, QLayout *layout, const QString &inputId, const QString &inputType);
    // If type is String: just create a line edit
    void createStringInput(InputObject &inputObject, QLayout *layout, const QString &inputId, const QJsonValue &inputValue, const QString &inputDescription);
    // If type is Number: create a spin box and set min, max and exclusive min and max
    void createNumberInput(InputObject &inputObject, QLayout *layout, const QString &inputId, const QJsonValue &inputValue);
    // If parameter is File: create a line edit (for the file path), a "Select file" button, and a "Set input" button
    void createFileInput(InputObject &inputObject, QWidget *widget, QLayout *layout, const QString &inputId, const QString &inputName, const QJsonValue &inputValue, const QString &inputDescription);

signals:
    void invocationChanged();

public slots:
private slots:
    void optionalGroupChanged(bool on);
    void mutuallyExclusiveGroupChanged(GroupObject *groupObject, int itemIndex);
    void emitInvocationChanged();
};

#endif // INVOCATIONGUIWIDGET_H
