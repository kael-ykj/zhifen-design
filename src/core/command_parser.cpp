#include "command_parser.h"
#include <QRegularExpression>
#include <QStringList>

namespace Zhifen {

CommandParser::CommandParser() {
    initCommands();
    initAliases();
}

CommandParser& CommandParser::instance() {
    static CommandParser inst;
    return inst;
}

void CommandParser::initCommands() {
    // 绘图命令
    m_commandMap["LINE"] = Cmd_Line;
    m_commandMap["L"] = Cmd_Line;
    m_commandMap["CIRCLE"] = Cmd_Circle;
    m_commandMap["C"] = Cmd_Circle;
    m_commandMap["ARC"] = Cmd_Arc;
    m_commandMap["A"] = Cmd_Arc;
    m_commandMap["RECTANGLE"] = Cmd_Rectangle;
    m_commandMap["REC"] = Cmd_Rectangle;
    m_commandMap["RECTANG"] = Cmd_Rectangle;

    // 编辑命令
    m_commandMap["MOVE"] = Cmd_Move;
    m_commandMap["M"] = Cmd_Move;
    m_commandMap["COPY"] = Cmd_Copy;
    m_commandMap["CO"] = Cmd_Copy;
    m_commandMap["CP"] = Cmd_Copy;
    m_commandMap["ROTATE"] = Cmd_Rotate;
    m_commandMap["RO"] = Cmd_Rotate;
    m_commandMap["SCALE"] = Cmd_Scale;
    m_commandMap["SC"] = Cmd_Scale;
    m_commandMap["MIRROR"] = Cmd_Mirror;
    m_commandMap["MI"] = Cmd_Mirror;
    m_commandMap["ERASE"] = Cmd_Erase;
    m_commandMap["E"] = Cmd_Erase;
    m_commandMap["DELETE"] = Cmd_Erase;
    m_commandMap["OFFSET"] = Cmd_Offset;
    m_commandMap["O"] = Cmd_Offset;
    m_commandMap["TRIM"] = Cmd_Trim;
    m_commandMap["TR"] = Cmd_Trim;
    m_commandMap["EXTEND"] = Cmd_Extend;
    m_commandMap["EX"] = Cmd_Extend;
    m_commandMap["BREAK"] = Cmd_Break;
    m_commandMap["BR"] = Cmd_Break;
    m_commandMap["FILLET"] = Cmd_Fillet;
    m_commandMap["F"] = Cmd_Fillet;
    m_commandMap["CHAMFER"] = Cmd_Chamfer;
    m_commandMap["CHA"] = Cmd_Chamfer;
    m_commandMap["ARRAY"] = Cmd_Array;
    m_commandMap["AR"] = Cmd_Array;
    m_commandMap["EXPLODE"] = Cmd_Explode;
    m_commandMap["X"] = Cmd_Explode;

    // 块命令
    m_commandMap["BLOCK"] = Cmd_Block;
    m_commandMap["B"] = Cmd_Block;
    m_commandMap["INSERT"] = Cmd_Insert;
    m_commandMap["I"] = Cmd_Insert;
    m_commandMap["GROUP"] = Cmd_Group;
    m_commandMap["G"] = Cmd_Group;
    m_commandMap["UNGROUP"] = Cmd_Ungroup;

    // 视图命令
    m_commandMap["ZOOM"] = Cmd_Zoom;
    m_commandMap["Z"] = Cmd_Zoom;
    m_commandMap["PAN"] = Cmd_Pan;
    m_commandMap["P"] = Cmd_Pan;
    m_commandMap["REGEN"] = Cmd_Regen;
    m_commandMap["RE"] = Cmd_Regen;
    m_commandMap["REDRAW"] = Cmd_Redraw;
    m_commandMap["R"] = Cmd_Redraw;

    // 图层和特性
    m_commandMap["LAYER"] = Cmd_Layer;
    m_commandMap["LA"] = Cmd_Layer;
    m_commandMap["COLOR"] = Cmd_Color;
    m_commandMap["COL"] = Cmd_Color;
    m_commandMap["COLOUR"] = Cmd_Color;
    m_commandMap["LINETYPE"] = Cmd_Linetype;
    m_commandMap["LT"] = Cmd_Linetype;
    m_commandMap["LINEWEIGHT"] = Cmd_Lineweight;
    m_commandMap["LW"] = Cmd_Lineweight;
    m_commandMap["PROPERTIES"] = Cmd_Properties;
    m_commandMap["PR"] = Cmd_Properties;
    m_commandMap["MATCHPROP"] = Cmd_Matchprop;
    m_commandMap["MA"] = Cmd_Matchprop;
    m_commandMap["PAINTER"] = Cmd_Matchprop;

    // 查询命令
    m_commandMap["LIST"] = Cmd_List;
    m_commandMap["LI"] = Cmd_List;
    m_commandMap["ID"] = Cmd_Id;
    m_commandMap["DIST"] = Cmd_Dist;
    m_commandMap["DI"] = Cmd_Dist;
    m_commandMap["AREA"] = Cmd_Area;
    m_commandMap["AA"] = Cmd_Area;
    m_commandMap["STATUS"] = Cmd_Status;
    m_commandMap["TIME"] = Cmd_Time;

    // 系统命令
    m_commandMap["UNITS"] = Cmd_Units;
    m_commandMap["UN"] = Cmd_Units;
    m_commandMap["LIMITS"] = Cmd_Limits;
    m_commandMap["GRID"] = Cmd_Grid;
    m_commandMap["SNAP"] = Cmd_Snap;
    m_commandMap["ORTHO"] = Cmd_Ortho;
    m_commandMap["OSNAP"] = Cmd_Osnap;
    m_commandMap["OS"] = Cmd_Osnap;
    m_commandMap["POLAR"] = Cmd_Polar;
    m_commandMap["OTRACK"] = Cmd_Otrack;
    m_commandMap["LWT"] = Cmd_Lwt;
    m_commandMap["SETVAR"] = Cmd_Setvar;
    m_commandMap["SET"] = Cmd_Setvar;
    m_commandMap["HELP"] = Cmd_Help;
    m_commandMap["?"] = Cmd_Help;
    m_commandMap["QUIT"] = Cmd_Quit;
    m_commandMap["EXIT"] = Cmd_Quit;
    m_commandMap["SAVE"] = Cmd_Save;
    m_commandMap["SAVEAS"] = Cmd_Save;
    m_commandMap["OPEN"] = Cmd_Open;
    m_commandMap["NEW"] = Cmd_New;
    m_commandMap["PLOT"] = Cmd_Plot;
    m_commandMap["PRINT"] = Cmd_Plot;
    m_commandMap["PREVIEW"] = Cmd_Preview;
    m_commandMap["PRE"] = Cmd_Preview;
    m_commandMap["PURGE"] = Cmd_Purge;
    m_commandMap["PU"] = Cmd_Purge;
    m_commandMap["AUDIT"] = Cmd_Audit;
    m_commandMap["RECOVER"] = Cmd_Recover;
    m_commandMap["UNDO"] = Cmd_Undo;
    m_commandMap["U"] = Cmd_Undo;
    m_commandMap["REDO"] = Cmd_Redo;
    m_commandMap["MULTIPLE"] = Cmd_Multiple;
    m_commandMap["CANCEL"] = Cmd_Cancel;
    m_commandMap["ESC"] = Cmd_Escape;

    // 初始化帮助信息
    m_helpMap[Cmd_Line] = "LINE (L): 绘制直线段。指定起点和终点。";
    m_helpMap[Cmd_Circle] = "CIRCLE (C): 绘制圆。指定圆心和半径。";
    m_helpMap[Cmd_Arc] = "ARC (A): 绘制圆弧。指定三点或起点/圆心/端点。";
    m_helpMap[Cmd_Rectangle] = "RECTANGLE (REC): 绘制矩形。指定两个对角点。";
    m_helpMap[Cmd_Move] = "MOVE (M): 移动对象。指定基点和位移。";
    m_helpMap[Cmd_Copy] = "COPY (CO): 复制对象。指定基点和位移。";
    m_helpMap[Cmd_Rotate] = "ROTATE (RO): 旋转对象。指定基点和角度。";
    m_helpMap[Cmd_Scale] = "SCALE (SC): 缩放对象。指定基点和比例因子。";
    m_helpMap[Cmd_Mirror] = "MIRROR (MI): 镜像对象。指定镜像线。";
    m_helpMap[Cmd_Erase] = "ERASE (E): 删除选中的对象。";
    m_helpMap[Cmd_Offset] = "OFFSET (O): 偏移对象。指定偏移距离。";
    m_helpMap[Cmd_Trim] = "TRIM (TR): 修剪对象。指定剪切边。";
    m_helpMap[Cmd_Extend] = "EXTEND (EX): 延伸对象。指定边界边。";
    m_helpMap[Cmd_Break] = "BREAK (BR): 打断对象。指定两个打断点。";
    m_helpMap[Cmd_Fillet] = "FILLET (F): 圆角。指定圆角半径。";
    m_helpMap[Cmd_Chamfer] = "CHAMFER (CHA): 倒角。指定倒角距离。";
    m_helpMap[Cmd_Array] = "ARRAY (AR): 阵列。矩形或环形阵列。";
    m_helpMap[Cmd_Explode] = "EXPLODE (X): 分解组合对象。";
    m_helpMap[Cmd_Block] = "BLOCK (B): 创建块定义。";
    m_helpMap[Cmd_Insert] = "INSERT (I): 插入块。";
    m_helpMap[Cmd_Zoom] = "ZOOM (Z): 缩放视图。选项: 全部(A)/范围(E)/窗口(W)/上一个(P)";
    m_helpMap[Cmd_Pan] = "PAN (P): 平移视图。";
    m_helpMap[Cmd_Regen] = "REGEN (RE): 重生成图形。";
    m_helpMap[Cmd_Layer] = "LAYER (LA): 图层管理。";
    m_helpMap[Cmd_Color] = "COLOR (COL): 设置对象颜色。";
    m_helpMap[Cmd_Properties] = "PROPERTIES (PR): 打开特性面板。";
    m_helpMap[Cmd_Matchprop] = "MATCHPROP (MA): 特性匹配（格式刷）。";
    m_helpMap[Cmd_List] = "LIST (LI): 列出对象信息。";
    m_helpMap[Cmd_Dist] = "DIST (DI): 测量两点距离。";
    m_helpMap[Cmd_Area] = "AREA (AA): 计算面积和周长。";
    m_helpMap[Cmd_Undo] = "UNDO (U): 撤销上一步操作。";
    m_helpMap[Cmd_Redo] = "REDO: 重做已撤销的操作。";
    m_helpMap[Cmd_Save] = "SAVE: 保存图形。";
    m_helpMap[Cmd_Open] = "OPEN: 打开图形文件。";
    m_helpMap[Cmd_Plot] = "PLOT: 打印图形。";
    m_helpMap[Cmd_Help] = "HELP (?): 显示帮助信息。";
}

void CommandParser::initAliases() {
    // 常用别名
    m_aliasMap["画线"] = Cmd_Line;
    m_aliasMap["画圆"] = Cmd_Circle;
    m_aliasMap["移动"] = Cmd_Move;
    m_aliasMap["复制"] = Cmd_Copy;
    m_aliasMap["删除"] = Cmd_Erase;
    m_aliasMap["旋转"] = Cmd_Rotate;
    m_aliasMap["缩放"] = Cmd_Scale;
    m_aliasMap["镜像"] = Cmd_Mirror;
    m_aliasMap["偏移"] = Cmd_Offset;
    m_aliasMap["修剪"] = Cmd_Trim;
    m_aliasMap["延伸"] = Cmd_Extend;
    m_aliasMap["分解"] = Cmd_Explode;
    m_aliasMap["创建块"] = Cmd_Block;
    m_aliasMap["插入块"] = Cmd_Insert;
    m_aliasMap["图层"] = Cmd_Layer;
    m_aliasMap["特性"] = Cmd_Properties;
    m_aliasMap["距离"] = Cmd_Dist;
    m_aliasMap["面积"] = Cmd_Area;
    m_aliasMap["撤销"] = Cmd_Undo;
    m_aliasMap["重做"] = Cmd_Redo;
    m_aliasMap["保存"] = Cmd_Save;
    m_aliasMap["打开"] = Cmd_Open;
    m_aliasMap["打印"] = Cmd_Plot;
    m_aliasMap["帮助"] = Cmd_Help;
}

CommandArgs CommandParser::parse(const QString &command) {
    CommandArgs args;
    args.rawCommand = command;
    args.valid = false;

    if (command.isEmpty()) return args;

    // 分割命令和参数
    QStringList parts = command.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) return args;

