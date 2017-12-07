#include "UploadCloud.h"
#include <iostream>


bool upload(LHDTSDK::LHDTTask &task, LHDTSDK::LHDTConfig &config, char* appPath)
{
    /// ��������
    LHDTSDK::LHDTTaskList tasklist, existlist;
    tasklist.push_back(task);

    LHDTSDK::LHDTInterface api;
    api.Initial(appPath); // ����app.exe ·��
    api.SetConfig(config);
    api.LaunchTransferTasks(tasklist, existlist, true);

    return true;
}