#ifndef LAYERPANEL_H
#define LAYERPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QPushButton>

class Document;

class LayerPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LayerPanel(QWidget *parent = nullptr);
    void setDocument(Document *doc);
    void refresh();

signals:
    void currentLayerChanged(const QString &layerName);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onAddLayer();
    void onDeleteLayer();

private:
    Document *m_document = nullptr;
    QListWidget *m_layerList;
    QPushButton *m_addBtn;
    QPushButton *m_deleteBtn;
};

#endif // LAYERPANEL_H
