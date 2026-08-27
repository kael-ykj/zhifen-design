import { EntityProps, EntityType, Point, Rect, generateId } from '../core/types';

export abstract class Entity {
  id: string;
  type: EntityType;
  layer: string;
  color: string;
  lineType: string;
  lineWidth: number;
  visible: boolean;
  locked: boolean;
  selected: boolean = false;

  constructor(props: Partial<EntityProps> = {}) {
    this.id = props.id || generateId();
    this.type = props.type || 'point';
    this.layer = props.layer || '0';
    this.color = props.color || 'ByLayer';
    this.lineType = props.lineType || 'ByLayer';
    this.lineWidth = props.lineWidth || -1; // -1 = ByLayer
    this.visible = props.visible !== undefined ? props.visible : true;
    this.locked = props.locked || false;
  }

  abstract getBounds(): Rect;
  abstract getCenter(): Point;
  abstract move(dx: number, dy: number): void;
  abstract rotate(center: Point, angleRad: number): void;
  abstract scale(center: Point, factor: number): void;
  abstract distanceToPoint(p: Point): number;
  abstract intersectsRect(r: Rect): boolean;
  abstract clone(): Entity;
  abstract toJSON(): any;
  abstract fromJSON(data: any): void;

  isSelectable(): boolean {
    return this.visible && !this.locked;
  }

  getEffectiveColor(layerColor: string): string {
    return this.color === 'ByLayer' ? layerColor : this.color;
  }
}
