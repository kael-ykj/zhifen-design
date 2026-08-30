#ifndef BLOCKEDITOR_H
#define BLOCKEDITOR_H

#include <QDialog>
#include <QString>
#include <QPointF>
#include "blocks/blockdefinition.h"

namespace Zhifen {

class CadScene;
class CadView;
class BlockManagerPanel;

class BlockEditor : public QDialog
{
    Q_OBJECT
public:
    explicit BlockEditor(const QString &blockName, QWidget *parent = nullptr);
    ~BlockEditor();

    QString blockName() const { return m_blockName; }

signals:
    void blockEdited(const QString &blockName);

private slots:
    void onSave();
    void onCancel();
    void onAddAttribute();
    void onEditAttribute();
    void onDeleteAttribute();
    void onAddLine();
    void onAddCircle();
    void onAddText();
    void onDeleteSelected();
    void onZoomExtents();

private:
    QString m_blockName;
    CadScene *m_scene;
    CadView *m_view;
    BlockDefinition *m_workingCopy;

    // 属性列表
    class QListWidget *m_attrList;
    class QPushButton *m_addAttrBtn;
    class QPushButton *m_editAttrBtn;
    class QPushButton *m_delAttrBtn;

    // 工具按钮
    class QPushButton *m_lineBtn;
    class QPushButton *m_circleBtn;
    class QPushButton *m_textBtn;
    class QPushButton *m_deleteBtn;
    class QPushButton *m_zoomBtn;

    // 保存/取消
    class QPushButton *m_saveBtn;
    class QPushButton *m_cancelBtn;

    void setupUI();
    void loadBlock();
    void saveBlock();
    void refreshAttributeList();
    void setupTools();
};

} // namespace Zhifen

#endif // BLOCKEDITOR_H
