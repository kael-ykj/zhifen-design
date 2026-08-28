#ifndef AUDIT_LOGGER_H
#define AUDIT_LOGGER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace Zhifen {

// 审计操作类型
enum AuditAction {
    Audit_DevicePlace = 0,      // 器件放置
    Audit_DeviceDelete,          // 器件删除
    Audit_DeviceMove,            // 器件移动
    Audit_DeviceCopy,            // 器件复制
    Audit_DeviceExplode,         // 器件炸开
    Audit_BatchDelete,           // 批量删除
    Audit_ModeSwitch,            // 模式切换
    Audit_CopyModeSwitch,        // 复制模式切换
    Audit_LinkCalculation,       // 链路预算
    Audit_ProjectSave,           // 项目保存
    Audit_ProjectLoad,           // 项目加载
    Audit_ExportDXF,             // DXF导出
    Audit_ExportDWG,             // DWG导出
    Audit_Import,                // 导入
    Audit_Print,                 // 打印
    Audit_Undo,                  // 撤销
    Audit_Redo,                  // 重做
    Audit_PluginOperation,       // 插件操作
    Audit_Other                  // 其他
};

// 审计日志条目
struct AuditEntry {
    QDateTime timestamp;
    AuditAction action;
    QString operatorName;
    QString details;
    qint64 projectRevision = 0;

    QString actionName() const {
        switch (action) {
        case Audit_DevicePlace: return "器件放置";
        case Audit_DeviceDelete: return "器件删除";
        case Audit_DeviceMove: return "器件移动";
        case Audit_DeviceCopy: return "器件复制";
        case Audit_DeviceExplode: return "器件炸开";
        case Audit_BatchDelete: return "批量删除";
        case Audit_ModeSwitch: return "模式切换";
        case Audit_CopyModeSwitch: return "复制模式切换";
        case Audit_LinkCalculation: return "链路预算";
        case Audit_ProjectSave: return "项目保存";
        case Audit_ProjectLoad: return "项目加载";
        case Audit_ExportDXF: return "DXF导出";
        case Audit_ExportDWG: return "DWG导出";
        case Audit_Import: return "导入";
        case Audit_Print: return "打印";
        case Audit_Undo: return "撤销";
        case Audit_Redo: return "重做";
        case Audit_PluginOperation: return "插件操作";
        case Audit_Other: return "其他";
        }
        return "未知";
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["timestamp"] = timestamp.toString(Qt::ISODate);
        obj["action"] = actionName();
        obj["operator"] = operatorName;
        obj["details"] = details;
        obj["revision"] = projectRevision;
        return obj;
    }

    QString toString() const {
        return QString("[%1] %2 - %3: %4")
            .arg(timestamp.toString("yyyy-MM-dd HH:mm:ss"))
            .arg(actionName())
            .arg(operatorName)
            .arg(details);
    }
};

// 审计日志记录器（单例）
class AuditLogger {
public:
    static AuditLogger& instance() {
        static AuditLogger logger;
        return logger;
    }

    // 记录审计日志
    void log(AuditAction action, const QString &details, const QString &operatorName = "currentUser") {
        AuditEntry entry;
        entry.timestamp = QDateTime::currentDateTime();
        entry.action = action;
        entry.operatorName = operatorName;
        entry.details = details;
        entry.projectRevision = m_currentRevision;
        m_entries.append(entry);
        m_dirty = true;
        // 自动保存到临时文件（防止崩溃丢失）
        if (m_autoSave) saveToFile(m_autoSavePath);
    }

    // 批量记录
    void logBatch(AuditAction action, const QStringList &detailsList, const QString &operatorName = "currentUser") {
        for (const auto &d : detailsList) log(action, d, operatorName);
    }

    // 获取所有日志
    const QList<AuditEntry>& entries() const { return m_entries; }

    // 获取高危操作日志
    QList<AuditEntry> highRiskEntries() const {
        QList<AuditEntry> result;
        for (const auto &e : m_entries) {
            if (e.action == Audit_DeviceExplode || e.action == Audit_BatchDelete ||
                e.action == Audit_ModeSwitch || e.action == Audit_CopyModeSwitch ||
                e.action == Audit_PluginOperation) {
                result.append(e);
            }
        }
        return result;
    }

