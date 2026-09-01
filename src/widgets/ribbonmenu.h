#ifndef RIBBONMENU_H
#define RIBBONMENU_H

#include <QTabWidget>
#include <QToolBar>
#include <QAction>
#include <QString>

namespace Zhifen {

class MainWindow;
using ::MainWindow;

class RibbonMenu : public QTabWidget
{
    Q_OBJECT
public:
    explicit RibbonMenu(QWidget *parent = nullptr);

    void setupActions(::MainWindow *mainWindow);

signals:
    void actionTriggered(const QString &actionName);

private:
    // 各个标签页
    QWidget *createHomeTab();
    QWidget *createDrawTab();
    QWidget *createEditTab();
    QWidget *createIndoorTab();
    QWidget *createDimensionTab();
    QWidget *createToolsTab();
    QWidget *createViewTab();
    QWidget *createOutputTab();

    // 工具组
    QWidget *createGroup(const QString &title, QWidget *parent);
    QToolBar *createToolBar(QWidget *parent);

    ::MainWindow *m_mainWindow = nullptr;

    // 常用动作
    QAction *m_actionLine = nullptr;
    QAction *m_actionCircle = nullptr;
    QAction *m_actionRect = nullptr;
    QAction *m_actionArc = nullptr;
    QAction *m_actionText = nullptr;
    QAction *m_actionMove = nullptr;
    QAction *m_actionCopy = nullptr;
    QAction *m_actionRotate = nullptr;
    QAction *m_actionScale = nullptr;
    QAction *m_actionMirror = nullptr;
    QAction *m_actionDelete = nullptr;
    QAction *m_actionUndo = nullptr;
    QAction *m_actionRedo = nullptr;
    QAction *m_actionAntenna = nullptr;
    QAction *m_actionFeeder = nullptr;
    QAction *m_actionCoupler = nullptr;
    QAction *m_actionSplitter = nullptr;
    QAction *m_actionSource = nullptr;
    QAction *m_actionDimLinear = nullptr;
    QAction *m_actionDimAligned = nullptr;
    QAction *m_actionDimRadius = nullptr;
    QAction *m_actionLayer = nullptr;
    QAction *m_actionBlock = nullptr;
    QAction *m_actionPrint = nullptr;
    QAction *m_actionExport = nullptr;
};

} // namespace Zhifen

#endif // RIBBONMENU_H
