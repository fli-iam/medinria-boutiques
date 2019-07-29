#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include "abstractfilehandler.h"
#include "toolboxwidget.h"

QT_BEGIN_NAMESPACE
class medDataIndex;
class medAbstractData;
QT_END_NAMESPACE

class FileHandler : public AbstractFileHandler
{
private:
    ToolBoxWidget *toolbox;

public:
    FileHandler(ToolBoxWidget *toolbox);

    void checkAcceptDragEvent(QDragEnterEvent *event) override;
    QList<FormatObject> getFileFormatsForMimeData(const QMimeData *mimeData) override;
    QList<FormatObject> getFileFormatsForCurrentInput() override;
    QString createTemporaryInputFileForMimeData(const QMimeData *mimeData, const QString &chosenType, const QString &chosenExtension) override;
    QString createTemporaryInputFileForCurrentInput(const QString &chosenType, const QString &chosenExtension) override;

private:
    bool createFileFromInput();
    bool createFileFromDataIndex(const medDataIndex &index);
    void createFileFromData(medAbstractData *data);
};

#endif // FILEHANDLER_H
