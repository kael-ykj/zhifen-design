#include "cadscene.h"
#include "blocks/blockreference.h"
#include "blocks/blockmanager.h"
#include "caditem.h"
#include "document.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>

CadScene::CadScene(QObject *parent)
    : QGraphicsScene(parent)
{
    setSceneRect(-1000, -1000, 2000, 2000);
    connect(this, &QGraphicsScene::selectionChanged, this, [this]() {
        emit selectionChangedCount(selectedItems().count());
    });
    connect(&Zhifen::BlockManager::instance(), &Zhifen::BlockManager::blockRedefined, this, &CadScene::onBlockRedefined);
}

QList<CadItem*> CadScene::selectedCadItems() const
{
    QList<CadItem*> items;
    for (auto item : selectedItems()) {
        CadItem *cad = dynamic_cast<CadItem*>(item);
        if (cad) items.append(cad);
    }
    return items;
}

void CadScene::clearSelection()
{
    QGraphicsScene::clearSelection();
}

void CadScene::addCadItem(CadItem *item)
{
    addItem(item);
}

void CadScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    // 深色背景
    painter->fillRect(rect, QColor(30, 30, 30));

    if (m_showGrid) {
        drawGrid(painter, rect);
    }
}

void CadScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (m_showAxis) {
        drawAxis(painter, rect);
    }
}

void CadScene::drawGrid(QPainter *painter, const QRectF &rect)
{
    painter->save();
    painter->setPen(QPen(QColor(60, 60, 60), 0));

    qreal left = qFloor(rect.left() / m_gridSize) * m_gridSize;
    qreal right = rect.right();
    qreal top = qFloor(rect.top() / m_gridSize) * m_gridSize;
    qreal bottom = rect.bottom();

    QVarLengthArray<QLineF, 100> lines;
    for (qreal x = left; x < right; x += m_gridSize) {
        lines.append(QLineF(x, top, x, bottom));
    }
    for (qreal y = top; y < bottom; y += m_gridSize) {
        lines.append(QLineF(left, y, right, y));
    }
    painter->drawLines(lines.data(), lines.size());
    painter->restore();
}

void CadScene::drawAxis(QPainter *painter, const QRectF &rect)
{
    painter->save();
    // X轴（红色）
    painter->setPen(QPen(QColor(255, 100, 100), 0));
    painter->drawLine(rect.left(), 0, rect.right(), 0);
    // Y轴（绿色）
    painter->setPen(QPen(QColor(100, 255, 100), 0));
    painter->drawLine(0, rect.top(), 0, rect.bottom());
    // 原点标记
    painter->setPen(QPen(QColor(255, 255, 100), 0));
    painter->setFont(QFont("Arial", 8));
    painter->drawText(2, -2, "0,0");
    painter->restore();
}


void CadScene::onBlockRedefined(const QString &name)
{
    // 更新所有引用该块的BlockReference
    for (QGraphicsItem *item : items()) {
        if (auto blockRef = dynamic_cast<Zhifen::BlockReference*>(item)) {
            if (blockRef->blockName() == name) {
                blockRef->prepareGeometryChange();
                blockRef->update();
            }
        }
    }
}
