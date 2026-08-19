#pragma once

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QPushButton>

class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget *parent = nullptr);
    void setDeviceId(const QString& deviceId);
    void clear();

private:
    QLabel* m_titleLabel{nullptr};
    QLineEdit* m_idEdit{nullptr};
    QLineEdit* m_modelEdit{nullptr};
    QLineEdit* m_xEdit{nullptr};
    QLineEdit* m_yEdit{nullptr};
    QLineEdit* m_noteEdit{nullptr};
    QPushButton* m_applyBtn{nullptr};
    QPushButton* m_deleteBtn{nullptr};
    QString m_currentDeviceId;
};
