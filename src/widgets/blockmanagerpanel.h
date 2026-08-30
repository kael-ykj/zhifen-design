#ifndef BLOCKMANAGERPANEL_H
#define BLOCKMANAGERPANEL_H

#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "blocks/blockmanager.h"

namespace Zhifen {

class BlockManagerPanel : public QDockWidget
{
    Q_OBJECT
public:
    explicit BlockManagerPanel(QWidget *parent = nullptr);

    void refresh();

signals:
    void insertBlockRequested(const QString &blockName);
    void editBlockRequested(const QString &blockName);

private slots:
    void onInsert();
    void onEdit();
    void onRename();
    void onDelete();
    void onSearch(const QString &text);
    void onCategoryChanged(int index);
    void onItemDoubleClicked(QListWidgetItem *item);
    void onItemSelectionChanged();

private:
    QListWidget *m_listWidget;
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryCombo;
    QPushButton *m_insertBtn;
    QPushButton *m_editBtn;
    QPushButton *m_renameBtn;
    QPushButton *m_deleteBtn;
    QLabel *m_infoLabel;

    void setupUI();
    void updateList();
    QString currentBlockName() const;
};

} // namespace Zhifen

#endif // BLOCKMANAGERPANEL_H
