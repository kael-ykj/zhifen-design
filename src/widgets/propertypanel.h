#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include <QWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QColorDialog>
#include <QScrollArea>
#include <QCheckBox>
#include <QGroupBox>

class CadItem;
class CadScene;
class LineItem;
class CircleItem;
class ArcItem;
class PolylineItem;
class RectangleItem;
class FeederItem;
namespace Zhifen { class DeviceItem; }

class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget *parent = nullptr);
    void setScene(CadScene *scene);
    void updateProperties();

private slots:
    void onLayerChanged(const QString &layer);
    void onColorChanged();
    void onLineWidthChanged(int index);
    void onLineTypeChanged(int index);
    void onColorByLayerChanged(int state);
    void onGeometryChanged();

private:
    CadScene *m_scene = nullptr;

    // 基础控件
    QLabel *m_emptyLabel;
    QWidget *m_contentWidget;
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;

    // 常规属性
    QLabel *m_typeLabel;
    QComboBox *m_layerCombo;
    QPushButton *m_colorButton;
    QComboBox *m_lineWidthCombo;
    QComboBox *m_lineTypeCombo;
    QCheckBox *m_colorByLayerCheck;

    // 几何属性（根据图元类型动态显示）
    QGroupBox *m_geometryGroup;
    QGroupBox *m_deviceGroup;
    QGroupBox *m_feederGroup;
    QLabel *m_deviceTypeLabel;
    QLabel *m_deviceNameLabel;
    QLabel *m_devicePowerLabel;
    QLabel *m_deviceLossLabel;
    QLabel *m_feederTypeLabel;
    QLabel *m_feederLengthLabel;
    QLabel *m_feederStartLabel;
    QLabel *m_feederEndLabel;
    QFormLayout *m_geometryLayout;
    QDoubleSpinBox *m_xSpin, *m_ySpin;
    QDoubleSpinBox *m_startXSpin, *m_startYSpin, *m_endXSpin, *m_endYSpin;
    QDoubleSpinBox *m_centerXSpin, *m_centerYSpin, *m_radiusSpin;
    QDoubleSpinBox *m_startAngleSpin, *m_spanAngleSpin;
    QDoubleSpinBox *m_widthSpin, *m_heightSpin;

    // 当前选中的图元
    QList<CadItem*> m_selectedItems;
    bool m_updating = false;

    // 方法
    void setupUI();
    void setupGeometryWidgets();
    void updateGeometryVisibility(CadItem *item);
    void loadGeometryValues(CadItem *item);
    void updateDeviceProperties(QGraphicsItem *item);
    void updateFeederProperties(QGraphicsItem *item);
    void applyGeometryChanges(CadItem *item);
    QColor currentColor() const;
    void setColorButtonColor(const QColor &color);
};

#endif // PROPERTYPANEL_H
