import { Document } from '../core/Document';
import { Entity } from '../entities/Entity';
import { Point, Rect, point, COLORS } from '../core/types';
import { LineEntity } from '../entities/LineEntity';
import { CircleEntity } from '../entities/CircleEntity';
import { ArcEntity } from '../entities/ArcEntity';
import { PolylineEntity } from '../entities/PolylineEntity';
import { RectangleEntity } from '../entities/RectangleEntity';
import { TextEntity } from '../entities/TextEntity';
import { DimensionEntity } from '../entities/DimensionEntity';
import { BlockReference, BlockDefinition } from '../entities/BlockEntity';

export interface ViewState {
  offsetX: number;
  offsetY: number;
  scale: number;
}

export class CanvasRenderer {
  canvas: HTMLCanvasElement;
  ctx: CanvasRenderingContext2D;
  document: Document;
  view: ViewState = { offsetX: 0, offsetY: 0, scale: 1 };
  backgroundColor: string = '#1E1E1E';
  gridColor: string = '#333333';
  gridSize: number = 10;
  showGrid: boolean = true;
  showAxis: boolean = true;
  blockDefinitions: Map<string, BlockDefinition> = new Map();

  constructor(canvas: HTMLCanvasElement, document: Document) {
    this.canvas = canvas;
    this.ctx = canvas.getContext('2d')!;
    this.document = document;
  }

  // 坐标变换：世界坐标 -> 屏幕坐标
  worldToScreen(p: Point): Point {
    return point(
      (p.x - this.view.offsetX) * this.view.scale + this.canvas.width / 2,
      -(p.y - this.view.offsetY) * this.view.scale + this.canvas.height / 2,
    );
  }

  // 屏幕坐标 -> 世界坐标
  screenToWorld(p: Point): Point {
    return point(
      (p.x - this.canvas.width / 2) / this.view.scale + this.view.offsetX,
      -(p.y - this.canvas.height / 2) / this.view.scale + this.view.offsetY,
    );
  }

  // 缩放（以屏幕点为中心）
  zoomAt(screenPoint: Point, factor: number): void {
    const worldBefore = this.screenToWorld(screenPoint);
    this.view.scale *= factor;
    const worldAfter = this.screenToWorld(screenPoint);
    this.view.offsetX += worldBefore.x - worldAfter.x;
    this.view.offsetY += worldBefore.y - worldAfter.y;
  }

  // 平移
  pan(dx: number, dy: number): void {
    this.view.offsetX -= dx / this.view.scale;
    this.view.offsetY += dy / this.view.scale;
  }

  // 缩放到全部
  zoomExtents(): void {
    const extents = this.document.getExtents();
    const cx = extents.x + extents.width / 2;
    const cy = extents.y + extents.height / 2;
    const scaleX = this.canvas.width / (extents.width * 1.2);
    const scaleY = this.canvas.height / (extents.height * 1.2);
    this.view.scale = Math.min(scaleX, scaleY);
    this.view.offsetX = cx;
    this.view.offsetY = cy;
  }

  // 主渲染函数
  render(): void {
    const ctx = this.ctx;
    ctx.fillStyle = this.backgroundColor;
    ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

    if (this.showGrid) this.drawGrid();
    if (this.showAxis) this.drawAxis();

    // 按图层顺序渲染
    for (const layer of this.document.getAllLayers()) {
      if (!layer.visible || layer.frozen) continue;
      for (const entity of layer.entities) {
        this.drawEntity(entity, layer.color);
      }
    }
  }

  private drawGrid(): void {
    const ctx = this.ctx;
    const topLeft = this.screenToWorld(point(0, 0));
    const bottomRight = this.screenToWorld(point(this.canvas.width, this.canvas.height));
    const startX = Math.floor(topLeft.x / this.gridSize) * this.gridSize;
    const startY = Math.floor(bottomRight.y / this.gridSize) * this.gridSize;
    const endX = Math.ceil(bottomRight.x / this.gridSize) * this.gridSize;
    const endY = Math.ceil(topLeft.y / this.gridSize) * this.gridSize;

    ctx.strokeStyle = this.gridColor;
    ctx.lineWidth = 0.5;
    ctx.beginPath();
    for (let x = startX; x <= endX; x += this.gridSize) {
      const p1 = this.worldToScreen(point(x, startY));
      const p2 = this.worldToScreen(point(x, endY));
      ctx.moveTo(p1.x, p1.y);
      ctx.lineTo(p2.x, p2.y);
    }
    for (let y = startY; y <= endY; y += this.gridSize) {
      const p1 = this.worldToScreen(point(startX, y));
      const p2 = this.worldToScreen(point(endX, y));
      ctx.moveTo(p1.x, p1.y);
      ctx.lineTo(p2.x, p2.y);
    }
    ctx.stroke();
  }

