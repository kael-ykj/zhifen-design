#pragma once
#include <QDialog>
#include <QPainter>
#include <QScrollArea>
#include <QWidget>
#include <QPixmap>
#include "core/zf_types.h"

class SystemDiagramView : public QWidget
{
    Q_OBJECT
public:
    explicit SystemDiagramView(const zf::SystemDiagram& diagram, QWidget *parent = nullptr);
    QSize sizeHint() const override;
    QPixmap exportToImage(int margin = 40);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    zf::SystemDiagram m_diagram;
    double m_scale{1.0};
    QColor nodeColor(zf::NodeType type) const;
    QString nodeLabel(zf::NodeType type) const;
    void drawDiagram(QPainter& painter, const QRect& rect);
};

class SystemDiagramDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SystemDiagramDialog(const zf::SystemDiagram& diagram, QWidget *parent = nullptr);

private slots:
    void onExportImage();
    void onPrint();

private:
    SystemDiagramView* m_view;
};
