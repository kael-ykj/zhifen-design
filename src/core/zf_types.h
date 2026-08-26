#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <variant>
#include <cstdint>
#include <cmath>
#include <functional>
#include <chrono>
#include <atomic>

namespace zf {

// ========= 工作模式 =========
enum class WorkMode : int {
    SKETCH_MODE = 0,
    FORMAL_MODE = 1
};

// ========= 复制模式 =========
enum class CopyDuplicateMode : int {
    LIGHT_COPY = 0,
    FULL_COPY = 1
};

// ========= 审计日志 =========
struct AuditEntry {
    std::string entryId;
    std::chrono::system_clock::time_point timestamp;
    std::string operatorInfo;
    std::string actionType;
    std::vector<std::string> targetObjectIds;
    std::string comment;
    bool isHighRisk{false};
};

// ========= 基础几何 =========
struct Point2D { double x{0}, y{0}; };
struct Point3D { double x{0}, y{0}, z{0}; };
struct Size2D { double w{0}, h{0}; };
struct Rect2D { Point2D origin; Size2D size; };
struct BBox3D { Point3D min; Point3D max; };
struct LineSegment2D { Point2D p0; Point2D p1; };
struct LineSegment3D { Point3D p0; Point3D p1; };
struct Polygon2D { std::vector<Point2D> pts; };
struct Plane3D { Point3D o; Point3D n; };
struct Color { uint8_t r{0}, g{0}, b{0}, a{255}; };

// ========= 业务枚举 =========
enum class DeviceCategory : int {
    UNKNOWN = 0,
    ANTENNA = 1,
    SPLITTER = 2,
    COUPLER = 3,
    COMBINER = 4,
    SIGNAL_SOURCE = 5,
    CABLE = 6,
    LOAD = 7
};

enum class AntennaType : int { OMNI = 0, DIRECTIONAL = 1 };
enum class CableType : int { CABLE_1_2 = 0, CABLE_7_8 = 1, CABLE_5_4 = 2 };
enum class WallMaterial : int { CONCRETE = 0, BRICK = 1, GLASS = 2, METAL = 3, DRYWALL = 4 };
enum class RoomType : int { OFFICE = 0, CORRIDOR = 1, STAIR = 2, ELEVATOR = 3, RESTROOM = 4 };
enum class NodeType : int { SOURCE = 0, SPLITTER = 1, COUPLER = 2, ANTENNA = 3, LOAD = 4 };
enum class LinkStatus : int { NOT_CALCULATED = 0, OK = 1, WARN = 2, ERROR = 3 };
enum class DistributionType : int { PASSIVE_DAS = 0, ACTIVE_DAS = 1, DIGITAL_DAS = 2, SMALL_CELL = 3, HYBRID = 4 };
enum class ToolType : int { SELECT = 0, PAN = 1, DRAW_WALL = 2, PLACE_DEVICE = 3, DRAW_CABLE = 4, EDIT = 5, MEASURE = 6 };
enum class SnapMode : int { GRID = 0, ENDPOINT = 1, MIDPOINT = 2, INTERSECTION = 3, PORT = 4 };
enum class SimModelType : int { MULTI_WALL = 0, RAY_TRACING = 1 };
enum class PropagationModelType : int { FREE_SPACE = 0, MULTI_WALL = 1, COST_HATA = 2 };
enum class CheckSeverity : int { INFO = 0, WARNING = 1, ERROR = 2, CRITICAL = 3 };
enum class ReportType : int { PDF = 0, DWG = 1, EXCEL = 2, WORD = 3 };

// ========= 频段 =========
struct Band {
    std::string bandId;
    std::string name;
    int freqStartMHz{0};
    int freqEndMHz{0};
    std::string operatorName;
};

// ========= 端口 =========
struct Port {
    std::string portId;
    std::string label;
    bool isInput{false};
    double maxPower_dBm{30.0};
};

// ========= 器件模板（基础型号参数，不含工程实例数据） =========
struct DeviceModel {
    std::string modelId;
    std::string displayName;
    DeviceCategory category{DeviceCategory::UNKNOWN};
    int freqMinMHz{0};
    int freqMaxMHz{0};
    double gain_dBi{0.0};
    double vswr{1.5};
    double powerCapacity_W{50.0};
    double insertionLoss_dB{0.0};
    double couplingLoss_dB{0.0};
    double throughLoss_dB{0.0};
    double isolation_dB{0.0};
    int portCount{2};
    double lossPer100m_900MHz{0.0};
    double lossPer100m_2100MHz{0.0};
    double lineWidth{1.0};
    std::string symbolName;
    std::map<std::string, double> defaultParams;
};

// ========= 器件实例 =========
struct DeviceInstance {
    std::string instanceId;
    std::string modelId;
    std::string label;
    std::string userNote;
    Point2D position;
    double rotation{0.0};
    double elevation_m{0.0};
    std::string floorId;
    std::string roomId;
    std::string cellId;
    std::string sourceId;

