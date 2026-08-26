#pragma once
#include <QDockWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QStringList>

class CommandLine : public QDockWidget
{
    Q_OBJECT
public:
    explicit CommandLine(QWidget *parent = nullptr);
    void appendOutput(const QString& text);
    void clear();

signals:
    void commandEntered(const QString& command);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onReturnPressed();

private:
    QTextEdit* m_output{nullptr};
    QLineEdit* m_input{nullptr};
    QStringList m_history;
    int m_historyIndex{-1};
};
