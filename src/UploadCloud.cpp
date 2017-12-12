#include "UploadCloud.h"
#include <iostream>


bool executeTask(LHDTSDK::LHDTTask &task, LHDTSDK::LHDTConfig &config, const char* appPath)
{
    /// 创建任务
	char *buf = new char[strlen(appPath)+1];
	strcpy(buf, appPath);
    LHDTSDK::LHDTTaskList tasklist, existlist;
    tasklist.push_back(task);

    LHDTSDK::LHDTInterface api;
    api.Initial(buf); // 传入app.exe 路径
    api.SetConfig(config);
    api.LaunchTransferTasks(tasklist, existlist, true);

	delete[] buf;
    return true;
}
