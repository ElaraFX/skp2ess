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
 * ע�⣺������������ò������API����Ҫ�ڷ��ڸú���֮ǰִ��
 * 
 * ������
 *   exePath����Ŀ����·��
 *   filename��skp�ļ���
 *   projectname��������
 *   outputprefix������ļ�ǰ׺
 *   outputtype������ļ�����
 *   outputpath������ļ�Ŀ¼
 * ����ֵ���ͣ�
 *    0    ��Ⱦ�ɹ�
 *    1    ESS����ʧ��
 *    2    ESS�ϴ�ʧ��
 *    3    �������������ݽ�������
 *    4    ��ȾͼƬʧ��
 */
//extern "C" __declspec(dllexport) int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath);
//
///** ���ܣ�������Ⱦ�ֱ��ʣ�*/ 
//extern "C" __declspec(dllexport) void setResolution(int x, int y);
//
///** ���ܣ����û���ģʽ��
// *  0: ��������
// *  1: ҹ��
// *  2: ��������
// */
//extern "C" __declspec(dllexport) void setEnviroment(unsigned int t);
//
///** ���ܣ������ϴ����ص�����ٶȣ�
// *  ��λMbps, 0~80 Ĭ��8Mbps
// */ 
//extern "C" __declspec(dllexport) void setTranferMaxSpeed(int s);
//
///** ���ܣ�������Ⱦ����������
// *  0: �е�����
// *  1: ������
// *  2: ������
// *  3: Ĭ��
// */
//extern "C" __declspec(dllexport) void setRenderQuality(unsigned int q);
//
///** ���ܣ���ȡ��ǰ����״̬��
// *	������state: ��ǰ״̬
// *  0: ��ʽת��(skp - ess)
// *  1: �ϴ�ess
// *  2: �Ŷӵȴ���Ⱦ
// *	3: ������Ⱦ
// *	4: �������ɽ���ļ�
// *  5: ���ؽ���ļ�
// *  6: ���̽Y��
// *  7: �ϴ��������ļ�ʧ��
// *  8: ��½����
// *  --------------------------
// *  param: ��״̬�����ϴ��������ص�ʱ�򣬸ò������Ի���ϴ����ؽ���
// */
//extern "C" __declspec(dllexport) void getState(unsigned int *state, float *param);
//
///** ���ܣ�������ƽ̨��½���û�������
// */
//extern "C" __declspec(dllexport) void setUsernamePassword(const char *username, const char *password);

#endif