    std::map<std::string, double> inputPower_dBm;
    std::map<std::string, double> outputPower_dBm;
    std::map<std::string, double> eirp_dBm;
    LinkStatus status{LinkStatus::NOT_CALCULATED};

    struct Connection {
        std::string targetInstanceId;
        std::string fromPortId;
        std::string toPortId;
        std::string cableSegmentId;
    };
    std::vector<Connection> connections;
    std::map<std::string, double> paramOverrides;

    std::string createdBy;
    std::string createdAt;
    bool locked{false};
    bool visible{true};
    bool sketchPlaceholder{false};
};

// ========= 馈线段 =========
struct CableSegment {
    std::string segmentId;
    std::string modelId;
    std::string floorId;
    std::vector<Point2D> routePoints;
    double length_m{0.0};
    std::string fromDeviceId;
    std::string toDeviceId;
    std::string fromPortId;
    std::string toPortId;
};

// ========= 建筑结构 =========
struct Wall {
    std::string wallId;
    WallMaterial material{WallMaterial::CONCRETE};
    double thickness_mm{200.0};
    double height_m{3.0};
    std::vector<Point2D> points;
    double attenuation_dB{0.0};
};

struct Door {
    std::string doorId;
    Point2D position;
    double width_m{0.9};
    double height_m{2.1};
};

struct Window {
    std::string windowId;
    Point2D position;
    double width_m{1.5};
    double height_m{1.2};
};

struct Column {
    std::string columnId;
    Point2D position;
    double width_m{0.5};
    double depth_m{0.5};
};

struct Furniture {
    std::string furnitureId;
    std::string name;
    Point2D position;
    double width_m{1.0};
    double depth_m{1.0};
    double height_m{1.0};
    double attenuation_dB{0.0};
};

struct Room {
    std::string roomId;
    std::string name;
    RoomType type{RoomType::OFFICE};
    std::vector<Point2D> polygon;
    double area_m2{0.0};
};

// ========= 图层 =========
enum class LayerType : int {
    BACKGROUND = 0,   // 底图
    WALL = 1,         // 墙体
    DEVICE = 2,       // 器件
    CABLE = 3,        // 馈线
    ANNOTATION = 4,   // 标注
    SYSTEM_DIAGRAM = 5, // 系统图
    HEATMAP = 6,      // 热力图
    AUXILIARY = 7,    // 辅助（网格/楼层分界）
    CUSTOM = 8        // 自定义
};

struct Layer {
    std::string layerId;
    std::string name;
    LayerType type{LayerType::CUSTOM};
    int color{0xFFFFFF};  // RGB
    bool visible{true};
    bool locked{false};
    bool frozen{false};
    double lineWidth{0.1}; // mm
    int order{0};
};

// ========= 打印窗口 =========
struct PrintWindow {
    std::string name;
    Point2D minPt;
    Point2D maxPt;
    std::string paperSize{"A1"};
    double scale{100.0}; // 1:100
    bool color{true};
};

// ========= 楼层 =========
struct Floor {
    std::string floorId;
    std::string floorName;
    int floorIndex{0};
    double elevation_m{0.0};
    double height_m{3.0};
    double netHeight_m{2.8};

    std::vector<Wall> walls;
    std::vector<Door> doors;
    std::vector<Window> windows;
    std::vector<Column> columns;
    std::vector<Room> rooms;
    std::vector<Furniture> furniture;

    std::vector<DeviceInstance> devices;
    std::vector<CableSegment> cables;

    std::string backgroundImage;
    double drawingScale{100.0};
    Point2D origin;
    double backgroundOpacity{0.5};
    bool backgroundGrayscale{true};
    bool backgroundLocked{true};

