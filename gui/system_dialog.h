#pragma once
#include <QDialog>
#include <QPainter>
#include <QScrollArea>
#include <QWidget>
#include "core/zf_types.h"

class SystemDiagramView : public QWidget
{
    Q_OBJECT
public:
    explicit SystemDiagramView(const zf::SystemDiagram& diagram, QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    zf::SystemDiagram m_diagram;
    double m_scale{1.0};
    QColor nodeColor(zf::NodeType type) const;
    QString nodeLabel(zf::NodeType type) const;
};

class SystemDiagramDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SystemDiagramDialog(const zf::SystemDiagram& diagram, QWidget *parent = nullptr);
};
