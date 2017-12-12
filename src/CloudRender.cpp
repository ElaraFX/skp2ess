#include "CloudRender.h"
#include "jsoncpp/json/json.h"
#include "lhdt_sdk.h"
#include "UploadCloud.h"
#include <Windows.h>
#include <iostream>

#define CLOUD_URL "http://render7.vsochina.com:10008"
#define JOB_STATUS_COMPLETE "4"
#define JOB_STATUS_FAILED "7"
#define CLOUD_CENTER_IP "cbs7.vsochina.com"
#define SERVER_PORT "9001"
#define REMOTE_PATH "/123/"

CurlHttp ch;
__declspec(thread) int g_res_x;
__declspec(thread) int g_res_y;

enum CLOUD_STATE
{
	CLOUD_STATE_INITIAL = 0,
	CLOUD_STATE_TRANSFERRING,
	CLOUD_STATE_RENDERING,
	CLOUD_STATE_RETURN,
	CLOUD_STATE_TRANSFER_FAILED,
};

__declspec(thread) volatile CLOUD_STATE c_state = CLOUD_STATE_INITIAL;
void callback(LHDTSDK::LHDTCallback c, LHDTSDK::LHDTTask t)
{
    if (c.status == LHDTSDK::LHDT_TS_TRANSFERRING)
	{
        std::cout << "bytes" << " : " << c.transferredBytes << "/" << c.totalBytes << std::endl;
	}
	else if (c.status == LHDTSDK::LHDT_TS_FINISHED)
	{
		std::cout << "Transfer complete!" << std::endl;
		c_state = CLOUD_STATE_RENDERING;
	}
	else if (c.status == LHDTSDK::LHDT_TS_TRANSFER_FAILURE)
	{
		std::cout << "Transfer failed!" << std::endl;
		c_state = CLOUD_STATE_TRANSFER_FAILED;
	}
}

std::string& replace_all(std::string& str,const std::string& old_value,const std::string& new_value)     
{     
    while(true)   
	{     
        std::string::size_type pos(0);     
        if((pos = str.find(old_value)) != std::string::npos)  
		{
            str.replace(pos,old_value.length(),new_value);
		}
        else
		{
			break;  
		}
    }     
    return str;     
} 

void extractFilePath(std::string &filepath, std::string &filename, std::string &filefolder)
{
	replace_all(filepath, "\\", "/");
	std::string::size_type pos(0);
	if((pos = filepath.find_last_of("/")) != std::string::npos) 
	{
		filefolder = filepath.substr(0, pos);
		filename = filepath.substr(pos + 1, filepath.size() - 1);
	}
}

