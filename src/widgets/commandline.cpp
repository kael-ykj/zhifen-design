#include "commandline.h"
#include <QKeyEvent>

CommandLine::CommandLine(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_history = new QTextEdit(this);
    m_history->setReadOnly(true);
    m_history->setMaximumHeight(80);
    m_history->setStyleSheet("background: #1e1e1e; color: #ccc; border: none; font-family: Consolas, monospace; font-size: 12px;");

    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("输入命令...");
    m_input->setStyleSheet("background: #252526; color: #fff; border: 1px solid #3c3c3c; padding: 4px 8px; font-family: Consolas, monospace; font-size: 12px;");

    layout->addWidget(m_history);
    layout->addWidget(m_input);

    connect(m_input, &QLineEdit::returnPressed, this, &CommandLine::onReturnPressed);
}

void CommandLine::appendMessage(const QString &message, const QString &type)
{
    QString color = "#ccc";
    if (type == "error") color = "#f44747";
    else if (type == "result") color = "#4ec9b0";
    else if (type == "command") color = "#569cd6";
    m_history->append(QString("<span style='color:%1'>%2</span>").arg(color, message.toHtmlEscaped()));
}

void CommandLine::clear()
{
    m_history->clear();
}

void CommandLine::onReturnPressed()
{
    QString cmd = m_input->text().trimmed();
    if (!cmd.isEmpty()) {
        m_historyList.append(cmd);
        m_historyIndex = m_historyList.size();
        appendMessage(cmd, "command");
        emit commandEntered(cmd);
    }
    m_input->clear();
}