  private drawAxis(): void {
    const ctx = this.ctx;
    const origin = this.worldToScreen(point(0, 0));
    // X轴
    ctx.strokeStyle = '#FF6666';
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(0, origin.y);
    ctx.lineTo(this.canvas.width, origin.y);
    ctx.stroke();
    // Y轴
    ctx.strokeStyle = '#66FF66';
    ctx.beginPath();
    ctx.moveTo(origin.x, 0);
    ctx.lineTo(origin.x, this.canvas.height);
    ctx.stroke();
    // 原点标记
    ctx.fillStyle = '#FFFF66';
    ctx.font = '12px Arial';
    ctx.fillText('0,0', origin.x + 5, origin.y - 5);
  }

  private drawEntity(entity: Entity, layerColor: string): void {
    const color = entity.getEffectiveColor(layerColor);
    const ctx = this.ctx;
    ctx.strokeStyle = entity.selected ? '#00FFFF' : color;
    ctx.fillStyle = entity.selected ? '#00FFFF' : color;
    ctx.lineWidth = Math.max(0.5, (entity.lineWidth > 0 ? entity.lineWidth : 0.25) * this.view.scale);

    if (entity instanceof LineEntity) this.drawLine(entity);
    else if (entity instanceof CircleEntity) this.drawCircle(entity);
    else if (entity instanceof ArcEntity) this.drawArc(entity);
    else if (entity instanceof PolylineEntity) this.drawPolyline(entity);
    else if (entity instanceof RectangleEntity) this.drawRectangle(entity);
    else if (entity instanceof TextEntity) this.drawText(entity);
    else if (entity instanceof DimensionEntity) this.drawDimension(entity);
    else if (entity instanceof BlockReference) this.drawBlockReference(entity);

    // 选中高亮
    if (entity.selected) {
      const b = entity.getBounds();
      const p1 = this.worldToScreen(point(b.x, b.y));
      const p2 = this.worldToScreen(point(b.x + b.width, b.y + b.height));
      ctx.strokeStyle = '#00FFFF';
      ctx.lineWidth = 1;
      ctx.setLineDash([5, 3]);
      ctx.strokeRect(Math.min(p1.x, p2.x), Math.min(p1.y, p2.y), Math.abs(p2.x - p1.x), Math.abs(p2.y - p1.y));
      ctx.setLineDash([]);
    }
  }

  private drawLine(e: LineEntity): void {
    const ctx = this.ctx;
    const p1 = this.worldToScreen(e.start);
    const p2 = this.worldToScreen(e.end);
    ctx.beginPath();
    ctx.moveTo(p1.x, p1.y);
    ctx.lineTo(p2.x, p2.y);
    ctx.stroke();
  }

  private drawCircle(e: CircleEntity): void {
    const ctx = this.ctx;
    const c = this.worldToScreen(e.center);
    const r = e.radius * this.view.scale;
    ctx.beginPath();
    ctx.arc(c.x, c.y, r, 0, Math.PI * 2);
    ctx.stroke();
  }

  private drawArc(e: ArcEntity): void {
    const ctx = this.ctx;
    const c = this.worldToScreen(e.center);
    const r = e.radius * this.view.scale;
    ctx.beginPath();
    // Canvas Y轴向下，需要反转角度
    ctx.arc(c.x, c.y, r, -e.startAngle, -e.endAngle, e.counterClockwise);
    ctx.stroke();
  }

  private drawPolyline(e: PolylineEntity): void {
    const ctx = this.ctx;
    if (e.vertices.length < 2) return;
    ctx.beginPath();
    const p0 = this.worldToScreen(point(e.vertices[0].x, e.vertices[0].y));
    ctx.moveTo(p0.x, p0.y);
    for (let i = 1; i < e.vertices.length; i++) {
      const p = this.worldToScreen(point(e.vertices[i].x, e.vertices[i].y));
      ctx.lineTo(p.x, p.y);
    }
    if (e.closed) ctx.closePath();
    ctx.stroke();
  }

  private drawRectangle(e: RectangleEntity): void {
    const ctx = this.ctx;
    const verts = e.getVertices();
    ctx.beginPath();
    const p0 = this.worldToScreen(verts[0]);
    ctx.moveTo(p0.x, p0.y);
    for (let i = 1; i < 4; i++) {
      const p = this.worldToScreen(verts[i]);
      ctx.lineTo(p.x, p.y);
    }
    ctx.closePath();
    ctx.stroke();
  }

