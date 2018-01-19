#include "UploadCloud.h"
#include "CloudRender.h"
#include <iostream>


bool executeTask(LHDTSDK::LHDTTask &task, LHDTSDK::LHDTConfig &config, const char* appPath)
{
    /// 创建任务
    LHDTSDK::LHDTTaskList tasklist, existlist;
    tasklist.push_back(task);

    g_cri.api.SetConfig(config);
    g_cri.api.LaunchTransferTasks(tasklist, existlist, true);

    return true;
}
