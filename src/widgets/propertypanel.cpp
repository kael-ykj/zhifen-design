#define _USE_MATH_DEFINES
#include <cmath>
#include "propertypanel.h"
#include "cadscene.h"
#include "caditem.h"
#include "lineitem.h"
#include "circleitem.h"
#include "arcitem.h"
#include "polylineitem.h"
#include "rectangleitem.h"
#include "document.h"
// #include "layer.h" - 图层通过document访问
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QColor>
#include <QPalette>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    setupGeometryWidgets();
}

void PropertyPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(2, 2, 2, 2);
    m_mainLayout->setSpacing(2);

    m_emptyLabel = new QLabel("选择对象以查看特性", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #666; padding: 20px; font-size: 12px;");

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; }");

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet("background: #252526;");
    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(4, 4, 4, 4);
    contentLayout->setSpacing(6);

    // 类型显示
    m_typeLabel = new QLabel(this);
    m_typeLabel->setStyleSheet("color: #fff; font-weight: bold; font-size: 13px; padding: 4px;");
    contentLayout->addWidget(m_typeLabel);

    // 常规属性组
    QGroupBox *generalGroup = new QGroupBox("常规", this);
    generalGroup->setStyleSheet("QGroupBox { color: #ccc; border: 1px solid #3c3c3c; border-radius: 4px; margin-top: 8px; padding-top: 8px; } QGroupBox::title { subcontrol-origin: margin; left: 6px; padding: 0 4px; }");
    QFormLayout *generalLayout = new QFormLayout(generalGroup);
    generalLayout->setContentsMargins(6, 8, 6, 6);
    generalLayout->setSpacing(4);

    // 图层
    m_layerCombo = new QComboBox(this);
    m_layerCombo->setEditable(true);
    m_layerCombo->setStyleSheet("QComboBox { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 2px; } QComboBox::drop-down { border: none; }");
    generalLayout->addRow("图层:", m_layerCombo);

    // 颜色
    QHBoxLayout *colorLayout = new QHBoxLayout();
    m_colorButton = new QPushButton(this);
    m_colorButton->setFixedSize(60, 22);
    m_colorButton->setStyleSheet("border: 1px solid #555;");
    colorLayout->addWidget(m_colorButton);
    colorLayout->addStretch();
    generalLayout->addRow("颜色:", colorLayout);

    // ByLayer
    m_colorByLayerCheck = new QCheckBox("ByLayer", this);
    m_colorByLayerCheck->setStyleSheet("color: #ccc;");
    generalLayout->addRow("", m_colorByLayerCheck);

    // 线宽
    m_lineWidthCombo = new QComboBox(this);
    m_lineWidthCombo->addItems({"0.00", "0.05", "0.09", "0.13", "0.15", "0.18", "0.20", "0.25", "0.30", "0.35", "0.40", "0.50", "0.53", "0.60", "0.70", "0.80", "0.90", "1.00", "1.06", "1.20", "1.40", "1.58", "2.00", "2.11"});
    m_lineWidthCombo->setStyleSheet("QComboBox { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 2px; }");
    generalLayout->addRow("线宽:", m_lineWidthCombo);

    // 线型
    m_lineTypeCombo = new QComboBox(this);
    m_lineTypeCombo->addItems({"Continuous", "Dashed", "Dotted", "DashDot", "Center", "Hidden", "ByLayer"});
    m_lineTypeCombo->setStyleSheet("QComboBox { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 2px; }");
    generalLayout->addRow("线型:", m_lineTypeCombo);

    contentLayout->addWidget(generalGroup);

    // 几何属性组
    m_geometryGroup = new QGroupBox("几何", this);
    m_geometryGroup->setStyleSheet("QGroupBox { color: #ccc; border: 1px solid #3c3c3c; border-radius: 4px; margin-top: 8px; padding-top: 8px; } QGroupBox::title { subcontrol-origin: margin; left: 6px; padding: 0 4px; }");
    m_geometryLayout = new QFormLayout(m_geometryGroup);
    m_geometryLayout->setContentsMargins(6, 8, 6, 6);
    m_geometryLayout->setSpacing(4);
    contentLayout->addWidget(m_geometryGroup);

    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    m_mainLayout->addWidget(m_emptyLabel);
    m_mainLayout->addWidget(m_scrollArea);
    m_scrollArea->hide();

    // 连接信号
    connect(m_layerCombo, &QComboBox::currentTextChanged, this, &PropertyPanel::onLayerChanged);
    connect(m_colorButton, &QPushButton::clicked, this, &PropertyPanel::onColorChanged);
    connect(m_lineWidthCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertyPanel::onLineWidthChanged);
    connect(m_lineTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PropertyPanel::onLineTypeChanged);
    connect(m_colorByLayerCheck, &QCheckBox::stateChanged, this, &PropertyPanel::onColorByLayerChanged);
}

