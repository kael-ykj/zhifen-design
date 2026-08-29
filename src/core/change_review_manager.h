#ifndef CHANGE_REVIEW_MANAGER_H
#define CHANGE_REVIEW_MANAGER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QMap>

namespace Zhifen {

// 变更类型
enum ChangeType {
    Change_Add = 0,      // 添加
    Change_Delete = 1,    // 删除
    Change_Modify = 2,    // 修改
    Change_Move = 3       // 移动
};

// 变更记录
struct ChangeRecord {
    int id;
    ChangeType type;
    QString objectName;    // 对象名称
    QString objectType;    // 对象类型（器件/馈线/文字等）
    QDateTime time;
    QString operatorName;
    QString description;   // 详细描述
    qreal oldValue = 0;    // 旧值（如位置/长度）
    qreal newValue = 0;    // 新值
};

// 校审级别
enum ReviewLevel {
    Review_Design = 0,     // 设计
    Review_Check = 1,      // 校对
    Review_Audit = 2       // 审核
};

// 校审状态
enum ReviewStatus {
    ReviewStatus_Pending = 0,   // 待校审
    ReviewStatus_Passed = 1,    // 通过
    ReviewStatus_Rejected = 2   // 驳回
};

// 校审记录
struct ReviewRecord {
    ReviewLevel level;
    ReviewStatus status;
    QString reviewer;
    QDateTime time;
    QString opinion;     // 校审意见
    QString signature;   // 签名
};

// 变更与校审管理器
class ChangeReviewManager
{
public:
    static ChangeReviewManager& instance();
    ~ChangeReviewManager();

    // 变更记录
    int addChange(ChangeType type, const QString &objectName, const QString &objectType,
                   const QString &description = "", qreal oldValue = 0, qreal newValue = 0);
    QList<ChangeRecord> changes() const { return m_changes; }
    QList<ChangeRecord> changesByType(ChangeType type) const;
    QList<ChangeRecord> changesByTime(const QDateTime &from, const QDateTime &to) const;
    int changeCount() const { return m_changes.size(); }
    QString changeReport() const;
    void clearChanges();

    // 设计校审
    void initReview();  // 初始化三级校审
    bool submitReview(ReviewLevel level, const QString &reviewer, const QString &opinion,
                      ReviewStatus status, const QString &signature = "");
    ReviewRecord reviewRecord(ReviewLevel level) const;
    QList<ReviewRecord> allReviews() const;
    ReviewStatus overallStatus() const;
    bool isAllPassed() const;
    void clearReview();

    // 保存/加载
    bool saveToFile(const QString &filePath);
    bool loadFromFile(const QString &filePath);

private:
    ChangeReviewManager();
    QList<ChangeRecord> m_changes;
    QMap<int, ReviewRecord> m_reviews;
    int m_nextChangeId = 1;
};

} // namespace Zhifen

#endif // CHANGE_REVIEW_MANAGER_H
