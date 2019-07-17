#include <QtWidgets>
#include "invocationguiwidget.h"

InvocationGUIWidget::InvocationGUIWidget(QWidget *parent, SearchToolsWidget *searchToolsWidget) : QWidget(parent), searchToolsWidget(searchToolsWidget)
{
    this->ignoreSignals = false;
    this->layout = new QVBoxLayout(this);
    this->setMinimumHeight(300);
    this->scrollArea = new QScrollArea(this);
    this->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    this->scrollArea->setWidgetResizable(true);
    this->scrollArea->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

    this->layout->addWidget(this->scrollArea);
    this->group = nullptr;
    this->setLayout(this->layout);
}

bool InvocationGUIWidget::inputIsMutuallyExclusive(const string &inputId) {
    try
    {
        const InputObject& inputObject = this->idToInputObject.at(inputId);
        return inputObject.group != nullptr && inputObject.group->description["mutually-exclusive"].toBool();
    } catch (const out_of_range& oor)
    {
        Q_UNUSED(oor)
        return false;
    }
}

void InvocationGUIWidget::removeMutuallyExclusiveParameters(const string& inputId) {
    if(this->inputIsMutuallyExclusive(inputId))
    {
        try
        {
            const InputObject& inputObject = this->idToInputObject.at(inputId);
            if(inputObject.description.contains("members"))
            {
                QJsonArray inputArray = inputObject.description["members"].toArray();
                this->ignoreSignals = true;
                for (int i=0 ; i<inputArray.size() ; ++i)
                {
                    const QString& qmember = inputArray[i].toString();
                    const string& member = qmember.toStdString();
                    if(this->idToInputObject.at(member).radioButton != nullptr)
                    {
                        this->idToInputObject.at(member).radioButton->setChecked(false);
                    }
                    this->invocationJSON->remove(qmember);
                }
                this->ignoreSignals = false;
            }
        } catch (const out_of_range& oor)
        {
            Q_UNUSED(oor)
            return;
        }
    }
}

QJsonArray InvocationGUIWidget::stringToArray(const string& string)
{
    QJsonDocument jsonDocument(QJsonDocument::fromJson(QByteArray::fromStdString(string)));
    return jsonDocument.array();
}

void InvocationGUIWidget::valueChanged(const string& inputId)
{
    if(this->ignoreSignals)
    {
        return;
    }
    this->removeMutuallyExclusiveParameters(inputId);
    this->invocationJSON->insert(QString::fromStdString(inputId), this->idToInputObject.at(inputId).getValue());
    emit invocationChanged();
}

pair<QGroupBox *, QVBoxLayout *> InvocationGUIWidget::createGroupAndLayout(const string& name)
{
    QGroupBox *group = new QGroupBox();
    group->setTitle(QString::fromStdString(name));
    QVBoxLayout *layout = new QVBoxLayout(group);
    group->setLayout(layout);
    return pair<QGroupBox *, QVBoxLayout *>(group, layout);
}

