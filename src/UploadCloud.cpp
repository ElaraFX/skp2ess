#include "UploadCloud.h"
#include "lhdt_sdk.h"
#include <Windows.h>
#include <iostream>

void callback_test(LHDTSDK::LHDTCallback c, LHDTSDK::LHDTTask t)
{
    if (c.status == LHDTSDK::LHDT_TS_TRANSFERRING)
        std::cout << "bytes" << " : " << c.transferredBytes << "/" << c.totalBytes << std::endl;
	else if (c.status == LHDTSDK::LHDT_TS_FINISHED)
	{
		std::cout << "over!" << std::endl;
	}
}

CurlHttp ch;

int uploadCloud(char* exePath)
{
	ch.init();
	std::string("localhost");
	// login
	std::string url_login = "http://render9.vsochina.com:10008/api/web/v1/user/login?username=30466622&password=a123456";
	std::string postfields = "";
	ch.post(url_login.c_str(), 10008, postfields.c_str());

	// upload ess
	std::string url_upload = "http://render9.vsochina.com:10008/api/web/v1/job/submit?";
	url_upload += "job={\"guid\":\"Elara\",\"scene_file\":\"/dance.mb\"}";
	ch.post(url_upload.c_str(), 10008, postfields.c_str());

	// -origin
    //LHDTSDK::LHDTConfig config;

    //config.method = LHDTSDK::LHDT;
 
    ///// �����·�� ͨ��web api��¼�ӿڻ�ȡ
    //config.rootPath = "/z/h/o/u/zhou88/studio_33436/"; 
    //config.userName = "zhou88"; // ��¼��

    ///// ������Ϣ��web api�� domain�ӿڻ�ȡ
    //config.serverId = 9; // ������Id
    //config.serverIp = "cbs9.vsochina.com"; // �����ķ�����Ip
    //config.serverPort = "9001"; // ����˿�

    ///// �Ƽ�ֵ3
    //config.retryCount = 3; // ���Դ���

    ///// Ĭ�����ã����Ƽ��޸�
    //config.app = "GF"; // app�� GF=GoldenFarm
    //config.appVersion = "2.0.0"; // ����Э��汾

    ///// ��������
    //LHDTSDK::LHDTTaskList tasklist, existlist;
    //LHDTSDK::LHDTTask task;
    //task.filename = "ac_5.ess";
    //task.local = "D:/Program Files/OSL/bin/";
    //task.remote = "/123/";
    //task.type = LHDTSDK::LHDTTransferType::LHDT_TT_UPLOAD; // �ϴ� or ����
    //task.callback = callback_test; // �ص�����
    //tasklist.push_back(task);

    //LHDTSDK::LHDTInterface api;
    //api.Initial(exePath); // ����app.exe ·��
    //api.SetConfig(config);
    //api.LaunchTransferTasks(tasklist, existlist, true);

    return system("pause");
}