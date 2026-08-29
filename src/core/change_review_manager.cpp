#include "change_review_manager.h"
#include <QFile>
#include <QTextStream>

namespace Zhifen {

ChangeReviewManager::ChangeReviewManager() {}
ChangeReviewManager::~ChangeReviewManager() {}

ChangeReviewManager& ChangeReviewManager::instance() {
    static ChangeReviewManager inst;
    return inst;
}

int ChangeReviewManager::addChange(ChangeType type, const QString &objectName,
                                     const QString &objectType, const QString &description,
                                     qreal oldValue, qreal newValue) {
    ChangeRecord rec;
    rec.id = m_nextChangeId++;
    rec.type = type;
    rec.objectName = objectName;
    rec.objectType = objectType;
    rec.time = QDateTime::currentDateTime();
    rec.operatorName = "设计";
    rec.description = description;
    rec.oldValue = oldValue;
    rec.newValue = newValue;
    m_changes.append(rec);
    return rec.id;
}

QList<ChangeRecord> ChangeReviewManager::changesByType(ChangeType type) const {
    QList<ChangeRecord> result;
    for (const ChangeRecord &rec : m_changes) {
        if (rec.type == type) result.append(rec);
    }
    return result;
}

QList<ChangeRecord> ChangeReviewManager::changesByTime(const QDateTime &from, const QDateTime &to) const {
    QList<ChangeRecord> result;
    for (const ChangeRecord &rec : m_changes) {
        if (rec.time >= from && rec.time <= to) result.append(rec);
    }
    return result;
}

QString ChangeReviewManager::changeReport() const {
    int addCount = changesByType(Change_Add).size();
    int delCount = changesByType(Change_Delete).size();
    int modCount = changesByType(Change_Modify).size();
    int moveCount = changesByType(Change_Move).size();

    QString report;
    report += "=== 变更统计报告 ===\n";
    report += QString("总变更数: %1\n").arg(m_changes.size());
    report += QString("添加: %1次\n").arg(addCount);
    report += QString("删除: %1次\n").arg(delCount);
    report += QString("修改: %1次\n").arg(modCount);
    report += QString("移动: %1次\n").arg(moveCount);
    report += "\n=== 最近变更 ===\n";
    int count = qMin(10, m_changes.size());
    for (int i = m_changes.size() - count; i < m_changes.size(); i++) {
        const ChangeRecord &rec = m_changes[i];
        QString typeStr;
        switch (rec.type) {
            case Change_Add: typeStr = "添加"; break;
            case Change_Delete: typeStr = "删除"; break;
            case Change_Modify: typeStr = "修改"; break;
            case Change_Move: typeStr = "移动"; break;
        }
        report += QString("[%1] %2 %3: %4\n")
            .arg(rec.time.toString("MM-dd hh:mm"))
            .arg(typeStr).arg(rec.objectType).arg(rec.description);
    }
    return report;
}

void ChangeReviewManager::clearChanges() {
    m_changes.clear();
    m_nextChangeId = 1;
}

void ChangeReviewManager::initReview() {
    m_reviews.clear();
    for (int i = 0; i <= 2; i++) {
        ReviewRecord rec;
        rec.level = static_cast<ReviewLevel>(i);
        rec.status = ReviewStatus_Pending;
        m_reviews[i] = rec;
    }
}

bool ChangeReviewManager::submitReview(ReviewLevel level, const QString &reviewer,
                                         const QString &opinion, ReviewStatus status,
                                         const QString &signature) {
    if (!m_reviews.contains(level)) return false;
    ReviewRecord &rec = m_reviews[level];
    rec.reviewer = reviewer;
    rec.opinion = opinion;
    rec.status = status;
    rec.time = QDateTime::currentDateTime();
    rec.signature = signature.isEmpty() ? reviewer : signature;
    return true;
}

ReviewRecord ChangeReviewManager::reviewRecord(ReviewLevel level) const {
    return m_reviews.value(level);
}

QList<ReviewRecord> ChangeReviewManager::allReviews() const {
    return m_reviews.values();
}

ReviewStatus ChangeReviewManager::overallStatus() const {
    for (int i = 0; i <= 2; i++) {
        if (m_reviews.value(i).status == ReviewStatus_Rejected)
            return ReviewStatus_Rejected;
    }
    for (int i = 0; i <= 2; i++) {
        if (m_reviews.value(i).status == ReviewStatus_Pending)
            return ReviewStatus_Pending;
    }
    return ReviewStatus_Passed;
}

bool ChangeReviewManager::isAllPassed() const {
    return overallStatus() == ReviewStatus_Passed;
}

void ChangeReviewManager::clearReview() {
    m_reviews.clear();
}

bool ChangeReviewManager::saveToFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "[Changes]\n";
    out << "Count=" << m_changes.size() << "\n";
    for (const ChangeRecord &rec : m_changes) {
        out << QString("\n[Change:%1]\n").arg(rec.id);
        out << "Type=" << rec.type << "\n";
        out << "ObjectName=" << rec.objectName << "\n";
        out << "ObjectType=" << rec.objectType << "\n";
        out << "Time=" << rec.time.toString(Qt::ISODate) << "\n";
        out << "Operator=" << rec.operatorName << "\n";
        out << "Description=" << rec.description << "\n";
    }

