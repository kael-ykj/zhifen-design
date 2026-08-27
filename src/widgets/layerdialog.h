#ifndef LAYERDIALOG_H
#define LAYERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QColorDialog>

class Document;
class LayerInfo;

class LayerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LayerDialog(Document *doc, QWidget *parent = nullptr);
private slots:
    void onNewLayer();
    void onDeleteLayer();
    void onSetCurrent();
    void onItemChanged(QTableWidgetItem *item);
    void onColorClicked();
    void refresh();
private:
    Document *m_document;
    QTableWidget *m_table;
    QPushButton *m_newBtn, *m_delBtn, *m_currentBtn, *m_okBtn;
    void setupUI();
    void loadLayers();
};

#endif // LAYERDIALOG_H
