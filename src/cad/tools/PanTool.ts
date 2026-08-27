import { Tool, ToolContext } from './Tool';
import { Point, point } from '../core/types';

export class PanTool extends Tool {
  private isPanning: boolean = false;
  private lastPoint: Point | null = null;

  constructor(context: ToolContext) {
    super(context, '平移');
  }

  onMouseDown(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0 || e.button === 1) {
      this.isPanning = true;
      this.lastPoint = point(e.clientX, e.clientY);
    }
  }

  onMouseMove(e: MouseEvent, worldPoint: Point): void {
    if (this.isPanning && this.lastPoint) {
      const dx = e.clientX - this.lastPoint.x;
      const dy = e.clientY - this.lastPoint.y;
      this.context.renderer.pan(dx, dy);
      this.lastPoint = point(e.clientX, e.clientY);
      this.context.onRequestRender();
    }
  }

  onMouseUp(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0 || e.button === 1) {
      this.isPanning = false;
      this.lastPoint = null;
    }
  }

  onKeyDown(e: KeyboardEvent): void {
    if (e.key === 'Escape') {
      this.isPanning = false;
      this.finished = true;
    }
  }

  onKeyUp(e: KeyboardEvent): void {}

  cancel(): void {
    this.isPanning = false;
  }

  getCursor(): string {
    return 'grab';
  }

  drawOverlay(ctx: CanvasRenderingContext2D): void {}
}
