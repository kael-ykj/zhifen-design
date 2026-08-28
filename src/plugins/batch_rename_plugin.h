#ifndef BATCH_RENAME_PLUGIN_H
#define BATCH_RENAME_PLUGIN_H

#include "iplugin.h"

namespace Zhifen {

// 批量重命名插件 - 示例插件
class BatchRenamePlugin : public IPlugin
{
public:
    BatchRenamePlugin();
    ~BatchRenamePlugin() override;

    QString name() const override { return "BatchRename"; }
    QString version() const override { return "1.0.0"; }
    QString author() const override { return "智分Design"; }
    QString description() const override { return "批量重命名器件，支持自定义前缀和起始编号"; }

    bool initialize(CoreApi *api) override;
    void execute() override;
    void unload() override;

private:
    QString m_prefix = "ANT";
    int m_startIndex = 1;
};

} // namespace Zhifen

#endif // BATCH_RENAME_PLUGIN_H
