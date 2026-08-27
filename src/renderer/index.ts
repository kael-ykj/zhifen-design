// 智分Design V3.1 - 渲染进程主逻辑
import { Document } from '../cad/core/Document';
import { CanvasRenderer } from '../cad/renderer/CanvasRenderer';
import { SnapManager } from '../cad/snap/SnapManager';
import { UndoRedoStack } from '../cad/core/UndoRedo';
import { Tool, ToolContext } from '../cad/tools/Tool';
import { SelectTool } from '../cad/tools/SelectTool';
import { LineTool } from '../cad/tools/LineTool';
import { CircleTool } from '../cad/tools/CircleTool';
import { PanTool } from '../cad/tools/PanTool';
import { ZoomTool } from '../cad/tools/ZoomTool';
import { DxfWriter } from '../cad/io/DxfWriter';
import { DxfReader } from '../cad/io/DxfReader';
import { point, Point } from '../cad/core/types';
import { Entity } from '../cad/entities/Entity';

const { ipcRenderer } = require('electron');

// 全局状态 - cadDoc是CAD文档对象，domDoc是浏览器document
let cadDoc: Document;
let renderer: CanvasRenderer;
let snapManager: SnapManager;
let undoStack: UndoRedoStack;
let currentTool: Tool | null = null;
let currentToolName: string = 'select';
let isPanning: boolean = false;
let panStart: Point | null = null;
let orthoMode: boolean = false;

// DOM元素
let canvas: HTMLCanvasElement;
let commandInput: HTMLInputElement;
let commandHistory: HTMLDivElement;
let statusMessage: HTMLSpanElement;
let statusTool: HTMLSpanElement;
let statusEntities: HTMLSpanElement;
let statusSelected: HTMLSpanElement;
let statusZoom: HTMLSpanElement;
let coordDisplay: HTMLSpanElement;
let layerSelect: HTMLSelectElement;
let layerList: HTMLDivElement;

// 初始化
function init(): void {
  const domDoc = window.document;
  canvas = domDoc.getElementById('cad-canvas') as HTMLCanvasElement;
  commandInput = domDoc.getElementById('command-input') as HTMLInputElement;
  commandHistory = domDoc.getElementById('command-history') as HTMLDivElement;
  statusMessage = domDoc.getElementById('status-message') as HTMLSpanElement;
  statusTool = domDoc.getElementById('status-tool') as HTMLSpanElement;
  statusEntities = domDoc.getElementById('status-entities') as HTMLSpanElement;
  statusSelected = domDoc.getElementById('status-selected') as HTMLSpanElement;
  statusZoom = domDoc.getElementById('status-zoom') as HTMLSpanElement;
  coordDisplay = domDoc.getElementById('coord-display') as HTMLSpanElement;
  layerSelect = domDoc.getElementById('layer-select') as HTMLSelectElement;
  layerList = domDoc.getElementById('layer-list') as HTMLDivElement;

  // 初始化文档
  cadDoc = new Document();
  cadDoc.name = '未命名';

  // 初始化渲染器
  renderer = new CanvasRenderer(canvas, cadDoc);
  resizeCanvas();

  // 初始化捕捉
  snapManager = new SnapManager(cadDoc);

  // 初始化撤销栈
  undoStack = new UndoRedoStack();

  // 设置默认工具
  setTool('select');

  // 绑定事件
  bindEvents();

  // 更新UI
  updateLayerUI();
  updateStatus();
  addCommandHistory('智分Design V3.1 已启动', 'result');
  addCommandHistory('输入命令或使用工具栏开始绘图', 'result');

  // 初始渲染
  render();
}

function resizeCanvas(): void {
  const container = canvas.parentElement!;
  canvas.width = container.clientWidth;
  canvas.height = container.clientHeight;
}

// 工具上下文
function getToolContext(): ToolContext {
  return {
    document: cadDoc,
    renderer,
    snapManager,
    onCommand: (cmd: string) => addCommandHistory(cmd, 'echo'),
    onStatus: (msg: string) => setStatus(msg),
    onRequestRender: () => render(),
  };
}

