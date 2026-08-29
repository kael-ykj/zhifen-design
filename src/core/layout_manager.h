#ifndef LAYOUT_MANAGER_H
#define LAYOUT_MANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QRectF>
#include <QGraphicsScene>
#include <QGraphicsView>

namespace Zhifen {

// 视口定义
struct Viewport {
    QString name;
    QRectF viewRect;      // 视口在布局中的位置
    QRectF modelRect;     // 显示的模型空间区域
    qreal scale = 1.0;    // 视口比例
    bool visible = true;
    bool locked = false;
};

// 布局定义
struct Layout {
    QString name;          // 布局名称（如"布局1"、"A3图纸"）
    QString paperSize;     // 纸张大小（A0/A1/A2/A3/A4）
    qreal paperWidth = 420;  // 纸张宽度mm
    qreal paperHeight = 297; // 纸张高度mm
    bool landscape = true; // 横向
    QList<Viewport> viewports; // 视口列表
    QGraphicsScene *scene = nullptr; // 布局场景
};

// 布局管理器
class LayoutManager
{
public:
    static LayoutManager& instance();
    ~LayoutManager();

    // 布局管理
    QString addLayout(const QString &name, const QString &paperSize = "A3");
    bool removeLayout(const QString &name);
    bool renameLayout(const QString &oldName, const QString &newName);
    Layout* layout(const QString &name);
    Layout* currentLayout() { return m_currentLayout; }
    void setCurrentLayout(const QString &name);
    QList<QString> layoutNames() const;
    int layoutCount() const { return m_layouts.size(); }

    // 模型空间
    bool isModelSpace() const { return m_inModelSpace; }
    void enterModelSpace() { m_inModelSpace = true; m_currentLayout = nullptr; }
    void enterLayout(const QString &name);

    // 视口管理
    bool addViewport(const QString &layoutName, const Viewport &vp);
    bool removeViewport(const QString &layoutName, int index);
    QList<Viewport> viewports(const QString &layoutName) const;

    // 纸张大小
    void setPaperSize(const QString &layoutName, const QString &paperSize, bool landscape);
    QRectF paperRect(const QString &paperSize, bool landscape) const;

    // 切换空间时的视图同步
    void syncViewToLayout(QGraphicsView *view, const QString &layoutName);
    void syncViewToModel(QGraphicsView *view);

private:
    LayoutManager();
    QMap<QString, Layout*> m_layouts;
    Layout *m_currentLayout = nullptr;
    bool m_inModelSpace = true;
};

} // namespace Zhifen

#endif // LAYOUT_MANAGER_H
