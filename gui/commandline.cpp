#include "commandline.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>

CommandLine::CommandLine(QWidget *parent)
    : QDockWidget("命令行", parent)
{
    setAllowedAreas(Qt::BottomDockWidgetArea);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    QWidget* container = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(2);

    m_output = new QTextEdit(this);
    m_output->setReadOnly(true);
    m_output->setMaximumHeight(100);
    m_output->setStyleSheet("QTextEdit { background: #1e1e1e; color: #d4d4d4; font-family: Consolas, monospace; font-size: 11px; }");
    layout->addWidget(m_output);

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("输入命令 (ZOOM/PAN/PRINT/ERASE/LAYER/HELP)...");
    m_input->setStyleSheet("QLineEdit { background: #252526; color: #d4d4d4; font-family: Consolas, monospace; font-size: 12px; padding: 4px; }");
    m_input->installEventFilter(this);
    layout->addWidget(m_input);

    connect(m_input, &QLineEdit::returnPressed, this, &CommandLine::onReturnPressed);

    setWidget(container);
    setMinimumHeight(140);

    appendOutput("智分Design V3.1 命令行就绪。输入 HELP 查看可用命令。");
}

void CommandLine::appendOutput(const QString& text)
{
    m_output->append(text);
    m_output->verticalScrollBar()->setValue(m_output->verticalScrollBar()->maximum());
}

void CommandLine::clear()
{
    m_output->clear();
}

void CommandLine::onReturnPressed()
{
    QString cmd = m_input->text().trimmed();
    if (cmd.isEmpty()) return;

    appendOutput("> " + cmd);
    m_history.prepend(cmd);
    if (m_history.size() > 100) m_history.removeLast();
    m_historyIndex = -1;

    emit commandEntered(cmd);
    m_input->clear();
}

bool CommandLine::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_input && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Up) {
            if (m_historyIndex < m_history.size() - 1) {
                m_historyIndex++;
                m_input->setText(m_history[m_historyIndex]);
            }
            return true;
        } else if (keyEvent->key() == Qt::Key_Down) {
            if (m_historyIndex > 0) {
                m_historyIndex--;
                m_input->setText(m_history[m_historyIndex]);
            } else if (m_historyIndex == 0) {
                m_historyIndex = -1;
                m_input->clear();
            }
            return true;
        }
    }
    return QDockWidget::eventFilter(obj, event);
}
