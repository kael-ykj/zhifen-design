#ifndef DEVICELIBRARYPANEL_H
#define DEVICELIBRARYPANEL_H

#include <QDockWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include "entities/deviceitem.h"

namespace Zhifen {

class DeviceLibraryPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit DeviceLibraryPanel(QWidget *parent = nullptr);

    void refresh();

signals:
    void deviceSelected(DeviceItem::DeviceType type, const QString &name);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onSearch(const QString &text);
    void onPlaceClicked();

private:
    QTreeWidget *m_treeWidget;
    QLineEdit *m_searchEdit;
    QPushButton *m_placeBtn;
    DeviceItem::DeviceType m_selectedType;
    QString m_selectedName;

    void setupUI();
    void buildTree();
    void addDeviceCategory(const QString &category, const QList<QPair<QString, DeviceItem::DeviceType>> &devices);
};

} // namespace Zhifen

#endif // DEVICELIBRARYPANEL_H
