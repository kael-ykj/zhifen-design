#ifndef VERSION_MANAGER_H
#define VERSION_MANAGER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QMap>
#include <QGraphicsScene>

namespace Zhifen {

// 版本快照
struct VersionSnapshot {
    int versionNo;           // 版本号
    QString name;            // 版本名称
    QString remark;          // 备注
    QDateTime created;       // 创建时间
    QString operatorName;    // 操作人
    int entityCount;         // 图元数量
    int deviceCount;         // 器件数量
    qreal feederLength;      // 馈线总长度
    QString sceneData;       // 场景序列化数据(JSON)
    bool isCurrent = false;  // 是否当前版本
};

// 版本管理器
class VersionManager
{
public:
    static VersionManager& instance();
    ~VersionManager();

    // 版本管理
    int saveVersion(QGraphicsScene *scene, const QString &name, const QString &remark = "");
    bool rollbackToVersion(int versionNo, QGraphicsScene *scene);
    bool deleteVersion(int versionNo);
    VersionSnapshot* version(int versionNo);
    QList<VersionSnapshot*> allVersions() const;
    int versionCount() const { return m_versions.size(); }
    int currentVersionNo() const { return m_currentVersion; }
    void clear();

    // 版本对比
    QString compareVersions(int v1, int v2) const;

    // 保存/加载
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

private:
    VersionManager();
    QMap<int, VersionSnapshot*> m_versions;
    int m_currentVersion = 0;
    int m_nextVersionNo = 1;

    // 序列化场景
    QString serializeScene(QGraphicsScene *scene);
    // 反序列化场景
    bool deserializeScene(QGraphicsScene *scene, const QString &data);
    // 统计场景
    void analyzeScene(QGraphicsScene *scene, int &entityCount, int &deviceCount, qreal &feederLength);
};

} // namespace Zhifen

#endif // VERSION_MANAGER_H
