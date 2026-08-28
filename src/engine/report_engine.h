#ifndef REPORT_ENGINE_H
#define REPORT_ENGINE_H

#include <QString>
#include <QList>
#include <QMap>
#include <QRectF>
#include <QGraphicsScene>
#include <QPainter>
#include <QPrinter>

namespace Zhifen {

// 图纸尺寸
enum PaperSize {
    Paper_A4 = 0,  // 210x297mm
    Paper_A3 = 1   // 297x420mm
};

// 运营商模板
enum OperatorTemplate {
    Op_ChinaMobile = 0,  // 中国移动
    Op_ChinaUnicom = 1,  // 中国联通
    Op_ChinaTelecom = 2  // 中国电信
};

// BOM条目
struct BomItem {
    QString category;    // 类别：天线/功分器/耦合器/馈线/信源
    QString name;        // 名称
    QString model;       // 型号
    QString unit;        // 单位
    int quantity = 0;    // 数量
};

// 图框信息
struct TitleBlockInfo {
    QString projectName;    // 项目名称
    QString drawingName;    // 图纸名称
    QString drawingNo;      // 图号
    QString designer;       // 设计
    QString reviewer;       // 审核
    QString date;           // 日期
    QString operatorName;   // 运营商
};

// 报表引擎
class ReportEngine
{
public:
    ReportEngine();
    ~ReportEngine();

    // 生成BOM表
    QList<BomItem> generateBom(QGraphicsScene *scene);

    // 绘制图框
    void drawTitleBlock(QPainter *painter, const QRectF &pageRect, const TitleBlockInfo &info, PaperSize paper);

    // 绘制BOM表
    void drawBomTable(QPainter *painter, const QPointF &pos, const QList<BomItem> &bom, qreal maxWidth);

    // 绘制链路预算报表
    void drawLinkReport(QPainter *painter, const QPointF &pos, const QString &reportText, qreal maxWidth);

    // 导出PDF（正式模式：图纸+图框+BOM+链路报告）
    bool exportPdfFormal(QGraphicsScene *scene, const QString &filePath,
                          const TitleBlockInfo &info, PaperSize paper,
                          const QString &linkReportText);

    // 导出PDF（草图模式：仅几何图纸）
    bool exportPdfSketch(QGraphicsScene *scene, const QString &filePath, PaperSize paper);

    // 获取运营商名称
    static QString operatorName(OperatorTemplate op);

private:
    // 绘制表格通用方法
    void drawTable(QPainter *painter, const QPointF &pos,
                    const QStringList &headers, const QList<QStringList> &rows,
                    const QList<qreal> &colWidths);

    // mm转像素（96dpi）
    static qreal mmToPx(qreal mm) { return mm * 3.78; }
};

} // namespace Zhifen

#endif // REPORT_ENGINE_H
