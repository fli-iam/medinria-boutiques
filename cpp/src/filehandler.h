#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "abstractfilehandler.h"
#include "toolboxwidget.h"

QT_BEGIN_NAMESPACE
class medDataIndex;
class medAbstractData;
class medBoutiquesToolBox;
QT_END_NAMESPACE

#define PREFERRED_FORMATS_SETTINGS_PATH "../preferredFormats.json"

class FileHandler : public AbstractFileHandler
{
private:
    medBoutiquesToolBox *toolbox;
    QJsonObject dataTypeToFormatAndExtension;
    QJsonArray preferredFormatsAndExtensions;

public:
    FileHandler(medBoutiquesToolBox *toolbox = nullptr);

    void checkAcceptDragEvent(QDragEnterEvent *event) override;
    QString createTemporaryInputFileForMimeData(const QMimeData *mimeData) override;
    QString createTemporaryInputFileForCurrentInput() override;

private:
    QList<FormatObject> getFileFormatsForData(medAbstractData *data);
    FormatAndExtension getFormatAndExtensionForData(medAbstractData *data);
    FormatAndExtension getFormatForInputFile(const QString &dataType, const QList<FormatObject> &fileFormats);
    QString createTemporaryInputFile(medAbstractData *data, const QString &chosenType, const QString &chosenExtension);
    void savePreferredFormatSettings();
};

#endif // FILEHANDLER_H
