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

/** ���ܣ��ú���ִ�����²��裺
 * 
 *   1. ���û������.skp�ļ�����Ϊ.ess�ļ�
 *   2. �ϴ�.ess�ļ����ƶ˲�ִ������Ⱦ����
 *   3. �ȴ���Ⱦ���������ؽ��ͼƬ�������ָ��·��
 *
 * �ú�����������������һֱ�ȵ���������ٷ���ֵ
 * 
 * ����ֵ���ͣ�
 *    0    ��Ⱦ�ɹ�
 *    1    ESS����ʧ��
 *    2    ESS�ϴ�ʧ��
 *    3    �������������ݽ�������
 *    4    ��ȾͼƬʧ��
 */
extern "C" __declspec(dllexport) int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath);


extern "C" __declspec(dllexport) void setResolution(int x, int y);

#endif