function setTool(toolName: string): void {
  if (currentTool) {
    currentTool.cancel();
  }
  currentToolName = toolName;
  const ctx = getToolContext();
  switch (toolName) {
    case 'select': currentTool = new SelectTool(ctx); break;
    case 'line': currentTool = new LineTool(ctx); break;
    case 'circle': currentTool = new CircleTool(ctx); break;
    case 'pan': currentTool = new PanTool(ctx); break;
    case 'zoom': currentTool = new ZoomTool(ctx); break;
    default: currentTool = new SelectTool(ctx); break;
  }
  // 更新工具栏按钮状态
  window.document.querySelectorAll('.tool-btn[data-tool]').forEach(btn => {
    btn.classList.toggle('active', (btn as HTMLElement).dataset.tool === toolName);
  });
  statusTool.textContent = `当前工具: ${getToolDisplayName(toolName)}`;
  setStatus(`${getToolDisplayName(toolName)}工具已激活`);
  canvas.style.cursor = currentTool.getCursor();
}

function getToolDisplayName(name: string): string {
  const names: Record<string, string> = {
    select: '选择', line: '直线', circle: '圆', arc: '圆弧',
    polyline: '多段线', rectangle: '矩形', text: '文字', dimension: '标注',
    move: '移动', copy: '复制', rotate: '旋转', scale: '缩放', erase: '删除',
    pan: '平移', zoom: '缩放',
  };
  return names[name] || name;
}

// 事件绑定
function bindEvents(): void {
  const domDoc = window.document;
  // 画布事件
  canvas.addEventListener('mousedown', onMouseDown);
  canvas.addEventListener('mousemove', onMouseMove);
  canvas.addEventListener('mouseup', onMouseUp);
  canvas.addEventListener('wheel', onWheel, { passive: false });
  canvas.addEventListener('contextmenu', (e) => e.preventDefault());
  canvas.addEventListener('dblclick', onDoubleClick);

  // 键盘事件
  window.addEventListener('keydown', onKeyDown);
  window.addEventListener('keyup', onKeyUp);

  // 窗口大小变化
  window.addEventListener('resize', () => {
    resizeCanvas();
    render();
  });

  // 工具栏按钮
  domDoc.querySelectorAll('.tool-btn[data-tool]').forEach(btn => {
    btn.addEventListener('click', () => {
      setTool((btn as HTMLElement).dataset.tool!);
    });
  });

  // 撤销/重做
  domDoc.getElementById('btn-undo')?.addEventListener('click', () => undo());
  domDoc.getElementById('btn-redo')?.addEventListener('click', () => redo());

  // 视图按钮
  domDoc.getElementById('btn-zoom-extents')?.addEventListener('click', () => {
    renderer.zoomExtents();
    render();
  });
  domDoc.getElementById('btn-zoom-window')?.addEventListener('click', () => setTool('zoom'));
  domDoc.getElementById('btn-pan')?.addEventListener('click', () => setTool('pan'));

  // 切换按钮
  domDoc.getElementById('btn-snap')?.addEventListener('click', (e) => {
    snapManager.setEnabled(!snapManager.enabled);
    (e.target as HTMLElement).closest('.tool-btn')?.classList.toggle('active', snapManager.enabled);
  });
  domDoc.getElementById('btn-grid')?.addEventListener('click', (e) => {
    renderer.showGrid = !renderer.showGrid;
    (e.target as HTMLElement).closest('.tool-btn')?.classList.toggle('active', renderer.showGrid);
    render();
  });
  domDoc.getElementById('btn-ortho')?.addEventListener('click', (e) => {
    orthoMode = !orthoMode;
    (e.target as HTMLElement).closest('.tool-btn')?.classList.toggle('active', orthoMode);
  });

  // 图层面板
  layerSelect?.addEventListener('change', (e) => {
    cadDoc.setCurrentLayer((e.target as HTMLSelectElement).value);
    updateLayerUI();
  });

  // 面板标签
  domDoc.querySelectorAll('.panel-tab').forEach(tab => {
    tab.addEventListener('click', () => {
      const panel = (tab as HTMLElement).dataset.panel!;
      const parent = tab.closest('.left-panel, .right-panel')!;
      parent.querySelectorAll('.panel-tab').forEach(t => t.classList.remove('active'));
      parent.querySelectorAll('.panel-page').forEach(p => p.classList.remove('active'));
      tab.classList.add('active');
      parent.querySelector(`#panel-${panel}`)?.classList.add('active');
    });
  });

  // 命令行
  commandInput?.addEventListener('keydown', (e) => {
    if (e.key === 'Enter') {
      executeCommand(commandInput.value);
      commandInput.value = '';
    }
    if (e.key === 'Escape') {
      commandInput.value = '';
      setTool('select');
    }
  });

  // IPC消息
  ipcRenderer.on('menu:new', () => newDocument());
  ipcRenderer.on('menu:open', () => openFile());
  ipcRenderer.on('menu:save', () => saveFile());
  ipcRenderer.on('menu:saveAs', () => saveFileAs());
  ipcRenderer.on('menu:exportDxf', () => exportDxf());
  ipcRenderer.on('menu:importDxf', () => importDxf());
  ipcRenderer.on('menu:undo', () => undo());
  ipcRenderer.on('menu:redo', () => redo());
  ipcRenderer.on('menu:line', () => setTool('line'));
  ipcRenderer.on('menu:circle', () => setTool('circle'));
  ipcRenderer.on('menu:zoomExtents', () => { renderer.zoomExtents(); render(); });
  ipcRenderer.on('menu:pan', () => setTool('pan'));
  ipcRenderer.on('menu:toggleSnap', () => {
    snapManager.setEnabled(!snapManager.enabled);
    domDoc.getElementById('btn-snap')?.classList.toggle('active', snapManager.enabled);
  });
  ipcRenderer.on('menu:toggleGrid', () => {
    renderer.showGrid = !renderer.showGrid;
    domDoc.getElementById('btn-grid')?.classList.toggle('active', renderer.showGrid);
    render();
  });
  ipcRenderer.on('menu:toggleOrtho', () => {
    orthoMode = !orthoMode;
    domDoc.getElementById('btn-ortho')?.classList.toggle('active', orthoMode);
  });
}

