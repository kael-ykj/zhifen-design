#ifndef BLOCKEDITOR_H
#define BLOCKEDITOR_H

#include <QDialog>
#include <QString>
#include <QPointF>
#include "blocks/blockdefinition.h"
#include "cad/cadscene.h"
#include "cad/cadview.h"
#include <QListWidget>
#include <QPushButton>

namespace Zhifen {



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
    QListWidget *m_attrList;
    QPushButton *m_addAttrBtn;
    QPushButton *m_editAttrBtn;
    QPushButton *m_delAttrBtn;

    // 工具按钮
    QPushButton *m_lineBtn;
    QPushButton *m_circleBtn;
    QPushButton *m_textBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_zoomBtn;

    // 保存/取消
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;

    void setupUI();
    void loadBlock();
    void saveBlock();
    void refreshAttributeList();
    void setupTools();
};

} // namespace Zhifen

#endif // BLOCKEDITOR_H