    std::string lastSimTimestamp;
    bool isStandardFloor{false};
    std::string referenceFloorId;
    std::map<std::string, std::string> metadata;

    std::vector<AuditEntry> floorAuditLog;
};

// ========= 系统图 =========
struct SystemNode {
    std::string nodeId;
    NodeType type{NodeType::SOURCE};
    std::string deviceInstanceId;
    Point2D layoutPos;
    std::string label;
};

struct SystemLink {
    std::string linkId;
    std::string fromNodeId;
    std::string toNodeId;
    std::string cableModelId;
    double length_m{0.0};
    double loss_dB{0.0};
};

struct SystemDiagram {
    std::string diagramId;
    std::string floorId;
    std::vector<SystemNode> nodes;
    std::vector<SystemLink> links;
};

// ========= 仿真结果 =========
struct CoverageGrid {
    std::string gridId;
    std::string floorId;
    std::string bandId;
    double resolution_m{1.0};
    int gridWidth{0};
    int gridHeight{0};
    Point2D origin;
    std::vector<double> rsrpValues;
};

struct CoverageStatistics {
    std::string floorId;
    std::string bandId;
    double areaTotal_m2{0.0};
    double areaCovered_m2{0.0};
    double coverageRate{0.0};
    double avgRsrp_dBm{-100.0};
    double weakCoverageArea_m2{0.0};
};

struct InterferenceReport {
    std::string reportId;
    std::string floorId;
    double avgSinr_dB{0.0};
    double interferenceArea_m2{0.0};
    std::vector<std::string> problemPoints;
};

struct SignalSourceConfig {
    std::string sourceId;
    std::string name;
    std::string deviceModelId;
    double txPower_dBm{20.0};
    std::vector<std::string> bandIds;
    std::string cellId;
};

// ========= 项目根对象 =========
struct Project {
    std::string projectId;
    std::string projectName;
    std::string operatorName;
    std::string scenarioType;
    DistributionType distType{DistributionType::PASSIVE_DAS};
    std::string designer;
    std::string description;

    std::vector<Floor> floors;
    std::vector<Layer> layers;
    std::vector<PrintWindow> printWindows;
    std::vector<Band> bands;
    std::vector<DeviceModel> deviceLibrary;
    std::vector<SignalSourceConfig> sources;
    std::vector<SystemDiagram> systemDiagrams;

    double coverageThreshold_dBm{-75.0};
    double coverageTarget{0.95};
    double maxAntennaPower_dBm{15.0};
    double maxFeederLoss_dB{3.0};
    double minSINR_dB{0.0};

    std::map<std::string, CoverageGrid> coverageResults;
    std::map<std::string, CoverageStatistics> coverageStats;
    std::vector<InterferenceReport> interferenceReports;

    int version{1};
    std::string createdAt;
    std::string modifiedAt;
    std::string filePath;
    std::map<std::string, std::string> metadata;

    WorkMode globalWorkMode{WorkMode::SKETCH_MODE};
    CopyDuplicateMode defaultCopyMode{CopyDuplicateMode::LIGHT_COPY};
    std::vector<AuditEntry> globalAuditLog;

    // 初始化默认8个图层
    void initDefaultLayers() {
        layers.clear();
        const char* names[] = {"底图", "墙体", "器件", "馈线", "标注", "系统图", "热力图", "辅助"};
        int colors[] = {0x808080, 0xFFFFFF, 0x00FF00, 0x00FFFF, 0x00FF00, 0x00FF00, 0xFF0000, 0xFF0000};
        bool visible[] = {true, true, true, true, true, true, false, false};
        bool locked[] = {true, false, false, false, false, false, false, false};
        for (int i = 0; i < 8; i++) {
            Layer l;
            l.layerId = "LAYER_" + std::to_string(i);
            l.name = names[i];
            l.type = static_cast<LayerType>(i);
            l.color = colors[i];
            l.visible = visible[i];
            l.locked = locked[i];
            l.order = i;
            layers.push_back(l);
        }
    }

    Layer* findLayer(LayerType type) {
        for (auto& l : layers) {
            if (l.type == type) return &l;
        }
        return nullptr;
    }

    bool isLayerVisible(LayerType type) const {
        for (const auto& l : layers) {
            if (l.type == type) return l.visible;
        }
        return true;
    }
};

} // namespace zf
