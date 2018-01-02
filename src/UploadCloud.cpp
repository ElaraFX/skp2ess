#include "UploadCloud.h"
#include "CloudRender.h"
#include <iostream>


bool executeTask(LHDTSDK::LHDTTask &task, LHDTSDK::LHDTConfig &config, const char* appPath)
{
    /// ��������
	char *buf = new char[strlen(appPath)+1];
	strcpy(buf, appPath);
    LHDTSDK::LHDTTaskList tasklist, existlist;
    tasklist.push_back(task);

    g_cri.api.Initial(buf); // ����app.exe ·��
    g_cri.api.SetConfig(config);
    g_cri.api.LaunchTransferTasks(tasklist, existlist, true);

	delete[] buf;
    return true;
}
