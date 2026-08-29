#ifndef DIMENSION_ITEM_H
#define DIMENSION_ITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QColor>
#include <QPointF>
#include <QRectF>

namespace Zhifen {

// 标注类型
enum DimensionType {
    Dim_Linear = 0,      // 线性标注（水平/垂直）
    Dim_Aligned = 1,     // 对齐标注
    Dim_Radius = 2,      // 半径标注
    Dim_Diameter = 3,    // 直径标注
    Dim_Angular = 4      // 角度标注
};

// 标注样式
struct DimensionStyle {
    qreal textHeight = 2.5;       // 文字高度
    qreal arrowSize = 2.5;        // 箭头大小
    qreal extensionLineOffset = 1.0; // 尺寸界线偏移
    qreal extensionLineExtend = 1.5; // 尺寸界线超出
    int precision = 0;            // 小数精度
    QColor color = QColor(0, 0, 255); // 标注颜色
    QString prefix = "";          // 前缀
    QString suffix = "";          // 后缀
};

// 标注图元基类
class DimensionItem : public QGraphicsItem
{
public:
    explicit DimensionItem(DimensionType type, QGraphicsItem *parent = nullptr);
    virtual ~DimensionItem();

    // 类型
    DimensionType dimensionType() const { return m_type; }

    // 样式
    void setStyle(const DimensionStyle &style) { m_style = style; update(); }
    DimensionStyle style() const { return m_style; }

    // 标注文字
    void setText(const QString &text) { m_text = text; update(); }
    QString text() const { return m_text; }

    // 测量值（自动计算）
    virtual qreal measuredValue() const = 0;

    // 自动生成文字
    virtual QString autoText() const;

    // QGraphicsItem接口
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    DimensionType m_type;
    DimensionStyle m_style;
    QString m_text;
    QRectF m_boundingRect;

    // 绘制箭头
    void drawArrow(QPainter *painter, const QPointF &pos, qreal angle);
    // 绘制文字
    void drawText(QPainter *painter, const QPointF &pos, const QString &text);
    // 更新边界
    virtual void updateBoundingRect();
};

// 线性标注
class LinearDimension : public DimensionItem
{
public:
    explicit LinearDimension(QGraphicsItem *parent = nullptr);

    void setPoints(const QPointF &p1, const QPointF &p2, const QPointF &dimPos);
    QPointF point1() const { return m_p1; }
    QPointF point2() const { return m_p2; }
    QPointF dimPosition() const { return m_dimPos; }

    // 水平/垂直
    void setHorizontal(bool horizontal) { m_horizontal = horizontal; update(); }
    bool isHorizontal() const { return m_horizontal; }

    qreal measuredValue() const override;

protected:
    QPointF m_p1, m_p2, m_dimPos;
    bool m_horizontal = true;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateBoundingRect() override;
};

// 对齐标注
class AlignedDimension : public DimensionItem
{
public:
    explicit AlignedDimension(QGraphicsItem *parent = nullptr);

    void setPoints(const QPointF &p1, const QPointF &p2, const QPointF &dimPos);
    QPointF point1() const { return m_p1; }
    QPointF point2() const { return m_p2; }

    qreal measuredValue() const override;

protected:
    QPointF m_p1, m_p2, m_dimPos;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateBoundingRect() override;
};

// 半径标注
class RadiusDimension : public DimensionItem
{
public:
    explicit RadiusDimension(QGraphicsItem *parent = nullptr);

    void setCircle(const QPointF &center, qreal radius, const QPointF &dimPos);
    QPointF center() const { return m_center; }
    qreal radius() const { return m_radius; }

    qreal measuredValue() const override { return m_radius; }

protected:
    QPointF m_center, m_dimPos;
    qreal m_radius = 0;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateBoundingRect() override;
};

// 直径标注
class DiameterDimension : public DimensionItem
{
public:
    explicit DiameterDimension(QGraphicsItem *parent = nullptr);

    void setCircle(const QPointF &center, qreal radius, const QPointF &dimPos);
    QPointF center() const { return m_center; }
    qreal diameter() const { return m_radius * 2; }

    qreal measuredValue() const override { return m_radius * 2; }

protected:
    QPointF m_center, m_dimPos;
    qreal m_radius = 0;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateBoundingRect() override;
};

// 角度标注
class AngularDimension : public DimensionItem
{
public:
    explicit AngularDimension(QGraphicsItem *parent = nullptr);

    void setLines(const QPointF &vertex, const QPointF &p1, const QPointF &p2);
    QPointF vertex() const { return m_vertex; }

    qreal measuredValue() const override;

protected:
    QPointF m_vertex, m_p1, m_p2;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void updateBoundingRect() override;
};

} // namespace Zhifen

#endif // DIMENSION_ITEM_H
