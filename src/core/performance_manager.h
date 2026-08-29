#ifndef PERFORMANCE_MANAGER_H
#define PERFORMANCE_MANAGER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QRectF>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QElapsedTimer>

namespace Zhifen {

// 性能设置
struct PerformanceSettings {
    bool viewportCulling = true;      // 视口裁剪
    bool lodEnabled = true;            // LOD细节层次
    bool itemCaching = true;           // 图元缓存
    bool incrementalSave = true;       // 增量保存
    bool backgroundLoading = true;     // 后台加载
    bool memoryOptimization = true;    // 内存优化
    qreal lodThreshold = 0.02;         // LOD阈值（缩放比例）
    int cacheSize = 256;               // 缓存大小MB
    int maxItemsPerFrame = 5000;       // 每帧最大绘制图元数
};

// 性能统计
struct PerformanceStats {
    int totalItems = 0;                // 总图元数
    int visibleItems = 0;              // 可见图元数
    int renderedItems = 0;             // 实际绘制图元数
    qreal fps = 0;                     // 帧率
    qreal frameTime = 0;               // 帧时间ms
    qreal memoryUsage = 0;             // 内存占用MB
    qreal loadProgress = 0;            // 加载进度0-100
    qreal saveProgress = 0;            // 保存进度0-100
    bool isLoading = false;            // 是否正在加载
    bool isSaving = false;             // 是否正在保存
    QString status;                    // 状态描述
};

// 脏图元记录（用于增量保存）
struct DirtyItemRecord {
    QString itemId;
    QPointF oldPos;
    QPointF newPos;
    QDateTime changedTime;
    QString changeType; // add/delete/modify
};

// 性能管理器
class PerformanceManager
{
public:
    static PerformanceManager& instance();
    ~PerformanceManager();

    // 性能设置
    void setSettings(const PerformanceSettings &settings) { m_settings = settings; }
    PerformanceSettings settings() const { return m_settings; }
    void setViewportCulling(bool enabled) { m_settings.viewportCulling = enabled; }
    void setLODEnabled(bool enabled) { m_settings.lodEnabled = enabled; }
    void setItemCaching(bool enabled) { m_settings.itemCaching = enabled; }

    // 视口裁剪
    bool isItemVisible(QGraphicsItem *item, const QRectF &viewportRect) const;
    QList<QGraphicsItem*> visibleItems(QGraphicsScene *scene, const QRectF &viewportRect) const;

    // LOD判断
    bool shouldDrawDetails(qreal scale) const;
    bool shouldDrawText(qreal scale) const;
    int lodLevel(qreal scale) const; // 0=最简,1=简单,2=完整

    // 图元缓存
    void enableItemCache(QGraphicsItem *item);
    void disableItemCache(QGraphicsItem *item);
    void updateItemCache(QGraphicsItem *item);

    // 性能监控
    void frameStarted();
    void frameEnded();
    PerformanceStats stats() const { return m_stats; }
    void updateStats(QGraphicsScene *scene, const QRectF &viewportRect);
    QString statsReport() const;

    // 增量保存
    void markItemDirty(const QString &itemId, const QString &changeType);
    void markItemClean(const QString &itemId);
    bool isItemDirty(const QString &itemId) const;
    QList<DirtyItemRecord> dirtyItems() const { return m_dirtyItems; }
    void clearDirtyItems();
    bool hasDirtyItems() const { return !m_dirtyItems.isEmpty(); }

    // 大文件加载
    void startLoading(int totalItems);
    void updateLoadProgress(int loadedItems);
    void finishLoading();
    bool isLoading() const { return m_stats.isLoading; }

    // 内存优化
    qreal estimateMemoryUsage(QGraphicsScene *scene) const;
    void optimizeMemory(QGraphicsScene *scene);

    // 性能测试
    QString runPerformanceTest(QGraphicsScene *scene, const QRectF &viewportRect);

private:
    PerformanceManager();
    PerformanceSettings m_settings;
    PerformanceStats m_stats;
    QList<DirtyItemRecord> m_dirtyItems;
    QMap<QString, bool> m_dirtyMap;

    // 帧率统计
    QElapsedTimer m_frameTimer;
    QList<qreal> m_frameTimes;
    int m_frameCount = 0;

    qreal calculateFps() const;
};

} // namespace Zhifen

#endif // PERFORMANCE_MANAGER_H
