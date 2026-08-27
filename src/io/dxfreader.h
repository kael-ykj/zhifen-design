#ifndef DXFREADER_H
#define DXFREADER_H

#include <QString>
#include <QPointF>

class CadScene;
class Document;

class DxfReader
{
public:
    explicit DxfReader(CadScene *scene, Document *doc = nullptr);

    bool read(const QString &filePath);
    bool readFromText(const QString &text);
    QString errorString() const { return m_error; }

private:
    CadScene *m_scene;
    Document *m_document;
    QString m_error;

    struct DxfPair { int code; QString value; };
    QList<DxfPair> parsePairs(const QString &text);
    void processEntities(const QList<DxfPair> &pairs);
    QPointF parsePoint(const QList<DxfPair> &pairs, int &index);
};

#endif // DXFREADER_H
