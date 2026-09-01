#include "labelitem.h"
#include <QPainter>
#include <QFontMetrics>

LabelItem::LabelItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

LabelItem::LabelItem(const QString &text, OperatorType op, QGraphicsItem *parent)
    : QGraphicsItem(parent), m_text(text), m_operator(op)
{
    setFlag(QGraphicsItem::ItemIsMovable);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF LabelItem::boundingRect() const
{
    QFontMetrics fm(m_font);
    int textWidth = fm.horizontalAdvance(m_text);
    int textHeight = fm.height();
    int logoWidth = m_showOperatorLogo && m_operator != Op_None ? 60 : 0;
    int width = qMax(textWidth + logoWidth + 20, 80);
    int height = qMax(textHeight + 10, 24);
    return QRectF(0, 0, width, height);
}

void LabelItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF rect = boundingRect();

    // 背景
    painter->fillRect(rect, m_backgroundColor);
    painter->setPen(QPen(m_borderColor, 1));
    painter->drawRect(rect);

    int xOffset = 5;

    // 运营商LOGO（文字+色块模拟）
    if (m_showOperatorLogo && m_operator != Op_None) {
        QColor opColor = operatorColor(m_operator);
        QString opName = operatorName(m_operator);

        // 左侧色块
        QRectF logoRect(2, 2, 8, rect.height() - 4);
        painter->fillRect(logoRect, opColor);

        // 运营商文字
        painter->setPen(opColor);
        QFont opFont = m_font;
        opFont.setBold(true);
        painter->setFont(opFont);
        painter->drawText(QRectF(12, 2, 50, rect.height() - 4), Qt::AlignVCenter | Qt::AlignLeft, opName);

        xOffset = 65;
    }

    // 标签文本
    painter->setPen(m_textColor);
    painter->setFont(m_font);
    painter->drawText(QRectF(xOffset, 0, rect.width() - xOffset - 5, rect.height()),
                      Qt::AlignVCenter | Qt::AlignLeft, m_text);
}
