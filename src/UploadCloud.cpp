#include "UploadCloud.h"
#include <iostream>


bool upload(LHDTSDK::LHDTTask &task, LHDTSDK::LHDTConfig &config, char* appPath)
{
    /// 创建任务
    LHDTSDK::LHDTTaskList tasklist, existlist;
    tasklist.push_back(task);

    LHDTSDK::LHDTInterface api;
    api.Initial(appPath); // 传入app.exe 路径
    api.SetConfig(config);
    api.LaunchTransferTasks(tasklist, existlist, true);

    return true;
}