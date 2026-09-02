#include "iconfactory.h"
#include <QPainter>
#include <QPixmap>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

namespace Zhifen {

IconFactory& IconFactory::instance()
{
    static IconFactory inst;
    return inst;
}

IconFactory::IconFactory()
{
    initIcons();
}

QIcon IconFactory::icon(const QString &name)
{
    if (m_icons.contains(name)) return m_icons[name];
    return drawIcon(name);
}

void IconFactory::initIcons()
{
    // 绘图工具
    m_icons["line"] = drawIcon("line");
    m_icons["circle"] = drawIcon("circle");
    m_icons["rectangle"] = drawIcon("rectangle");
    m_icons["arc"] = drawIcon("arc");
    m_icons["text"] = drawIcon("text");
    m_icons["feeder"] = drawIcon("feeder");
    
    // 编辑工具
    m_icons["select"] = drawIcon("select");
    m_icons["move"] = drawIcon("move");
    m_icons["copy"] = drawIcon("copy");
    m_icons["rotate"] = drawIcon("rotate");
    m_icons["scale"] = drawIcon("scale");
    m_icons["mirror"] = drawIcon("mirror");
    m_icons["delete"] = drawIcon("delete");
    m_icons["undo"] = drawIcon("undo");
    m_icons["redo"] = drawIcon("redo");
    
    // 室分器件
    m_icons["antenna"] = drawIcon("antenna");
    m_icons["coupler"] = drawIcon("coupler");
    m_icons["splitter"] = drawIcon("splitter");
    m_icons["source"] = drawIcon("source");
    m_icons["load"] = drawIcon("load");
    m_icons["amplifier"] = drawIcon("amplifier");
    
    // 文件操作
    m_icons["new"] = drawIcon("new");
    m_icons["open"] = drawIcon("open");
    m_icons["save"] = drawIcon("save");
    m_icons["print"] = drawIcon("print");
    
    // 标注
    m_icons["dim_linear"] = drawIcon("dim_linear");
    m_icons["dim_aligned"] = drawIcon("dim_aligned");
    m_icons["dim_radius"] = drawIcon("dim_radius");
    
    // 其他
    m_icons["layer"] = drawIcon("layer");
    m_icons["block"] = drawIcon("block");
    m_icons["export"] = drawIcon("export");
    m_icons["help"] = drawIcon("help");
    m_icons["about"] = drawIcon("about");
}

QIcon IconFactory::drawIcon(const QString &name, int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    
    QPen pen(QColor(220, 220, 220), 2);
    p.setPen(pen);
    
    if (name == "line") {
        p.drawLine(4, size-4, size-4, 4);
    } else if (name == "circle") {
        p.drawEllipse(4, 4, size-8, size-8);
    } else if (name == "rectangle") {
        p.drawRect(4, 4, size-8, size-8);
    } else if (name == "arc") {
        QRectF rect(4, 4, size-8, size-8);
        p.drawArc(rect, 30*16, 120*16);
    } else if (name == "text") {
        p.setFont(QFont("Arial", 16, QFont::Bold));
        p.drawText(pix.rect(), Qt::AlignCenter, "A");
    } else if (name == "feeder") {
        p.drawLine(4, size/2, size-4, size/2);
        p.setPen(QPen(QColor(100, 200, 100), 2));
        p.drawLine(4, size/2+4, size-4, size/2+4);
    } else if (name == "select") {
        QPainterPath path;
        path.moveTo(6, 4);
        path.lineTo(6, size-6);
        path.lineTo(12, size-10);
        path.lineTo(16, size-4);
        path.lineTo(20, size-6);
        path.lineTo(16, size-12);
        path.lineTo(size-6, size-12);
        path.closeSubpath();
        p.fillPath(path, QColor(0, 120, 200, 180));
        p.drawPath(path);
    } else if (name == "move") {
        p.drawLine(size/2, 4, size/2, size-4);
        p.drawLine(4, size/2, size-4, size/2);
        // 箭头
        p.drawLine(size/2, 4, size/2-5, 10);
        p.drawLine(size/2, 4, size/2+5, 10);
        p.drawLine(4, size/2, 10, size/2-5);
        p.drawLine(4, size/2, 10, size/2+5);
    } else if (name == "copy") {
        p.drawRect(4, 4, size-12, size-12);
        p.setPen(QPen(QColor(100, 180, 255), 2));
        p.drawRect(10, 10, size-12, size-12);
    } else if (name == "rotate") {
        QRectF rect(6, 6, size-12, size-12);
        p.drawArc(rect, 45*16, 270*16);
        // 箭头
        p.drawLine(size-8, size/2-4, size-4, size/2);
        p.drawLine(size-8, size/2+4, size-4, size/2);
    } else if (name == "scale") {
        p.drawRect(4, 4, size-8, size-8);
        p.setPen(QPen(QColor(255, 180, 0), 2, Qt::DashLine));
        p.drawRect(10, 10, size-20, size-20);
    } else if (name == "mirror") {
        p.drawLine(4, size-4, size/2-2, 4);
        p.setPen(QPen(QColor(100, 180, 255), 2, Qt::DashLine));
        p.drawLine(size/2+2, 4, size-4, size-4);
        p.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
        p.drawLine(size/2, 2, size/2, size-2);
    } else if (name == "delete") {
        p.drawLine(8, 8, size-8, size-8);
        p.drawLine(size-8, 8, 8, size-8);
    } else if (name == "undo") {
        p.drawArc(QRectF(4, 4, size-8, size-8), 90*16, 180*16);
        p.drawLine(4, size/2, 10, size/2-6);
        p.drawLine(4, size/2, 10, size/2+6);
    } else if (name == "redo") {
        p.drawArc(QRectF(4, 4, size-8, size-8), -90*16, 180*16);
        p.drawLine(size-4, size/2, size-10, size/2-6);
        p.drawLine(size-4, size/2, size-10, size/2+6);
    } else if (name == "antenna") {
        // 吸顶天线标准符号：三角形+底边
        p.drawLine(size/2, 4, 6, size-6);
        p.drawLine(size/2, 4, size-6, size-6);
        p.drawLine(6, size-6, size-6, size-6);
        p.drawLine(size/2, 4, size/2, size-6);
    } else if (name == "coupler") {
        // 耦合器标准符号：矩形+箭头
        p.drawRect(4, size/2-6, size-8, 12);
        p.drawLine(4, size/2, size-4, size/2);
        p.drawLine(size/2, size/2-6, size/2+6, size/2-12);
        p.drawLine(size/2+6, size/2-12, size/2+10, size/2-8);
    } else if (name == "splitter") {
        // 功分器标准符号：一分二
        p.drawLine(4, size/2, size/3, size/2);
        p.drawLine(size/3, size/2, size-4, size/4);
        p.drawLine(size/3, size/2, size-4, size*3/4);
    } else if (name == "source") {
        // 信源：圆形+B
        p.drawEllipse(4, 4, size-8, size-8);
        p.setFont(QFont("Arial", 14, QFont::Bold));
        p.drawText(pix.rect(), Qt::AlignCenter, "B");
    } else if (name == "load") {
        // 负载：矩形+斜线
        p.drawRect(4, size/2-6, size-12, 12);
        for (int i = 0; i < 4; i++) {
            p.drawLine(8+i*5, size/2-6, 12+i*5, size/2+6);
        }
    } else if (name == "amplifier") {
        // 放大器：三角形
        QPainterPath path;
        path.moveTo(4, 6);
        path.lineTo(size-6, size/2);
        path.lineTo(4, size-6);
        path.closeSubpath();
        p.fillPath(path, QColor(255, 200, 0, 150));
        p.drawPath(path);
    } else if (name == "new") {
        p.drawRect(6, 4, size-12, size-8);
        p.drawLine(size/2, 10, size/2, size-8);
        p.drawLine(10, size/2-2, size-10, size/2-2);
    } else if (name == "open") {
        p.drawRect(4, 8, size-8, size-12);
        p.drawLine(4, 8, size/2, 4);
        p.drawLine(size/2, 4, size-4, 8);
    } else if (name == "save") {
        p.drawRect(4, 4, size-8, size-8);
        p.fillRect(8, 8, size-16, size/2, QColor(0, 120, 200, 100));
        p.drawRect(8, size-12, size-16, 6);
    } else if (name == "print") {
        p.drawRect(6, 10, size-12, size-16);
        p.drawRect(8, 4, size-16, 8);
        p.drawRect(8, size-10, size-16, 6);
    } else if (name == "dim_linear") {
        p.drawLine(4, size/2, size-4, size/2);
        p.drawLine(4, size/2-4, 4, size/2+4);
        p.drawLine(size-4, size/2-4, size-4, size/2+4);
        p.drawText(QRect(0, 0, size, size/2), Qt::AlignCenter, "1000");
    } else if (name == "dim_aligned") {
        p.drawLine(4, size-4, size-4, 4);
        p.drawLine(4, size-4, 8, size-8);
        p.drawLine(4, size-4, 8, size);
    } else if (name == "dim_radius") {
        p.drawEllipse(size/2-8, size/2-8, 16, 16);
        p.drawLine(size/2, size/2, size-4, 4);
    } else if (name == "layer") {
        p.fillRect(4, 4, size-8, 6, QColor(200, 50, 50, 150));
        p.fillRect(4, 12, size-8, 6, QColor(50, 200, 50, 150));
        p.fillRect(4, 20, size-8, 6, QColor(50, 50, 200, 150));
    } else if (name == "block") {
        p.drawRect(4, 4, size/2-2, size/2-2);
        p.drawRect(size/2+2, 4, size/2-6, size/2-2);
        p.drawRect(4, size/2+2, size/2-2, size/2-6);
        p.drawRect(size/2+2, size/2+2, size/2-6, size/2-6);
    } else if (name == "export") {
        p.drawRect(4, 8, size-12, size-16);
        p.drawLine(size-10, size/2, size-4, size/2);
        p.drawLine(size-8, size/2-4, size-4, size/2);
        p.drawLine(size-8, size/2+4, size-4, size/2);
    } else if (name == "help") {
        p.drawEllipse(4, 4, size-8, size-8);
        p.setFont(QFont("Arial", 16, QFont::Bold));
        p.drawText(pix.rect(), Qt::AlignCenter, "?");
    } else if (name == "about") {
        p.drawEllipse(4, 4, size-8, size-8);
        p.setFont(QFont("Arial", 16, QFont::Bold));
        p.drawText(pix.rect(), Qt::AlignCenter, "i");
    } else {
        p.drawRect(4, 4, size-8, size-8);
    }
    
    p.end();
    return QIcon(pix);
}

} // namespace Zhifen
