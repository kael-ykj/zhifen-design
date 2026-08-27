import { app, BrowserWindow, Menu, ipcMain, dialog } from 'electron';
import * as path from 'path';
import * as fs from 'fs';

let mainWindow: BrowserWindow | null = null;

function createWindow(): void {
  mainWindow = new BrowserWindow({
    width: 1600,
    height: 900,
    minWidth: 1024,
    minHeight: 600,
    title: '智分Design V3.1 - 专业室分设计CAD软件',
    icon: path.join(__dirname, '../../build/icon.png'),
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false,
    },
  });

  mainWindow.loadFile(path.join(__dirname, '../renderer/index.html'));

  // 开发环境打开DevTools
  if (process.env.NODE_ENV === 'development') {
    mainWindow.webContents.openDevTools();
  }

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

function createMenu(): void {
  const template: Electron.MenuItemConstructorOptions[] = [
    {
      label: '文件',
      submenu: [
        { label: '新建', accelerator: 'Ctrl+N', click: () => mainWindow?.webContents.send('menu:new') },
        { label: '打开', accelerator: 'Ctrl+O', click: () => mainWindow?.webContents.send('menu:open') },
        { type: 'separator' },
        { label: '保存', accelerator: 'Ctrl+S', click: () => mainWindow?.webContents.send('menu:save') },
        { label: '另存为', accelerator: 'Ctrl+Shift+S', click: () => mainWindow?.webContents.send('menu:saveAs') },
        { type: 'separator' },
        { label: '导入DXF', click: () => mainWindow?.webContents.send('menu:importDxf') },
        { label: '导出DXF', click: () => mainWindow?.webContents.send('menu:exportDxf') },
        { label: '导出PDF', click: () => mainWindow?.webContents.send('menu:exportPdf') },
        { label: '打印', accelerator: 'Ctrl+P', click: () => mainWindow?.webContents.send('menu:print') },
        { type: 'separator' },
        { label: '退出', accelerator: 'Ctrl+Q', click: () => app.quit() },
      ],
    },
    {
      label: '编辑',
      submenu: [
        { label: '撤销', accelerator: 'Ctrl+Z', click: () => mainWindow?.webContents.send('menu:undo') },
        { label: '重做', accelerator: 'Ctrl+Y', click: () => mainWindow?.webContents.send('menu:redo') },
        { type: 'separator' },
        { label: '剪切', accelerator: 'Ctrl+X', click: () => mainWindow?.webContents.send('menu:cut') },
        { label: '复制', accelerator: 'Ctrl+C', click: () => mainWindow?.webContents.send('menu:copy') },
        { label: '粘贴', accelerator: 'Ctrl+V', click: () => mainWindow?.webContents.send('menu:paste') },
        { label: '删除', accelerator: 'Delete', click: () => mainWindow?.webContents.send('menu:erase') },
        { type: 'separator' },
        { label: '移动', accelerator: 'M', click: () => mainWindow?.webContents.send('menu:move') },
        { label: '复制', accelerator: 'CO', click: () => mainWindow?.webContents.send('menu:copyEntity') },
        { label: '旋转', accelerator: 'RO', click: () => mainWindow?.webContents.send('menu:rotate') },
        { label: '缩放', accelerator: 'SC', click: () => mainWindow?.webContents.send('menu:scaleEntity') },
        { label: '镜像', accelerator: 'MI', click: () => mainWindow?.webContents.send('menu:mirror') },
        { label: '偏移', accelerator: 'O', click: () => mainWindow?.webContents.send('menu:offset') },
        { label: '修剪', accelerator: 'TR', click: () => mainWindow?.webContents.send('menu:trim') },
        { label: '延伸', accelerator: 'EX', click: () => mainWindow?.webContents.send('menu:extend') },
      ],
    },
    {
      label: '视图',
      submenu: [
        { label: '重画', accelerator: 'R', click: () => mainWindow?.webContents.send('menu:redraw') },
        { label: '全部重生成', accelerator: 'RE', click: () => mainWindow?.webContents.send('menu:regen') },
        { type: 'separator' },
        { label: '缩放窗口', accelerator: 'Z', click: () => mainWindow?.webContents.send('menu:zoomWindow') },
        { label: '缩放到全部', accelerator: 'Z+E', click: () => mainWindow?.webContents.send('menu:zoomExtents') },
        { label: '平移', accelerator: 'P', click: () => mainWindow?.webContents.send('menu:pan') },
        { type: 'separator' },
        { label: '显示网格', accelerator: 'F7', click: () => mainWindow?.webContents.send('menu:toggleGrid') },
        { label: '对象捕捉', accelerator: 'F3', click: () => mainWindow?.webContents.send('menu:toggleSnap') },
        { label: '正交模式', accelerator: 'F8', click: () => mainWindow?.webContents.send('menu:toggleOrtho') },
        { type: 'separator' },
        { label: '全屏', accelerator: 'F11', click: () => mainWindow?.setFullScreen(!mainWindow.isFullScreen()) },
      ],
    },
    {
      label: '绘图',
      submenu: [
        { label: '直线', accelerator: 'L', click: () => mainWindow?.webContents.send('menu:line') },
        { label: '圆', accelerator: 'C', click: () => mainWindow?.webContents.send('menu:circle') },
        { label: '圆弧', accelerator: 'A', click: () => mainWindow?.webContents.send('menu:arc') },
        { label: '多段线', accelerator: 'PL', click: () => mainWindow?.webContents.send('menu:polyline') },
        { label: '矩形', accelerator: 'REC', click: () => mainWindow?.webContents.send('menu:rectangle') },
        { label: '正多边形', accelerator: 'POL', click: () => mainWindow?.webContents.send('menu:polygon') },
        { type: 'separator' },
        { label: '单行文字', accelerator: 'DT', click: () => mainWindow?.webContents.send('menu:text') },
        { label: '多行文字', accelerator: 'MT', click: () => mainWindow?.webContents.send('menu:mtext') },
        { type: 'separator' },
        { label: '线性标注', accelerator: 'DLI', click: () => mainWindow?.webContents.send('menu:dimLinear') },
        { label: '对齐标注', accelerator: 'DAL', click: () => mainWindow?.webContents.send('menu:dimAligned') },
        { label: '半径标注', accelerator: 'DRA', click: () => mainWindow?.webContents.send('menu:dimRadius') },
        { label: '直径标注', accelerator: 'DDI', click: () => mainWindow?.webContents.send('menu:dimDiameter') },
        { type: 'separator' },
        { label: '创建块', accelerator: 'B', click: () => mainWindow?.webContents.send('menu:block') },
        { label: '插入块', accelerator: 'I', click: () => mainWindow?.webContents.send('menu:insert') },
      ],
    },
    {
      label: '室分设计',
      submenu: [
        { label: '器件库', click: () => mainWindow?.webContents.send('menu:deviceLibrary') },
        { label: '放置天线', click: () => mainWindow?.webContents.send('menu:placeAntenna') },
        { label: '放置器件', click: () => mainWindow?.webContents.send('menu:placeDevice') },
        { label: '绘制馈线', click: () => mainWindow?.webContents.send('menu:drawCable') },
        { type: 'separator' },
        { label: '链路预算', click: () => mainWindow?.webContents.send('menu:linkBudget') },
        { label: '功率计算', click: () => mainWindow?.webContents.send('menu:powerCalc') },
        { label: '材料统计', click: () => mainWindow?.webContents.send('menu:materialList') },
        { type: 'separator' },
        { label: '生成系统图', click: () => mainWindow?.webContents.send('menu:genSystemDiagram') },
        { label: '规范检查', click: () => mainWindow?.webContents.send('menu:complianceCheck') },
        { type: 'separator' },
        { label: 'AI智能布放', click: () => mainWindow?.webContents.send('menu:aiPlace') },
        { label: 'AI建筑底图精简', click: () => mainWindow?.webContents.send('menu:aiSimplify') },
        { label: 'AI快速估算', click: () => mainWindow?.webContents.send('menu:aiEstimate') },
      ],
    },
    {
      label: '工具',
      submenu: [
        { label: '查询距离', accelerator: 'DI', click: () => mainWindow?.webContents.send('menu:dist') },
        { label: '查询面积', click: () => mainWindow?.webContents.send('menu:area') },
        { label: '列表', accelerator: 'LI', click: () => mainWindow?.webContents.send('menu:list') },
        { type: 'separator' },
        { label: '图层管理', accelerator: 'LA', click: () => mainWindow?.webContents.send('menu:layerManager') },
        { label: '特性', accelerator: 'Ctrl+1', click: () => mainWindow?.webContents.send('menu:properties') },
        { type: 'separator' },
        { label: '选项设置', click: () => mainWindow?.webContents.send('menu:options') },
      ],
    },
    {
      label: '帮助',
      submenu: [
        { label: '使用手册', click: () => mainWindow?.webContents.send('menu:manual') },
        { label: '新手教程', click: () => mainWindow?.webContents.send('menu:tutorial') },
        { label: '快捷键列表', click: () => mainWindow?.webContents.send('menu:shortcuts') },
        { type: 'separator' },
        { label: '关于智分Design', click: () => {
          dialog.showMessageBox(mainWindow!, {
            type: 'info',
            title: '关于智分Design',
            message: '智分Design V3.1',
            detail: 'AI驱动的专业室分设计CAD软件\n\n集天越、AIDP、infoCAD等主流室分软件之大成\n支持传统DAS与数字化室分双方案\nAI智能布放、自动计算、自动生图\n\n版本: 3.1.0\n构建日期: 2026-08-27',
          });
        }},
      ],
    },
  ];

  const menu = Menu.buildFromTemplate(template);
  Menu.setApplicationMenu(menu);
}