    QString cmdName = parts.first().toUpper();
    args.args = parts.mid(1);

    // 检查别名
    if (m_aliasMap.contains(cmdName)) {
        args.type = m_aliasMap[cmdName];
        args.valid = true;
    } else if (m_commandMap.contains(cmdName)) {
        args.type = m_commandMap[cmdName];
        args.valid = true;
    } else {
        args.type = Cmd_Unknown;
    }

    // 解析点坐标 (x,y)
    for (const QString &arg : args.args) {
        QRegularExpression re("^([+-]?\\d*\\.?\\d+)[,，]([+-]?\\d*\\.?\\d+)$");
        QRegularExpressionMatch match = re.match(arg);
        if (match.hasMatch()) {
            args.points.append(QPointF(match.captured(1).toDouble(),
                                        match.captured(2).toDouble()));
        }
    }

    // 解析数值
    if (!args.args.isEmpty()) {
        bool ok;
        qreal val = args.args.last().toDouble(&ok);
        if (ok) args.value = val;
    }

    // 解析文字
    if (args.args.size() >= 2) {
        args.text = args.args.mid(1).join(" ");
    }

    return args;
}

QString CommandParser::commandName(CommandType type) const {
    for (auto it = m_commandMap.begin(); it != m_commandMap.end(); ++it) {
        if (it.value() == type && it.key().length() > 1) {
            return it.key();
        }
    }
    return "UNKNOWN";
}

