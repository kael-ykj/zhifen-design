#pragma once
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>
#include "core/zf_types.h"

class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget *parent = nullptr);
    void setProject(zf::Project* project);
    void setDeviceId(const QString& deviceId);
    void clear();
    void refresh();

signals:
    void propertyChanged();
    void deviceDeleted(const QString& deviceId);

private slots:
    void onApply();
    void onDelete();

private:
    zf::Project* m_project{nullptr};
    QLabel* m_titleLabel{nullptr};
    QLineEdit* m_idEdit{nullptr};
    QLineEdit* m_modelEdit{nullptr};
    QLineEdit* m_categoryEdit{nullptr};
    QLineEdit* m_xEdit{nullptr};
    QLineEdit* m_yEdit{nullptr};
    QLineEdit* m_noteEdit{nullptr};
    QLabel* m_powerLabel{nullptr};
    QPushButton* m_applyBtn{nullptr};
    QPushButton* m_deleteBtn{nullptr};
    QString m_currentDeviceId;
};
