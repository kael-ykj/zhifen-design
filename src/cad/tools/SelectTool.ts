import { Tool, ToolContext } from './Tool';
import { Point, point, rectFromPoints } from '../core/types';
import { Entity } from '../entities/Entity';

export class SelectTool extends Tool {
  private startPoint: Point | null = null;
  private currentPoint: Point | null = null;
  private isDragging: boolean = false;
  private isBoxSelect: boolean = false;
  private moveStart: Point | null = null;
  private movingEntities: Entity[] = [];

  constructor(context: ToolContext) {
    super(context, '选择');
  }

  onMouseDown(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0) {
      this.startPoint = worldPoint;
      this.isDragging = true;
      // 检查是否点击在已选中的实体上（用于移动）
      const entity = this.context.document.getEntityAtPoint(worldPoint, 5 / this.context.renderer.view.scale);
      if (entity && entity.selected) {
        this.moveStart = worldPoint;
        this.movingEntities = this.context.document.getSelectedEntities();
        this.isBoxSelect = false;
      } else {
        this.isBoxSelect = true;
        if (!e.shiftKey) {
          this.context.document.clearSelection();
        }
      }
      this.context.onRequestRender();
    }
  }

  onMouseMove(e: MouseEvent, worldPoint: Point): void {
    this.currentPoint = worldPoint;
    if (this.isDragging && this.isBoxSelect) {
      this.context.onRequestRender();
    }
    if (this.isDragging && this.moveStart && this.movingEntities.length > 0) {
      const dx = worldPoint.x - this.moveStart.x;
      const dy = worldPoint.y - this.moveStart.y;
      for (const ent of this.movingEntities) {
        ent.move(dx, dy);
      }
      this.moveStart = worldPoint;
      this.context.onRequestRender();
    }
  }

  onMouseUp(e: MouseEvent, worldPoint: Point): void {
    if (e.button === 0 && this.isDragging) {
      this.isDragging = false;
      if (this.isBoxSelect && this.startPoint) {
        const dist = Math.sqrt((worldPoint.x - this.startPoint.x) ** 2 + (worldPoint.y - this.startPoint.y) ** 2);
        if (dist < 3 / this.context.renderer.view.scale) {
          // 单击选择
          const entity = this.context.document.getEntityAtPoint(worldPoint, 5 / this.context.renderer.view.scale);
          if (entity) {
            if (e.shiftKey) {
              entity.selected = !entity.selected;
            } else {
              this.context.document.clearSelection();
              entity.selected = true;
            }
          } else if (!e.shiftKey) {
            this.context.document.clearSelection();
          }
        } else {
          // 框选
          const r = rectFromPoints(this.startPoint, worldPoint);
          const intersect = this.startPoint.x > worldPoint.x; // 从右向左=交叉选择
          const entities = this.context.document.getEntitiesInRect(r, !intersect);
          for (const ent of entities) {
            ent.selected = true;
          }
        }
      }
      this.startPoint = null;
      this.currentPoint = null;
      this.moveStart = null;
      this.movingEntities = [];
      this.context.onRequestRender();
    }
  }

  onKeyDown(e: KeyboardEvent): void {
    if (e.key === 'Delete' || e.key === 'Backspace') {
      const selected = this.context.document.getSelectedEntities();
      for (const ent of selected) {
        this.context.document.removeEntity(ent.id);
      }
      this.context.onStatus(`已删除 ${selected.length} 个对象`);
      this.context.onRequestRender();
    }
    if (e.key === 'Escape') {
      this.context.document.clearSelection();
      this.context.onRequestRender();
    }
    if (e.ctrlKey && e.key === 'a') {
      e.preventDefault();
      for (const ent of this.context.document.getAllEntities()) {
        ent.selected = true;
      }
      this.context.onRequestRender();
    }
  }

  onKeyUp(e: KeyboardEvent): void {}

  cancel(): void {
    this.isDragging = false;
    this.startPoint = null;
    this.currentPoint = null;
  }

  getCursor(): string {
    return 'crosshair';
  }

  drawOverlay(ctx: CanvasRenderingContext2D): void {
    if (this.isDragging && this.isBoxSelect && this.startPoint && this.currentPoint) {
      this.context.renderer.drawSelectionBox(this.startPoint, this.currentPoint, this.startPoint.x > this.currentPoint.x);
    }
  }
}
