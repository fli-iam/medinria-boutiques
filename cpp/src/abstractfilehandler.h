#ifndef ABSTRACTFILEHANDLER_H
#define ABSTRACTFILEHANDLER_H

#include <QWidget>
#include <QMimeData>

struct FormatAndExtension {
    QString type;
    QString extension;
    FormatAndExtension() {}
    FormatAndExtension(const QString &type, const QString &extension): type(type), extension(extension) {}
    FormatAndExtension(const QJsonArray &typeAndExtension);
};

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

class AbstractFileHandler : public QObject
{
    Q_OBJECT

public:
    AbstractFileHandler();
    virtual ~AbstractFileHandler();

    virtual void checkAcceptDragEvent(QDragEnterEvent *event) = 0;
    virtual QString createTemporaryInputFileForMimeData(const QMimeData *mimeData) = 0;
    virtual QString createTemporaryInputFileForCurrentInput() = 0;
};

#endif // ABSTRACTFILEHANDLER_H
