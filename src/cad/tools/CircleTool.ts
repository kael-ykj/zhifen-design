import { Tool, ToolContext } from './Tool';
import { Point, point } from '../core/types';
import { CircleEntity } from '../entities/CircleEntity';

export class CircleTool extends Tool {
  private center: Point | null = null;
  private currentPoint: Point | null = null;
  private mode: 'center-radius' | 'center-diameter' | 'two-point' | 'three-point' = 'center-radius';
  private points: Point[] = [];

  constructor(context: ToolContext) {
    super(context, '圆');
  }

  onMouseDown(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0) {
      const snapPoint = this.getSnapPoint(worldPoint, point(e.clientX, e.clientY));
      if (this.mode === 'center-radius') {
        if (!this.center) {
          this.center = snapPoint;
          this.context.onStatus('指定圆的半径:');
        } else {
          const radius = this.dist(this.center, snapPoint);
          if (radius > 0) {
            const circle = new CircleEntity({ ...this.center }, radius, this.context.document.currentLayer);
            this.context.document.addEntity(circle);
            this.context.onStatus(`已绘制圆，半径: ${radius.toFixed(2)}`);
          }
          this.center = null;
        }
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
    if (e.key === 'Escape' || e.key === 'Enter') {
      this.finish();
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
    if (this.center && this.currentPoint) {
      const c = this.context.renderer.worldToScreen(this.center);
      const r = this.dist(this.center, this.currentPoint) * this.context.renderer.view.scale;
      ctx.save();
      ctx.strokeStyle = '#FFFF00';
      ctx.lineWidth = 1;
      ctx.setLineDash([5, 3]);
      ctx.beginPath();
      ctx.arc(c.x, c.y, r, 0, Math.PI * 2);
      ctx.stroke();
      ctx.setLineDash([]);
      ctx.fillStyle = '#FFFF00';
      ctx.font = '12px Arial';
      ctx.fillText(`半径: ${this.dist(this.center, this.currentPoint).toFixed(2)}`, c.x + r + 5, c.y);
      ctx.restore();
    }
  }

  private finish(): void {
    this.center = null;
    this.currentPoint = null;
    this.points = [];
    this.finished = true;
    this.context.onStatus('圆命令结束');
    this.context.onRequestRender();
  }

  private dist(p1: Point, p2: Point): number {
    return Math.sqrt((p2.x - p1.x) ** 2 + (p2.y - p1.y) ** 2);
  }
}
