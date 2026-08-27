import { Tool, ToolContext } from './Tool';
import { Point, point } from '../core/types';
import { LineEntity } from '../entities/LineEntity';

export class LineTool extends Tool {
  private startPoint: Point | null = null;
  private currentPoint: Point | null = null;
  private points: Point[] = [];

  constructor(context: ToolContext) {
    super(context, '直线');
  }

  onMouseDown(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0) {
      const snapPoint = this.getSnapPoint(worldPoint, point(e.clientX, e.clientY));
      if (!this.startPoint) {
        this.startPoint = snapPoint;
        this.points = [snapPoint];
        this.context.onStatus('指定下一点:');
      } else {
        this.points.push(snapPoint);
        const line = new LineEntity({ ...this.points[this.points.length - 2] }, { ...snapPoint }, this.context.document.currentLayer);
        this.context.document.addEntity(line);
        this.startPoint = snapPoint;
        this.context.onStatus(`已绘制线段，长度: ${this.points.length > 1 ? this.dist(this.points[this.points.length-2], this.points[this.points.length-1]).toFixed(2) : '0'}`);
      }
      this.context.onRequestRender();
    } else if (e.button === 2) {
      this.finish();
    }
  }

  onMouseMove(e: MouseEvent, worldPoint: Point): void {
    this.currentPoint = this.getSnapPoint(worldPoint, point(e.clientX, e.clientY));
    this.context.onRequestRender();
  }

  onMouseUp(e: MouseEvent, worldPoint: Point): void {}

  onKeyDown(e: KeyboardEvent): void {
    if (e.key === 'Escape') {
      this.finish();
    }
    if (e.key === 'Enter') {
      this.finish();
    }
    if (e.key === 'u' || e.key === 'U') {
      // 撤销上一段
      if (this.points.length > 1) {
        this.points.pop();
        this.startPoint = this.points[this.points.length - 1];
        // 移除最后一条线
        const entities = this.context.document.getAllEntities();
        if (entities.length > 0) {
          const last = entities[entities.length - 1];
          if (last instanceof LineEntity) {
            this.context.document.removeEntity(last.id);
          }
        }
        this.context.onRequestRender();
      }
    }
  }

  onKeyUp(e: KeyboardEvent): void {}

  cancel(): void {
    this.finish();
  }

  getCursor(): string {
    return 'crosshair';
  }

  drawOverlay(ctx: CanvasRenderingContext2D): void {
    if (this.startPoint && this.currentPoint) {
      const p1 = this.context.renderer.worldToScreen(this.startPoint);
      const p2 = this.context.renderer.worldToScreen(this.currentPoint);
      ctx.save();
      ctx.strokeStyle = '#FFFF00';
      ctx.lineWidth = 1;
      ctx.setLineDash([5, 3]);
      ctx.beginPath();
      ctx.moveTo(p1.x, p1.y);
      ctx.lineTo(p2.x, p2.y);
      ctx.stroke();
      ctx.setLineDash([]);
      // 显示长度和角度
      const len = this.dist(this.startPoint, this.currentPoint);
      const ang = (Math.atan2(this.currentPoint.y - this.startPoint.y, this.currentPoint.x - this.startPoint.x) * 180 / Math.PI).toFixed(1);
      ctx.fillStyle = '#FFFF00';
      ctx.font = '12px Arial';
      ctx.fillText(`长度: ${len.toFixed(2)}  角度: ${ang}°`, p2.x + 10, p2.y - 10);
      ctx.restore();
    }
  }

  private finish(): void {
    this.startPoint = null;
    this.currentPoint = null;
    this.points = [];
    this.finished = true;
    this.context.onStatus('直线命令结束');
    this.context.onRequestRender();
  }

  private dist(p1: Point, p2: Point): number {
    return Math.sqrt((p2.x - p1.x) ** 2 + (p2.y - p1.y) ** 2);
  }
}
