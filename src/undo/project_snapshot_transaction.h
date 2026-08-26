#pragma once

#include <memory>
#include <string>
#include "core/zf_types.h"
#include "undo/undo_redo_stack.h"

namespace zf {

// 工程快照事务：保存整个Project的快照，用于撤销/重做
class ProjectSnapshotTransaction : public ITransaction {
public:
    ProjectSnapshotTransaction(Project* target,
                               const Project& beforeSnapshot,
                               const Project& afterSnapshot,
                               const std::string& desc = "工程修改")
        : m_target(target)
        , m_before(beforeSnapshot)
        , m_after(afterSnapshot)
        , m_desc(desc) {}

    void redo() override {
        if (m_target) *m_target = m_after;
    }

    void undo() override {
        if (m_target) *m_target = m_before;
    }

    std::string description() const override { return m_desc; }

    const Project& beforeSnapshot() const { return m_before; }
    const Project& afterSnapshot() const { return m_after; }

private:
    Project* m_target{nullptr};
    Project m_before;
    Project m_after;
    std::string m_desc;
};

} // namespace zf
