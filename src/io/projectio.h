#ifndef PROJECTIO_H
#define PROJECTIO_H

#include <QString>
#include <QJsonObject>

class CadScene;
class Document;

class ProjectIO
{
public:
    explicit ProjectIO(CadScene *scene, Document *doc);

    bool save(const QString &filePath);
    bool load(const QString &filePath);
    QString errorString() const { return m_error; }

private:
    CadScene *m_scene;
    Document *m_document;
    QString m_error;

    QJsonObject serializeLayers();
    void deserializeLayers(const QJsonObject &root);
    QJsonObject serializeEntities();
    void deserializeEntities(const QJsonObject &root);
};

#endif // PROJECTIO_H
