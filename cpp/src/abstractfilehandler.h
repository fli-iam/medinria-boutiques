#ifndef ABSTRACTFILEHANDLER_H
#define ABSTRACTFILEHANDLER_H

#include <QWidget>
#include <QMimeData>
#include "configuration.h"

// A pair: type and extension, can be created from a QJsonArray where the first value is the type, and the second is the extension
struct FormatAndExtension {
    QString type;
    QString extension;
    FormatAndExtension() {}
    FormatAndExtension(const QString &type, const QString &extension): type(type), extension(extension) {}
    FormatAndExtension(const QJsonArray &typeAndExtension);
};

// The type, description and the compatible extensions (returned from possible writers of the given data in medInria)
struct FormatObject {
    QString type;
    QString description;
    QStringList extensions;

    FormatObject() {}
    FormatObject(const QString &type, const QString &description, const QStringList &extensions):
    type(type),
    description(description),
    extensions(extensions)
    {}
};

// See description in filehandler.h
class AbstractFileHandler : public QObject
{
    Q_OBJECT

public:
    AbstractFileHandler();
    virtual ~AbstractFileHandler();

    virtual void checkAcceptDragEvent(QDragEnterEvent *event) = 0;
    virtual QString createTemporaryInputFileForMimeData(const QMimeData *mimeData) = 0;
    virtual QString createTemporaryInputFileForCurrentInput() = 0;
    virtual bool hasKnownExtension(const QString &fileName) = 0;
};

#endif // ABSTRACTFILEHANDLER_H
