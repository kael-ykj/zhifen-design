#include "tool.h"
#include "cadview.h"
#include "cadscene.h"

Tool::Tool(CadView *view, QObject *parent)
    : QObject(parent), m_view(view), m_scene(nullptr)
{
    if (m_view) {
        m_scene = qobject_cast<CadScene*>(m_view->scene());
    }
}

Tool::~Tool()
{
}

void Tool::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}

void Tool::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
}
