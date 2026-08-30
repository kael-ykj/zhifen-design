#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QDockWidget>
#include <QToolBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QString>

namespace Zhifen {

class ToolBox : public QDockWidget
{
    Q_OBJECT
public:
    explicit ToolBox(QWidget *parent = nullptr);

signals:
    void toolSelected(const QString &toolName);

private:
    QToolBox *m_toolBox;

    QWidget *createDrawPage();
    QWidget *createEditPage();
    QWidget *createIndoorPage();
    QWidget *createDimensionPage();

    QToolButton *createToolButton(const QString &text, const QString &toolName);
};

} // namespace Zhifen

#endif // TOOLBOX_H
