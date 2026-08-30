#include "shortcutmanager.h"
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Zhifen {

ShortcutManager::ShortcutManager()
{
    initCategoryNames();
    loadDefaults();
}

ShortcutManager& ShortcutManager::instance()
{
    static ShortcutManager inst;
    return inst;
}

void ShortcutManager::initCategoryNames()
{
    m_categoryNames[SC_File] = "文件";
    m_categoryNames[SC_Edit] = "编辑";
    m_categoryNames[SC_View] = "视图";
    m_categoryNames[SC_Draw] = "绘图";
    m_categoryNames[SC_Modify] = "修改";
    m_categoryNames[SC_Dimension] = "标注";
    m_categoryNames[SC_Layer] = "图层";
    m_categoryNames[SC_Tools] = "工具";
    m_categoryNames[SC_Window] = "窗口";
    m_categoryNames[SC_Help] = "帮助";
    m_categoryNames[SC_Custom] = "自定义";
}

void ShortcutManager::loadDefaults()
{
    // 文件
    registerShortcut("file.new", "新建", QKeySequence::New, SC_File);
    registerShortcut("file.open", "打开", QKeySequence::Open, SC_File);
    registerShortcut("file.save", "保存", QKeySequence::Save, SC_File);
    registerShortcut("file.saveas", "另存为", QKeySequence::SaveAs, SC_File);
    registerShortcut("file.print", "打印", QKeySequence::Print, SC_File);
    registerShortcut("file.quit", "退出", QKeySequence::Quit, SC_File);

    // 编辑
    registerShortcut("edit.undo", "撤销", QKeySequence::Undo, SC_Edit);
    registerShortcut("edit.redo", "重做", QKeySequence::Redo, SC_Edit);
    registerShortcut("edit.cut", "剪切", QKeySequence::Cut, SC_Edit);
    registerShortcut("edit.copy", "复制", QKeySequence::Copy, SC_Edit);
    registerShortcut("edit.paste", "粘贴", QKeySequence::Paste, SC_Edit);
    registerShortcut("edit.delete", "删除", QKeySequence::Delete, SC_Edit);
    registerShortcut("edit.selectall", "全部选择", QKeySequence::SelectAll, SC_Edit);
    registerShortcut("edit.find", "查找", QKeySequence::Find, SC_Edit);

    // 视图
    registerShortcut("view.zoom", "缩放", Qt::Key_Z, SC_View);
    registerShortcut("view.zoom.extents", "全部缩放", QKeySequence("Ctrl+Shift+E"), SC_View);
    registerShortcut("view.zoom.window", "窗口缩放", QKeySequence("Ctrl+W"), SC_View);
    registerShortcut("view.zoom.in", "放大", QKeySequence("Ctrl++"), SC_View);
    registerShortcut("view.zoom.out", "缩小", QKeySequence("Ctrl+-"), SC_View);
    registerShortcut("view.pan", "平移", Qt::Key_P, SC_View);
    registerShortcut("view.regen", "重生成", QKeySequence("RE"), SC_View);
    registerShortcut("view.redraw", "重画", QKeySequence("R"), SC_View);

    // 绘图
    registerShortcut("draw.line", "直线", Qt::Key_L, SC_Draw);
    registerShortcut("draw.circle", "圆", Qt::Key_C, SC_Draw);
    registerShortcut("draw.arc", "圆弧", Qt::Key_A, SC_Draw);
    registerShortcut("draw.rectangle", "矩形", QKeySequence("REC"), SC_Draw);
    registerShortcut("draw.polyline", "多段线", QKeySequence("PL"), SC_Draw);
    registerShortcut("draw.polygon", "多边形", QKeySequence("POL"), SC_Draw);
    registerShortcut("draw.ellipse", "椭圆", QKeySequence("EL"), SC_Draw);
    registerShortcut("draw.text", "文字", QKeySequence("DT"), SC_Draw);
    registerShortcut("draw.mtext", "多行文字", QKeySequence("MT"), SC_Draw);
    registerShortcut("draw.hatch", "图案填充", QKeySequence("H"), SC_Draw);
    registerShortcut("draw.block", "创建块", QKeySequence("B"), SC_Draw);
    registerShortcut("draw.insert", "插入块", QKeySequence("I"), SC_Draw);
    registerShortcut("draw.point", "点", QKeySequence("PO"), SC_Draw);

    // 修改
    registerShortcut("modify.move", "移动", Qt::Key_M, SC_Modify);
    registerShortcut("modify.copy", "复制", QKeySequence("CO"), SC_Modify);
    registerShortcut("modify.rotate", "旋转", QKeySequence("RO"), SC_Modify);
    registerShortcut("modify.scale", "缩放", QKeySequence("SC"), SC_Modify);
    registerShortcut("modify.mirror", "镜像", QKeySequence("MI"), SC_Modify);
    registerShortcut("modify.offset", "偏移", QKeySequence("O"), SC_Modify);
    registerShortcut("modify.trim", "修剪", QKeySequence("TR"), SC_Modify);
    registerShortcut("modify.extend", "延伸", QKeySequence("EX"), SC_Modify);
    registerShortcut("modify.break", "打断", QKeySequence("BR"), SC_Modify);
    registerShortcut("modify.fillet", "圆角", QKeySequence("F"), SC_Modify);
    registerShortcut("modify.chamfer", "倒角", QKeySequence("CHA"), SC_Modify);
    registerShortcut("modify.explode", "分解", QKeySequence("X"), SC_Modify);
    registerShortcut("modify.array", "阵列", QKeySequence("AR"), SC_Modify);
    registerShortcut("modify.stretch", "拉伸", QKeySequence("S"), SC_Modify);
    registerShortcut("modify.lengthen", "拉长", QKeySequence("LEN"), SC_Modify);
    registerShortcut("modify.join", "合并", QKeySequence("J"), SC_Modify);

    // 标注
    registerShortcut("dim.linear", "线性标注", QKeySequence("DLI"), SC_Dimension);
    registerShortcut("dim.aligned", "对齐标注", QKeySequence("DAL"), SC_Dimension);
    registerShortcut("dim.radius", "半径标注", QKeySequence("DRA"), SC_Dimension);
    registerShortcut("dim.diameter", "直径标注", QKeySequence("DDI"), SC_Dimension);
    registerShortcut("dim.angular", "角度标注", QKeySequence("DAN"), SC_Dimension);
    registerShortcut("dim.baseline", "基线标注", QKeySequence("DBA"), SC_Dimension);
    registerShortcut("dim.continue", "连续标注", QKeySequence("DCO"), SC_Dimension);
    registerShortcut("dim.leader", "引线标注", QKeySequence("LE"), SC_Dimension);
    registerShortcut("dim.tolerance", "公差", QKeySequence("TOL"), SC_Dimension);
    registerShortcut("dim.center", "圆心标记", QKeySequence("DCE"), SC_Dimension);
    registerShortcut("dim.edit", "编辑标注", QKeySequence("DED"), SC_Dimension);
    registerShortcut("dim.style", "标注样式", QKeySequence("D"), SC_Dimension);

    // 图层
    registerShortcut("layer.manager", "图层特性", Qt::Key_F2, SC_Layer);
    registerShortcut("layer.make", "将对象的图层置为当前", QKeySequence("MAC"), SC_Layer);
    registerShortcut("layer.previous", "上一个图层", QKeySequence("LAYERP"), SC_Layer);
    registerShortcut("layer.on", "打开图层", QKeySequence("LAYON"), SC_Layer);
    registerShortcut("layer.off", "关闭图层", QKeySequence("LAYOFF"), SC_Layer);
    registerShortcut("layer.freeze", "冻结图层", QKeySequence("LAYFRZ"), SC_Layer);
    registerShortcut("layer.thaw", "解冻图层", QKeySequence("LAYTHW"), SC_Layer);
    registerShortcut("layer.lock", "锁定图层", QKeySequence("LAYLCK"), SC_Layer);
    registerShortcut("layer.unlock", "解锁图层", QKeySequence("LAYULK"), SC_Layer);
    registerShortcut("layer.isolate", "隔离图层", QKeySequence("LAYISO"), SC_Layer);
    registerShortcut("layer.unisolate", "取消隔离", QKeySequence("LAYUNISO"), SC_Layer);
    registerShortcut("layer.match", "图层匹配", QKeySequence("LAYMCH"), SC_Layer);
    registerShortcut("layer.copy", "将对象复制到新图层", QKeySequence("COPYTOLAYER"), SC_Layer);

    // 工具
    registerShortcut("tools.grid", "网格", Qt::Key_F7, SC_Tools);
    registerShortcut("tools.snap", "对象捕捉", Qt::Key_F3, SC_Tools);
    registerShortcut("tools.ortho", "正交", Qt::Key_F8, SC_Tools);
    registerShortcut("tools.polar", "极轴追踪", Qt::Key_F10, SC_Tools);
    registerShortcut("tools.otrack", "对象追踪", Qt::Key_F11, SC_Tools);
    registerShortcut("tools.dyninput", "动态输入", Qt::Key_F12, SC_Tools);
    registerShortcut("tools.lwt", "线宽显示", QKeySequence("LWT"), SC_Tools);
    registerShortcut("tools.ucs", "UCS", QKeySequence("UC"), SC_Tools);
    registerShortcut("tools.dsettings", "草图设置", QKeySequence("DS"), SC_Tools);
    registerShortcut("tools.options", "选项", QKeySequence("OP"), SC_Tools);
    registerShortcut("tools.matchprop", "特性匹配", QKeySequence("MA"), SC_Tools);
    registerShortcut("tools.properties", "特性", QKeySequence("PR"), SC_Tools);
    registerShortcut("tools.qselect", "快速选择", QKeySequence("QSELECT"), SC_Tools);

    // 窗口
    registerShortcut("window.cascade", "层叠", QKeySequence("Ctrl+Shift+C"), SC_Window);
    registerShortcut("window.tileh", "水平平铺", QKeySequence("Ctrl+Shift+H"), SC_Window);
    registerShortcut("window.tilev", "垂直平铺", QKeySequence("Ctrl+Shift+V"), SC_Window);
    registerShortcut("window.close", "关闭", QKeySequence("Ctrl+F4"), SC_Window);
    registerShortcut("window.closeall", "全部关闭", QKeySequence("Ctrl+Shift+F4"), SC_Window);

    // 帮助
    registerShortcut("help.help", "帮助", QKeySequence::HelpContents, SC_Help);
    registerShortcut("help.about", "关于", QKeySequence("ABOUT"), SC_Help);
}

void ShortcutManager::registerShortcut(const QString &id, const QString &name,
                                        const QKeySequence &defaultKey,
                                        ShortcutCategory category,
                                        QAction *action)
{
    if (m_shortcuts.contains(id)) {
        // 更新已有快捷键
        ShortcutDef *def = m_shortcuts[id];
        def->name = name;
        def->defaultKey = defaultKey;
        def->category = category;
        def->action = action;
        if (action) action->setShortcut(def->currentKey);
    } else {
        ShortcutDef *def = new ShortcutDef;
        def->id = id;
        def->name = name;
        def->defaultKey = defaultKey;
        def->currentKey = defaultKey;
        def->category = category;
        def->action = action;
        def->editable = true;
        m_shortcuts[id] = def;
        if (action) action->setShortcut(defaultKey);
    }
}

QKeySequence ShortcutManager::shortcut(const QString &id) const
{
    if (m_shortcuts.contains(id)) {
        return m_shortcuts[id]->currentKey;
    }
    return QKeySequence();
}

ShortcutDef* ShortcutManager::shortcutDef(const QString &id)
{
    return m_shortcuts.value(id, nullptr);
}

QList<ShortcutDef*> ShortcutManager::allShortcuts() const
{
    return m_shortcuts.values();
}

QList<ShortcutDef*> ShortcutManager::shortcutsByCategory(ShortcutCategory category) const
{
    QList<ShortcutDef*> result;
    for (auto def : m_shortcuts) {
        if (def->category == category) {
            result.append(def);
        }
    }
    return result;
}

bool ShortcutManager::setShortcut(const QString &id, const QKeySequence &key)
{
    if (!m_shortcuts.contains(id)) return false;
    if (!m_shortcuts[id]->editable) return false;

    // 检查冲突
    QString conflict = findConflict(key, id);
    if (!conflict.isEmpty()) {
        return false;
    }

    m_shortcuts[id]->currentKey = key;
    if (m_shortcuts[id]->action) {
        m_shortcuts[id]->action->setShortcut(key);
    }
    emit shortcutChanged(id, key);
    return true;
}

void ShortcutManager::resetShortcut(const QString &id)
{
    if (m_shortcuts.contains(id)) {
        m_shortcuts[id]->currentKey = m_shortcuts[id]->defaultKey;
        if (m_shortcuts[id]->action) {
            m_shortcuts[id]->action->setShortcut(m_shortcuts[id]->defaultKey);
        }
        emit shortcutChanged(id, m_shortcuts[id]->defaultKey);
    }
}

void ShortcutManager::resetAll()
{
    for (auto def : m_shortcuts) {
        def->currentKey = def->defaultKey;
        if (def->action) {
            def->action->setShortcut(def->defaultKey);
        }
    }
}

QString ShortcutManager::findConflict(const QKeySequence &key, const QString &excludeId) const
{
    if (key.isEmpty()) return QString();
    for (auto def : m_shortcuts) {
        if (def->id != excludeId && def->currentKey == key) {
            return def->name;
        }
    }
    return QString();
}

QString ShortcutManager::categoryName(ShortcutCategory category) const
{
    return m_categoryNames.value(category, "未知");
}

bool ShortcutManager::exportConfig(const QString &filePath) const
{
    QJsonObject root;
    QJsonArray shortcuts;
    for (auto def : m_shortcuts) {
        QJsonObject obj;
        obj["id"] = def->id;
        obj["name"] = def->name;
        obj["default"] = def->defaultKey.toString();
        obj["current"] = def->currentKey.toString();
        obj["category"] = def->category;
        shortcuts.append(obj);
    }
    root["shortcuts"] = shortcuts;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool ShortcutManager::importConfig(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonArray shortcuts = root["shortcuts"].toArray();
    for (auto val : shortcuts) {
        QJsonObject obj = val.toObject();
        QString id = obj["id"].toString();
        if (m_shortcuts.contains(id)) {
            QKeySequence key(obj["current"].toString());
            m_shortcuts[id]->currentKey = key;
            if (m_shortcuts[id]->action) {
                m_shortcuts[id]->action->setShortcut(key);
            }
        }
    }
    return true;
}

void ShortcutManager::applyAll()
{
    for (auto def : m_shortcuts) {
        if (def->action) {
            def->action->setShortcut(def->currentKey);
        }
    }
}

QAction* ShortcutManager::actionForShortcut(const QKeySequence &key) const
{
    for (auto def : m_shortcuts) {
        if (def->currentKey == key && def->action) {
            return def->action;
        }
    }
    return nullptr;
}

} // namespace Zhifen