// 鼠标事件
function onMouseDown(e: MouseEvent): void {
  const rect = canvas.getBoundingClientRect();
  const screenPoint = point(e.clientX - rect.left, e.clientY - rect.top);
  const worldPoint = renderer.screenToWorld(screenPoint);

  // 中键平移
  if (e.button === 1) {
    isPanning = true;
    panStart = screenPoint;
    canvas.style.cursor = 'grabbing';
    return;
  }

  // 右键确认/取消
  if (e.button === 2) {
    if (currentTool && currentToolName !== 'select') {
      setTool('select');
    }
    return;
  }

  if (currentTool) {
    currentTool.onMouseDown(e, worldPoint);
  }
  updateStatus();
}

function onMouseMove(e: MouseEvent): void {
  const rect = canvas.getBoundingClientRect();
  const screenPoint = point(e.clientX - rect.left, e.clientY - rect.top);
  const worldPoint = renderer.screenToWorld(screenPoint);

  // 中键平移
  if (isPanning && panStart) {
    const dx = screenPoint.x - panStart.x;
    const dy = screenPoint.y - panStart.y;
    renderer.pan(dx, dy);
    panStart = screenPoint;
    render();
    return;
  }

  // 更新坐标显示
  coordDisplay.textContent = `X: ${worldPoint.x.toFixed(2)}  Y: ${worldPoint.y.toFixed(2)}`;

  if (currentTool) {
    currentTool.onMouseMove(e, worldPoint);
  }
  render();
}

function onMouseUp(e: MouseEvent): void {
  if (e.button === 1 && isPanning) {
    isPanning = false;
    panStart = null;
    canvas.style.cursor = currentTool?.getCursor() || 'crosshair';
    return;
  }

  const rect = canvas.getBoundingClientRect();
  const screenPoint = point(e.clientX - rect.left, e.clientY - rect.top);
  const worldPoint = renderer.screenToWorld(screenPoint);

  if (currentTool) {
    currentTool.onMouseUp(e, worldPoint);
    // 检查工具是否完成
    if (currentTool.finished && currentToolName !== 'select') {
      setTool('select');
    }
  }
  updateStatus();
}

