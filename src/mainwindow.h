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

namespace Zhifen { enum SystemDiagramMode; }
namespace Zhifen { enum PaperSize; }
namespace Zhifen { enum DimensionType; }

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
    void onImportBottomMap();
    void onBatchImport();
    void onFloorManager();
    void onPluginManager();
    void onExportDxf();
    void onExportDwgSketch();
    void onExportDwgFinal();
    void onPrint();
    void onExportPdf(Zhifen::PaperSize paper, bool formal);
    void onUndo();
    void onRedo();
    void onZoomExtents();
    void onLinkCalculation();
    void onBomReport();
    void onGenerateSystemDiagram(Zhifen::SystemDiagramMode mode);
    void onCoverageSimulation();
    void onSmartRoute();
    void onPowerBalance();
    void onElevatorTool();
    void onLeakyCableTool();
    void onBuildingToBuildingTool();
    void onAddDimension(Zhifen::DimensionType type);
    void onSheetSetManager();
    void onCreateBlock();
    void onInsertBlock();
    void onBlockManager();
    void onModelSpace();
    void onLayoutSpace();
    void onVersionManager();
    void onChangeLog();
    void onDesignReview();
    void onInterferenceAnalysis();
    void onCapacityPlanning();
    void onFrequencyPlanning();
    void onPerformanceSettings();
    void onPerformanceMonitor();
    void onPerformanceTest();
    void onImportTianyue();
    void onImportAIDP();
    void onImportDifu();
    void onExportTianyue();
    void onExportAIDP();
    void onAuditLog();
    void onToggleCopyMode();
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
    QAction *m_importDxfAct, *m_importBottomMapAct, *m_batchImportAct, *m_floorManagerAct, *m_pluginManagerAct, *m_exportDxfAct, *m_exportDwgSketchAct, *m_exportDwgFinalAct, *m_printAct, *m_exportPdfSketchAct, *m_exportPdfFormalAct, *m_exitAct;
    QAction *m_undoAct, *m_redoAct;
    QAction *m_selectAct, *m_lineAct, *m_circleAct, *m_arcAct, *m_polylineAct, *m_rectangleAct, *m_feederAct, *m_textAct;
    QAction *m_moveAct, *m_copyAct, *m_rotateAct, *m_scaleAct, *m_mirrorAct, *m_explodeAct, *m_offsetAct, *m_queryDistAct, *m_queryAreaAct, *m_queryPointAct, *m_linkCalcAct, *m_bomAct, *m_sysDiagramSketchAct, *m_sysDiagramFormalAct, *m_coverageSimAct, *m_smartRouteAct, *m_powerBalanceAct, *m_elevatorToolAct, *m_leakyCableToolAct, *m_b2bToolAct, *m_dimLinearAct, *m_dimAlignedAct, *m_dimRadiusAct, *m_dimDiameterAct, *m_dimAngularAct, *m_sheetSetAct, *m_createBlockAct, *m_insertBlockAct, *m_blockManagerAct, *m_modelSpaceAct, *m_layoutSpaceAct, *m_versionMgrAct, *m_changeLogAct, *m_reviewAct, *m_interferenceAct, *m_capacityAct, *m_frequencyAct, *m_perfSettingsAct, *m_perfMonitorAct, *m_perfTestAct, *m_importTianyueAct, *m_importAIDPAct, *m_importDifuAct, *m_exportTianyueAct, *m_exportAIDPAct, *m_auditLogAct, *m_copyModeAct, *m_eraseAct;
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