CommandType CommandParser::commandType(const QString &name) const {
    QString upper = name.toUpper();
    if (m_commandMap.contains(upper)) return m_commandMap[upper];
    if (m_aliasMap.contains(upper)) return m_aliasMap[upper];
    return Cmd_Unknown;
}

QString CommandParser::commandHelp(CommandType type) const {
    return m_helpMap.value(type, "暂无帮助信息。");
}

QString CommandParser::allCommandsHelp() const {
    QString help;
    help += "=== 智分Design CAD命令帮助 ===\n\n";
    help += "【绘图命令】\n";
    help += "  LINE(L) 画线  CIRCLE(C) 画圆  ARC(A) 画弧  RECTANGLE(REC) 画矩形\n\n";
    help += "【编辑命令】\n";
    help += "  MOVE(M) 移动  COPY(CO) 复制  ROTATE(RO) 旋转  SCALE(SC) 缩放\n";
    help += "  MIRROR(MI) 镜像  ERASE(E) 删除  OFFSET(O) 偏移  TRIM(TR) 修剪\n";
    help += "  EXTEND(EX) 延伸  BREAK(BR) 打断  FILLET(F) 圆角  EXPLODE(X) 分解\n\n";
    help += "【块命令】\n";
    help += "  BLOCK(B) 创建块  INSERT(I) 插入块  GROUP(G) 组\n\n";
    help += "【视图命令】\n";
    help += "  ZOOM(Z) 缩放  PAN(P) 平移  REGEN(RE) 重生成\n\n";
    help += "【查询命令】\n";
    help += "  DIST(DI) 距离  AREA(AA) 面积  LIST(LI) 列表\n\n";
    help += "【系统命令】\n";
    help += "  LAYER(LA) 图层  PROPERTIES(PR) 特性  UNDO(U) 撤销  REDO 重做\n";
    help += "  SAVE 保存  OPEN 打开  PLOT 打印  HELP(?) 帮助\n\n";
    help += "输入 HELP <命令名> 查看详细帮助。\n";
    return help;
}

void CommandParser::addAlias(const QString &alias, CommandType type) {
    m_aliasMap[alias.toUpper()] = type;
}

bool CommandParser::isAlias(const QString &name) const {
    return m_aliasMap.contains(name.toUpper());
}

} // namespace Zhifen
