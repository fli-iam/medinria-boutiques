#include "filehandler.h"

#include <QtWidgets>

FileHandler::FileHandler(ToolBoxWidget *toolbox): toolbox(toolbox)
{

}

bool FileHandler::createFileFromInput()
{
    return true;
}

bool FileHandler::createFileFromDataIndex(const medDataIndex &index)
{
    Q_UNUSED(index)
    return true;
}

void FileHandler::createFileFromData(medAbstractData* data)
{
    Q_UNUSED(data)
}

void FileHandler::checkAcceptDragEvent(QDragEnterEvent *event)
{
    Q_UNUSED(event)
    event->acceptProposedAction();
}

QList<FormatObject> FileHandler::getFileFormatsForMimeData(const QMimeData *mimeData)
{
    Q_UNUSED(mimeData)

    QList<FormatObject> formats;
    formats.push_back(FormatObject("Format 1", "A very nice format", QStringList({".ext1", ".ext2"})));
    formats.push_back(FormatObject("Format 2", "Another very nice format", QStringList({".ext3", ".ext4"})));
    return formats;
}

QList<FormatObject> FileHandler::getFileFormatsForCurrentInput()
{
    QList<FormatObject> formats;
    formats.push_back(FormatObject("Format 1", "A very nice format", QStringList({".ext1", ".ext2"})));
    formats.push_back(FormatObject("Format 2", "Another very nice format", QStringList({".ext3", ".ext4"})));
    return formats;
}

QString FileHandler::createTemporaryInputFileForMimeData(const QMimeData *mimeData, const QString &chosenType, const QString &chosenExtension)
{
    Q_UNUSED(mimeData)
    Q_UNUSED(chosenType)
    Q_UNUSED(chosenExtension)

    QTemporaryFile file("XXXXXX" + chosenType + chosenExtension);
    if (file.open()) {
        QString absoluteFilePath = QFileInfo(file).absoluteFilePath();
        return absoluteFilePath;
    }
    return "";
}

QString FileHandler::createTemporaryInputFileForCurrentInput(const QString &chosenType, const QString &chosenExtension)
{
    Q_UNUSED(chosenType)
    Q_UNUSED(chosenExtension)

    QTemporaryFile file("XXXXXX" + chosenType + chosenExtension);
    if (file.open()) {
        QString absoluteFilePath = QFileInfo(file).absoluteFilePath();
        return absoluteFilePath;
    }
    return "";
}
