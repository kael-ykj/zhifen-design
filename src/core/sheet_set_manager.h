#ifndef SHEET_SET_MANAGER_H
#define SHEET_SET_MANAGER_H

#include <QString>
#include <QList>
#include <QRectF>
#include <QGraphicsScene>
#include <QPrinter>

namespace Zhifen {

// 图纸信息
struct SheetInfo {
    QString name;           // 图纸名称（如"一层平面布置图"）
    QString sheetNo;        // 图号（如"ZF-2026-001"）
    QString scale = "1:100"; // 比例
    QString projectName;    // 项目名称
    QString designer;       // 设计
    QString reviewer;       // 审核
    QString date;           // 日期
    QString operatorName;   // 运营商
    QRectF drawingArea;     // 图纸区域
    int order = 0;          // 排序
    bool selected = true;   // 是否选中（用于批量打印）
};

// 图号配置
struct SheetNumberConfig {
    QString prefix = "ZF";  // 前缀
    int startNumber = 1;    // 起始编号
    int step = 1;           // 步长
    int digits = 3;         // 位数
    QString year = "2026";  // 年份
    QString separator = "-"; // 分隔符
};

// 打印配置
struct PrintConfig {
    QPrinter::PaperSize paperSize = QPrinter::A3;
    QPrinter::Orientation orientation = QPrinter::Landscape;
    qreal scale = 1.0;      // 打印比例
    bool fitToPage = true;  // 适应页面
    bool printTitleBlock = true; // 打印图签
    bool monochrome = false; // 黑白打印
};

// 图纸集管理器
class SheetSetManager
{
public:
    SheetSetManager();
    ~SheetSetManager();

    // 单例
    static SheetSetManager& instance();

    // 图纸集管理
    int addSheet(const SheetInfo &sheet);
    bool removeSheet(int index);
    bool moveSheet(int from, int to);
    SheetInfo sheet(int index) const;
    bool updateSheet(int index, const SheetInfo &sheet);
    QList<SheetInfo> allSheets() const { return m_sheets; }
    int sheetCount() const { return m_sheets.size(); }
    void clear();

    // 自动编号
    void autoNumber(const SheetNumberConfig &config);
    QString generateSheetNo(const SheetNumberConfig &config, int index) const;

    // 图号配置
    void setNumberConfig(const SheetNumberConfig &config) { m_numberConfig = config; }
    SheetNumberConfig numberConfig() const { return m_numberConfig; }

    // 打印配置
    void setPrintConfig(const PrintConfig &config) { m_printConfig = config; }
    PrintConfig printConfig() const { return m_printConfig; }

    // 批量打印
    bool printAll(QGraphicsScene *scene, QWidget *parent = nullptr);
    bool printSelected(QGraphicsScene *scene, const QList<int> &indices, QWidget *parent = nullptr);

    // 批量PDF导出
    bool exportAllToPdf(QGraphicsScene *scene, const QString &outputPath, bool merge = true);
    bool exportSelectedToPdf(QGraphicsScene *scene, const QList<int> &indices,
                              const QString &outputPath, bool merge = true);

    // 生成预览
    QPixmap generatePreview(QGraphicsScene *scene, int index, const QSize &size);

    // 保存/加载图纸集
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

private:
    QList<SheetInfo> m_sheets;
    SheetNumberConfig m_numberConfig;
    PrintConfig m_printConfig;

    // 打印单张图纸
    bool printSheet(QGraphicsScene *scene, const SheetInfo &sheet, QPrinter *printer);
    // 导出单张图纸到PDF
    bool exportSheetToPdf(QGraphicsScene *scene, const SheetInfo &sheet, const QString &filePath);
    // 绘制图签
    void drawTitleBlock(QPainter *painter, const QRectF &pageRect, const SheetInfo &sheet);
};

} // namespace Zhifen

#endif // SHEET_SET_MANAGER_H