int CloudRender(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath)
{
	c_state = CLOUD_STATE_INITIAL;
	ch.init();
	// login
	std::string url_login = CLOUD_URL;
	url_login += "/api/web/v1/user/login?username=30466622&password=a123456";
	std::string postfields = "";
	ch.post(url_login.c_str(), 10008, postfields.c_str());

	Json::Value root;
	Json::Reader reader;
	std::string username;
	std::string token;
	if(!reader.parse(ch.retBuffer, root))
	{
		std::cout<<"return json error."<<std::endl;
		return 0;
	}
	username = root["data"]["username"].asString();
	token = root["data"]["token"].asString();

	// upload
	c_state = CLOUD_STATE_TRANSFERRING;
    LHDTSDK::LHDTConfig config;
	config.method = LHDTSDK::LHDT;
 
    /// 传输根路径 通过web api登录接口获取
    config.rootPath = root["data"]["path"].asString().c_str(); 
    config.userName = root["data"]["username"].asString().c_str(); // 登录名

    /// 以下信息从web api的 domain接口获取
    config.serverId = 7; // 分中心Id
    config.serverIp = CLOUD_CENTER_IP; // 分中心服务器Ip
    config.serverPort = SERVER_PORT; // 传输端口
    config.retryCount = 3; // 重试次数
    config.app = "GF"; // app名 GF=GoldenFarm
    config.appVersion = "2.0.0"; // 传输协议版本

	LHDTSDK::LHDTTask task;
	task.filename = filename;
    task.local = "./";
    task.remote = REMOTE_PATH;
    task.type = LHDTSDK::LHDTTransferType::LHDT_TT_UPLOAD; // 上传 or 下载
    task.callback = callback; // 回调函数
	executeTask(task, config, exePath);

	// waiting upload
	while (c_state <= CLOUD_STATE_TRANSFERRING)
	{
		Sleep(2000);
	}

	if (c_state == CLOUD_STATE_TRANSFER_FAILED)
	{
		return 1;
	}

	// sibmit job
	std::string url_render_task = CLOUD_URL;
	char res_x[32] = "", res_y[32] = "";
	sprintf(res_x, "%d", g_res_x);
	sprintf(res_y, "%d", g_res_y);
	url_render_task +=  "/api/web/v1/job/submit?";
	url_render_task +=  "username=" + username;
	url_render_task +=  "&token=" + token;
	url_render_task +=  "&job={";
	url_render_task +=  "\"guid\":\"Elara\",";
	url_render_task +=  "\"scene_file\":\"/123/";
	url_render_task +=  filename;
	url_render_task +=  "\",";
	url_render_task +=  "\"job_name\":\"";
	url_render_task +=  filename;
	url_render_task +=  "\",";
	url_render_task +=  "\"project_dir\":\"/123/\",";
	url_render_task +=  "\"output_dir\":\"/output_image/\",";
	url_render_task +=  "\"image_width\":\"";
	url_render_task +=  res_x;
	url_render_task +=  "\",";
	url_render_task +=  "\"image_height\":\"";
	url_render_task +=  res_y;
	url_render_task +=  "\",";
	url_render_task +=  "\"image_format\":\"";
	url_render_task +=  outputtype;
	url_render_task +=  "\",";
	url_render_task +=  "\"filename_prefix\":\"";
	url_render_task +=  outputprefix;
	url_render_task +=  "\",";
	url_render_task +=  "\"do_analysis\":true,";
	url_render_task +=  "\"priority\":\"50\",";
	url_render_task +=  "\"sub_task_frames\":1,";
	url_render_task +=  "\"start_frame\":1,";
	url_render_task +=  "\"stop_frame\":1,";
	url_render_task +=  "\"by_frame\":1,";
	url_render_task +=  "\"pool_id\":\"3425c1b338afc5cb1cb0bba1acad553d\"";
	url_render_task +=  "}";
	ch.post(url_render_task.c_str(), 10008, postfields.c_str());

	std::string job_id;
	std::string job_status = "0";
	if(!reader.parse(ch.retBuffer, root))
	{
		std::cout<<"return json error."<<std::endl;
		return 2;
	}
	job_id = root["data"]["job_ids"][0].asString();

	
	std::string url_job_info = CLOUD_URL;
	url_job_info +=  "/api/web/v1/job/info?";
	url_job_info +=  "username=" + username;
	url_job_info +=  "&token=" + token;
	url_job_info +=  "&job_id=" + job_id;
	do{
		ch.get(url_job_info.c_str(), 10008);
		if(!reader.parse(ch.retBuffer, root))
		{
			std::cout<<"return json error."<<std::endl;
			return 2;
		}
		job_status = root["data"]["job_status"].asString();
		std::cout << "job_status:" << job_status << std::endl;
		Sleep(10000);
	} while(job_status != JOB_STATUS_COMPLETE && job_status != JOB_STATUS_FAILED);

	if (job_status == JOB_STATUS_FAILED)
	{
		std::cout<<"job failed!"<<std::endl;
		return 3;
	}

	// get output file
	std::string output_path;
	std::string url_output_file = CLOUD_URL;
	url_output_file += "/api/web/v1/job/output_files2?";
	url_output_file +=  "username=" + username;
	url_output_file +=  "&token=" + token;
	url_output_file +=  "&job_id=" + job_id;

	do{
		ch.get(url_output_file.c_str(), 10008);
		if(!reader.parse(ch.retBuffer, root))
		{
			std::cout<<"return json error."<<std::endl;
			return 2;
		}
		if (root["data"]["status"].asInt() == 1)
		{
			output_path = root["data"]["files"][0]["file"].asString();
		}
		else
		{
			Sleep(10000);
		}
	} while(root["data"]["status"].asInt() != 1);

	// download output file
	std::string outfilename;
	std::string outfilefolder;
	LHDTSDK::LHDTTask task_download;
	extractFilePath(output_path, outfilename, outfilefolder);
	task_download.filename = outfilename.c_str();
    task_download.local = outputpath;
    task_download.remote = outfilefolder.c_str();
    task_download.type = LHDTSDK::LHDTTransferType::LHDT_TT_DOWNLOAD; // 上传 or 下载
    task_download.callback = callback; // 回调函数
	executeTask(task_download, config, exePath);

    return 0;
}