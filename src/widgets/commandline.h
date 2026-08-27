#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>

class CommandLine : public QWidget
{
    Q_OBJECT
public:
    explicit CommandLine(QWidget *parent = nullptr);

    void appendMessage(const QString &message, const QString &type = "");
    void clear();

signals:
    void commandEntered(const QString &command);

private slots:
    void onReturnPressed();

private:
    QTextEdit *m_history;
    QLineEdit *m_input;
    QStringList m_historyList;
    int m_historyIndex = -1;
};

#endif // COMMANDLINE_H
