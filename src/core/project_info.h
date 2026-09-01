#ifndef PROJECT_INFO_H
#define PROJECT_INFO_H

#include <QString>
#include <QDate>
#include <QJsonObject>

namespace Zhifen {

// 工程信息
struct ProjectInfo {
    QString projectName;        // 项目名称
    QString constructionUnit;   // 建设单位
    QString designUnit;         // 设计单位
    QString designer;           // 设计人
    QString reviewer;            // 审核人
    QString checker;            // 校对人
    QString draftsman;          // 绘图人
    QString drawingNumber;      // 图号
    QDate drawDate;             // 出图日期
    QString projectCode;        // 项目编号
    QString contactPerson;      // 联系人
    QString contactPhone;       // 联系电话

    ProjectInfo() {
        drawDate = QDate::currentDate();
        designUnit = "智分Design";
    }

    // 序列化为JSON
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["projectName"] = projectName;
        obj["constructionUnit"] = constructionUnit;
        obj["designUnit"] = designUnit;
        obj["designer"] = designer;
        obj["reviewer"] = reviewer;
        obj["checker"] = checker;
        obj["draftsman"] = draftsman;
        obj["drawingNumber"] = drawingNumber;
        obj["drawDate"] = drawDate.toString("yyyy-MM-dd");
        obj["projectCode"] = projectCode;
        obj["contactPerson"] = contactPerson;
        obj["contactPhone"] = contactPhone;
        return obj;
    }

    // 从JSON反序列化
    void fromJson(const QJsonObject &obj) {
        projectName = obj.value("projectName").toString();
        constructionUnit = obj.value("constructionUnit").toString();
        designUnit = obj.value("designUnit").toString("智分Design");
        designer = obj.value("designer").toString();
        reviewer = obj.value("reviewer").toString();
        checker = obj.value("checker").toString();
        draftsman = obj.value("draftsman").toString();
        drawingNumber = obj.value("drawingNumber").toString();
        drawDate = QDate::fromString(obj.value("drawDate").toString(), "yyyy-MM-dd");
        if (!drawDate.isValid()) drawDate = QDate::currentDate();
        projectCode = obj.value("projectCode").toString();
        contactPerson = obj.value("contactPerson").toString();
        contactPhone = obj.value("contactPhone").toString();
    }
};

// 工程信息管理器（单例）
class ProjectInfoManager
{
public:
    static ProjectInfoManager& instance() {
        static ProjectInfoManager inst;
        return inst;
    }

    ProjectInfo& info() { return m_info; }
    void setInfo(const ProjectInfo &info) { m_info = info; }

    // 获取图签文本
    QString titleBlockText() const {
        return QString("项目名称：%1\n建设单位：%2\n设计单位：%3\n设计人：%4  审核人：%5\n校对人：%6  绘图人：%7\n图号：%8  出图日期：%9")
            .arg(m_info.projectName)
            .arg(m_info.constructionUnit)
            .arg(m_info.designUnit)
            .arg(m_info.designer)
            .arg(m_info.reviewer)
            .arg(m_info.checker)
            .arg(m_info.draftsman)
            .arg(m_info.drawingNumber)
            .arg(m_info.drawDate.toString("yyyy年MM月dd日"));
    }

private:
    ProjectInfoManager() {}
    ProjectInfo m_info;
};

} // namespace Zhifen

#endif // PROJECT_INFO_H
