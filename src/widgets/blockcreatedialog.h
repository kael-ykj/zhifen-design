#ifndef BLOCKCREATEDIALOG_H
#define BLOCKCREATEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QCheckBox>
#include <QPointF>

namespace Zhifen {

class BlockCreateDialog : public QDialog
{
    Q_OBJECT
public:
    explicit BlockCreateDialog(QWidget *parent = nullptr);

    QString blockName() const { return m_nameEdit->text().trimmed(); }
    QPointF basePoint() const { return m_basePoint; }
    bool deleteObjects() const { return m_deleteCheck->isChecked(); }
    bool convertToBlock() const { return m_convertCheck->isChecked(); }

    void setBasePoint(const QPointF &pt) {
        m_basePoint = pt;
        m_basePointLabel->setText(QString("基点: (%.2f, %.2f)").arg(pt.x()).arg(pt.y()));
    }

private slots:
    void onPickPoint();
    void onOk();
    void onCancel();

signals:
    void pickPointRequested();

private:
    QLineEdit *m_nameEdit;
    QLabel *m_basePointLabel;
    QPushButton *m_pickBtn;
    QCheckBox *m_deleteCheck;
    QCheckBox *m_convertCheck;
    QPushButton *m_okBtn;
    QPushButton *m_cancelBtn;
    QPointF m_basePoint;

    void setupUI();
};

} // namespace Zhifen

#endif // BLOCKCREATEDIALOG_H