  private drawText(e: TextEntity): void {
    const ctx = this.ctx;
    const p = this.worldToScreen(e.position);
    ctx.save();
    ctx.translate(p.x, p.y);
    ctx.rotate(-e.rotation);
    ctx.font = `${e.height * this.view.scale}px ${e.font}`;
    ctx.textAlign = e.alignment === 'center' || e.alignment === 'middle' ? 'center' : e.alignment;
    ctx.textBaseline = 'middle';
    ctx.fillText(e.text, 0, 0);
    ctx.restore();
  }

  private drawDimension(e: DimensionEntity): void {
    const ctx = this.ctx;
    const p1 = this.worldToScreen(e.point1);
    const p2 = this.worldToScreen(e.point2);
    const dp = this.worldToScreen(e.dimensionLinePoint);
    // 简化：绘制尺寸线和文字
    ctx.strokeStyle = e.selected ? '#00FFFF' : '#00FFFF';
    ctx.lineWidth = 0.5;
    ctx.beginPath();
    ctx.moveTo(p1.x, p1.y);
    ctx.lineTo(dp.x, dp.y);
    ctx.moveTo(p2.x, p2.y);
    ctx.lineTo(dp.x, dp.y);
    ctx.moveTo(dp.x, dp.y);
    ctx.lineTo(dp.x + (p2.x - p1.x) * 0.3, dp.y);
    ctx.stroke();
    // 文字
    ctx.font = `${e.textHeight * this.view.scale}px Arial`;
    ctx.fillStyle = e.selected ? '#00FFFF' : '#00FFFF';
    ctx.textAlign = 'center';
    ctx.fillText(e.getDisplayText(), dp.x + (p2.x - p1.x) * 0.15, dp.y - 5);
  }

  private drawBlockReference(e: BlockReference): void {
    if (!e.blockDefinition) {
      // 绘制占位框
      const ctx = this.ctx;
      const p = this.worldToScreen(e.position);
      ctx.strokeRect(p.x - 10, p.y - 10, 20, 20);
      ctx.font = '10px Arial';
      ctx.fillText(e.blockName, p.x + 12, p.y);
      return;
    }
    const ctx = this.ctx;
    ctx.save();
    const p = this.worldToScreen(e.position);
    ctx.translate(p.x, p.y);
    ctx.rotate(-e.rotation);
    ctx.scale(e.scaleX * this.view.scale, e.scaleY * this.view.scale);
    ctx.translate(-e.blockDefinition.basePoint.x, -e.blockDefinition.basePoint.y);
    // 临时设置视图缩放为1来绘制块内实体
    const origScale = this.view.scale;
    this.view.scale = 1;
    const origOffsetX = this.view.offsetX;
    const origOffsetY = this.view.offsetY;
    this.view.offsetX = 0;
    this.view.offsetY = 0;
    for (const ent of e.blockDefinition.entities) {
      this.drawEntity(ent, '#FFFFFF');
    }
    this.view.scale = origScale;
    this.view.offsetX = origOffsetX;
    this.view.offsetY = origOffsetY;
    ctx.restore();
  }

  // 绘制临时图元（用于工具预览）
  drawTempEntity(entity: Entity, color: string = '#FFFF00'): void {
    const origSelected = entity.selected;
    entity.selected = false;
    const origColor = entity.color;
    entity.color = color;
    this.drawEntity(entity, color);
    entity.color = origColor;
    entity.selected = origSelected;
  }

  // 绘制选择框
  drawSelectionBox(p1: Point, p2: Point, intersect: boolean = false): void {
    const ctx = this.ctx;
    const s1 = this.worldToScreen(p1);
    const s2 = this.worldToScreen(p2);
    ctx.fillStyle = intersect ? 'rgba(0, 255, 255, 0.1)' : 'rgba(0, 255, 0, 0.1)';
    ctx.strokeStyle = intersect ? '#00FFFF' : '#00FF00';
    ctx.lineWidth = 1;
    ctx.setLineDash([5, 3]);
    ctx.fillRect(Math.min(s1.x, s2.x), Math.min(s1.y, s2.y), Math.abs(s2.x - s1.x), Math.abs(s2.y - s1.y));
    ctx.strokeRect(Math.min(s1.x, s2.x), Math.min(s1.y, s2.y), Math.abs(s2.x - s1.x), Math.abs(s2.y - s1.y));
    ctx.setLineDash([]);
  }
}
