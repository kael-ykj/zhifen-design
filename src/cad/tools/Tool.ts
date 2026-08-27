import { Document } from '../core/Document';
import { CanvasRenderer } from '../renderer/CanvasRenderer';
import { SnapManager } from '../snap/SnapManager';
import { Point, point } from '../core/types';

export type ToolType = 'select' | 'line' | 'circle' | 'arc' | 'polyline' | 'rectangle' | 'text' | 'dimension' | 'pan' | 'zoom' | 'move' | 'copy' | 'rotate' | 'scale' | 'erase' | 'trim' | 'extend' | 'offset' | 'mirror';

export interface ToolContext {
  document: Document;
  renderer: CanvasRenderer;
  snapManager: SnapManager;
  onCommand: (cmd: string) => void;
  onStatus: (msg: string) => void;
  onRequestRender: () => void;
}

export abstract class Tool {
  context: ToolContext;
  name: string;
  finished: boolean = false;

  constructor(context: ToolContext, name: string) {
    this.context = context;
    this.name = name;
  }

  abstract onMouseDown(e: MouseEvent, worldPoint: Point): void;
  abstract onMouseMove(e: MouseEvent, worldPoint: Point): void;
  abstract onMouseUp(e: MouseEvent, worldPoint: Point): void;
  abstract onKeyDown(e: KeyboardEvent): void;
  abstract onKeyUp(e: KeyboardEvent): void;
  abstract cancel(): void;
  abstract getCursor(): string;

  protected getSnapPoint(worldPoint: Point, screenPoint: Point): Point {
    const snap = this.context.snapManager.computeSnap(worldPoint, screenPoint, this.context.renderer.view.scale);
    if (snap) return snap.point;
    return worldPoint;
  }

  protected saveState(description: string): void {
    // 由外部实现撤销/重做
  }
}
