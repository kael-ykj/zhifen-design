#include "performance_manager.h"
#include <QGraphicsView>
#include <QPainter>
#include <QPixmap>
#include <QThread>
#include <QDateTime>
#include <QtMath>

namespace Zhifen {

PerformanceManager::PerformanceManager() {
    m_frameTimer.start();
}

PerformanceManager::~PerformanceManager() {}

PerformanceManager& PerformanceManager::instance() {
    static PerformanceManager inst;
    return inst;
}

bool PerformanceManager::isItemVisible(QGraphicsItem *item, const QRectF &viewportRect) const {
    if (!item || !m_settings.viewportCulling) return true;
    QRectF itemRect = item->sceneBoundingRect();
    return viewportRect.intersects(itemRect);
}

QList<QGraphicsItem*> PerformanceManager::visibleItems(QGraphicsScene *scene, const QRectF &viewportRect) const {
    QList<QGraphicsItem*> result;
    if (!scene) return result;

    for (QGraphicsItem *item : scene->items()) {
        if (isItemVisible(item, viewportRect)) {
            result.append(item);
        }
    }
    return result;
}

bool PerformanceManager::shouldDrawDetails(qreal scale) const {
    if (!m_settings.lodEnabled) return true;
    return scale > m_settings.lodThreshold * 5;
}

bool PerformanceManager::shouldDrawText(qreal scale) const {
    if (!m_settings.lodEnabled) return true;
    return scale > m_settings.lodThreshold * 10;
}

int PerformanceManager::lodLevel(qreal scale) const {
    if (!m_settings.lodEnabled) return 2;
    if (scale < m_settings.lodThreshold) return 0;
    if (scale < m_settings.lodThreshold * 5) return 1;
    return 2;
}

void PerformanceManager::enableItemCache(QGraphicsItem *item) {
    if (!item || !m_settings.itemCaching) return;
    item->setCacheMode(QGraphicsItem::ItemCoordinateCache);
}

void PerformanceManager::disableItemCache(QGraphicsItem *item) {
    if (!item) return;
    item->setCacheMode(QGraphicsItem::NoCache);
}

void PerformanceManager::updateItemCache(QGraphicsItem *item) {
    if (!item) return;
    item->update();
}

void PerformanceManager::frameStarted() {
    m_frameTimer.restart();
}

void PerformanceManager::frameEnded() {
    qreal elapsed = m_frameTimer.elapsed();
    m_frameTimes.append(elapsed);
    if (m_frameTimes.size() > 60) {
        m_frameTimes.removeFirst();
    }
    m_frameCount++;
    m_stats.frameTime = elapsed;
    m_stats.fps = calculateFps();
}

qreal PerformanceManager::calculateFps() const {
    if (m_frameTimes.isEmpty()) return 0;
    qreal total = 0;
    for (qreal t : m_frameTimes) {
        total += t;
    }
    qreal avg = total / m_frameTimes.size();
    return avg > 0 ? 1000.0 / avg : 0;
}

void PerformanceManager::updateStats(QGraphicsScene *scene, const QRectF &viewportRect) {
    if (!scene) return;

    m_stats.totalItems = scene->items().size();
    m_stats.visibleItems = 0;
    m_stats.renderedItems = 0;

    for (QGraphicsItem *item : scene->items()) {
        if (isItemVisible(item, viewportRect)) {
            m_stats.visibleItems++;
            if (m_stats.renderedItems < m_settings.maxItemsPerFrame) {
                m_stats.renderedItems++;
            }
        }
    }

    m_stats.memoryUsage = estimateMemoryUsage(scene);
}

QString PerformanceManager::statsReport() const {
    QString report;
    report += "=== 性能监控报告 ===\n";
    report += QString("总图元数: %1\n").arg(m_stats.totalItems);
    report += QString("可见图元数: %1\n").arg(m_stats.visibleItems);
    report += QString("实际绘制数: %1\n").arg(m_stats.renderedItems);
    report += QString("帧率: %1 FPS\n").arg(m_stats.fps, 0, 'f', 1);
    report += QString("帧时间: %1 ms\n").arg(m_stats.frameTime, 0, 'f', 1);
    report += QString("内存占用: %1 MB\n").arg(m_stats.memoryUsage, 0, 'f', 1);
    if (m_stats.isLoading) {
        report += QString("加载进度: %1%\n").arg(m_stats.loadProgress, 0, 'f', 1);
    }
    report += QString("视口裁剪: %1\n").arg(m_settings.viewportCulling ? "开启" : "关闭");
    report += QString("LOD: %1\n").arg(m_settings.lodEnabled ? "开启" : "关闭");
    report += QString("图元缓存: %1\n").arg(m_settings.itemCaching ? "开启" : "关闭");
    return report;
}

void PerformanceManager::markItemDirty(const QString &itemId, const QString &changeType) {
    if (m_dirtyMap.contains(itemId)) return;
    DirtyItemRecord record;
    record.itemId = itemId;
    record.changeType = changeType;
    record.changedTime = QDateTime::currentDateTime();
    m_dirtyItems.append(record);
    m_dirtyMap[itemId] = true;
}

void PerformanceManager::markItemClean(const QString &itemId) {
    m_dirtyMap.remove(itemId);
    for (int i = 0; i < m_dirtyItems.size(); i++) {
        if (m_dirtyItems[i].itemId == itemId) {
            m_dirtyItems.removeAt(i);
            break;
        }
    }
}

bool PerformanceManager::isItemDirty(const QString &itemId) const {
    return m_dirtyMap.value(itemId, false);
}

void PerformanceManager::clearDirtyItems() {
    m_dirtyItems.clear();
    m_dirtyMap.clear();
}

void PerformanceManager::startLoading(int totalItems) {
    m_stats.isLoading = true;
    m_stats.loadProgress = 0;
    m_stats.status = QString("正在加载 %1 个图元...").arg(totalItems);
}

void PerformanceManager::updateLoadProgress(int loadedItems) {
    if (loadedItems > 0) {
        m_stats.loadProgress = qMin(100.0, loadedItems * 100.0 / 1.0);
    }
}

void PerformanceManager::finishLoading() {
    m_stats.isLoading = false;
    m_stats.loadProgress = 100;
    m_stats.status = "加载完成";
}

qreal PerformanceManager::estimateMemoryUsage(QGraphicsScene *scene) const {
    if (!scene) return 0;
    // 每个图元约占1KB（简化估算）
    return scene->items().size() * 0.001; // MB
}

void PerformanceManager::optimizeMemory(QGraphicsScene *scene) {
    if (!scene || !m_settings.memoryOptimization) return;

    // 启用所有复杂图元的缓存
    for (QGraphicsItem *item : scene->items()) {
        if (item->type() == QGraphicsItem::UserType + 1) { // DeviceItem
            enableItemCache(item);
        }
    }
}

QString PerformanceManager::runPerformanceTest(QGraphicsScene *scene, const QRectF &viewportRect) {
    QString report;
    report += "=== 性能测试报告 ===\n\n";

    // 测试1: 图元数量
    updateStats(scene, viewportRect);
    report += QString("1. 图元统计\n");
    report += QString("   总图元: %1\n").arg(m_stats.totalItems);
    report += QString("   可见图元: %1\n").arg(m_stats.visibleItems);
    report += QString("   裁剪率: %1%\n\n").arg(m_stats.totalItems > 0 ?
        (1 - m_stats.visibleItems * 1.0 / m_stats.totalItems) * 100 : 0, 0, 'f', 1);

    // 测试2: 渲染性能
    report += "2. 渲染性能\n";
    QElapsedTimer timer;
    timer.start();
    for (int i = 0; i < 10; i++) {
        QList<QGraphicsItem*> vis = visibleItems(scene, viewportRect);
        Q_UNUSED(vis);
    }
    qreal cullTime = timer.elapsed() / 10.0;
    report += QString("   视口裁剪耗时: %1 ms/次\n").arg(cullTime, 0, 'f', 2);
    report += QString("   估算帧率: %1 FPS\n\n").arg(cullTime > 0 ? 1000.0 / cullTime : 999, 0, 'f', 1);

    // 测试3: 内存使用
    report += "3. 内存使用\n";
    report += QString("   估算内存: %1 MB\n").arg(m_stats.memoryUsage, 0, 'f', 2);
    report += QString("   每图元平均: %1 KB\n\n").arg(m_stats.totalItems > 0 ?
        m_stats.memoryUsage * 1024 / m_stats.totalItems : 0, 0, 'f', 2);

    // 测试4: 性能设置
    report += "4. 性能设置\n";
    report += QString("   视口裁剪: %1\n").arg(m_settings.viewportCulling ? "开启" : "关闭");
    report += QString("   LOD细节层次: %1\n").arg(m_settings.lodEnabled ? "开启" : "关闭");
    report += QString("   图元缓存: %1\n").arg(m_settings.itemCaching ? "开启" : "关闭");
    report += QString("   增量保存: %1\n").arg(m_settings.incrementalSave ? "开启" : "关闭");
    report += QString("   每帧最大绘制: %1 图元\n").arg(m_settings.maxItemsPerFrame);

    // 性能评级
    report += "\n5. 性能评级\n";
    QString rating;
    if (m_stats.fps >= 50 && m_stats.frameTime <= 20) rating = "优秀";
    else if (m_stats.fps >= 30 && m_stats.frameTime <= 33) rating = "良好";
    else if (m_stats.fps >= 20 && m_stats.frameTime <= 50) rating = "一般";
    else rating = "需优化";
    report += QString("   综合评级: %1\n").arg(rating);

    if (m_stats.totalItems > 10000) {
        report += "   建议: 大型图纸，建议开启所有性能优化选项\n";
    }

    return report;
}

} // namespace Zhifen
