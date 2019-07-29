#ifndef ABSTRACTFILEHANDLER_H
#define ABSTRACTFILEHANDLER_H

#include <QWidget>
#include <QMimeData>

struct FormatObject {
    QString type;
    QString description;
    QStringList extensions;

    FormatObject(const QString &type, const QString &description, const QStringList &extensions):
    type(type),
    description(description),
    extensions(extensions)
    {}
};

class AbstractFileHandler
{
public:
    AbstractFileHandler();
    virtual ~AbstractFileHandler();

    virtual void checkAcceptDragEvent(QDragEnterEvent *event) = 0;
    virtual QList<FormatObject> getFileFormatsForMimeData(const QMimeData *mimeData) = 0;
    virtual QList<FormatObject> getFileFormatsForCurrentInput() = 0;
    virtual QString createTemporaryInputFileForMimeData(const QMimeData *mimeData, const QString &chosenType, const QString &chosenExtension) = 0;
    virtual QString createTemporaryInputFileForCurrentInput(const QString &chosenType, const QString &chosenExtension) = 0;
};

#endif // ABSTRACTFILEHANDLER_H
