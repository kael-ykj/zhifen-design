import { Entity } from './Entity';
import { Point, Rect, point } from '../core/types';

export interface BlockAttribute {
  tag: string;
  prompt: string;
  value: string;
  position: Point;
  height: number;
}

export class BlockDefinition {
  name: string;
  entities: Entity[] = [];
  basePoint: Point = point(0, 0);
  description: string = '';

  constructor(name: string) {
    this.name = name;
  }

  addEntity(entity: Entity): void {
    this.entities.push(entity);
  }

  getBounds(): Rect {
    if (this.entities.length === 0) return { x: 0, y: 0, width: 0, height: 0 };
    let minX = Infinity, minY = Infinity, maxX = -Infinity, maxY = -Infinity;
    for (const e of this.entities) {
      const b = e.getBounds();
      minX = Math.min(minX, b.x); minY = Math.min(minY, b.y);
      maxX = Math.max(maxX, b.x + b.width); maxY = Math.max(maxY, b.y + b.height);
    }
    return { x: minX, y: minY, width: maxX - minX, height: maxY - minY };
  }
}

export class BlockReference extends Entity {
  blockName: string;
  position: Point;
  scaleX: number = 1;
  scaleY: number = 1;
  rotation: number = 0;
  attributes: BlockAttribute[] = [];
  blockDefinition: BlockDefinition | null = null;

  constructor(blockName: string, position: Point, layer: string = '0') {
    super({ type: 'block', layer });
    this.blockName = blockName;
    this.position = position;
  }

  getBounds(): Rect {
    if (!this.blockDefinition) return { x: this.position.x, y: this.position.y, width: 10, height: 10 };
    const b = this.blockDefinition.getBounds();
    const w = b.width * Math.abs(this.scaleX);
    const h = b.height * Math.abs(this.scaleY);
    const cos = Math.abs(Math.cos(this.rotation));
    const sin = Math.abs(Math.sin(this.rotation));
    const rw = w * cos + h * sin;
    const rh = w * sin + h * cos;
    return { x: this.position.x - rw / 2, y: this.position.y - rh / 2, width: rw, height: rh };
  }

  getCenter(): Point {
    return { ...this.position };
  }

  move(dx: number, dy: number): void {
    this.position.x += dx; this.position.y += dy;
  }

  rotate(center: Point, angleRad: number): void {
    const cos = Math.cos(angleRad); const sin = Math.sin(angleRad);
    const dx = this.position.x - center.x; const dy = this.position.y - center.y;
    this.position = point(center.x + dx * cos - dy * sin, center.y + dx * sin + dy * cos);
    this.rotation += angleRad;
  }

  scale(center: Point, factor: number): void {
    this.position = point(center.x + (this.position.x - center.x) * factor, center.y + (this.position.y - center.y) * factor);
    this.scaleX *= factor; this.scaleY *= factor;
  }

  distanceToPoint(p: Point): number {
    const b = this.getBounds();
    if (p.x >= b.x && p.x <= b.x + b.width && p.y >= b.y && p.y <= b.y + b.height) return 0;
    const closestX = Math.max(b.x, Math.min(p.x, b.x + b.width));
    const closestY = Math.max(b.y, Math.min(p.y, b.y + b.height));
    return Math.sqrt((p.x - closestX) ** 2 + (p.y - closestY) ** 2);
  }

  intersectsRect(r: Rect): boolean {
    const b = this.getBounds();
    return b.x < r.x + r.width && b.x + b.width > r.x && b.y < r.y + r.height && b.y + b.height > r.y;
  }

  clone(): Entity {
    const br = new BlockReference(this.blockName, { ...this.position }, this.layer);
    br.scaleX = this.scaleX; br.scaleY = this.scaleY; br.rotation = this.rotation;
    br.attributes = this.attributes.map(a => ({ ...a, position: { ...a.position } }));
    br.blockDefinition = this.blockDefinition;
    br.color = this.color; br.lineType = this.lineType; br.lineWidth = this.lineWidth;
    return br;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, blockName: this.blockName, position: this.position, scaleX: this.scaleX, scaleY: this.scaleY, rotation: this.rotation, attributes: this.attributes };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.blockName = data.blockName; this.position = data.position;
    this.scaleX = data.scaleX || 1; this.scaleY = data.scaleY || 1;
    this.rotation = data.rotation || 0; this.attributes = data.attributes || [];
  }
}
