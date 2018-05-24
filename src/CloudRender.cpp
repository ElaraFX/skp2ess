﻿#include "CloudRender.h"
#include "jsoncpp/json/json.h"
#include "lhdt_sdk.h"
#include "UploadCloud.h"
#include <Windows.h>

#define CLOUD_URL "http://render12.vsochina.com:10008"
#define JOB_STATUS_RENDER "1"
#define JOB_STATUS_STOP "2"
#define JOB_STATUS_COMPLETE "4"
#define JOB_STATUS_FAILED "7"
#define CLOUD_CENTER_IP "cbs12.vsochina.com"
#define SERVER_PORT "33001"
#define REMOTE_PATH "/123/"
#define APP_ID "GJJ"
#define APP_TOKEN "2a16e8dc4c77ba2e8e53ed1ff8be4c61"

cloud_render_info g_cri;

std::string string_To_UTF8(const std::string & str)
{
  int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

  wchar_t * pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴  
  ZeroMemory(pwBuf, nwLen * 2 + 2);

  ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

  int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

  char * pBuf = new char[nLen + 1];
  ZeroMemory(pBuf, nLen + 1);

  ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

  std::string retStr(pBuf);

  delete[]pwBuf;
  delete[]pBuf;

  pwBuf = NULL;
  pBuf = NULL;

  return retStr;
}

void callback_upload(LHDTSDK::LHDTCallback c, LHDTSDK::LHDTTask t)
{
    if (c.status == LHDTSDK::LHDT_TS_TRANSFERRING)
	{
        std::cout << "bytes" << " : " << c.transferredBytes << "/" << c.totalBytes << std::endl;
		g_cri.paramTransfer = float(c.transferredBytes) / max(float(c.totalBytes), 1);
	}
	else if (c.status == LHDTSDK::LHDT_TS_FINISHED)
	{
		std::cout << "Transfer complete! Id:" << c.id << std::endl;
		g_cri.paramTransfer = 1;
		for (int i = 0; i < MAX_MODEL_SCENES; i++)
		{
			g_cri.c_state[i] = CLOUD_STATE_WAIT_RENDER;
		}
	}
	else if (c.status == LHDTSDK::LHDT_TS_TRANSFER_FAILURE)
	{
		std::cout << "Transfer failed! Id:" << c.id << std::endl;
		for (int i = 0; i < MAX_MODEL_SCENES; i++)
		{
			g_cri.c_state[i] = CLOUD_STATE_TRANSFER_FAILED;
		}
	}
}

