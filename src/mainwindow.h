#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QUndoStack>

class CadView;
class CadScene;
class Document;
class Tool;
class CommandLine;
class LayerPanel;
class PropertyPanel;
class DevicePanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onImportDxf();
    void onExportDxf();
    void onPrint();
    void onUndo();
    void onRedo();
    void onZoomExtents();
    void onZoomIn();
    void onZoomOut();
    void onToggleGrid();
    void onToggleSnap();
    void onToggleOrtho();
    void onCoordinateChanged(const QPointF &pos);
    void onCommandEntered(const QString &command);
    void onToolFinished();
    void onSelectionChanged(int count);
    void setCurrentTool(const QString &toolName);

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void createStatusBar();
    void updateTitle();

    CadView *m_view;
    CadScene *m_scene;
    Document *m_document;
    Tool *m_currentTool;
    CommandLine *m_commandLine;
    LayerPanel *m_layerPanel;
    PropertyPanel *m_propertyPanel;
    DevicePanel *m_devicePanel;

    // Actions
    QAction *m_newAct, *m_openAct, *m_saveAct, *m_saveAsAct;
    QAction *m_importDxfAct, *m_exportDxfAct, *m_printAct, *m_exitAct;
    QAction *m_undoAct, *m_redoAct;
    QAction *m_selectAct, *m_lineAct, *m_circleAct, *m_arcAct, *m_polylineAct, *m_rectangleAct;
    QAction *m_textAct, *m_dimLinearAct, *m_dimAlignedAct, *m_dimRadiusAct, *m_dimDiameterAct, *m_dimAngularAct;
    QAction *m_moveAct, *m_copyAct, *m_rotateAct, *m_scaleAct, *m_mirrorAct, *m_explodeAct, *m_eraseAct;
    QAction *m_panAct, *m_zoomAct, *m_zoomExtentsAct;
    QAction *m_gridAct, *m_snapAct, *m_orthoAct;
    QAction *m_layerManagerAct;

    QToolBar *m_drawToolBar;
    QToolBar *m_editToolBar;
    QToolBar *m_viewToolBar;
    QStatusBar *m_statusBar;
    QLabel *m_coordLabel;
    QLabel *m_toolLabel;
    QLabel *m_entityCountLabel;
    QLabel *m_selectedLabel;

    QActionGroup *m_toolGroup;
    QUndoStack *m_undoStack;
};

#endif // MAINWINDOW_H