void InvocationGUIWidget::parseDescriptor(QJsonObject *invocationJSON)
{
    this->invocationJSON = invocationJSON;

    if(this->group != nullptr)
    {
        this->scrollArea->takeWidget();
        this->group->deleteLater();
        this->group = nullptr;
    }

    this->group = new QWidget(this);
    this->scrollArea->setWidget(this->group);

    QVBoxLayout *groupLayout = new QVBoxLayout(this->group);
    this->group->setLayout(groupLayout);

    SearchResult *searchResult = this->searchToolsWidget->getSelectedTool();

    QString descriptorFileName = QString::fromStdString(searchResult->id).replace(QChar('.'), QChar('-')) + "json";
    QDir cacheDirectory(QDir::homePath() + "/.cache/boutiques");

    QFile file(cacheDirectory.absoluteFilePath(descriptorFileName));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QJsonDocument jsonDocument(QJsonDocument::fromJson(file.readAll()));

    QJsonObject json = jsonDocument.object();

    auto mainInputsGroupAndLayout = this->createGroupAndLayout("Main parameters");
    auto optionalInputsGroupAndLayout = this->createGroupAndLayout("Optional parameters");

    if ( !(json.contains("inputs") && json["inputs"].isArray() && (!json.contains("groups") || json["groups"].isArray()) ) )
    {
        QMessageBox::critical(this, "Error in descriptor file", "No inputs or groups.");
        return;
    }

    QJsonArray inputArray = json["inputs"].toArray();
    for (int i = 0 ; i<inputArray.size() ; ++i)
    {
        InputObject inputObject;
        inputObject.description = inputArray[i].toObject();
        this->idToInputObject.insert(pair<string, InputObject>(inputObject.description["id"].toString().toStdString(), inputObject));
    }

    vector<pair<QGroupBox*, QVBoxLayout*>> destinationLayouts;
    QJsonArray groupArray = json["groups"].toArray();
    for (int i = 0 ; i<groupArray.size() ; ++i)
    {
        this->groupObjects.emplace_back();
        GroupObject &groupObject = this->groupObjects.back();
        groupObject.description = groupArray[i].toObject();
        if ( !(json.contains("members") && json["members"].isArray()) )
        {
            continue;
        }
        auto groupAndLayout = this->createGroupAndLayout(groupObject.description["name"].toString().toStdString());
        bool groupIsOptional = true;
        destinationLayouts.push_back(groupAndLayout);
        QJsonArray memberArray = groupObject.description["members"].toArray();
        try
        {
            for (int j = 0 ; j<memberArray.size() ; ++j)
            {
                InputObject &inputObject = this->idToInputObject.at(memberArray[j].toString().toStdString());
                inputObject.group = &this->groupObjects.back();
                if(inputObject.description["optional"].toBool()) {
                    groupIsOptional = false;
                }
            }
        } catch (const out_of_range& oor)
        {
            Q_UNUSED(oor)
            QMessageBox::critical(this, "Error in descriptor file", "Group member is not an input.");
            return;
        }
    }

    for (auto& idAndInputObject: idToInputObject)
    {
        const string &inputId = idAndInputObject.first;
        InputObject &inputObject = idAndInputObject.second;

        QWidget *widget = nullptr;
        QLayout *parentLayout = nullptr;

        if(inputObject.group != nullptr)
        {
            parentLayout = inputObject.group->layout;
        }
        else if(inputObject.description["optional"].toBool())
        {
            parentLayout = optionalInputsGroupAndLayout.second;
        }
        else
        {
            parentLayout = mainInputsGroupAndLayout.second;
        }

        const QString& inputName = inputObject.description["name"].toString();
        const QString& inputType = inputObject.description["type"].toString();
        const QString& inputDescription = inputObject.description["description"].toString();
        const QJsonValue& inputValue = this->invocationJSON->value(QString::fromStdString(inputId));

        bool inputIsMutuallyExclusive = this->inputIsMutuallyExclusive(inputId);

        //  if input is part of a mutually exclusive group:
        //      create a horizontal layout to put the radio button
        //      along with the corresponding widget
        //      (an open file button, a text field or a spinbox when type is File, String or Number respectively) if necessary (nothing if type is Flag)
        //      the later widget will be a child of this horizontal layout, and the idToGetValue will be set accordingly
        if(inputIsMutuallyExclusive)
        {
            QWidget *hWidget = new QWidget();
            QHBoxLayout *hLayout = new QHBoxLayout(hWidget);
            hWidget->setLayout(hLayout);
            QRadioButton *radioButton = new QRadioButton(inputName);
            widget = radioButton;
            radioButton->setChecked(inputValue.toBool());
            inputObject.radioButton = radioButton;

            // when the widget value changes, the invocationJSON is updated by calling getValue() (with the help of self.idToGetValue)
            // getValue will be overriden if input is not a Flag

            inputObject.getValue = [radioButton]() { return QJsonValue(radioButton->isChecked()); };
            connect(radioButton, &QRadioButton::toggled, [this, inputId]() { this->valueChanged(inputId); } );
            widget->setToolTip(inputDescription);
            hLayout->addWidget(widget);
            parentLayout = hLayout;
        }


        if(inputType == "File")
        {
            QPushButton *pushButton= new QPushButton("Select" + inputObject.description["name"].toString());
            widget = pushButton;

            inputObject.getValue = [this, inputName]() { return QJsonValue(QFileDialog::getOpenFileName(this, "Select " + inputName)); };
            connect(pushButton, &QPushButton::clicked, [this, inputId]() { this->valueChanged(inputId); } );
        }

        if(inputType == "String" || inputType == "Number")
        {
            widget = new QGroupBox();
            QHBoxLayout *layout = new QHBoxLayout();
            QLabel *label = new QLabel(inputName + ":");
            layout->addWidget(label);

            QWidget *subWidget = nullptr;
            if(inputObject.description["list"].toBool())
            {
                QLineEdit *lineEdit = new QLineEdit();
                subWidget = lineEdit;
                lineEdit->setPlaceholderText(QString("Comma separated ") + (inputType == "String" ? "strings" : "numbers") + ".");
                QString listString;

                QJsonDocument valueList;
                valueList.setArray(inputValue.toArray());
                lineEdit->setText(QString::fromUtf8(valueList.toJson()).remove('[').remove(']'));

                inputObject.getValue = [this, lineEdit]() { return QJsonValue(this->stringToArray(lineEdit->text().toStdString())); };
                connect(lineEdit, &QLineEdit::textChanged, [this, inputId](){ this->valueChanged(inputId); });
            }
            else
            {
                if(inputType == "String")
                {
                    QLineEdit *lineEdit = new QLineEdit();
                    subWidget = lineEdit;
                    lineEdit->setPlaceholderText(inputDescription);
                    lineEdit->setText(inputValue.toString());

                    inputObject.getValue = [lineEdit]() { return lineEdit->text(); };
                    connect(lineEdit, &QLineEdit::textChanged, [this, inputId](){ this->valueChanged(inputId); });
                }
                else
                {
                    if(inputObject.description["integer"].toBool())
                    {
                        QSpinBox *spinBox = new QSpinBox();
                        if(inputObject.description["minimum"].isDouble()) {
                            spinBox->setMinimum(static_cast<int>(inputObject.description["minimum"].toDouble()));
                        }
                        if(inputObject.description["maximum"].isDouble()) {
                            spinBox->setMinimum(static_cast<int>(inputObject.description["maximum"].toDouble()));
                        }
                        spinBox->setValue(static_cast<int>(inputValue.toDouble()));
                        inputObject.getValue = [spinBox](){ return QJsonValue(spinBox->value()); };
                        connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this, inputId](){ this->valueChanged(inputId); });
                        subWidget = spinBox;
                    }
                    else
                    {
                        QDoubleSpinBox *spinBox = new QDoubleSpinBox();
                        if(inputObject.description["minimum"].isDouble()) {
                            spinBox->setMinimum(inputObject.description["minimum"].toDouble());
                        }
                        if(inputObject.description["maximum"].isDouble()) {
                            spinBox->setMinimum(inputObject.description["maximum"].toDouble());
                        }
                        spinBox->setValue(inputValue.toDouble());
                        inputObject.getValue = [spinBox](){ return QJsonValue(spinBox->value()); };
                        connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this, inputId](){ this->valueChanged(inputId); });
                        subWidget = spinBox;
                    }
                }
            }
            widget->setToolTip(inputDescription);
            layout->addWidget(subWidget);
            widget->setLayout(layout);
        }

        if(inputType == "Flag")
        {
            if(!inputIsMutuallyExclusive)
            {
                QCheckBox *checkBox = new QCheckBox(inputName);
                checkBox->setCheckState(inputValue.toBool() ? Qt::Checked : Qt::Unchecked);

                inputObject.getValue = [checkBox](){ return QJsonValue(checkBox->isChecked()); };
                connect(checkBox, &QCheckBox::stateChanged, [this, inputId](){ this->valueChanged(inputId); });
                checkBox->setToolTip(inputDescription);
                widget = checkBox;
            }

            if(!inputIsMutuallyExclusive || inputType != "Flag")
            {
                parentLayout->addWidget(widget);
            }
        }
    }
    for(auto& destinationLayout: destinationLayouts)
    {
        destinationLayout.second->addWidget(destinationLayout.first);
    }
    groupLayout->addWidget(mainInputsGroupAndLayout.first);
    groupLayout->addWidget(optionalInputsGroupAndLayout.first);
}
