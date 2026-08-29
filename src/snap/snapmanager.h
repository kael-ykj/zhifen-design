#ifndef SNAPMANAGER_H
#define SNAPMANAGER_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QSet>

class CadScene;
class CadView;
class CadItem;

enum SnapType { SnapEndpoint, SnapMidpoint, SnapCenter, SnapIntersection, SnapNearest, SnapQuadrant, SnapPerpendicular, SnapTangent, SnapExtension, SnapParallel };

struct SnapResult {
    QPointF point;
    SnapType type;
    CadItem *item = nullptr;
};

class SnapManager : public QObject
{
    Q_OBJECT
public:
    explicit SnapManager(CadScene *scene, CadView *view, QObject *parent = nullptr);

    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void toggleType(SnapType type) { if (m_types.contains(type)) m_types.remove(type); else m_types.insert(type); }
    void setTypes(const QSet<SnapType> &types) { m_types = types; }

    SnapResult* computeSnap(const QPointF &worldPos);
    bool hasSnap() const { return m_currentSnap != nullptr; }
    void clearSnap() { delete m_currentSnap; m_currentSnap = nullptr; }
    void drawSnapMarker(QPainter *painter);

private:
    CadScene *m_scene;
    CadView *m_view;
    bool m_enabled = true;
    QSet<SnapType> m_types = {SnapEndpoint, SnapMidpoint, SnapCenter, SnapIntersection, SnapNearest, SnapPerpendicular, SnapTangent};
    SnapResult *m_currentSnap = nullptr;
    qreal m_tolerance = 10.0; // 屏幕像素
};

#endif // SNAPMANAGER_H
