#include "system_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>
#include <algorithm>
#include <cmath>

SystemDiagramView::SystemDiagramView(const zf::SystemDiagram& diagram, QWidget *parent)
    : QWidget(parent), m_diagram(diagram)
{
    double maxX = 200, maxY = 200;
    for (const auto& node : m_diagram.nodes) {
        maxX = std::max(maxX, node.layoutPos.x + 150);
        maxY = std::max(maxY, node.layoutPos.y + 80);
    }
    setFixedSize((int)(maxX * m_scale + 40), (int)(maxY * m_scale + 40));
}

QSize SystemDiagramView::sizeHint() const
{
    return size();
}

QColor SystemDiagramView::nodeColor(zf::NodeType type) const
{
    switch (type) {
        case zf::NodeType::SOURCE: return QColor(200, 50, 50);
        case zf::NodeType::SPLITTER: return QColor(200, 150, 50);
        case zf::NodeType::COUPLER: return QColor(150, 50, 200);
        case zf::NodeType::ANTENNA: return QColor(50, 100, 200);
        case zf::NodeType::LOAD: return QColor(100, 100, 100);
        default: return QColor(128, 128, 128);
    }
}

QString SystemDiagramView::nodeLabel(zf::NodeType type) const
{
    switch (type) {
        case zf::NodeType::SOURCE: return "信源";
        case zf::NodeType::SPLITTER: return "功分";
        case zf::NodeType::COUPLER: return "耦合";
        case zf::NodeType::ANTENNA: return "天线";
        case zf::NodeType::LOAD: return "负载";
        default: return "器件";
    }
}

void SystemDiagramView::drawDiagram(QPainter& painter, const QRect& rect)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect, QColor(250, 250, 250));

    // 计算缩放比例，使内容适配rect
    double contentW = width() - 40;
    double contentH = height() - 40;
    double scaleX = (double)rect.width() / contentW;
    double scaleY = (double)rect.height() / contentH;
    double scale = std::min(scaleX, scaleY);

    double ox = rect.x() + (rect.width() - contentW * scale) / 2 + 20 * scale;
    double oy = rect.y() + (rect.height() - contentH * scale) / 2 + 20 * scale;

    // 绘制连接线
    painter.setPen(QPen(QColor(80, 80, 80), 2 * scale));
    for (const auto& link : m_diagram.links) {
        const zf::SystemNode* from = nullptr;
        const zf::SystemNode* to = nullptr;
        for (const auto& n : m_diagram.nodes) {
            if (n.nodeId == link.fromNodeId) from = &n;
            if (n.nodeId == link.toNodeId) to = &n;
        }
        if (from && to) {
            QPointF p1(ox + from->layoutPos.x * m_scale * scale, oy + from->layoutPos.y * m_scale * scale);
            QPointF p2(ox + to->layoutPos.x * m_scale * scale, oy + to->layoutPos.y * m_scale * scale);
            painter.drawLine(p1, p2);
            if (link.loss_dB > 0) {
                QPointF mid = (p1 + p2) / 2;
                painter.setPen(QColor(180, 0, 0));
                painter.setFont(QFont("Arial", 8 * scale));
                painter.drawText(mid + QPointF(2 * scale, -2 * scale), QString("%1dB").arg(link.loss_dB, 0, 'f', 1));
                painter.setPen(QPen(QColor(80, 80, 80), 2 * scale));
            }
        }
    }

    // 绘制节点
    for (const auto& node : m_diagram.nodes) {
        double x = ox + node.layoutPos.x * m_scale * scale;
        double y = oy + node.layoutPos.y * m_scale * scale;
        double w = 100 * scale, h = 40 * scale;

        painter.setBrush(nodeColor(node.type));
        painter.setPen(QPen(Qt::black, 1 * scale));
        painter.drawRoundedRect(QRectF(x - w/2, y - h/2, w, h), 6 * scale, 6 * scale);

        painter.setPen(Qt::white);
        painter.setFont(QFont("Arial", 9 * scale, QFont::Bold));
        painter.drawText(QRectF(x - w/2, y - h/2, w, h), Qt::AlignCenter,
            nodeLabel(node.type) + "\n" + QString::fromStdString(node.deviceInstanceId));
    }

    // 标题
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 11 * scale, QFont::Bold));
    painter.drawText(rect.x() + 10 * scale, rect.y() + 15 * scale,
        QString("系统图: %1  节点: %2  连接: %3")
        .arg(QString::fromStdString(m_diagram.diagramId))
        .arg(m_diagram.nodes.size())
        .arg(m_diagram.links.size()));
}

void SystemDiagramView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    drawDiagram(painter, rect());
}

QPixmap SystemDiagramView::exportToImage(int margin)
{
    int w = width() + margin * 2;
    int h = height() + margin * 2;
    QPixmap pixmap(w, h);
    pixmap.fill(QColor(255, 255, 255));
    QPainter painter(&pixmap);
    drawDiagram(painter, QRect(margin, margin, width(), height()));
    painter.end();
    return pixmap;
}

SystemDiagramDialog::SystemDiagramDialog(const zf::SystemDiagram& diagram, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("系统图 - 智分Design V3.1");
    resize(900, 600);

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* title = new QLabel(QString("系统图视图  |  节点: %1  连接: %2")
        .arg(diagram.nodes.size()).arg(diagram.links.size()), this);
    title->setStyleSheet("font-weight: bold; font-size: 13px; padding: 4px;");
    layout->addWidget(title);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    m_view = new SystemDiagramView(diagram, this);
    scroll->setWidget(m_view);
    layout->addWidget(scroll, 1);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* exportBtn = new QPushButton("导出图片", this);
    connect(exportBtn, &QPushButton::clicked, this, &SystemDiagramDialog::onExportImage);
    btnLayout->addWidget(exportBtn);

    QPushButton* printBtn = new QPushButton("打印", this);
    connect(printBtn, &QPushButton::clicked, this, &SystemDiagramDialog::onPrint);
    btnLayout->addWidget(printBtn);

    btnLayout->addStretch();
    QPushButton* closeBtn = new QPushButton("关闭", this);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
}

void SystemDiagramDialog::onExportImage()
{
    QString path = QFileDialog::getSaveFileName(this, "导出系统图", "系统图.png",
        "PNG Images (*.png);;JPEG Images (*.jpg)");
    if (path.isEmpty()) return;

    QPixmap pixmap = m_view->exportToImage();
    if (pixmap.save(path)) {
        QMessageBox::information(this, "导出成功", "系统图已导出:\n" + path);
    } else {
        QMessageBox::warning(this, "导出失败", "系统图导出失败");
    }
}

void SystemDiagramDialog::onPrint()
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle("打印系统图");
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QRect pageRect = printer.pageRect();
        m_view->drawDiagram(painter, pageRect);
        painter.end();
    }
}
