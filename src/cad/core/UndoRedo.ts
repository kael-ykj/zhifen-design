import { Entity } from '../entities/Entity';

interface HistoryState {
  entities: Map<string, any>; // id -> serialized entity
  selectedIds: string[];
  description: string;
}

export class UndoRedoStack {
  private undoStack: HistoryState[] = [];
  private redoStack: HistoryState[] = [];
  private maxSize: number = 50;

  pushState(entities: Entity[], selectedIds: string[], description: string): void {
    const state: HistoryState = {
      entities: new Map(),
      selectedIds: [...selectedIds],
      description,
    };
    for (const e of entities) {
      state.entities.set(e.id, JSON.parse(JSON.stringify(e)));
    }
    this.undoStack.push(state);
    if (this.undoStack.length > this.maxSize) {
      this.undoStack.shift();
    }
    this.redoStack = []; // 清空重做栈
  }

  canUndo(): boolean {
    return this.undoStack.length > 0;
  }

  canRedo(): boolean {
    return this.redoStack.length > 0;
  }

  undo(currentEntities: Entity[]): HistoryState | null {
    if (this.undoStack.length === 0) return null;
    const state = this.undoStack.pop()!;
    // 保存当前状态到重做栈
    const currentState: HistoryState = {
      entities: new Map(),
      selectedIds: currentEntities.filter(e => e.selected).map(e => e.id),
      description: 'redo',
    };
    for (const e of currentEntities) {
      currentState.entities.set(e.id, JSON.parse(JSON.stringify(e)));
    }
    this.redoStack.push(currentState);
    return state;
  }

  redo(currentEntities: Entity[]): HistoryState | null {
    if (this.redoStack.length === 0) return null;
    const state = this.redoStack.pop()!;
    // 保存当前状态到撤销栈
    const currentState: HistoryState = {
      entities: new Map(),
      selectedIds: currentEntities.filter(e => e.selected).map(e => e.id),
      description: 'undo',
    };
    for (const e of currentEntities) {
      currentState.entities.set(e.id, JSON.parse(JSON.stringify(e)));
    }
    this.undoStack.push(currentState);
    return state;
  }

  getUndoDescription(): string {
    if (this.undoStack.length === 0) return '';
    return this.undoStack[this.undoStack.length - 1].description;
  }

  getRedoDescription(): string {
    if (this.redoStack.length === 0) return '';
    return this.redoStack[this.redoStack.length - 1].description;
  }

  clear(): void {
    this.undoStack = [];
    this.redoStack = [];
  }
}