// IPC处理
ipcMain.handle('file:save', async (event, data: { content: string; filePath?: string }) => {
  if (data.filePath) {
    fs.writeFileSync(data.filePath, data.content, 'utf-8');
    return { success: true, filePath: data.filePath };
  }
  const result = await dialog.showSaveDialog(mainWindow!, {
    title: '保存文件',
    filters: [{ name: '智分Design文件', extensions: ['zfd'] }],
  });
  if (!result.canceled && result.filePath) {
    fs.writeFileSync(result.filePath, data.content, 'utf-8');
    return { success: true, filePath: result.filePath };
  }
  return { success: false };
});

ipcMain.handle('file:open', async () => {
  const result = await dialog.showOpenDialog(mainWindow!, {
    title: '打开文件',
    filters: [
      { name: '智分Design文件', extensions: ['zfd'] },
      { name: 'DXF文件', extensions: ['dxf'] },
      { name: '所有文件', extensions: ['*'] },
    ],
    properties: ['openFile'],
  });
  if (!result.canceled && result.filePaths.length > 0) {
    const content = fs.readFileSync(result.filePaths[0], 'utf-8');
    return { success: true, filePath: result.filePaths[0], content };
  }
  return { success: false };
});

ipcMain.handle('file:exportDxf', async (event, content: string) => {
  const result = await dialog.showSaveDialog(mainWindow!, {
    title: '导出DXF',
    filters: [{ name: 'DXF文件', extensions: ['dxf'] }],
  });
  if (!result.canceled && result.filePath) {
    fs.writeFileSync(result.filePath, content, 'utf-8');
    return { success: true, filePath: result.filePath };
  }
  return { success: false };
});

app.whenReady().then(() => {
  createWindow();
  createMenu();
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', () => {
  if (mainWindow === null) createWindow();
});