    // 按操作类型筛选
    QList<AuditEntry> filterByAction(AuditAction action) const {
        QList<AuditEntry> result;
        for (const auto &e : m_entries) if (e.action == action) result.append(e);
        return result;
    }

    // 保存到文件（JSON格式）
    bool saveToFile(const QString &filePath) const {
        QJsonArray arr;
        for (const auto &e : m_entries) arr.append(e.toJson());
        QJsonObject root;
        root["audit_log"] = arr;
        root["count"] = m_entries.size();
        root["export_time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        QJsonDocument doc(root);
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        return true;
    }

    // 从文件加载
    bool loadFromFile(const QString &filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
        QByteArray data = file.readAll();
        file.close();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return false;
        QJsonObject root = doc.object();
        QJsonArray arr = root["audit_log"].toArray();
        m_entries.clear();
        for (const auto &v : arr) {
            QJsonObject obj = v.toObject();
            AuditEntry e;
            e.timestamp = QDateTime::fromString(obj["timestamp"].toString(), Qt::ISODate);
            e.operatorName = obj["operator"].toString();
            e.details = obj["details"].toString();
            e.projectRevision = obj["revision"].toInteger();
            QString act = obj["action"].toString();
            if (act == "器件放置") e.action = Audit_DevicePlace;
            else if (act == "器件删除") e.action = Audit_DeviceDelete;
            else if (act == "器件移动") e.action = Audit_DeviceMove;
            else if (act == "器件复制") e.action = Audit_DeviceCopy;
            else if (act == "器件炸开") e.action = Audit_DeviceExplode;
            else if (act == "批量删除") e.action = Audit_BatchDelete;
            else if (act == "模式切换") e.action = Audit_ModeSwitch;
            else if (act == "复制模式切换") e.action = Audit_CopyModeSwitch;
            else if (act == "链路预算") e.action = Audit_LinkCalculation;
            else if (act == "项目保存") e.action = Audit_ProjectSave;
            else if (act == "项目加载") e.action = Audit_ProjectLoad;
            else if (act == "DXF导出") e.action = Audit_ExportDXF;
            else if (act == "DWG导出") e.action = Audit_ExportDWG;
            else if (act == "导入") e.action = Audit_Import;
            else if (act == "打印") e.action = Audit_Print;
            else if (act == "撤销") e.action = Audit_Undo;
            else if (act == "重做") e.action = Audit_Redo;
            else if (act == "插件操作") e.action = Audit_PluginOperation;
            else e.action = Audit_Other;
            m_entries.append(e);
        }
        m_dirty = false;
        return true;
    }

    // 导出为文本报告
    QString toTextReport() const {
        QString text;
        text += "========== 审计日志报告 ==========\n";
        text += QString("导出时间: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        text += QString("日志总数: %1\n").arg(m_entries.size());
        text += QString("高危操作数: %1\n\n").arg(highRiskEntries().size());
        text += "----- 全部日志 -----\n";
        for (const auto &e : m_entries) text += e.toString() + "\n";
        text += "\n----- 高危操作 -----\n";
        for (const auto &e : highRiskEntries()) text += e.toString() + "\n";
        text += "==================================\n";
        return text;
    }

    // 清空日志
    void clear() { m_entries.clear(); m_dirty = true; }

    // 设置当前项目版本号
    void setCurrentRevision(qint64 rev) { m_currentRevision = rev; }
    qint64 currentRevision() const { return m_currentRevision; }

    // 设置自动保存
    void setAutoSave(bool enabled, const QString &path = "") {
        m_autoSave = enabled;
        if (!path.isEmpty()) m_autoSavePath = path;
    }

    bool isDirty() const { return m_dirty; }
    void markClean() { m_dirty = false; }

private:
    AuditLogger() = default;
    ~AuditLogger() = default;
    AuditLogger(const AuditLogger&) = delete;
    AuditLogger& operator=(const AuditLogger&) = delete;

    QList<AuditEntry> m_entries;
    qint64 m_currentRevision = 0;
    bool m_dirty = false;
    bool m_autoSave = false;
    QString m_autoSavePath;
};

} // namespace Zhifen

#endif // AUDIT_LOGGER_H
