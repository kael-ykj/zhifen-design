#ifndef SYSTEM_DIAGRAM_GENERATOR_H
#define SYSTEM_DIAGRAM_GENERATOR_H

#include <QString>
#include <QList>
#include <QMap>
#include <QPointF>
#include <QGraphicsScene>

namespace Zhifen {

// 系统图模式
enum SystemDiagramMode {
    SDM_Sketch = 0,  // 草图模式：简易示意
    SDM_Formal = 1   // 正式模式：标注电平/损耗/编号
};

// 拓扑节点
struct TopoNode {
    QString id;
    QString type;        // 信源/功分器/耦合器/合路器/天线
    QString name;
    int level = 0;
    QPointF pos;
    qreal inputPower = 0.0;
    qreal outputPower = 0.0;
    qreal loss = 0.0;
    QString deviceId;    // 器件编号
    void *sourceItem = nullptr;  // 关联的平面图器件指针
    int sourceIndex = -1;        // 关联的平面图器件索引
    QList<TopoNode*> children;
    TopoNode *parent = nullptr;
};

// 系统图连接
struct TopoConnection {
    TopoNode *from = nullptr;
    TopoNode *to = nullptr;
    qreal loss = 0.0;
    qreal length = 0.0;
};

// 系统图生成结果
struct SystemDiagramResult {
    bool success = false;
    QList<TopoNode*> nodes;
    QList<TopoConnection> connections;
    QStringList errors;
    QRectF boundingRect;
};

// 系统图生成器
class SystemDiagramGenerator
{
public:
    SystemDiagramGenerator();
    ~SystemDiagramGenerator();

    // 从场景生成系统图
    SystemDiagramResult generate(QGraphicsScene *scene, SystemDiagramMode mode);

    // 渲染系统图到场景
    void renderToScene(const SystemDiagramResult &result, QGraphicsScene *targetScene, SystemDiagramMode mode);

    // 清理
    void clear();

private:
    QList<TopoNode*> m_allNodes;

    // 解析拓扑
    void parseTopology(QGraphicsScene *scene, SystemDiagramResult &result);

    // 布局
    void layout(SystemDiagramResult &result);

    // 计算电平（正式模式）
    void calculatePower(SystemDiagramResult &result);

    // 分配器件编号
    void assignDeviceIds(SystemDiagramResult &result);

    // 创建设备节点
    TopoNode* createNode(const QString &type, const QString &name, int level);

    // 判断器件类型
    static QString classifyDevice(const QString &entityType);

    // 获取器件输出端口数
    static int outputPortCount(const QString &type);
};

} // namespace Zhifen

#endif // SYSTEM_DIAGRAM_GENERATOR_H
