// CAD核心类型定义
export interface Point {
  x: number;
  y: number;
}

export interface Rect {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface Color {
  r: number;
  g: number;
  b: number;
  a?: number;
}

export type EntityType = 'line' | 'circle' | 'arc' | 'polyline' | 'rectangle' | 'text' | 'dimension' | 'block' | 'point' | 'hatch';

export interface EntityProps {
  id: string;
  type: EntityType;
  layer: string;
  color?: string;
  lineType?: string;
  lineWidth?: number;
  visible?: boolean;
  locked?: boolean;
}

// 工具函数
export function point(x: number, y: number): Point {
  return { x, y };
}

export function distance(p1: Point, p2: Point): number {
  return Math.sqrt((p2.x - p1.x) ** 2 + (p2.y - p1.y) ** 2);
}

export function midpoint(p1: Point, p2: Point): Point {
  return { x: (p1.x + p2.x) / 2, y: (p1.y + p2.y) / 2 };
}

export function angle(p1: Point, p2: Point): number {
  return Math.atan2(p2.y - p1.y, p2.x - p1.x);
}

export function rectFromPoints(p1: Point, p2: Point): Rect {
  return {
    x: Math.min(p1.x, p2.x),
    y: Math.min(p1.y, p2.y),
    width: Math.abs(p2.x - p1.x),
    height: Math.abs(p2.y - p1.y),
  };
}

export function pointInRect(p: Point, r: Rect): boolean {
  return p.x >= r.x && p.x <= r.x + r.width && p.y >= r.y && p.y <= r.y + r.height;
}

export function generateId(): string {
  return 'ent_' + Math.random().toString(36).substr(2, 9) + '_' + Date.now().toString(36);
}

// 常用颜色
export const COLORS = {
  red: '#FF0000',
  yellow: '#FFFF00',
  green: '#00FF00',
  cyan: '#00FFFF',
  blue: '#0000FF',
  magenta: '#FF00FF',
  white: '#FFFFFF',
  black: '#000000',
  gray: '#808080',
  lightGray: '#C0C0C0',
  orange: '#FFA500',
};

// ACI颜色索引(AutoCAD Color Index)
export const ACI_COLORS: Record<number, string> = {
  0: '#000000', 1: '#FF0000', 2: '#FFFF00', 3: '#00FF00',
  4: '#00FFFF', 5: '#0000FF', 6: '#FF00FF', 7: '#FFFFFF',
  8: '#808080', 9: '#C0C0C0',
};
