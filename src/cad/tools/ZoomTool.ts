import { Tool, ToolContext } from './Tool';
import { Point, point } from '../core/types';

export class ZoomTool extends Tool {
  private mode: 'window' | 'real-time' = 'window';
  private startPoint: Point | null = null;
  private currentPoint: Point | null = null;

  constructor(context: ToolContext) {
    super(context, '缩放');
  }

  onMouseDown(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0) {
      if (this.mode === 'window') {
        if (!this.startPoint) {
          this.startPoint = point(e.clientX, e.clientY);
          this.context.onStatus('指定对角点:');
        } else {
          const endPoint = point(e.clientX, e.clientY);
          const width = Math.abs(endPoint.x - this.startPoint.x);
          const height = Math.abs(endPoint.y - this.startPoint.y);
          if (width > 5 && height > 5) {
            // 计算缩放
            const scaleX = this.context.renderer.canvas.width / width;
            const scaleY = this.context.renderer.canvas.height / height;
            const newScale = Math.min(scaleX, scaleY);
            const centerScreen = point((this.startPoint.x + endPoint.x) / 2, (this.startPoint.y + endPoint.y) / 2);
            const centerWorld = this.context.renderer.screenToWorld(centerScreen);
            this.context.renderer.view.scale = newScale;
            this.context.renderer.view.offsetX = centerWorld.x;
            this.context.renderer.view.offsetY = centerWorld.y;
          }
          this.startPoint = null;
          this.currentPoint = null;
          this.finished = true;
        }
      }
      this.context.onRequestRender();
    }
  }

  onMouseMove(e: MouseEvent, worldPoint: Point): void {
    this.currentPoint = point(e.clientX, e.clientY);
    if (this.mode === 'real-time') {
      // 实时缩放
    }
    this.context.onRequestRender();
  }

  onMouseUp(e: MouseEvent, worldPoint: Point): void {}

  onKeyDown(e: KeyboardEvent): void {
    if (e.key === 'Escape') {
      this.startPoint = null;
      this.currentPoint = null;
      this.finished = true;
    }
    if (e.key === 'e' || e.key === 'E') {
      this.context.renderer.zoomExtents();
      this.finished = true;
      this.context.onRequestRender();
    }
  }

  onKeyUp(e: KeyboardEvent): void {}

  cancel(): void {
    this.startPoint = null;
    this.currentPoint = null;
  }

  getCursor(): string {
    return 'crosshair';
  }

  drawOverlay(ctx: CanvasRenderingContext2D): void {
    if (this.startPoint && this.currentPoint && this.mode === 'window') {
      ctx.save();
      ctx.strokeStyle = '#00FF00';
      ctx.lineWidth = 1;
      ctx.setLineDash([5, 3]);
      ctx.strokeRect(
        Math.min(this.startPoint.x, this.currentPoint.x),
        Math.min(this.startPoint.y, this.currentPoint.y),
        Math.abs(this.currentPoint.x - this.startPoint.x),
        Math.abs(this.currentPoint.y - this.startPoint.y),
      );
      ctx.setLineDash([]);
      ctx.restore();
    }
  }
}