function onWheel(e: WheelEvent): void {
  e.preventDefault();
  const rect = canvas.getBoundingClientRect();
  const screenPoint = point(e.clientX - rect.left, e.clientY - rect.top);
  const factor = e.deltaY < 0 ? 1.1 : 0.9;
  renderer.zoomAt(screenPoint, factor);
  render();
  updateStatus();
}

function onDoubleClick(e: MouseEvent): void {
  // 双击编辑文字等
}

// 键盘事件
function onKeyDown(e: KeyboardEvent): void {
  // 如果焦点在输入框，不处理快捷键
  if (e.target === commandInput) return;

  // 撤销/重做
  if (e.ctrlKey && e.key === 'z') { e.preventDefault(); undo(); return; }
  if (e.ctrlKey && e.key === 'y') { e.preventDefault(); redo(); return; }
  if (e.ctrlKey && e.key === 's') { e.preventDefault(); saveFile(); return; }
  if (e.ctrlKey && e.key === 'o') { e.preventDefault(); openFile(); return; }
  if (e.ctrlKey && e.key === 'n') { e.preventDefault(); newDocument(); return; }

  // F键切换
  if (e.key === 'F3') { snapManager.setEnabled(!snapManager.enabled); window.document.getElementById('btn-snap')?.classList.toggle('active', snapManager.enabled); return; }
  if (e.key === 'F7') { renderer.showGrid = !renderer.showGrid; window.document.getElementById('btn-grid')?.classList.toggle('active', renderer.showGrid); render(); return; }
  if (e.key === 'F8') { orthoMode = !orthoMode; window.document.getElementById('btn-ortho')?.classList.toggle('active', orthoMode); return; }

  // 空格=回车
  if (e.key === ' ' && currentToolName !== 'select') {
    e.preventDefault();
    setTool('select');
    return;
  }

  // Esc取消
  if (e.key === 'Escape') {
    if (currentToolName !== 'select') {
      setTool('select');
    } else {
      cadDoc.clearSelection();
      render();
    }
    return;
  }

  // 快捷键命令
  const key = e.key.toLowerCase();
  const shortcuts: Record<string, string> = {
    'l': 'line', 'c': 'circle', 'a': 'arc', 'pl': 'polyline',
    'rec': 'rectangle', 'm': 'move', 'co': 'copy', 'ro': 'rotate',
    'sc': 'scale', 'e': 'erase', 'tr': 'trim', 'ex': 'extend',
    'o': 'offset', 'mi': 'mirror', 'p': 'pan', 'z': 'zoom',
    'dt': 'text', 'mt': 'mtext', 'dli': 'dimLinear', 'dal': 'dimAligned',
    'di': 'dist', 'li': 'list', 'la': 'layerManager', 'b': 'block',
    'i': 'insert', 'h': 'hatch',
  };

  if (shortcuts[key] && !e.ctrlKey && !e.altKey) {
    e.preventDefault();
    const tool = shortcuts[key];
    if (['line', 'circle', 'arc', 'polyline', 'rectangle', 'pan', 'zoom'].includes(tool)) {
      setTool(tool);
      addCommandHistory(tool, 'echo');
    } else {
      addCommandHistory(`命令 '${tool}' 即将支持`, 'result');
    }
    return;
  }

  // 传递给当前工具
  if (currentTool) {
    currentTool.onKeyDown(e);
  }
}

function onKeyUp(e: KeyboardEvent): void {
  if (currentTool) {
    currentTool.onKeyUp(e);
  }
}

