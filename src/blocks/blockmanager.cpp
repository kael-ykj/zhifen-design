#include "blockmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace Zhifen {

BlockManager::BlockManager()
{
    initDefaultBlocks();
}

BlockManager& BlockManager::instance()
{
    static BlockManager inst;
    return inst;
}

void BlockManager::addBlock(BlockDefinition *block)
{
    if (!block) return;
    if (m_blocks.contains(block->name())) {
        // 重定义
        delete m_blocks[block->name()];
    }
    m_blocks[block->name()] = block;
    if (!m_referenceCounts.contains(block->name())) {
        m_referenceCounts[block->name()] = 0;
    }
    emit blockAdded(block->name());
}

bool BlockManager::removeBlock(const QString &name)
{
    if (!m_blocks.contains(name)) return false;
    if (m_referenceCounts.value(name, 0) > 0) return false; // 被引用时不能删除
    delete m_blocks[name];
    m_blocks.remove(name);
    m_referenceCounts.remove(name);
    m_blockCategories.remove(name);
    emit blockRemoved(name);
    return true;
}

BlockDefinition* BlockManager::block(const QString &name)
{
    return m_blocks.value(name, nullptr);
}

bool BlockManager::hasBlock(const QString &name) const
{
    return m_blocks.contains(name);
}

QStringList BlockManager::allBlockNames() const
{
    return m_blocks.keys();
}

QList<BlockDefinition*> BlockManager::allBlocks() const
{
    return m_blocks.values();
}

bool BlockManager::renameBlock(const QString &oldName, const QString &newName)
{
    if (!m_blocks.contains(oldName) || m_blocks.contains(newName)) return false;
    BlockDefinition *def = m_blocks[oldName];
    def->setName(newName);
    m_blocks.remove(oldName);
    m_blocks[newName] = def;

    int count = m_referenceCounts.value(oldName, 0);
    m_referenceCounts.remove(oldName);
    m_referenceCounts[newName] = count;

    if (m_blockCategories.contains(oldName)) {
        QString cat = m_blockCategories[oldName];
        m_blockCategories.remove(oldName);
        m_blockCategories[newName] = cat;
    }

    emit blockRenamed(oldName, newName);
    return true;
}

void BlockManager::redefineBlock(const QString &name, BlockDefinition *newDef)
{
    if (!m_blocks.contains(name)) {
        addBlock(newDef);
        return;
    }
    delete m_blocks[name];
    newDef->setName(name);
    m_blocks[name] = newDef;
    emit blockRedefined(name);
}

int BlockManager::referenceCount(const QString &name) const
{
    return m_referenceCounts.value(name, 0);
}

void BlockManager::incrementReference(const QString &name)
{
    m_referenceCounts[name] = m_referenceCounts.value(name, 0) + 1;
}

void BlockManager::decrementReference(const QString &name)
{
    if (m_referenceCounts.contains(name) && m_referenceCounts[name] > 0) {
        m_referenceCounts[name]--;
    }
}

QStringList BlockManager::categories() const
{
    return m_blockCategories.values().toSet().toList();
}

QStringList BlockManager::blocksByCategory(const QString &category) const
{
    QStringList result;
    for (auto it = m_blockCategories.begin(); it != m_blockCategories.end(); ++it) {
        if (it.value() == category) {
            result.append(it.key());
        }
    }
    return result;
}

void BlockManager::setBlockCategory(const QString &blockName, const QString &category)
{
    m_blockCategories[blockName] = category;
}