void PropertyPanel::setupGeometryWidgets()
{
    auto createSpin = [this](const QString &name) -> QDoubleSpinBox* {
        QDoubleSpinBox *spin = new QDoubleSpinBox(this);
        spin->setRange(-1000000, 1000000);
        spin->setDecimals(3);
        spin->setSingleStep(1.0);
        spin->setStyleSheet("QDoubleSpinBox { background: #3c3c3c; color: #fff; border: 1px solid #555; padding: 1px; }");
        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertyPanel::onGeometryChanged);
        return spin;
    };

    // 位置
    m_xSpin = createSpin("X");
    m_ySpin = createSpin("Y");
    m_geometryLayout->addRow("X:", m_xSpin);
    m_geometryLayout->addRow("Y:", m_ySpin);

    // 直线
    m_startXSpin = createSpin("起点X");
    m_startYSpin = createSpin("起点Y");
    m_endXSpin = createSpin("终点X");
    m_endYSpin = createSpin("终点Y");
    m_geometryLayout->addRow("起点X:", m_startXSpin);
    m_geometryLayout->addRow("起点Y:", m_startYSpin);
    m_geometryLayout->addRow("终点X:", m_endXSpin);
    m_geometryLayout->addRow("终点Y:", m_endYSpin);

    // 圆/弧
    m_centerXSpin = createSpin("圆心X");
    m_centerYSpin = createSpin("圆心Y");
    m_radiusSpin = createSpin("半径");
    m_geometryLayout->addRow("圆心X:", m_centerXSpin);
    m_geometryLayout->addRow("圆心Y:", m_centerYSpin);
    m_geometryLayout->addRow("半径:", m_radiusSpin);

    // 弧
    m_startAngleSpin = createSpin("起始角");
    m_startAngleSpin->setRange(-360, 360);
    m_spanAngleSpin = createSpin("跨角");
    m_spanAngleSpin->setRange(-360, 360);
    m_geometryLayout->addRow("起始角:", m_startAngleSpin);
    m_geometryLayout->addRow("跨角:", m_spanAngleSpin);

    // 矩形
    m_widthSpin = createSpin("宽度");
    m_heightSpin = createSpin("高度");
    m_geometryLayout->addRow("宽度:", m_widthSpin);
    m_geometryLayout->addRow("高度:", m_heightSpin);
}

void PropertyPanel::setScene(CadScene *scene)
{
    m_scene = scene;
    if (m_scene) {
        connect(m_scene, &CadScene::selectionChangedCount, this, &PropertyPanel::updateProperties);
    }
    updateProperties();
}

