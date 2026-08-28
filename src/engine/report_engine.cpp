#include "report_engine.h"
#include "../entities/caditem.h"
#include "../devices/deviceitem.h"
#include "../entities/feederitem.h"
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QPrinter>
#include <QFont>
#include <QFontMetricsF>
#include <QDate>
#include <QtMath>

namespace Zhifen {

ReportEngine::ReportEngine() {}
ReportEngine::~ReportEngine() {}

QString ReportEngine::operatorName(OperatorTemplate op) {
    switch (op) {
    case Op_ChinaMobile: return "中国移动";
    case Op_ChinaUnicom: return "中国联通";
    case Op_ChinaTelecom: return "中国电信";
    }
    return "未知";
}

QList<BomItem> ReportEngine::generateBom(QGraphicsScene *scene) {
    QList<BomItem> bom;
    if (!scene) return bom;

    QMap<QString, BomItem> bomMap;

    for (auto *item : scene->items()) {
        auto *cad = dynamic_cast<CadItem*>(item);
        if (!cad) continue;

        QString etype = cad->entityType();
        QString category;
        QString name = etype;
        QString model = "-";
        QString unit = "个";

        if (auto *dev = dynamic_cast<DeviceItem*>(item)) {
            name = dev->deviceTypeName();
            model = dev->model();
            DeviceType dt = dev->deviceType();
            if (dt >= DevAntennaOmni && dt <= DevAntennaGrid) category = "天线";
            else if (dt >= DevSplitter2 && dt <= DevSplitter4) category = "功分器";
            else if (dt >= DevCoupler5 && dt <= DevCoupler40) category = "耦合器";
            else if (dt == DevCombiner || dt == DevHybrid) category = "合路器";
            else if (dt >= DevSourceRRU && dt <= DevDryAmp) category = "信源";
            else if (dt >= DevpRRU && dt <= DevPOESwitch) category = "数字化室分";
            else if (dt >= DevFeederHalf && dt <= DevNetworkCable) { category = "馈线"; unit = "米"; }
            else if (dt >= DevLeakyCable158 && dt <= DevLeakyCable138) { category = "漏缆"; unit = "米"; }
            else category = "其他";
        } else if (auto *feeder = dynamic_cast<FeederItem*>(item)) {
            category = "馈线";
            name = feeder->feederTypeName();
            unit = "米";
            // 馈线按长度统计
            QString key = category + "_" + name;
            if (bomMap.contains(key)) {
                bomMap[key].quantity += qRound(feeder->length());
            } else {
                BomItem bi;
                bi.category = category;
                bi.name = name;
                bi.model = "-";
                bi.unit = unit;
                bi.quantity = qRound(feeder->length());
                bomMap[key] = bi;
            }
            continue;
        } else {
            category = "其他";
        }

        QString key = category + "_" + name;
        if (bomMap.contains(key)) {
            bomMap[key].quantity++;
        } else {
            BomItem bi;
            bi.category = category;
            bi.name = name;
            bi.model = model;
            bi.unit = unit;
            bi.quantity = 1;
            bomMap[key] = bi;
        }
    }

    // 按类别排序
    QStringList catOrder = {"信源", "数字化室分", "天线", "功分器", "耦合器", "合路器", "馈线", "漏缆", "其他"};
    for (const auto &cat : catOrder) {
        for (auto it = bomMap.begin(); it != bomMap.end(); ++it) {
            if (it.value().category == cat) bom.append(it.value());
        }
    }

    return bom;
}

void ReportEngine::drawTitleBlock(QPainter *painter, const QRectF &pageRect, const TitleBlockInfo &info, PaperSize paper) {
    Q_UNUSED(paper);
    painter->save();

    QPen borderPen(QColor(0, 0, 0), 2);
    painter->setPen(borderPen);
    painter->setBrush(Qt::NoBrush);

    // 外框
    painter->drawRect(pageRect.adjusted(10, 10, -10, -10));

    // 内框
    painter->setPen(QPen(QColor(0, 0, 0), 1));
    painter->drawRect(pageRect.adjusted(25, 20, -15, -15));

    // 标题栏（右下角）
    qreal tbWidth = 180;
    qreal tbHeight = 60;
    QRectF tbRect(pageRect.right() - 15 - tbWidth, pageRect.bottom() - 15 - tbHeight, tbWidth, tbHeight);
    painter->drawRect(tbRect);

    // 标题栏分隔线
    painter->drawLine(tbRect.left(), tbRect.top() + 20, tbRect.right(), tbRect.top() + 20);
    painter->drawLine(tbRect.left(), tbRect.top() + 40, tbRect.right(), tbRect.top() + 40);
    painter->drawLine(tbRect.left() + 60, tbRect.top(), tbRect.left() + 60, tbRect.bottom());
    painter->drawLine(tbRect.left() + 120, tbRect.top() + 20, tbRect.left() + 120, tbRect.bottom());

    QFont titleFont("SimSun", 10, QFont::Bold);
    QFont normalFont("SimSun", 8);
    QFont smallFont("SimSun", 7);

    // 项目名称
    painter->setFont(titleFont);
    painter->drawText(QRectF(tbRect.left() + 2, tbRect.top() + 2, tbWidth - 4, 16),
                      Qt::AlignCenter, info.projectName);

    // 图纸名称
    painter->setFont(normalFont);
    painter->drawText(QRectF(tbRect.left() + 2, tbRect.top() + 22, tbWidth - 4, 16),
                      Qt::AlignCenter, info.drawingName);

    // 设计/审核/日期
    painter->setFont(smallFont);
    painter->drawText(QRectF(tbRect.left() + 2, tbRect.top() + 42, 56, 16), Qt::AlignCenter, "设计: " + info.designer);
    painter->drawText(QRectF(tbRect.left() + 62, tbRect.top() + 42, 56, 16), Qt::AlignCenter, "审核: " + info.reviewer);
    painter->drawText(QRectF(tbRect.left() + 122, tbRect.top() + 42, 56, 16), Qt::AlignCenter, info.date);

    // 图号（标题栏上方）
    painter->drawText(QRectF(tbRect.left(), tbRect.top() - 18, tbWidth, 14),
                      Qt::AlignRight, "图号: " + info.drawingNo);

    // 运营商（左上角）
    painter->setFont(normalFont);
    painter->drawText(QRectF(pageRect.left() + 30, pageRect.top() + 25, 200, 20),
                      Qt::AlignLeft, info.operatorName);

    painter->restore();
}

void ReportEngine::drawTable(QPainter *painter, const QPointF &pos,
                               const QStringList &headers, const QList<QStringList> &rows,
                               const QList<qreal> &colWidths) {
    painter->save();
    QPen pen(QColor(0, 0, 0), 0.8);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    qreal rowHeight = 18;
    qreal x = pos.x();
    qreal y = pos.y();

    // 表头
    QFont headerFont("SimSun", 8, QFont::Bold);
    painter->setFont(headerFont);
    painter->fillRect(QRectF(x, y, colWidths.sum(), rowHeight), QColor(220, 220, 220));
    for (int i = 0; i < headers.size(); i++) {
        painter->drawRect(QRectF(x, y, colWidths[i], rowHeight));
        painter->drawText(QRectF(x + 2, y + 2, colWidths[i] - 4, rowHeight - 4),
                          Qt::AlignCenter, headers[i]);
        x += colWidths[i];
    }
    y += rowHeight;

    // 数据行
    QFont cellFont("SimSun", 7);
    painter->setFont(cellFont);
    for (const auto &row : rows) {
        x = pos.x();
        for (int i = 0; i < row.size() && i < colWidths.size(); i++) {
            painter->drawRect(QRectF(x, y, colWidths[i], rowHeight));
            painter->drawText(QRectF(x + 2, y + 2, colWidths[i] - 4, rowHeight - 4),
                              Qt::AlignCenter, row[i]);
            x += colWidths[i];
        }
        y += rowHeight;
    }

    painter->restore();
}

void ReportEngine::drawBomTable(QPainter *painter, const QPointF &pos, const QList<BomItem> &bom, qreal maxWidth) {
    QStringList headers = {"序号", "类别", "名称", "型号", "单位", "数量"};
    QList<qreal> colWidths = {30, 50, 70, 60, 30, 30};
    QList<QStringList> rows;
    for (int i = 0; i < bom.size(); i++) {
        QStringList row;
        row << QString::number(i + 1) << bom[i].category << bom[i].name
            << bom[i].model << bom[i].unit << QString::number(bom[i].quantity);
        rows.append(row);
    }

    // 标题
    painter->save();
    QFont titleFont("SimSun", 10, QFont::Bold);
    painter->setFont(titleFont);
    painter->drawText(QRectF(pos.x(), pos.y() - 18, maxWidth, 16), Qt::AlignLeft, "材料表(BOM)");
    painter->restore();

    drawTable(painter, pos, headers, rows, colWidths);
}

void ReportEngine::drawLinkReport(QPainter *painter, const QPointF &pos, const QString &reportText, qreal maxWidth) {
    painter->save();

    // 标题
    QFont titleFont("SimSun", 10, QFont::Bold);
    painter->setFont(titleFont);
    painter->drawText(QRectF(pos.x(), pos.y() - 18, maxWidth, 16), Qt::AlignLeft, "链路预算报告");

    // 内容
    QFont contentFont("Consolas", 7);
    painter->setFont(contentFont);
    painter->drawText(QRectF(pos.x(), pos.y(), maxWidth, 400), Qt::AlignLeft | Qt::TextWordWrap, reportText);

    painter->restore();
}

bool ReportEngine::exportPdfFormal(QGraphicsScene *scene, const QString &filePath,
                                     const TitleBlockInfo &info, PaperSize paper,
                                     const QString &linkReportText) {
    if (!scene) return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(paper == Paper_A3 ? QPrinter::A3 : QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);

    QPainter painter(&printer);
    QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);

