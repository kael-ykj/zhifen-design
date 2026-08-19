#pragma once

#include <vector>
#include <memory>
#include <string>
#include "zf_types.h"
#include "zf_error.h"

namespace zf {

class ITransaction {
public:
    virtual ~ITransaction() = default;
    virtual void redo() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

class GeometryTransaction : public ITransaction {
public:
    void redo() override {}
    void undo() override {}
    std::string description() const override { return "几何操作: " + geoOpType; }
    std::string itemId;
    Point2D oldPos;
    Point2D newPos;
    double oldRotation{0.0};
    double newRotation{0.0};
    std::string geoOpType;
};

class BusinessTransaction : public ITransaction {
public:
    void redo() override {}
    void undo() override {}
    std::string description() const override { return "业务操作: " + bizOpType; }
    std::string targetId;
    std::string bizOpType;
    std::vector<uint8_t> oldStateBlob;
    std::vector<uint8_t> newStateBlob;
};

class CombinedTransaction : public ITransaction {
public:
    void redo() override {
        if (geoPart) geoPart->redo();
        if (bizPart) bizPart->redo();
    }
    void undo() override {
        if (bizPart) bizPart->undo();
        if (geoPart) geoPart->undo();
    }
    std::string description() const override {
        return geoPart ? geoPart->description() : (bizPart ? bizPart->description() : "组合操作");
    }
    std::unique_ptr<GeometryTransaction> geoPart;
    std::unique_ptr<BusinessTransaction> bizPart;
};

class UndoRedoDoubleStack {
public:
    UndoRedoDoubleStack() = default;

    void pushTransaction(std::unique_ptr<ITransaction> tx) {
        if (m_macroDepth > 0) {
            m_macroBuffer.push_back(std::move(tx));
            return;
        }
        m_undoStack.push_back(std::move(tx));
        m_redoStack.clear();
    }

    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }

    void undo() {
        if (!canUndo()) return;
        auto tx = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        tx->undo();
        m_redoStack.push_back(std::move(tx));
    }

    void redo() {
        if (!canRedo()) return;
        auto tx = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        tx->redo();
        m_undoStack.push_back(std::move(tx));
    }

    void clear() {
        m_undoStack.clear();
        m_redoStack.clear();
        m_macroBuffer.clear();
        m_macroDepth = 0;
    }

    int stackSize() const { return static_cast<int>(m_undoStack.size()); }

    void beginMacro(const std::string& label) {
        m_macroDepth++;
        if (m_macroDepth == 1) {
            m_currentMacroLabel = label;
            m_macroBuffer.clear();
        }
    }

    void endMacro() {
        if (m_macroDepth <= 0) return;
        m_macroDepth--;
        if (m_macroDepth == 0 && !m_macroBuffer.empty()) {
            auto macroTx = std::make_unique<CombinedTransaction>();
            m_undoStack.push_back(std::move(macroTx));
            m_redoStack.clear();
            m_macroBuffer.clear();
        }
    }

private:
    std::vector<std::unique_ptr<ITransaction>> m_undoStack;
    std::vector<std::unique_ptr<ITransaction>> m_redoStack;
    int m_macroDepth{0};
    std::string m_currentMacroLabel;
    std::vector<std::unique_ptr<ITransaction>> m_macroBuffer;
};

} // namespace zf
