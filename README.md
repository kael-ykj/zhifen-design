# 智分Design V3.1.0-FINAL

室内分布系统专业设计软件（P0内核演示版）

## 说明
- 仅供个人学习研究，不出售、不商用、不分发
- 当前版本为P0内核阶段，包含核心业务逻辑，控制台演示程序
- 完整GUI版本将在P1-P3阶段完成

## 已实现模块
- 双工作模式（草图/正式）隔离架构
- 器件库（天线/功分/耦合/合路/信源/馈线）
- Undo/Redo双事务栈
- 交互工具系统（放置/绘制/编辑复制）
- 2D图形引擎骨架（场景/视图/图层/捕捉）
- 项目JSON序列化持久化
- DWG双通道导出接口（草图/正式归档）
- 审计日志系统
- 告警小红标机制

## 编译
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 运行
```bash
./build/ZhiFenDesign
```
