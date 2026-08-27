import { Entity } from './Entity';
import { Point, Rect, point } from '../core/types';

export type TextAlignment = 'left' | 'center' | 'right' | 'middle';

export class TextEntity extends Entity {
  position: Point;
  text: string;
  height: number;
  rotation: number = 0;
  alignment: TextAlignment = 'left';
  font: string = 'Arial';
  widthFactor: number = 1;
  obliqueAngle: number = 0;

  constructor(position: Point, text: string, height: number = 2.5, layer: string = '0') {
    super({ type: 'text', layer });
    this.position = position;
    this.text = text;
    this.height = height;
  }

  getBounds(): Rect {
    const width = this.text.length * this.height * 0.6 * this.widthFactor;
    let x = this.position.x;
    if (this.alignment === 'center') x -= width / 2;
    else if (this.alignment === 'right') x -= width;
    else if (this.alignment === 'middle') { x -= width / 2; }
    return { x, y: this.position.y - this.height / 2, width, height: this.height };
  }

  getCenter(): Point {
    const b = this.getBounds();
    return point(b.x + b.width / 2, b.y + b.height / 2);
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
    this.height *= factor;
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
    const t = new TextEntity({ ...this.position }, this.text, this.height, this.layer);
    t.rotation = this.rotation; t.alignment = this.alignment;
    t.font = this.font; t.widthFactor = this.widthFactor; t.obliqueAngle = this.obliqueAngle;
    t.color = this.color; t.lineType = this.lineType; t.lineWidth = this.lineWidth;
    return t;
  }

  toJSON(): any {
    return { id: this.id, type: this.type, layer: this.layer, color: this.color, lineType: this.lineType, lineWidth: this.lineWidth, position: this.position, text: this.text, height: this.height, rotation: this.rotation, alignment: this.alignment, font: this.font, widthFactor: this.widthFactor, obliqueAngle: this.obliqueAngle };
  }

  fromJSON(data: any): void {
    this.id = data.id; this.layer = data.layer; this.color = data.color;
    this.lineType = data.lineType; this.lineWidth = data.lineWidth;
    this.position = data.position; this.text = data.text; this.height = data.height;
    this.rotation = data.rotation || 0; this.alignment = data.alignment || 'left';
    this.font = data.font || 'Arial'; this.widthFactor = data.widthFactor || 1;
    this.obliqueAngle = data.obliqueAngle || 0;
  }
}