    // 1. 绘制图框
    drawTitleBlock(&painter, pageRect, info, paper);

    // 2. 绘制图纸（场景内容）
    QRectF drawingArea = pageRect.adjusted(35, 30, -210, -90);
    painter.save();
    painter.setViewport(drawingArea.toRect());
    painter.setWindow(scene->itemsBoundingRect().toRect());
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter);
    painter.restore();

    // 3. 绘制BOM表（右侧）
    QList<BomItem> bom = generateBom(scene);
    QPointF bomPos(pageRect.right() - 200, pageRect.top() + 50);
    drawBomTable(&painter, bomPos, bom, 190);

    // 4. 绘制链路预算报表（BOM表下方）
    if (!linkReportText.isEmpty()) {
        QPointF linkPos(pageRect.right() - 200, bomPos.y() + 200);
        if (linkPos.y() < pageRect.bottom() - 100) {
            drawLinkReport(&painter, linkPos, linkReportText, 190);
        }
    }

    painter.end();
    return true;
}

bool ReportEngine::exportPdfSketch(QGraphicsScene *scene, const QString &filePath, PaperSize paper) {
    if (!scene) return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(paper == Paper_A3 ? QPrinter::A3 : QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);

    QPainter painter(&printer);
    QRectF pageRect = printer.pageRect(QPrinter::DevicePixel);

    // 草图模式：仅绘制几何图纸，不带业务报表
    painter.save();
    painter.setViewport(pageRect.adjusted(20, 20, -20, -20).toRect());
    painter.setWindow(scene->itemsBoundingRect().toRect());
    painter.setRenderHint(QPainter::Antialiasing);
    scene->render(&painter);
    painter.restore();

    painter.end();
    return true;
}

} // namespace Zhifen
