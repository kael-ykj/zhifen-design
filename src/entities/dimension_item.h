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

// 箭头类型
enum ArrowType {
    Arrow_ClosedFilled = 0,   // 闭合实心
    Arrow_ClosedBlank = 1,    // 闭合空心
    Arrow_Open = 2,           // 开放
    Arrow_ArchTick = 3,       // 建筑标记
    Arrow_Oblique = 4,        // 倾斜
    Arrow_Dot = 5,            // 点
    Arrow_None = 6            // 无
};

// 文字位置
enum DimTextPosition {
    TextPos_Above = 0,        // 尺寸线上方
    TextPos_Centered = 1,     // 尺寸线居中
    TextPos_Outside = 2       // 尺寸线外侧
};

// 文字对齐
enum DimTextAlignment {
    TextAlign_Horizontal = 0,  // 水平
    TextAlign_Aligned = 1,     // 与尺寸线对齐
    TextAlign_ISO = 2          // ISO标准
};

// 标注样式
struct DimensionStyle {
    QString name = "Standard";  // 样式名称

    // 线
    QColor dimLineColor = QColor(0, 0, 255);    // 尺寸线颜色
    qreal dimLineWidth = 0.25;                   // 尺寸线线宽
    bool dimLineSuppress1 = false;               // 抑制尺寸线1
    bool dimLineSuppress2 = false;               // 抑制尺寸线2
    QColor extLineColor = QColor(0, 0, 255);    // 尺寸界线颜色
    qreal extLineWidth = 0.25;                   // 尺寸界线线宽
    qreal extLineOffset = 0.625;                 // 尺寸界线偏移
    qreal extLineExtend = 1.25;                  // 尺寸界线超出
    bool extLineSuppress1 = false;               // 抑制尺寸界线1
    bool extLineSuppress2 = false;               // 抑制尺寸界线2
    qreal extLineFixedLength = 0;                // 固定长度尺寸界线

    // 符号和箭头
    ArrowType arrowType = Arrow_ClosedFilled;    // 箭头类型
    qreal arrowSize = 2.5;                       // 箭头大小
    ArrowType firstArrow = Arrow_ClosedFilled;   // 第一个箭头
    ArrowType secondArrow = Arrow_ClosedFilled;  // 第二个箭头
    qreal centerMarkSize = 2.5;                  // 圆心标记大小
    bool centerMark = true;                      // 圆心标记
    bool centerLine = false;                     // 中心线
    qreal arcLengthSymbolHeight = 2.5;           // 弧长符号高度

    // 文字
    QString textStyle = "Standard";              // 文字样式
    qreal textHeight = 2.5;                      // 文字高度
    QColor textColor = QColor(0, 0, 255);       // 文字颜色
    DimTextPosition textPosition = TextPos_Above; // 文字位置
    DimTextAlignment textAlignment = TextAlign_Aligned; // 文字对齐
    qreal textOffset = 0.625;                    // 文字偏移
    bool textInside = true;                      // 文字在尺寸界线内
    bool textOutside = true;                     // 文字在尺寸界线外
    QString textPrefix = "";                     // 文字前缀
    QString textSuffix = "";                     // 文字后缀
    bool drawFrameAroundText = false;            // 文字加框

    // 调整
    bool fitTextOrArrows = true;                 // 文字或箭头最佳效果
    bool suppressArrowsIfNotFit = false;         // 放不下时抑制箭头
    qreal scaleFactor = 1.0;                     // 全局比例
    bool scaleToLayout = false;                  // 按布局缩放

    // 主单位
    int linearPrecision = 0;                     // 线性精度
    QString linearDecimalSeparator = ".";        // 小数分隔符
    QString linearPrefix = "";                   // 线性前缀
    QString linearSuffix = "";                   // 线性后缀
    qreal linearScaleFactor = 1.0;               // 线性比例因子
    bool linearSuppressLeadingZeros = false;     // 抑制前导零
    bool linearSuppressTrailingZeros = false;    // 抑制后续零
    int angularPrecision = 0;                    // 角度精度
    QString angularUnits = "Decimal Degrees";    // 角度单位

    // 换算单位
    bool altUnitsEnabled = false;                // 启用换算单位
    int altPrecision = 2;                        // 换算精度
    qreal altScaleFactor = 25.4;                 // 换算比例因子
    QString altPrefix = "";                      // 换算前缀
    QString altSuffix = "mm";                    // 换算后缀

    // 公差
    bool toleranceEnabled = false;               // 启用公差
    int toleranceType = 0;                       // 公差类型(0=无,1=对称,2=极限偏差,3=极限尺寸,4=基本尺寸)
    qreal toleranceUpper = 0.0;                  // 上偏差
    qreal toleranceLower = 0.0;                  // 下偏差
    int tolerancePrecision = 0;                  // 公差精度
    qreal toleranceHeightScale = 0.5;            // 公差高度比例
    int toleranceVerticalPosition = 1;           // 公差垂直位置(0=下,1=中,2=上)
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
