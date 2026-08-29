#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QPointF>
#include <QGraphicsScene>
#include <QGraphicsView>

namespace Zhifen {

// 命令类型
enum CommandType {
    Cmd_Unknown = 0,
    Cmd_Line,           // 画线
    Cmd_Circle,         // 画圆
    Cmd_Arc,            // 画弧
    Cmd_Rectangle,      // 画矩形
    Cmd_Move,           // 移动
    Cmd_Copy,           // 复制
    Cmd_Rotate,         // 旋转
    Cmd_Scale,          // 缩放
    Cmd_Mirror,         // 镜像
    Cmd_Erase,          // 删除
    Cmd_Offset,         // 偏移
    Cmd_Trim,           // 修剪
    Cmd_Extend,         // 延伸
    Cmd_Break,          // 打断
    Cmd_Fillet,         // 圆角
    Cmd_Chamfer,        // 倒角
    Cmd_Array,          // 阵列
    Cmd_Group,          // 组
    Cmd_Ungroup,        // 解组
    Cmd_Block,          // 创建块
    Cmd_Insert,         // 插入块
    Cmd_Explode,        // 分解
    Cmd_Zoom,           // 缩放视图
    Cmd_Pan,            // 平移视图
    Cmd_Regen,          // 重生成
    Cmd_Redraw,         // 重画
    Cmd_Layer,          // 图层
    Cmd_Color,          // 颜色
    Cmd_Linetype,       // 线型
    Cmd_Lineweight,     // 线宽
    Cmd_Style,          // 文字样式
    Cmd_DimStyle,       // 标注样式
    Cmd_Units,          // 单位
    Cmd_Limits,         // 图形界限
    Cmd_Grid,           // 栅格
    Cmd_Snap,           // 捕捉
    Cmd_Ortho,          // 正交
    Cmd_Osnap,          // 对象捕捉
    Cmd_Polar,          // 极轴
    Cmd_Otrack,         // 对象追踪
    Cmd_Lwt,            // 线宽显示
    Cmd_Properties,     // 特性
    Cmd_Matchprop,      // 特性匹配
    Cmd_List,           // 列表
    Cmd_Id,             // 点坐标
    Cmd_Dist,           // 距离
    Cmd_Area,           // 面积
    Cmd_Massprop,       // 质量特性
    Cmd_Time,           // 时间
    Cmd_Status,         // 状态
    Cmd_Setvar,         // 系统变量
    Cmd_Help,           // 帮助
    Cmd_Quit,           // 退出
    Cmd_Save,           // 保存
    Cmd_Open,           // 打开
    Cmd_New,            // 新建
    Cmd_Plot,           // 打印
    Cmd_Preview,        // 预览
    Cmd_Purge,          // 清理
    Cmd_Audit,          // 核查
    Cmd_Recover,        // 修复
    Cmd_Undo,           // 放弃
    Cmd_Redo,           // 重做
    Cmd_Multiple,       // 重复
    Cmd_Cancel,         // 取消
    Cmd_Escape          // 退出当前命令
};

// 命令参数
struct CommandArgs {
    CommandType type;
    QString rawCommand;
    QStringList args;
    QList<QPointF> points;
    qreal value = 0;
    QString text;
    bool valid = false;
};

// 命令解析器
class CommandParser
{
public:
    static CommandParser& instance();

    // 解析命令
    CommandArgs parse(const QString &command);

    // 命令类型转字符串
    QString commandName(CommandType type) const;

    // 字符串转命令类型
    CommandType commandType(const QString &name) const;

    // 获取命令帮助
    QString commandHelp(CommandType type) const;
    QString allCommandsHelp() const;

    // 命令别名
    void addAlias(const QString &alias, CommandType type);
    bool isAlias(const QString &name) const;

private:
    CommandParser();
    void initCommands();
    void initAliases();

    QMap<QString, CommandType> m_commandMap;
    QMap<QString, CommandType> m_aliasMap;
    QMap<CommandType, QString> m_helpMap;
};

} // namespace Zhifen

#endif // COMMAND_PARSER_H
