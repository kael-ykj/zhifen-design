#include "texttool.h"
#include "cadview.h"
#include "cadscene.h"
#include "entities/textitem.h"
#include <QMouseEvent>
#include <QPainter>
#include <QInputDialog>
TextTool::TextTool(CadView *view, QObject *parent) : Tool(view, parent) {}
void TextTool::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) { emit finished(); return; }
    if (event->button() != Qt::LeftButton) return;
    m_pos = m_view->mapToScene(event->pos());
    bool ok;
    QString text = QInputDialog::getText(nullptr, "输入文字", "文字内容:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty() && m_scene) {
        m_scene->addItem(new TextItem(m_pos, text, 2.5));
    }
    emit statusMessage("指定文字起点:");
}
void TextTool::mouseMoveEvent(QMouseEvent *event) { m_lastWorldPos = m_view->mapToScene(event->pos()); }
void TextTool::mouseReleaseEvent(QMouseEvent *) {}
void TextTool::keyPressEvent(QKeyEvent *event) { if (event->key()==Qt::Key_Escape){emit finished();event->accept();} }
void TextTool::drawOverlay(QPainter *) {}
void TextTool::deactivate() { m_hasPos = false; }
