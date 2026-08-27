#ifndef DXFWRITER_H
#define DXFWRITER_H

#include <QString>

class CadScene;

class DxfWriter
{
public:
    explicit DxfWriter(CadScene *scene);

    bool write(const QString &filePath);
    QString writeToString();

private:
    CadScene *m_scene;
};

#endif // DXFWRITER_H
