#ifndef TOOL_H
#define TOOL_H

#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QPointF>
#include <QCursor>

class CadView;
class CadScene;

class Tool : public QObject
{
    Q_OBJECT
public:
    explicit Tool(CadView *view, QObject *parent = nullptr);
    virtual ~Tool();

    virtual QString name() const = 0;
    virtual QCursor cursor() const { return Qt::CrossCursor; }

    virtual void mousePressEvent(QMouseEvent *event) = 0;
    virtual void mouseMoveEvent(QMouseEvent *event) = 0;
    virtual void mouseReleaseEvent(QMouseEvent *event) = 0;
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event) {}
    virtual void drawOverlay(QPainter *painter) {}

    virtual void activate() {}
    virtual void deactivate() {}

signals:
    void finished();
    void statusMessage(const QString &message);

protected:
    CadView *m_view;
    CadScene *m_scene;
    QPointF m_lastWorldPos;
};

#endif // TOOL_H