// 命令执行
function executeCommand(cmd: string): void {
  cmd = cmd.trim().toLowerCase();
  if (!cmd) return;
  addCommandHistory(cmd, 'echo');

  const commands: Record<string, () => void> = {
    'line': () => setTool('line'),
    'l': () => setTool('line'),
    'circle': () => setTool('circle'),
    'c': () => setTool('circle'),
    'pan': () => setTool('pan'),
    'p': () => setTool('pan'),
    'zoom': () => setTool('zoom'),
    'z': () => setTool('zoom'),
    'select': () => setTool('select'),
    'undo': () => undo(),
    'u': () => undo(),
    'redo': () => redo(),
    'regen': () => { renderer.zoomExtents(); render(); addCommandHistory('已重生成', 'result'); },
    're': () => { renderer.zoomExtents(); render(); },
    'zoomextents': () => { renderer.zoomExtents(); render(); },
    'ze': () => { renderer.zoomExtents(); render(); },
    'clear': () => { commandHistory.innerHTML = ''; },
    'help': () => { addCommandHistory('可用命令: line, circle, pan, zoom, undo, redo, regen, clear, help', 'result'); },
    'quit': () => { window.close(); },
    'exit': () => { window.close(); },
  };

  if (commands[cmd]) {
    commands[cmd]();
  } else {
    addCommandHistory(`未知命令: ${cmd}，输入 help 查看可用命令`, 'error');
  }
}

// 文件操作
function newDocument(): void {
  if (cadDoc.modified) {
    if (!confirm('当前文档已修改，是否保存？')) return;
    saveFile();
  }
  cadDoc = new Document();
  renderer.document = cadDoc;
  snapManager.document = cadDoc;
  renderer.zoomExtents();
  updateLayerUI();
  updateStatus();
  render();
  addCommandHistory('已创建新文档', 'result');
}

async function openFile(): Promise<void> {
  const result = await ipcRenderer.invoke('file:open');
  if (result.success) {
    try {
      if (result.filePath.endsWith('.dxf')) {
        const reader = new DxfReader(new Document());
        cadDoc = reader.read(result.content);
        renderer.document = cadDoc;
        snapManager.document = cadDoc;
      } else {
        const data = JSON.parse(result.content);
        // 加载项目文件
      }
      cadDoc.filePath = result.filePath;
      cadDoc.modified = false;
      renderer.zoomExtents();
      updateLayerUI();
      updateStatus();
      render();
      addCommandHistory(`已打开: ${result.filePath}`, 'result');
    } catch (err) {
      addCommandHistory(`打开文件失败: ${err}`, 'error');
    }
  }
}

async function saveFile(): Promise<void> {
  if (!cadDoc.filePath) {
    await saveFileAs();
    return;
  }
  const content = JSON.stringify(cadDoc.toJSON(), null, 2);
  const result = await ipcRenderer.invoke('file:save', { content, filePath: cadDoc.filePath });
  if (result.success) {
    cadDoc.modified = false;
    addCommandHistory(`已保存: ${result.filePath}`, 'result');
  }
}

async function saveFileAs(): Promise<void> {
  const content = JSON.stringify(cadDoc.toJSON(), null, 2);
  const result = await ipcRenderer.invoke('file:save', { content });
  if (result.success) {
    cadDoc.filePath = result.filePath;
    cadDoc.modified = false;
    addCommandHistory(`已保存: ${result.filePath}`, 'result');
  }
}

async function exportDxf(): Promise<void> {
  const writer = new DxfWriter(cadDoc);
  const content = writer.write();
  const result = await ipcRenderer.invoke('file:exportDxf', content);
  if (result.success) {
    addCommandHistory(`已导出DXF: ${result.filePath}`, 'result');
  }
}

async function importDxf(): Promise<void> {
  const result = await ipcRenderer.invoke('file:open');
  if (result.success && result.filePath.endsWith('.dxf')) {
    try {
      const reader = new DxfReader(new Document());
      cadDoc = reader.read(result.content);
      renderer.document = cadDoc;
      snapManager.document = cadDoc;
      renderer.zoomExtents();
      updateLayerUI();
      updateStatus();
      render();
      addCommandHistory(`已导入DXF: ${result.filePath}`, 'result');
    } catch (err) {
      addCommandHistory(`导入DXF失败: ${err}`, 'error');
    }
  }
}

