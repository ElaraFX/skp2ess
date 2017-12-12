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
 * 返回值解释：
 *    0    渲染成功
 *    1    ESS翻译失败
 *    2    ESS上传失败
 *    3    服务器返回数据解析出错
 *    4    渲染图片失败
 */
extern "C" __declspec(dllexport) int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath);


extern "C" __declspec(dllexport) void setResolution(int x, int y);

#endif
