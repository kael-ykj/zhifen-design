#include "layout_manager.h"
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QPainter>

namespace Zhifen {

LayoutManager::LayoutManager() {}
LayoutManager::~LayoutManager() {
    for (Layout *l : m_layouts) {
        if (l->scene) delete l->scene;
        delete l;
    }
    m_layouts.clear();
}

LayoutManager& LayoutManager::instance() {
    static LayoutManager inst;
    return inst;
}

QRectF LayoutManager::paperRect(const QString &paperSize, bool landscape) const {
    qreal w = 420, h = 297; // A3 default
    if (paperSize == "A0") { w = 1189; h = 841; }
    else if (paperSize == "A1") { w = 841; h = 594; }
    else if (paperSize == "A2") { w = 594; h = 420; }
    else if (paperSize == "A3") { w = 420; h = 297; }
    else if (paperSize == "A4") { w = 297; h = 210; }
    return landscape ? QRectF(0, 0, w, h) : QRectF(0, 0, h, w);
}

QString LayoutManager::addLayout(const QString &name, const QString &paperSize) {
    if (name.isEmpty()) return QString();
    QString layoutName = name;
    int suffix = 1;
    while (m_layouts.contains(layoutName)) {
        layoutName = QString("%1_%2").arg(name).arg(suffix++);
    }

    Layout *layout = new Layout();
    layout->name = layoutName;
    layout->paperSize = paperSize;
    QRectF rect = paperRect(paperSize, true);
    layout->paperWidth = rect.width();
    layout->paperHeight = rect.height();
    layout->landscape = true;
    layout->scene = new QGraphicsScene();
    layout->scene->setSceneRect(0, 0, layout->paperWidth, layout->paperHeight);

    // 添加纸张边框
    QGraphicsRectItem *paper = layout->scene->addRect(
        QRectF(0, 0, layout->paperWidth, layout->paperHeight),
        QPen(QColor(0, 0, 0), 0.5), QBrush(QColor(255, 255, 255)));
    paper->setZValue(-100);

    // 添加默认视口
    Viewport vp;
    vp.name = "主视口";
    vp.viewRect = QRectF(10, 10, layout->paperWidth - 20, layout->paperHeight - 70);
    vp.modelRect = QRectF(-100, -100, 200, 200);
    vp.scale = 1.0;
    layout->viewports.append(vp);

    // 绘制视口边框
    QGraphicsRectItem *vpRect = layout->scene->addRect(
        vp.viewRect, QPen(QColor(100, 100, 255), 0.3, Qt::DashLine));
    vpRect->setData(0, "VIEWPORT");

    // 标题栏
    QGraphicsRectItem *titleBlock = layout->scene->addRect(
        QRectF(layout->paperWidth - 180, layout->paperHeight - 60, 180, 60),
        QPen(QColor(0, 0, 0), 0.5));
    QGraphicsTextItem *title = new QGraphicsTextItem("智分Design", titleBlock);
    title->setFont(QFont("SimSun", 8, QFont::Bold));
    title->setPos(5, 5);

    m_layouts[layoutName] = layout;
    return layoutName;
}

bool LayoutManager::removeLayout(const QString &name) {
    if (!m_layouts.contains(name)) return false;
    Layout *layout = m_layouts[name];
    if (layout->scene) delete layout->scene;
    delete layout;
    m_layouts.remove(name);
    if (m_currentLayout == layout) {
        m_currentLayout = nullptr;
        m_inModelSpace = true;
    }
    return true;
}

bool LayoutManager::renameLayout(const QString &oldName, const QString &newName) {
    if (!m_layouts.contains(oldName) || m_layouts.contains(newName)) return false;
    Layout *layout = m_layouts[oldName];
    layout->name = newName;
    m_layouts.remove(oldName);
    m_layouts[newName] = layout;
    return true;
}

Layout* LayoutManager::layout(const QString &name) {
    return m_layouts.value(name, nullptr);
}

void LayoutManager::setCurrentLayout(const QString &name) {
    if (m_layouts.contains(name)) {
        m_currentLayout = m_layouts[name];
        m_inModelSpace = false;
    }
}

void LayoutManager::enterLayout(const QString &name) {
    setCurrentLayout(name);
}

QList<QString> LayoutManager::layoutNames() const {
    return m_layouts.keys();
}

bool LayoutManager::addViewport(const QString &layoutName, const Viewport &vp) {
    Layout *layout = m_layouts.value(layoutName);
    if (!layout) return false;
    layout->viewports.append(vp);
    return true;
}

bool LayoutManager::removeViewport(const QString &layoutName, int index) {
    Layout *layout = m_layouts.value(layoutName);
    if (!layout || index < 0 || index >= layout->viewports.size()) return false;
    layout->viewports.removeAt(index);
    return true;
}

QList<Viewport> LayoutManager::viewports(const QString &layoutName) const {
    Layout *layout = m_layouts.value(layoutName);
    if (!layout) return QList<Viewport>();
    return layout->viewports;
}

void LayoutManager::setPaperSize(const QString &layoutName, const QString &paperSize, bool landscape) {
    Layout *layout = m_layouts.value(layoutName);
    if (!layout) return;
    layout->paperSize = paperSize;
    layout->landscape = landscape;
    QRectF rect = paperRect(paperSize, landscape);
    layout->paperWidth = rect.width();
    layout->paperHeight = rect.height();
    if (layout->scene) {
        layout->scene->setSceneRect(0, 0, layout->paperWidth, layout->paperHeight);
    }
}

void LayoutManager::syncViewToLayout(QGraphicsView *view, const QString &layoutName) {
    Layout *layout = m_layouts.value(layoutName);
    if (!layout || !view) return;
    view->setScene(layout->scene);
    view->fitInView(layout->scene->sceneRect(), Qt::KeepAspectRatio);
}

void LayoutManager::syncViewToModel(QGraphicsView *view) {
    if (!view) return;
    // 视图切换回模型空间由调用方设置scene
}

} // namespace Zhifen