// 撤销/重做
function undo(): void {
  addCommandHistory('撤销功能开发中', 'result');
}

function redo(): void {
  addCommandHistory('重做功能开发中', 'result');
}

// 渲染
function render(): void {
  renderer.render();
  // 绘制工具覆盖层
  if (currentTool && (currentTool as any).drawOverlay) {
    (currentTool as any).drawOverlay(renderer.ctx);
  }
  // 绘制捕捉标记
  if (snapManager.currentSnap) {
    const sp = renderer.worldToScreen(snapManager.currentSnap.point);
    snapManager.drawSnapMarker(renderer.ctx, sp, snapManager.currentSnap.type);
  }
}

// UI更新
function updateLayerUI(): void {
  const domDoc = window.document;
  // 更新下拉框
  if (layerSelect) {
    layerSelect.innerHTML = '';
    for (const layer of cadDoc.getAllLayers()) {
      const opt = domDoc.createElement('option');
      opt.value = layer.name;
      opt.textContent = layer.name;
      if (layer.name === cadDoc.currentLayer) opt.selected = true;
      layerSelect.appendChild(opt);
    }
  }
  // 更新图层列表
  if (layerList) {
    layerList.innerHTML = '';
    for (const layer of cadDoc.getAllLayers()) {
      const item = domDoc.createElement('div');
      item.className = 'layer-item' + (layer.name === cadDoc.currentLayer ? ' active' : '');
      item.innerHTML = `
        <span class="layer-visibility">${layer.visible ? '👁' : '🚫'}</span>
        <span class="layer-color" style="background:${layer.color}"></span>
        <span class="layer-name">${layer.name}</span>
      `;
      item.addEventListener('click', () => {
        cadDoc.setCurrentLayer(layer.name);
        updateLayerUI();
      });
      layerList.appendChild(item);
    }
  }
}

function updateStatus(): void {
  const allEntities = cadDoc.getAllEntities();
  const selected = allEntities.filter(e => e.selected);
  statusEntities.textContent = `对象: ${allEntities.length}`;
  statusSelected.textContent = `已选: ${selected.length}`;
  statusZoom.textContent = `缩放: ${(renderer.view.scale * 100).toFixed(0)}%`;

  // 更新特性面板
  const propsContent = window.document.getElementById('properties-content');
  if (propsContent) {
    if (selected.length === 0) {
      propsContent.innerHTML = '<p class="empty-hint">选择对象以查看特性</p>';
    } else if (selected.length === 1) {
      const e = selected[0];
      const b = e.getBounds();
      propsContent.innerHTML = `
        <div class="prop-row"><span class="prop-label">类型</span><span class="prop-value">${e.type}</span></div>
        <div class="prop-row"><span class="prop-label">图层</span><span class="prop-value">${e.layer}</span></div>
        <div class="prop-row"><span class="prop-label">颜色</span><span class="prop-value">${e.color}</span></div>
        <div class="prop-row"><span class="prop-label">X</span><span class="prop-value">${b.x.toFixed(2)}</span></div>
        <div class="prop-row"><span class="prop-label">Y</span><span class="prop-value">${b.y.toFixed(2)}</span></div>
        <div class="prop-row"><span class="prop-label">宽度</span><span class="prop-value">${b.width.toFixed(2)}</span></div>
        <div class="prop-row"><span class="prop-label">高度</span><span class="prop-value">${b.height.toFixed(2)}</span></div>
      `;
    } else {
      propsContent.innerHTML = `<p class="empty-hint">已选择 ${selected.length} 个对象</p>`;
    }
  }
}

function setStatus(msg: string): void {
  statusMessage.textContent = msg;
}

function addCommandHistory(text: string, type: string = ''): void {
  const domDoc = window.document;
  const div = domDoc.createElement('div');
  div.className = type ? `cmd-${type}` : '';
  div.textContent = text;
  commandHistory.appendChild(div);
  commandHistory.scrollTop = commandHistory.scrollHeight;
}

// 启动
window.addEventListener('DOMContentLoaded', init);
