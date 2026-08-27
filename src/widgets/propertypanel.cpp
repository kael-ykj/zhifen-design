#include "propertypanel.h"
#include "cadscene.h"
#include "caditem.h"
#include <QVBoxLayout>
#include <QScrollArea>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    m_emptyLabel = new QLabel("选择对象以查看特性", this);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet("color: #666; padding: 20px;");

    m_contentWidget = new QWidget(this);
    m_formLayout = new QFormLayout(m_contentWidget);
    m_formLayout->setContentsMargins(4, 4, 4, 4);
    m_formLayout->setSpacing(4);

    m_typeLabel = new QLabel(this);
    m_layerLabel = new QLabel(this);
    m_xLabel = new QLabel(this);
    m_yLabel = new QLabel(this);
    m_widthLabel = new QLabel(this);
    m_heightLabel = new QLabel(this);

    m_formLayout->addRow("类型:", m_typeLabel);
    m_formLayout->addRow("图层:", m_layerLabel);
    m_formLayout->addRow("X:", m_xLabel);
    m_formLayout->addRow("Y:", m_yLabel);
    m_formLayout->addRow("宽度:", m_widthLabel);
    m_formLayout->addRow("高度:", m_heightLabel);

    for (int i = 0; i < m_formLayout->rowCount(); i++) {
        QLabel *label = qobject_cast<QLabel*>(m_formLayout->labelForField(m_formLayout->itemAt(i, QFormLayout::FieldRole)->widget()));
        if (label) label->setStyleSheet("color: #999;");
    }

    mainLayout->addWidget(m_emptyLabel);
    mainLayout->addWidget(m_contentWidget);
    m_contentWidget->hide();
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
    QList<CadItem*> selected = m_scene->selectedCadItems();

    if (selected.isEmpty()) {
        m_emptyLabel->show();
        m_contentWidget->hide();
        return;
    }

    m_emptyLabel->hide();
    m_contentWidget->show();

    if (selected.size() == 1) {
        CadItem *item = selected.first();
        m_typeLabel->setText(item->entityType());
        m_layerLabel->setText(item->layer());
        QRectF b = item->boundingRect();
        m_xLabel->setText(QString::number(b.x(), 'f', 2));
        m_yLabel->setText(QString::number(b.y(), 'f', 2));
        m_widthLabel->setText(QString::number(b.width(), 'f', 2));
        m_heightLabel->setText(QString::number(b.height(), 'f', 2));
    } else {
        m_typeLabel->setText(QString("已选择 %1 个对象").arg(selected.size()));
        m_layerLabel->setText("-");
        m_xLabel->setText("-");
        m_yLabel->setText("-");
        m_widthLabel->setText("-");
        m_heightLabel->setText("-");
    }
}
