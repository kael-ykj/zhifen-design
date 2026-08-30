#ifndef CADSCENE_H
#define CADSCENE_H

#include <QGraphicsScene>
#include "blocks/blockmanager.h"
#include <QPointF>
#include <QSet>

class CadItem;
class Document;

class CadScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CadScene(QObject *parent = nullptr);

    void setDocument(Document *doc) { m_document = doc; }
    Document* document() const { return m_document; }

    // 网格
    void setGridSize(qreal size) { m_gridSize = size; }
    qreal gridSize() const { return m_gridSize; }
    void setShowGrid(bool show) { m_showGrid = show; update(); }
    bool showGrid() const { return m_showGrid; }

    // 坐标轴
    void setShowAxis(bool show) { m_showAxis = show; update(); }
    bool showAxis() const { return m_showAxis; }

    // 选择
    QList<CadItem*> selectedCadItems() const;
    void clearSelection();

    // 添加图元
    void addCadItem(CadItem *item);

private slots:
    void onBlockRedefined(const QString &name);

signals:
    void selectionChangedCount(int count);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    Document *m_document = nullptr;
    qreal m_gridSize = 10.0;
    bool m_showGrid = true;
    bool m_showAxis = true;

    void drawGrid(QPainter *painter, const QRectF &rect);
    void drawAxis(QPainter *painter, const QRectF &rect);
};

#endif // CADSCENE_H