bool BlockManager::exportBlock(const QString &name, const QString &filePath) const
{
    if (!m_blocks.contains(name)) return false;
    BlockDefinition *def = m_blocks[name];

    QJsonObject root;
    root["name"] = def->name();
    root["description"] = def->description();
    root["baseX"] = def->basePoint().x();
    root["baseY"] = def->basePoint().y();

    QJsonArray attrs;
    for (const auto &attr : def->attributes()) {
        QJsonObject obj;
        obj["tag"] = attr.tag;
        obj["prompt"] = attr.prompt;
        obj["default"] = attr.defaultValue;
        obj["visible"] = attr.visible;
        obj["constant"] = attr.constant;
        obj["verify"] = attr.verify;
        obj["preset"] = attr.preset;
        attrs.append(obj);
    }
    root["attributes"] = attrs;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool BlockManager::importBlock(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    BlockDefinition *def = new BlockDefinition(root["name"].toString());
    def->setDescription(root["description"].toString());
    def->setBasePoint(QPointF(root["baseX"].toDouble(), root["baseY"].toDouble()));

    QJsonArray attrs = root["attributes"].toArray();
    for (auto val : attrs) {
        QJsonObject obj = val.toObject();
        AttributeDefinition attr;
        attr.tag = obj["tag"].toString();
        attr.prompt = obj["prompt"].toString();
        attr.defaultValue = obj["default"].toString();
        attr.visible = obj["visible"].toBool();
        attr.constant = obj["constant"].toBool();
        attr.verify = obj["verify"].toBool();
        attr.preset = obj["preset"].toBool();
        def->addAttribute(attr);
    }

    addBlock(def);
    return true;
}

bool BlockManager::exportAllBlocks(const QString &filePath) const
{
    QJsonObject root;
    QJsonArray blocks;
    for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
        QJsonObject obj;
        obj["name"] = it.value()->name();
        obj["description"] = it.value()->description();
        obj["baseX"] = it.value()->basePoint().x();
        obj["baseY"] = it.value()->basePoint().y();
        blocks.append(obj);
    }
    root["blocks"] = blocks;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) return false;
    file.write(doc.toJson());
    return true;
}

bool BlockManager::importBlocks(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    QJsonArray blocks = root["blocks"].toArray();
    for (auto val : blocks) {
        QJsonObject obj = val.toObject();
        BlockDefinition *def = new BlockDefinition(obj["name"].toString());
        def->setDescription(obj["description"].toString());
        def->setBasePoint(QPointF(obj["baseX"].toDouble(), obj["baseY"].toDouble()));
        addBlock(def);
    }
    return true;
}

void BlockManager::initDefaultBlocks()
{
    // 室分设计常用器件块
    QStringList deviceBlocks = {
        "全向吸顶天线", "板状天线", "壁挂天线", "功分器", "耦合器",
        "合路器", "负载", "衰减器", "光纤天线", "漏缆",
        "射灯天线", "外引天线", "5G pRRU", "RHUB", "BBU",
        "基站", "直放站", "干放", "光纤远端机", "光纤近端机"
    };

    for (const QString &name : deviceBlocks) {
        BlockDefinition *def = new BlockDefinition(name);
        def->setDescription("室分器件: " + name);
        def->setBasePoint(QPointF(0, 0));

        // 添加属性
        AttributeDefinition attr;
        attr.tag = "型号";
        attr.prompt = "请输入型号";
        attr.defaultValue = "";
        attr.visible = true;
        attr.position = QPointF(0, 15);
        def->addAttribute(attr);

        AttributeDefinition attr2;
        attr2.tag = "编号";
        attr2.prompt = "请输入编号";
        attr2.defaultValue = "";
        attr2.visible = true;
        attr2.position = QPointF(0, -15);
        def->addAttribute(attr2);

        addBlock(def);
        setBlockCategory(name, "室分器件");
    }

    // 常用图块
    QStringList commonBlocks = {"图签", "标题栏", "指北针", "比例尺", "图例"};
    for (const QString &name : commonBlocks) {
        BlockDefinition *def = new BlockDefinition(name);
        def->setDescription("常用图块: " + name);
        def->setBasePoint(QPointF(0, 0));
        addBlock(def);
        setBlockCategory(name, "常用图块");
    }
}

void BlockManager::clear()
{
    qDeleteAll(m_blocks);
    m_blocks.clear();
    m_referenceCounts.clear();
    m_blockCategories.clear();
}

} // namespace Zhifen