    out << "\n[Reviews]\n";
    out << "Count=" << m_reviews.size() << "\n";
    for (int i = 0; i <= 2; i++) {
        if (m_reviews.contains(i)) {
            const ReviewRecord &rec = m_reviews[i];
            out << QString("\n[Review:%1]\n").arg(i);
            out << "Level=" << rec.level << "\n";
            out << "Status=" << rec.status << "\n";
            out << "Reviewer=" << rec.reviewer << "\n";
            out << "Opinion=" << rec.opinion << "\n";
            out << "Signature=" << rec.signature << "\n";
        }
    }
    file.close();
    return true;
}

bool ChangeReviewManager::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QTextStream in(&file);
    clearChanges();
    clearReview();
    ChangeRecord *currentChange = nullptr;
    ReviewRecord *currentReview = nullptr;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("[Change:")) {
            int id = line.mid(8, line.length() - 9).toInt();
            ChangeRecord rec;
            rec.id = id;
            m_changes.append(rec);
            currentChange = &m_changes.last();
            if (id >= m_nextChangeId) m_nextChangeId = id + 1;
        } else if (currentChange && line.startsWith("Type=")) {
            currentChange->type = static_cast<ChangeType>(line.mid(5).toInt());
        } else if (currentChange && line.startsWith("ObjectName=")) {
            currentChange->objectName = line.mid(11);
        } else if (currentChange && line.startsWith("ObjectType=")) {
            currentChange->objectType = line.mid(11);
        } else if (currentChange && line.startsWith("Time=")) {
            currentChange->time = QDateTime::fromString(line.mid(5), Qt::ISODate);
        } else if (currentChange && line.startsWith("Operator=")) {
            currentChange->operatorName = line.mid(9);
        } else if (currentChange && line.startsWith("Description=")) {
            currentChange->description = line.mid(12);
        } else if (line.startsWith("[Review:")) {
            int level = line.mid(8, line.length() - 9).toInt();
            ReviewRecord rec;
            rec.level = static_cast<ReviewLevel>(level);
            m_reviews[level] = rec;
        } else if (line.startsWith("Status=")) {
            int level = -1;
            for (int i = 0; i <= 2; i++) {
                if (m_reviews.contains(i) && m_reviews[i].status == ReviewStatus_Pending
                    && m_reviews[i].reviewer.isEmpty()) {
                    level = i;
                    break;
                }
            }
            if (level >= 0) m_reviews[level].status = static_cast<ReviewStatus>(line.mid(7).toInt());
        } else if (line.startsWith("Reviewer=")) {
            for (int i = 0; i <= 2; i++) {
                if (m_reviews.contains(i) && m_reviews[i].reviewer.isEmpty()) {
                    m_reviews[i].reviewer = line.mid(9);
                    break;
                }
            }
        } else if (line.startsWith("Opinion=")) {
            for (int i = 0; i <= 2; i++) {
                if (m_reviews.contains(i) && m_reviews[i].opinion.isEmpty()) {
                    m_reviews[i].opinion = line.mid(8);
                    break;
                }
            }
        }
    }
    file.close();
    return true;
}

} // namespace Zhifen
