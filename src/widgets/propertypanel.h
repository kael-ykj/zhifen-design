#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class CadItem;
class CadScene;

class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget *parent = nullptr);
    void setScene(CadScene *scene);
    void updateProperties();

private:
    CadScene *m_scene = nullptr;
    QLabel *m_emptyLabel;
    QWidget *m_contentWidget;
    QFormLayout *m_formLayout;
    QLabel *m_typeLabel;
    QLabel *m_layerLabel;
    QLabel *m_xLabel;
    QLabel *m_yLabel;
    QLabel *m_widthLabel;
    QLabel *m_heightLabel;
};

#endif // PROPERTYPANEL_H
