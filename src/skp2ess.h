/**************************************************************************
 * Copyright (C) 2017 Rendease Co., Ltd.
 * All rights reserved.
 *
 * This program is commercial software: you must not redistribute it 
 * and/or modify it without written permission from Rendease Co., Ltd.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * End User License Agreement for more details.
 *
 * You should have received a copy of the End User License Agreement along 
 * with this program.  If not, see <http://www.rendease.com/licensing/>
 *************************************************************************/

#ifndef SKP2ESS_H
#define SKP2ESS_H

#include <string>

/** 功能：该函数执行以下步骤：
 * 
 *   1. 将用户输入的.skp文件翻译为.ess文件
 *   2. 上传.ess文件到云端并执行云渲染任务
 *   3. 等待渲染结束并下载结果图片，输出到指定路径
 *
 * 该函数是阻塞函数，会一直等到任务完成再返回值
 *
 * 注意：后面的所有设置参数类的API都需要在放在该函数之前执行
 * 
 * 参数：
 *   exePath：项目程序路径
 *   filename：skp文件名
 *   projectname：工程名
 *   outputprefix：输出文件前缀
 *   outputtype：输出文件类型
 *   outputpath：输出文件目录
 *   projectfolder：服务器上场景目录(用于服务器上精模的查找)
 * 返回值解释：
 *    0    渲染成功
 *    1    ESS翻译失败
 *    2    ESS上传失败
 *    3    服务器返回数据解析出错
 *    4    渲染图片失败
 */
extern "C" __declspec(dllexport) int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath, const char* projectfolder);

/** 功能：转换skp到ess
 *   filename：skp文件名
 *   projectname：工程名
 *   outputpath：输出文件目录
 */
extern "C" __declspec(dllexport) int skp2ess(const char* filename, const char* projectname, const char* outputpath);

/** 功能：lhdt_sdk api uninitial
 */
extern "C" __declspec(dllexport) void apiUninitial();

/** 功能：lhdt_sdk api initial
 */
extern "C" __declspec(dllexport) void apiInitial(char *exePath);

/** 功能：设置渲染分辨率：*/ 
extern "C" __declspec(dllexport) void setResolution(int x, int y);

/** 功能：设置相机类型：
 *  0: 普通透视相机
 *  1: cubmap全景相机
 *  2: 球面全景相机
 */
extern "C" __declspec(dllexport) void setCameraType(unsigned int t);

/** 功能：设置环境模式：
 *  0: 白天室内
 *  1: 夜晚
 *  2: 白天室外
 */
extern "C" __declspec(dllexport) void setEnviroment(unsigned int t);

/** 功能：设置上传下载的最大速度：
 *  单位Mbps, 0~80 默认8Mbps
 */ 
extern "C" __declspec(dllexport) void setTranferMaxSpeed(int s);

/** 功能：设置渲染质量参数：
 *  0: 中等质量
 *  1: 低质量
 *  2: 高质量
 *  3: 默认
 */
extern "C" __declspec(dllexport) void setRenderQuality(unsigned int q);

/** 功能：设置多场景渲染参数：
 *  参数：scene_indices: 参与渲染的场景编号
 *  参数：num: 参与渲染的场景数目，如果只渲染默认场景，请给0
 */
extern "C" __declspec(dllexport) void setScenes(int *scene_indices, int num);

/** 功能：获取当前任务状态：
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 *	参数：state: 当前状态
 *  0: 格式转换(skp - ess)
 *  1: 上传ess
 *  2: 排队等待渲染
 *  3: 正在渲染
 *  4: 正在生成结果文件
 *  5: 下载结果文件
 *  6: 流程結束
 *  7: 上传或下载文件失败
 *  8: 登陆错误
 *  9: 任务停止
 *  10: 渲染任务失败
 *  11: 任务未开始
 *  --------------------------
 *  param: 当状态处于上传或者下载的时候，该参数可以获得上传下载进度
 */
extern "C" __declspec(dllexport) void getState(int scene_index, unsigned int *state, float *param);

/** 功能：删除当前任务：
 *  注意：删除的任务无法恢复
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 */
extern "C" __declspec(dllexport) void deleteRenderJob(int scene_index);

/** 功能：暂停当前任务：
 *  注意：暂停的任务如果不删除，会一直保留在任务队列，导致主函数skpCloudRender无法退出
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 */
extern "C" __declspec(dllexport) void stopRenderJob(int scene_index);

/** 功能：继续暂停的任务：
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 */
extern "C" __declspec(dllexport) void resumeRenderJob(int scene_index);

/** 功能：重新启动任务(相当于先暂停再继续任务)：
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 */
extern "C" __declspec(dllexport) void restartRenderJob(int scene_index);

/** 功能：通过场景序号获得JOB_ID号：
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 *  注意：如果此时渲染作业还未提交，则JOB_ID号为空
 *  返回值：true: 场景存在，false: 场景不存在
 */
extern "C" __declspec(dllexport) bool getJobID(int scene_index, char *job_id);

/** 功能：通过场景序号获得作业ID号：
 *  参数：scene_index: skp中场景序号，如果是默认场景，请给0
 *  注意：如果此时渲染作业还未提交，则作业ID号为空
 *  返回值：true: 场景存在，false: 场景不存在
 */
extern "C" __declspec(dllexport) bool getJobWorkID(int scene_index, char *job_id);

/** 功能：设置云平台登陆的用户名密码
 */
extern "C" __declspec(dllexport) void setUsernamePassword(const char *username, const char *password);

/** 功能：设置曝光级别(数值越低亮度越高，默认值0)
 */
extern "C" __declspec(dllexport) void setExposureValue(float e);

/** 功能：关闭曝光级别设置，转而使用每个环境模式的默认曝光值
 */
extern "C" __declspec(dllexport) void disableExposureValue();

/** 功能：设置客户端ID
 */
extern "C" __declspec(dllexport) void setClientID(char *client_id);

/** 功能：设置户外HDR文件(参数为HDR文件名，如"day.hdr")
 */
extern "C" __declspec(dllexport) void setHDRname(char *hdr_name);

/** 功能：设置户外HDR亮度(不设置的话，默认值为1)
 */
extern "C" __declspec(dllexport) void setHDRmultipler(float multipler);


#endif
