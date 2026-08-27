import { Entity } from '../entities/Entity';

export interface LayerState {
  name: string;
  color: string;
  lineType: string;
  lineWidth: number;
  visible: boolean;
  locked: boolean;
  frozen: boolean;
  plot: boolean;
}

export class Layer {
  name: string;
  color: string;
  lineType: string;
  lineWidth: number;
  visible: boolean;
  locked: boolean;
  frozen: boolean;
  plot: boolean;
  entities: Entity[] = [];

  constructor(name: string, color: string = '#FFFFFF') {
    this.name = name;
    this.color = color;
    this.lineType = 'Continuous';
    this.lineWidth = 0.25;
    this.visible = true;
    this.locked = false;
    this.frozen = false;
    this.plot = true;
  }

  addEntity(entity: Entity): void {
    this.entities.push(entity);
  }

  removeEntity(id: string): boolean {
    const idx = this.entities.findIndex(e => e.id === id);
    if (idx >= 0) {
      this.entities.splice(idx, 1);
      return true;
    }
    return false;
  }

  getEntity(id: string): Entity | undefined {
    return this.entities.find(e => e.id === id);
  }

  toJSON(): LayerState {
    return {
      name: this.name,
      color: this.color,
      lineType: this.lineType,
      lineWidth: this.lineWidth,
      visible: this.visible,
      locked: this.locked,
      frozen: this.frozen,
      plot: this.plot,
    };
  }

  static fromJSON(state: LayerState): Layer {
    const layer = new Layer(state.name, state.color);
    layer.lineType = state.lineType;
    layer.lineWidth = state.lineWidth;
    layer.visible = state.visible;
    layer.locked = state.locked;
    layer.frozen = state.frozen;
    layer.plot = state.plot;
    return layer;
  }
}
