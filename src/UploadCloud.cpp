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
 
    ///// 传输根路径 通过web api登录接口获取
    //config.rootPath = "/z/h/o/u/zhou88/studio_33436/"; 
    //config.userName = "zhou88"; // 登录名

    ///// 以下信息从web api的 domain接口获取
    //config.serverId = 9; // 分中心Id
    //config.serverIp = "cbs9.vsochina.com"; // 分中心服务器Ip
    //config.serverPort = "9001"; // 传输端口

    ///// 推荐值3
    //config.retryCount = 3; // 重试次数

    ///// 默认配置，不推荐修改
    //config.app = "GF"; // app名 GF=GoldenFarm
    //config.appVersion = "2.0.0"; // 传输协议版本

    ///// 创建任务
    //LHDTSDK::LHDTTaskList tasklist, existlist;
    //LHDTSDK::LHDTTask task;
    //task.filename = "ac_5.ess";
    //task.local = "D:/Program Files/OSL/bin/";
    //task.remote = "/123/";
    //task.type = LHDTSDK::LHDTTransferType::LHDT_TT_UPLOAD; // 上传 or 下载
    //task.callback = callback_test; // 回调函数
    //tasklist.push_back(task);

    //LHDTSDK::LHDTInterface api;
    //api.Initial(exePath); // 传入app.exe 路径
    //api.SetConfig(config);
    //api.LaunchTransferTasks(tasklist, existlist, true);

    return system("pause");
}