void callback_download(LHDTSDK::LHDTCallback c, LHDTSDK::LHDTTask t)
{
    if (c.status == LHDTSDK::LHDT_TS_TRANSFERRING)
	{
        std::cout << "bytes" << " : " << c.transferredBytes << "/" << c.totalBytes << std::endl;
		g_cri.paramTransfer = float(c.transferredBytes) / max(float(c.totalBytes), 1);
	}
	else if (c.status == LHDTSDK::LHDT_TS_FINISHED)
	{
		std::cout << "Transfer complete! Id:" << c.id << std::endl;
		g_cri.paramTransfer = 1;
		g_cri.finished_tasks++;
		g_cri.c_state[g_cri.cur_download_scene] = CLOUD_STATE_RETURN;
	}
	else if (c.status == LHDTSDK::LHDT_TS_TRANSFER_FAILURE)
	{
		std::cout << "Transfer failed! Id:" << c.id << std::endl;
		g_cri.finished_tasks++;
		g_cri.c_state[g_cri.cur_download_scene] = CLOUD_STATE_TRANSFER_FAILED;
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

int upload_ess(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath, char* username, char* token)
{
	// login
	std::string url_login = CLOUD_URL;
	url_login += "/api/web/v2/user/login?username=";
	url_login += g_cri.username + "&password=";
	url_login += g_cri.password;
	url_login += "&client_id=";
	url_login += g_cri.clientid;
	url_login += "&appid=";
	url_login += APP_ID;
	url_login += "&apptoken=";
	url_login += APP_TOKEN;
	std::string postfields = "";
	g_cri.ch.post(url_login.c_str(), 10008, postfields.c_str());

	Json::Value root;
	Json::Reader reader;
	if(!reader.parse(g_cri.ch.retBuffer, root))
	{
		std::cout<<"return json error."<<std::endl;
		return 0;
	}
	if (root["ret"].asInt() != 0)
	{
		// 登陆失败
		for (int i = 0; i < MAX_MODEL_SCENES; i++)
		{
			g_cri.c_state[i] = CLOUD_STATE_LOGIN_FAILED;
		}
		return 0;
	}
	memcpy(username, root["data"]["username"].asString().c_str(), root["data"]["username"].asString().size());
	memcpy(token, root["data"]["token"].asString().c_str(), root["data"]["token"].asString().size());
	g_cri.token = token;

	// upload
	for (int i = 0; i < MAX_MODEL_SCENES; i++)
	{
		g_cri.c_state[i] = CLOUD_STATE_TRANSFERRING;
	}
	
	// init
	char *buf = new char[strlen(exePath)+1];
	strcpy(buf, exePath);
	g_cri.initial(buf);
	delete[] buf;

	g_cri.paramTransfer = 0;
    LHDTSDK::LHDTConfig config;
	config.method = LHDTSDK::ASPERA;
 
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
	if (g_cri.transferMaxSpeed > 0)
	{
		config.maxUploadRate = g_cri.transferMaxSpeed;
	}

	LHDTSDK::LHDTTask task;
	task.filename = filename;
    task.local = outputpath;
    task.remote = REMOTE_PATH;
    task.type = LHDTSDK::LHDTTransferType::LHDT_TT_UPLOAD; // 上传 or 下载
    task.callback = callback_upload; // 回调函数
	executeTask(task, config, exePath);
	return 1;
}

int submit_task(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath, const char* username, 
	const char* token, const char* projectfolder)
{
	// sibmit job
	std::string url_render_task = CLOUD_URL;
	char res_x[32] = "", res_y[32] = "";
	sprintf(res_x, "%d", g_cri.res_x);
	sprintf(res_y, "%d", g_cri.res_y);
	std::string filename_encoded = g_cri.ch.escape(string_To_UTF8(filename));
	std::string prefix_encoded = g_cri.ch.escape(string_To_UTF8(outputprefix));
	std::string project_encoded = g_cri.ch.escape(string_To_UTF8(projectfolder));
	url_render_task +=  "/api/web/v2/job/submit?";
	url_render_task +=  "username=";
	url_render_task +=  username;
	url_render_task +=  "&token=";
	url_render_task +=  token;
	url_render_task += "&client_id=";
	url_render_task += g_cri.clientid;
	url_render_task += "&appid=";
	url_render_task += APP_ID;
	url_render_task += "&apptoken=";
	url_render_task += APP_TOKEN;
	url_render_task +=  "&job={";
	url_render_task +=  "\"guid\":\"Elara\",";
	url_render_task +=  "\"scene_file\":\"";
	url_render_task +=  REMOTE_PATH;
	url_render_task +=  filename_encoded;
	url_render_task +=  "\",";
	url_render_task +=  "\"job_name\":\"";
	url_render_task +=  filename_encoded;
	url_render_task +=  "\",";
	url_render_task +=  "\"project_dir\":\"";
	url_render_task +=  project_encoded;
	url_render_task +=  "\",";
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
	url_render_task +=  prefix_encoded;
	url_render_task +=  "\",";
	url_render_task +=  "\"do_analysis\":true,";
	url_render_task +=  "\"priority\":\"50\",";
	url_render_task +=  "\"sub_task_frames\":1,";
	url_render_task +=  "\"start_frame\":1,";
	url_render_task +=  "\"stop_frame\":1,";
	url_render_task +=  "\"by_frame\":1,";
	url_render_task +=  "\"pool_id\":\"85eca1b25098c8a86dff105aa7068290\"";	

	// handle cameras
	if (g_skp2ess_set.camera_num > 0)
	{
		url_render_task +=  ",";
		url_render_task += "\"cameras\":[";
		for (int i = 0; i < MAX_MODEL_SCENES; i++)
		{
			if (g_skp2ess_set.cameras_index[i] == true)
			{
				char c_i[8] = "";
				sprintf(c_i, "%d", i);
				url_render_task += "\"inst_SceneCamera_";
				url_render_task += c_i;
				url_render_task += "\",";
			}
		}
		url_render_task.erase(url_render_task.size() - 1, 1);
		url_render_task += "]";
	}
	url_render_task +=  "}";
	g_cri.ch.post(url_render_task.c_str(), 10008, "");

	Json::Value root;
	Json::Reader reader;	
	if(!reader.parse(g_cri.ch.retBuffer, root))
	{
		std::cout<<"return json error."<<std::endl;
		std::cout<<g_cri.ch.retBuffer<<std::endl;
		return 2;
	}
	int max_cameras = max(1, g_skp2ess_set.camera_num);
	for (int index = 0; index < max_cameras; index++)
	{
		g_cri.job_ids[index] = root["data"]["job_ids"][index].asString();
	}
	return 0;
}

void stopRenderJobBySceneIndex(int scene_index)
{
	if (g_cri.c_state[scene_index] == CLOUD_STATE_WAIT_RENDER || g_cri.c_state[scene_index] == CLOUD_STATE_RENDERING)
	{
		std::string url_render_task = CLOUD_URL;
		url_render_task += "/api/web/v2/job/stop?";
		url_render_task += "username=";
		url_render_task += g_cri.username;
		url_render_task += "&token=";
		url_render_task += g_cri.token;
		url_render_task += "&job_id=";
		url_render_task += g_cri.job_ids[scene_index];
		url_render_task += "&client_id=";
		url_render_task += g_cri.clientid;
		url_render_task += "&appid=";
		url_render_task += APP_ID;
		url_render_task += "&apptoken=";
		url_render_task += APP_TOKEN;
		g_cri.ch.post(url_render_task.c_str(), 10008, "");
		g_cri.c_state[scene_index] = CLOUD_STATE_STOP;
	}
}

void resumeRenderJobBySceneIndex(int scene_index)
{
	if (g_cri.c_state[scene_index] == CLOUD_STATE_STOP)
	{
		std::string url_render_task = CLOUD_URL;
		url_render_task += "/api/web/v2/job/recover?";
		url_render_task += "username=";
		url_render_task += g_cri.username;
		url_render_task += "&token=";
		url_render_task += g_cri.token;
		url_render_task += "&job_id=";
		url_render_task += g_cri.job_ids[scene_index];
		url_render_task += "&client_id=";
		url_render_task += g_cri.clientid;
		url_render_task += "&appid=";
		url_render_task += APP_ID;
		url_render_task += "&apptoken=";
		url_render_task += APP_TOKEN;
		g_cri.ch.post(url_render_task.c_str(), 10008, "");
		g_cri.c_state[scene_index] = CLOUD_STATE_WAIT_RENDER;
	}
}

void restartRenderJobBySceneIndex(int scene_index)
{
	std::string url_render_task = CLOUD_URL;
	url_render_task += "/api/web/v2/job/restart?";
	url_render_task += "username=";
	url_render_task += g_cri.username;
	url_render_task += "&token=";
	url_render_task += g_cri.token;
	url_render_task += "&job_id=";
	url_render_task += g_cri.job_ids[scene_index];
	url_render_task += "&client_id=";
	url_render_task += g_cri.clientid;
	url_render_task += "&appid=";
	url_render_task += APP_ID;
	url_render_task += "&apptoken=";
	url_render_task += APP_TOKEN;
	g_cri.ch.post(url_render_task.c_str(), 10008, "");
}

void abandonRenderJobBySceneIndex(int scene_index)
{
	if (g_cri.c_state[scene_index] == CLOUD_STATE_WAIT_RENDER || g_cri.c_state[scene_index] == CLOUD_STATE_RENDERING)
	{
		std::string url_render_task = CLOUD_URL;
		url_render_task += "/api/web/v2/job/delete?";
		url_render_task += "username=";
		url_render_task += g_cri.username;
		url_render_task += "&token=";
		url_render_task += g_cri.token;
		url_render_task += "&job_id=";
		url_render_task += g_cri.job_ids[scene_index];
		url_render_task += "&client_id=";
		url_render_task += g_cri.clientid;
		url_render_task += "&appid=";
		url_render_task += APP_ID;
		url_render_task += "&apptoken=";
		url_render_task += APP_TOKEN;
		g_cri.ch.post(url_render_task.c_str(), 10008, "");
		g_cri.c_state[scene_index] = CLOUD_STATE_UNFIND;
	}
}

int CloudRender(const char* exePath, const char* filename, const char* outputprefix, const char* outputtype, const char* outputpath, const char* projectfolder)
{
	for (int i = 0; i < MAX_MODEL_SCENES; i++)
	{
		g_cri.c_state[i] = CLOUD_STATE_INITIAL;
	}
	char username[128] = "";
	char token[128] = "";
	std::string essFilename = string_To_UTF8(filename);
	if (!upload_ess(exePath, essFilename.c_str(), outputprefix, outputtype, outputpath, username, token))
	{
		return 1;
	}

	// waiting upload
	while (g_cri.c_state[0] <= CLOUD_STATE_TRANSFERRING)
	{
		Sleep(2000);
	}
	
	//g_cri.api.UnInitial();
	if (g_cri.c_state[0] == CLOUD_STATE_TRANSFER_FAILED)
	{
		return 1;
	}

	submit_task(exePath, filename, outputprefix, outputtype, outputpath, username, token, projectfolder);
	
	int max_cameras = max(1, g_skp2ess_set.camera_num);
	std::string url_job_info[MAX_MODEL_SCENES];
	std::string url_output_file[MAX_MODEL_SCENES];
	for (int index = 0; index < max_cameras; index++)
	{
		url_job_info[index] = CLOUD_URL;
		url_job_info[index] +=  "/api/web/v2/job/info?";
		url_job_info[index] +=  "username=";
		url_job_info[index] +=  username;
		url_job_info[index] +=  "&token=";
		url_job_info[index] +=  token;
		url_job_info[index] +=  "&job_id=";
		url_job_info[index] +=  g_cri.job_ids[index];
		url_job_info[index] += "&client_id=";
		url_job_info[index] += g_cri.clientid;
		url_job_info[index] += "&appid=";
		url_job_info[index] += APP_ID;
		url_job_info[index] += "&apptoken=";
		url_job_info[index] += APP_TOKEN;

		url_output_file[index] = CLOUD_URL;
		url_output_file[index] += "/api/web/v2/job/output_files2?";
		url_output_file[index] +=  "username=";
		url_output_file[index] +=  username;
		url_output_file[index] +=  "&token=";
		url_output_file[index] +=  token;
		url_output_file[index] +=  "&job_id=";
		url_output_file[index] +=  g_cri.job_ids[index];
		url_output_file[index] += "&client_id=";
		url_output_file[index] += g_cri.clientid;
		url_output_file[index] += "&appid=";
		url_output_file[index] += APP_ID;
		url_output_file[index] += "&apptoken=";
		url_output_file[index] += APP_TOKEN;
	}
	
	// 以下信息从web api的 domain接口获取
	LHDTSDK::LHDTConfig config;
	config.method = LHDTSDK::ASPERA;
	config.serverId = 7; // 分中心Id
	config.serverIp = CLOUD_CENTER_IP; // 分中心服务器Ip
	config.serverPort = SERVER_PORT; // 传输端口
	config.retryCount = 3; // 重试次数
	config.app = "GF"; // app名 GF=GoldenFarm
	config.appVersion = "2.0.0"; // 传输协议版本
	if (g_cri.transferMaxSpeed > 0)
	{
		config.maxDownloadRate = g_cri.transferMaxSpeed;
	}

	// get scene index for task index
	int *scene_index_to_task_index = new int[max_cameras];
	for (int i = 0, j = 0; i < MAX_MODEL_SCENES; i++)
	{
		if (g_skp2ess_set.cameras_index[i] == true)
		{
			scene_index_to_task_index[j] = i;
			j++;
		}

		if (j >= max_cameras) 
			break;
	}

	int finish_render[MAX_MODEL_SCENES] = {false};
	int finished_render_task = 0;
	do{
		finished_render_task = 0;
		for (int index = 0; index < max_cameras; index++)
		{
			if (finish_render[index])
			{
				finished_render_task++;
				continue;
			}

			if (g_cri.c_state[scene_index_to_task_index[index]] == CLOUD_STATE_UNFIND)
			{
				finish_render[index] = true;
				g_cri.finished_tasks++;
			}

			Json::Value root;
			Json::Reader reader;
			std::string job_status = "0";
			g_cri.ch.get(url_job_info[index].c_str(), 10008);
			if(!reader.parse(g_cri.ch.retBuffer, root))
			{
				std::cout<<"return json error."<<std::endl;
				std::cout<<g_cri.ch.retBuffer<<std::endl;
				return 2;
			}
			job_status = root["data"]["job_status"].asString();
			g_cri.jobwork_ids[scene_index_to_task_index[index]] = root["data"]["job_id"].asString();
			std::cout << "job_status:" << job_status << std::endl;
			if (job_status == JOB_STATUS_RENDER)
			{
				g_cri.c_state[scene_index_to_task_index[index]] = CLOUD_STATE_RENDERING;
			}
			else if (job_status == JOB_STATUS_STOP)
			{
				g_cri.c_state[scene_index_to_task_index[index]] = CLOUD_STATE_STOP;
			}
			else if (job_status == JOB_STATUS_COMPLETE)
			{
				// get output file
				g_cri.c_state[scene_index_to_task_index[index]] = CLOUD_STATE_WAITING_OUTPUT;
				std::string output_path, final_output_path, final_outputfolder;
				g_cri.ch.get(url_output_file[index].c_str(), 10008);
				if(!reader.parse(g_cri.ch.retBuffer, root))
				{
					std::cout<<"return json error."<<std::endl;
					std::cout<<g_cri.ch.retBuffer<<std::endl;
				}
				if (root["data"]["status"].asInt() == 1)
				{
					// download file
					finish_render[index] = true;
					g_cri.c_state[scene_index_to_task_index[index]] = CLOUD_STATE_DOWNLOADING;
					g_cri.cur_download_scene = scene_index_to_task_index[index];
					char index_s[16] = "";
					sprintf(index_s, "%d", index);
					output_path = root["data"]["files"][0]["file"].asString();
					g_cri.paramTransfer = 0;
					std::string outfilename;
					std::string outfilefolder;
					LHDTSDK::LHDTTask task_download;
					
					// 下载前需要知道文件是否被占用，以重命名
					extractFilePath(output_path, outfilename, outfilefolder);
					int rename_index = 0;
					char rename_index_str[16] = "";
					final_outputfolder = outputpath;
					final_output_path = final_outputfolder;
					final_output_path += outfilename;
					do 
					{
						if (!isFileOccupied(final_output_path.c_str()))
						{
							break;
						}
						else
						{
							sprintf(rename_index_str, "rename_%d/", rename_index);
							final_outputfolder += rename_index_str;
							if (ACCESS(final_outputfolder.c_str(), 0))
							{
								if (MKDIR(final_outputfolder.c_str()))
								{
									printf("Create directory: \'%s\' failed!\n", final_outputfolder.c_str());
								}
							}
							final_output_path = final_outputfolder;
							final_output_path += outfilename;
							rename_index++;
						}
					} while(1);

					task_download.filename = outfilename.c_str();
					task_download.local = final_outputfolder.c_str();
					task_download.remote = outfilefolder.c_str();
					task_download.type = LHDTSDK::LHDTTransferType::LHDT_TT_DOWNLOAD; // 上传 or 下载
					task_download.callback = callback_download; // 回调函数
					
					executeTask(task_download, config, exePath);
				}
			}
			else if (job_status == JOB_STATUS_FAILED)
			{
				g_cri.finished_tasks++;
				std::cout<<"job failed!"<<std::endl;
				g_cri.c_state[scene_index_to_task_index[index]] = CLOUD_STATE_RENDER_FAILED;
			}
		}
		if(g_cri.finished_tasks < max_cameras)
		{
			Sleep(30000);
		}
	} while(g_cri.finished_tasks < max_cameras);
	
	delete[] scene_index_to_task_index;
	g_cri.api.UnInitial();
	g_cri.reset();
    return 0;
}