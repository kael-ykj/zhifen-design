#ifndef LABELITEM_H
#define LABELITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QColor>
#include <QFont>

class LabelItem : public QGraphicsItem
{
public:
    enum OperatorType {
        Op_None = 0,
        Op_ChinaMobile,    // 中国移动
        Op_ChinaUnicom,    // 中国联通
        Op_ChinaTelecom,   // 中国电信
        Op_ChinaBroadnet   // 中国广电
    };

    explicit LabelItem(QGraphicsItem *parent = nullptr);
    explicit LabelItem(const QString &text, OperatorType op = Op_None, QGraphicsItem *parent = nullptr);

    void setText(const QString &text) { prepareGeometryChange(); m_text = text; }
    QString text() const { return m_text; }

    void setOperator(OperatorType op) { prepareGeometryChange(); m_operator = op; }
    OperatorType operatorType() const { return m_operator; }

    void setTextColor(const QColor &color) { m_textColor = color; }
    QColor textColor() const { return m_textColor; }

    void setBackgroundColor(const QColor &color) { m_backgroundColor = color; }
    QColor backgroundColor() const { return m_backgroundColor; }

    void setBorderColor(const QColor &color) { m_borderColor = color; }
    QColor borderColor() const { return m_borderColor; }

    void setFont(const QFont &font) { prepareGeometryChange(); m_font = font; }
    QFont font() const { return m_font; }

    void setShowOperatorLogo(bool show) { prepareGeometryChange(); m_showOperatorLogo = show; }
    bool showOperatorLogo() const { return m_showOperatorLogo; }

    // 获取运营商颜色
    static QColor operatorColor(OperatorType op) {
        switch (op) {
        case Op_ChinaMobile: return QColor(0, 102, 204);   // 移动蓝
        case Op_ChinaUnicom: return QColor(220, 0, 0);     // 联通红
        case Op_ChinaTelecom: return QColor(0, 153, 102);  // 电信绿
        case Op_ChinaBroadnet: return QColor(255, 153, 0); // 广电橙
        default: return QColor(100, 100, 100);
        }
    }

    // 获取运营商名称
    static QString operatorName(OperatorType op) {
        switch (op) {
        case Op_ChinaMobile: return "中国移动";
        case Op_ChinaUnicom: return "中国联通";
        case Op_ChinaTelecom: return "中国电信";
        case Op_ChinaBroadnet: return "中国广电";
        default: return "";
        }
    }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    QString m_text;
    OperatorType m_operator = Op_None;
    QColor m_textColor = QColor(0, 0, 0);
    QColor m_backgroundColor = QColor(255, 255, 255, 200);
    QColor m_borderColor = QColor(0, 0, 0);
    QFont m_font = QFont("Arial", 8);
    bool m_showOperatorLogo = true;
};

#endif // LABELITEM_H
