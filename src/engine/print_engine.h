#ifndef PRINT_ENGINE_H
#define PRINT_ENGINE_H

#include <QObject>
#include <QString>
#include <QRectF>
#include <QGraphicsScene>
#include <QPrinter>
#include <QPainter>
#include "report_engine.h"

namespace Zhifen {

// 打印设置
struct PrintSettings {
    PaperSize paperSize = Paper_A3;
    QPrinter::Orientation orientation = QPrinter::Landscape;
    bool printTitleBlock = true;      // 打印图签
    bool printLegend = true;          // 打印图例
    bool printMaterialTable = false;  // 打印材料表
    bool printBorder = true;          // 打印边框
    double scale = 100;               // 打印比例(%)
    int margin = 10;                  // 边距(mm)
};

// 专业打印引擎
class PrintEngine : public QObject
{
    Q_OBJECT
public:
    explicit PrintEngine(QObject *parent = nullptr);

    // 打印场景（带图签/图例/材料表）
    void printScene(QGraphicsScene *scene, QPrinter *printer, const PrintSettings &settings, const TitleBlockInfo &info);

    // 打印预览渲染
    void renderToPainter(QGraphicsScene *scene, QPainter *painter, const QRectF &targetRect, const PrintSettings &settings, const TitleBlockInfo &info);

    // 批量打印（多个场景）
    void printMultipleScenes(const QList<QGraphicsScene*> &scenes, QPrinter *printer, const PrintSettings &settings, const QList<TitleBlockInfo> &infos);

private:
    // 绘制图签
    void drawTitleBlock(QPainter *painter, const QRectF &rect, const TitleBlockInfo &info);

    // 绘制图例
    void drawLegend(QPainter *painter, const QRectF &rect);

    // 绘制材料表
    void drawMaterialTable(QPainter *painter, const QRectF &rect, QGraphicsScene *scene);

    // 绘制边框
    void drawBorder(QPainter *painter, const QRectF &rect);

    // 获取纸张尺寸(mm)
    QSizeF paperSizeMm(PaperSize size) const;
};

} // namespace Zhifen

#endif // PRINT_ENGINE_H