void PropertyPanel::updateProperties()
{
    if (!m_scene) return;
    m_selectedItems = m_scene->selectedCadItems();

    if (m_selectedItems.isEmpty()) {
        m_emptyLabel->show();
        m_scrollArea->hide();
        return;
    }

    m_emptyLabel->hide();
    m_scrollArea->show();
    m_updating = true;

    if (m_selectedItems.size() == 1) {
        CadItem *item = m_selectedItems.first();
        m_typeLabel->setText(item->entityType());

        // 图层
        m_layerCombo->blockSignals(true);
        m_layerCombo->clear();
        if (m_scene->document()) {
            for (auto &layer : m_scene->document()->getAllLayers()) {
                m_layerCombo->addItem(layer.name);
            }
        }
        m_layerCombo->setCurrentText(item->layer());
        m_layerCombo->blockSignals(false);

        // 颜色
        m_colorByLayerCheck->blockSignals(true);
        m_colorByLayerCheck->setChecked(item->isColorByLayer());
        m_colorByLayerCheck->blockSignals(false);
        setColorButtonColor(item->effectiveColor());

        // 线宽
        m_lineWidthCombo->blockSignals(true);
        QString lwStr = QString::number(item->lineWidth(), 'f', 2);
        int idx = m_lineWidthCombo->findText(lwStr);
        m_lineWidthCombo->setCurrentIndex(idx >= 0 ? idx : 7); // 默认0.25
        m_lineWidthCombo->blockSignals(false);

        // 几何属性
        updateGeometryVisibility(item);
        loadGeometryValues(item);
    } else {
        m_typeLabel->setText(QString("已选择 %1 个对象").arg(m_selectedItems.size()));
        m_geometryGroup->hide();
    }

    m_updating = false;
}

void PropertyPanel::updateGeometryVisibility(CadItem *item)
{
    // 先隐藏所有
    for (int i = 0; i < m_geometryLayout->rowCount(); i++) {
        m_geometryLayout->itemAt(i, QFormLayout::LabelRole)->widget()->hide();
        m_geometryLayout->itemAt(i, QFormLayout::FieldRole)->widget()->hide();
    }
    m_geometryGroup->show();

    // 位置（所有图元都有）
    m_geometryLayout->labelForField(m_xSpin)->show();
    m_xSpin->show();
    m_geometryLayout->labelForField(m_ySpin)->show();
    m_ySpin->show();

    if (dynamic_cast<LineItem*>(item)) {
        m_geometryLayout->labelForField(m_startXSpin)->show(); m_startXSpin->show();
        m_geometryLayout->labelForField(m_startYSpin)->show(); m_startYSpin->show();
        m_geometryLayout->labelForField(m_endXSpin)->show(); m_endXSpin->show();
        m_geometryLayout->labelForField(m_endYSpin)->show(); m_endYSpin->show();
    } else if (dynamic_cast<CircleItem*>(item)) {
        m_geometryLayout->labelForField(m_centerXSpin)->show(); m_centerXSpin->show();
        m_geometryLayout->labelForField(m_centerYSpin)->show(); m_centerYSpin->show();
        m_geometryLayout->labelForField(m_radiusSpin)->show(); m_radiusSpin->show();
    } else if (dynamic_cast<ArcItem*>(item)) {
        m_geometryLayout->labelForField(m_centerXSpin)->show(); m_centerXSpin->show();
        m_geometryLayout->labelForField(m_centerYSpin)->show(); m_centerYSpin->show();
        m_geometryLayout->labelForField(m_radiusSpin)->show(); m_radiusSpin->show();
        m_geometryLayout->labelForField(m_startAngleSpin)->show(); m_startAngleSpin->show();
        m_geometryLayout->labelForField(m_spanAngleSpin)->show(); m_spanAngleSpin->show();
    } else if (dynamic_cast<RectangleItem*>(item)) {
        m_geometryLayout->labelForField(m_widthSpin)->show(); m_widthSpin->show();
        m_geometryLayout->labelForField(m_heightSpin)->show(); m_heightSpin->show();
    }
}

