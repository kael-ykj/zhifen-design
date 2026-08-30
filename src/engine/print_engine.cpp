#include "print_engine.h"
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QDateTime>
#include <QtMath>

namespace Zhifen {

PrintEngine::PrintEngine(QObject *parent) : QObject(parent) {}

QSizeF PrintEngine::paperSizeMm(PaperSize size) const
{
    switch (size) {
    case Paper_A4: return QSizeF(210, 297);
    case Paper_A3: return QSizeF(297, 420);
    }
    return QSizeF(297, 420);
}

void PrintEngine::printScene(QGraphicsScene *scene, QPrinter *printer, const PrintSettings &settings, const TitleBlockInfo &info)
{
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    QRectF pageRect = printer->pageRect();
    renderToPainter(scene, &painter, pageRect, settings, info);
}

void PrintEngine::renderToPainter(QGraphicsScene *scene, QPainter *painter, const QRectF &targetRect, const PrintSettings &settings, const TitleBlockInfo &info)
{
    if (!scene || !painter) return;

    painter->save();

    // 计算绘图区域（留出图签和边距）
    QRectF drawRect = targetRect;
    double margin = settings.margin * painter->device()->logicalDpiX() / 25.4; // mm转像素
    drawRect.adjust(margin, margin, -margin, -margin);

    // 图签区域（底部，高度约占15%）
    QRectF titleRect;
    if (settings.printTitleBlock) {
        double titleHeight = drawRect.height() * 0.12;
        titleRect = QRectF(drawRect.left(), drawRect.bottom() - titleHeight, drawRect.width(), titleHeight);
        drawRect.setBottom(titleRect.top() - 5);
    }

    // 图例区域（右侧，宽度约占15%）
    QRectF legendRect;
    if (settings.printLegend) {
        double legendWidth = drawRect.width() * 0.15;
        legendRect = QRectF(drawRect.right() - legendWidth, drawRect.top(), legendWidth, drawRect.height());
        drawRect.setRight(legendRect.left() - 5);
    }

    // 绘制边框
    if (settings.printBorder) {
        drawBorder(painter, targetRect.adjusted(margin/2, margin/2, -margin/2, -margin/2));
    }

    // 渲染场景内容
    QRectF sceneRect = scene->itemsBoundingRect();
    if (!sceneRect.isEmpty()) {
        double scale = qMin(drawRect.width() / sceneRect.width(), drawRect.height() / sceneRect.height());
        scale *= settings.scale / 100.0;

        painter->save();
        painter->translate(drawRect.center());
        painter->scale(scale, scale);
        painter->translate(-sceneRect.center());
        scene->render(painter);
        painter->restore();
    }

    // 绘制图签
    if (settings.printTitleBlock) {
        drawTitleBlock(painter, titleRect, info);
    }

    // 绘制图例
    if (settings.printLegend) {
        drawLegend(painter, legendRect);
    }

    // 绘制材料表
    if (settings.printMaterialTable) {
        QRectF matRect = QRectF(drawRect.left(), drawRect.top(), qMin(drawRect.width()*0.3, 200.0), drawRect.height()*0.4);
        drawMaterialTable(painter, matRect, scene);
    }

    painter->restore();
}

void PrintEngine::drawTitleBlock(QPainter *painter, const QRectF &rect, const TitleBlockInfo &info)
{
    painter->save();
    QPen pen(Qt::black, 1.5);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // 外框
    painter->drawRect(rect);

    // 分隔线
    double colWidth = rect.width() / 6;
    double rowHeight = rect.height() / 3;

    // 横线
    painter->drawLine(QPointF(rect.left(), rect.top() + rowHeight), QPointF(rect.right(), rect.top() + rowHeight));
    painter->drawLine(QPointF(rect.left(), rect.top() + 2*rowHeight), QPointF(rect.right(), rect.top() + 2*rowHeight));

    // 竖线
    for (int i = 1; i < 6; i++) {
        painter->drawLine(QPointF(rect.left() + i*colWidth, rect.top()), QPointF(rect.left() + i*colWidth, rect.bottom()));
    }

    // 文字
    QFont font("Microsoft YaHei", 8);
    painter->setFont(font);

    auto drawText = [&](double x, double y, double w, double h, const QString &label, const QString &value) {
        painter->drawText(QRectF(x, y, w, h*0.4), Qt::AlignCenter, label);
        QFont valFont("Microsoft YaHei", 9, QFont::Bold);
        painter->setFont(valFont);
        painter->drawText(QRectF(x, y + h*0.4, w, h*0.6), Qt::AlignCenter, value);
        painter->setFont(font);
    };

    // 第一行
    drawText(rect.left(), rect.top(), colWidth, rowHeight, "工程名称", info.projectName);
    drawText(rect.left()+colWidth, rect.top(), colWidth*2, rowHeight, "图纸名称", info.drawingName);
    drawText(rect.left()+3*colWidth, rect.top(), colWidth, rowHeight, "图纸编号", info.drawingNumber);
    drawText(rect.left()+4*colWidth, rect.top(), colWidth, rowHeight, "版本", info.version);
    drawText(rect.left()+5*colWidth, rect.top(), colWidth, rowHeight, "日期", info.date.isEmpty() ? QDateTime::currentDateTime().toString("yyyy-MM-dd") : info.date);

    // 第二行
    drawText(rect.left(), rect.top()+rowHeight, colWidth, rowHeight, "设计人", info.designer);
    drawText(rect.left()+colWidth, rect.top()+rowHeight, colWidth, rowHeight, "审核人", info.reviewer);
    drawText(rect.left()+2*colWidth, rect.top()+rowHeight, colWidth, rowHeight, "批准人", info.approver);
    drawText(rect.left()+3*colWidth, rect.top()+rowHeight, colWidth*3, rowHeight, "设计单位", info.company);

    // 第三行（合并为设计说明）
    painter->drawText(QRectF(rect.left()+5, rect.top()+2*rowHeight+2, rect.width()-10, rowHeight-4),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      "设计说明: 本图纸依据YD/T 5015-2015《通信工程制图标准》及相关规范设计，未经许可不得擅自修改。");

    painter->restore();
}

void PrintEngine::drawLegend(QPainter *painter, const QRectF &rect)
{
    painter->save();
    QPen pen(Qt::black, 1);
    painter->setPen(pen);

    // 标题
    QFont titleFont("Microsoft YaHei", 10, QFont::Bold);
    painter->setFont(titleFont);
    painter->drawText(QRectF(rect.left(), rect.top(), rect.width(), 20), Qt::AlignCenter, "图 例");

    // 图例项
    QFont font("Microsoft YaHei", 7);
    painter->setFont(font);

    struct LegendItem { QString name; QColor color; QString shape; };
    QList<LegendItem> items = {
        {"全向吸顶天线", Qt::black, "circle"},
        {"定向壁挂天线", Qt::black, "triangle"},
        {"射灯天线", Qt::black, "diamond"},
        {"二功分器", Qt::black, "splitter"},
        {"耦合器", Qt::black, "coupler"},
        {"合路器", Qt::black, "combiner"},
        {"RRU", Qt::black, "rect"},
        {"BBU", Qt::black, "rect"},
        {"1/2馈线", Qt::black, "line"},
        {"7/8馈线", Qt::darkGray, "line"},
        {"接地", Qt::black, "ground"},
    };

    double y = rect.top() + 25;
    double itemHeight = (rect.height() - 30) / items.size();

    for (const auto &item : items) {
        // 绘制符号
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        double symX = rect.left() + 10;
        double symY = y + itemHeight/2;

        if (item.shape == "circle") {
            painter->drawEllipse(QPointF(symX+6, symY), 6, 6);
            painter->drawLine(QPointF(symX+6, symY-6), QPointF(symX+6, symY+6));
            painter->drawLine(QPointF(symX, symY), QPointF(symX+12, symY));
        } else if (item.shape == "triangle") {
            QPolygonF tri;
            tri << QPointF(symX, symY+6) << QPointF(symX+12, symY+6) << QPointF(symX+6, symY-6);
            painter->drawPolygon(tri);
        } else if (item.shape == "diamond") {
            QPolygonF dia;
            dia << QPointF(symX+6, symY-6) << QPointF(symX+12, symY) << QPointF(symX+6, symY+6) << QPointF(symX, symY);
            painter->drawPolygon(dia);
        } else if (item.shape == "rect") {
            painter->drawRect(QRectF(symX, symY-5, 12, 10));
        } else if (item.shape == "line") {
            painter->drawLine(QPointF(symX, symY), QPointF(symX+12, symY));
        } else {
            painter->drawRect(QRectF(symX, symY-4, 12, 8));
        }

        // 文字
        painter->drawText(QRectF(rect.left()+28, y, rect.width()-30, itemHeight), Qt::AlignVCenter, item.name);
        y += itemHeight;
    }

    painter->restore();
}

void PrintEngine::drawMaterialTable(QPainter *painter, const QRectF &rect, QGraphicsScene *scene)
{
    painter->save();
    QPen pen(Qt::black, 1);
    painter->setPen(pen);

    // 标题
    QFont titleFont("Microsoft YaHei", 9, QFont::Bold);
    painter->setFont(titleFont);
    painter->drawText(QRectF(rect.left(), rect.top(), rect.width(), 20), Qt::AlignCenter, "主要材料表");

    // 统计器件数量
    QMap<QString, int> materialCount;
    const auto items = scene->items();
    for (QGraphicsItem *item : items) {
        if (item->data(0).isValid()) {
            QString name = item->data(0).toString();
            if (!name.isEmpty()) {
                materialCount[name]++;
            }
        }
    }

    // 绘制表格
    QFont font("Microsoft YaHei", 7);
    painter->setFont(font);

    double y = rect.top() + 22;
    double rowHeight = 16;
    double colWidth = rect.width() / 3;

    // 表头
    painter->drawRect(QRectF(rect.left(), y, rect.width(), rowHeight));
    painter->drawLine(QPointF(rect.left()+colWidth, y), QPointF(rect.left()+colWidth, y+rowHeight));
    painter->drawLine(QPointF(rect.left()+2*colWidth, y), QPointF(rect.left()+2*colWidth, y+rowHeight));
    painter->drawText(QRectF(rect.left(), y, colWidth, rowHeight), Qt::AlignCenter, "序号");
    painter->drawText(QRectF(rect.left()+colWidth, y, colWidth, rowHeight), Qt::AlignCenter, "材料名称");
    painter->drawText(QRectF(rect.left()+2*colWidth, y, colWidth, rowHeight), Qt::AlignCenter, "数量");
    y += rowHeight;

    int idx = 1;
    for (auto it = materialCount.begin(); it != materialCount.end() && y < rect.bottom() - rowHeight; ++it) {
        painter->drawRect(QRectF(rect.left(), y, rect.width(), rowHeight));
        painter->drawLine(QPointF(rect.left()+colWidth, y), QPointF(rect.left()+colWidth, y+rowHeight));
        painter->drawLine(QPointF(rect.left()+2*colWidth, y), QPointF(rect.left()+2*colWidth, y+rowHeight));
        painter->drawText(QRectF(rect.left(), y, colWidth, rowHeight), Qt::AlignCenter, QString::number(idx));
        painter->drawText(QRectF(rect.left()+colWidth, y, colWidth, rowHeight), Qt::AlignCenter, it.key());
        painter->drawText(QRectF(rect.left()+2*colWidth, y, colWidth, rowHeight), Qt::AlignCenter, QString::number(it.value()));
        y += rowHeight;
        idx++;
    }

    if (materialCount.isEmpty()) {
        painter->drawText(QRectF(rect.left(), y, rect.width(), rowHeight), Qt::AlignCenter, "（暂无材料统计）");
    }

    painter->restore();
}

void PrintEngine::drawBorder(QPainter *painter, const QRectF &rect)
{
    painter->save();
    QPen pen(Qt::black, 2);
    painter->setPen(pen);
    painter->drawRect(rect);

    // 内框
    pen.setWidth(1);
    painter->setPen(pen);
    painter->drawRect(rect.adjusted(5, 5, -5, -5));
    painter->restore();
}

void PrintEngine::printMultipleScenes(const QList<QGraphicsScene*> &scenes, QPrinter *printer, const PrintSettings &settings, const QList<TitleBlockInfo> &infos)
{
    for (int i = 0; i < scenes.size(); i++) {
        if (i > 0) printer->newPage();
        TitleBlockInfo info = (i < infos.size()) ? infos[i] : TitleBlockInfo();
        printScene(scenes[i], printer, settings, info);
    }
}

} // namespace Zhifen
