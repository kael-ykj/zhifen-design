#ifndef ATTRIBUTEDIALOG_H
#define ATTRIBUTEDIALOG_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QPointF>
#include "blocks/blockdefinition.h"

namespace Zhifen {

class AttributeDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AttributeDialog(QWidget *parent = nullptr);

    // 设置块定义和当前属性值
    void setBlock(BlockDefinition *def, const QMap<QString, QString> &currentValues);

    // 获取编辑后的属性值
    QMap<QString, QString> attributeValues() const { return m_values; }

private slots:
    void onOk();
    void onCancel();

private:
    QMap<QString, QString> m_values;
    QMap<QString, class QLineEdit*> m_edits;
    class QVBoxLayout *m_attrLayout;
    class QPushButton *m_okBtn;
    class QPushButton *m_cancelBtn;

    void setupUI();
    void clearAttributes();
};

} // namespace Zhifen

#endif // ATTRIBUTEDIALOG_H