void PropertyPanel::loadGeometryValues(CadItem *item)
{
    QPointF pos = item->pos();
    m_xSpin->setValue(pos.x());
    m_ySpin->setValue(pos.y());

    if (auto line = dynamic_cast<LineItem*>(item)) {
        m_startXSpin->setValue(line->startPoint().x());
        m_startYSpin->setValue(line->startPoint().y());
        m_endXSpin->setValue(line->endPoint().x());
        m_endYSpin->setValue(line->endPoint().y());
    } else if (auto circle = dynamic_cast<CircleItem*>(item)) {
        m_centerXSpin->setValue(circle->centerPoint().x());
        m_centerYSpin->setValue(circle->centerPoint().y());
        m_radiusSpin->setValue(circle->radius());
    } else if (auto arc = dynamic_cast<ArcItem*>(item)) {
        m_centerXSpin->setValue(arc->centerPoint().x());
        m_centerYSpin->setValue(arc->centerPoint().y());
        m_radiusSpin->setValue(arc->radius());
        m_startAngleSpin->setValue(arc->startAngle() * 180 / M_PI);
        m_spanAngleSpin->setValue(arc->spanAngle() * 180 / M_PI);
    } else if (auto rect = dynamic_cast<RectangleItem*>(item)) {
        m_widthSpin->setValue(rect->rectangle().width());
        m_heightSpin->setValue(rect->rectangle().height());
    }
}

void PropertyPanel::applyGeometryChanges(CadItem *item)
{
    item->setPos(m_xSpin->value(), m_ySpin->value());

    if (auto line = dynamic_cast<LineItem*>(item)) {
        line->setLine(QPointF(m_startXSpin->value(), m_startYSpin->value()),
                      QPointF(m_endXSpin->value(), m_endYSpin->value()));
    } else if (auto circle = dynamic_cast<CircleItem*>(item)) {
        circle->setCircle(QPointF(m_centerXSpin->value(), m_centerYSpin->value()),
                          m_radiusSpin->value());
    } else if (auto arc = dynamic_cast<ArcItem*>(item)) {
        arc->setArc(QPointF(m_centerXSpin->value(), m_centerYSpin->value()),
                    m_radiusSpin->value(),
                    m_startAngleSpin->value() * M_PI / 180,
                    m_spanAngleSpin->value() * M_PI / 180);
    } else if (auto rect = dynamic_cast<RectangleItem*>(item)) {
        QRectF r = rect->rectangle();
        r.setWidth(m_widthSpin->value());
        r.setHeight(m_heightSpin->value());
        rect->setRectangle(r);
    }
}

void PropertyPanel::onLayerChanged(const QString &layer)
{
    if (m_updating) return;
    for (auto item : m_selectedItems) {
        item->setLayer(layer);
        item->update();
    }
}

void PropertyPanel::onColorChanged()
{
    if (m_updating || m_selectedItems.isEmpty()) return;
    QColor initial = m_selectedItems.first()->effectiveColor();
    QColor color = QColorDialog::getColor(initial, this, "选择颜色");
    if (color.isValid()) {
        for (auto item : m_selectedItems) {
            item->setColor(color);
            item->setColorByLayer(false);
            item->update();
        }
        setColorButtonColor(color);
        m_colorByLayerCheck->blockSignals(true);
        m_colorByLayerCheck->setChecked(false);
        m_colorByLayerCheck->blockSignals(false);
    }
}

void PropertyPanel::onLineWidthChanged(int index)
{
    if (m_updating) return;
    qreal width = m_lineWidthCombo->itemText(index).toDouble();
    for (auto item : m_selectedItems) {
        item->setLineWidth(width);
        item->update();
    }
}

void PropertyPanel::onLineTypeChanged(int index)
{
    Q_UNUSED(index);
    // 线型暂未实现到图元，预留接口
}

void PropertyPanel::onColorByLayerChanged(int state)
{
    if (m_updating) return;
    bool byLayer = (state == Qt::Checked);
    for (auto item : m_selectedItems) {
        item->setColorByLayer(byLayer);
        item->update();
    }
    if (!m_selectedItems.isEmpty()) {
        setColorButtonColor(m_selectedItems.first()->effectiveColor());
    }
}

void PropertyPanel::onGeometryChanged()
{
    if (m_updating || m_selectedItems.size() != 1) return;
    applyGeometryChanges(m_selectedItems.first());
}

QColor PropertyPanel::currentColor() const
{
    return m_colorButton->palette().button().color();
}

void PropertyPanel::setColorButtonColor(const QColor &color)
{
    m_colorButton->setStyleSheet(QString("background-color: %1; border: 1px solid #555;").arg(color.name()));
    m_colorButton->setText(color.name());
}
