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
 *   projectfolder���������ϳ���Ŀ¼(���ڷ������Ͼ�ģ�Ĳ���)
 * ����ֵ���ͣ�
 *    0    ��Ⱦ�ɹ�
 *    1    ESS����ʧ��
 *    2    ESS�ϴ�ʧ��
 *    3    �������������ݽ�������
 *    4    ��ȾͼƬʧ��
 */
extern "C" __declspec(dllexport) int skpCloudRender(const char* exePath, const char* filename, const char* projectname, const char* outputprefix, const char* outputtype, const char* outputpath, const char* projectfolder);

/** ���ܣ�ת��skp��ess
 *   filename��skp�ļ���
 *   projectname��������
 *   outputpath������ļ�Ŀ¼
 */
extern "C" __declspec(dllexport) int skp2ess(const char* filename, const char* projectname, const char* outputpath);

/** ���ܣ�lhdt_sdk api uninitial
 */
extern "C" __declspec(dllexport) void apiUninitial();

/** ���ܣ�lhdt_sdk api initial
 */
extern "C" __declspec(dllexport) void apiInitial(char *exePath);

/** ���ܣ�������Ⱦ�ֱ��ʣ�*/ 
extern "C" __declspec(dllexport) void setResolution(int x, int y);

/** ���ܣ�����������ͣ�
 *  0: ��ͨ͸�����
 *  1: cubmapȫ�����
 *  2: ����ȫ�����
 */
extern "C" __declspec(dllexport) void setCameraType(unsigned int t);

/** ���ܣ����û���ģʽ��
 *  0: ��������
 *  1: ҹ��
 *  2: ��������
 */
extern "C" __declspec(dllexport) void setEnviroment(unsigned int t);

/** ���ܣ������ϴ����ص�����ٶȣ�
 *  ��λMbps, 0~80 Ĭ��8Mbps
 */ 
extern "C" __declspec(dllexport) void setTranferMaxSpeed(int s);

/** ���ܣ�������Ⱦ����������
 *  0: �е�����
 *  1: ������
 *  2: ������
 *  3: Ĭ��
 */
extern "C" __declspec(dllexport) void setRenderQuality(unsigned int q);

/** ���ܣ����öೡ����Ⱦ������
 *  ������scene_indices: ������Ⱦ�ĳ������
 *  ������num: ������Ⱦ�ĳ�����Ŀ�����ֻ��ȾĬ�ϳ��������0
 */
extern "C" __declspec(dllexport) void setScenes(int *scene_indices, int num);

/** ���ܣ���ȡ��ǰ����״̬��
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 *	������state: ��ǰ״̬
 *  0: ��ʽת��(skp - ess)
 *  1: �ϴ�ess
 *  2: �Ŷӵȴ���Ⱦ
 *  3: ������Ⱦ
 *  4: �������ɽ���ļ�
 *  5: ���ؽ���ļ�
 *  6: ���̽Y��
 *  7: �ϴ��������ļ�ʧ��
 *  8: ��½����
 *  9: ����ֹͣ
 *  10: ��Ⱦ����ʧ��
 *  11: ����δ��ʼ
 *  --------------------------
 *  param: ��״̬�����ϴ��������ص�ʱ�򣬸ò������Ի���ϴ����ؽ���
 */
extern "C" __declspec(dllexport) void getState(int scene_index, unsigned int *state, float *param);

/** ���ܣ�ɾ����ǰ����
 *  ע�⣺ɾ���������޷��ָ�
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 */
extern "C" __declspec(dllexport) void deleteRenderJob(int scene_index);

/** ���ܣ���ͣ��ǰ����
 *  ע�⣺��ͣ�����������ɾ������һֱ������������У�����������skpCloudRender�޷��˳�
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 */
extern "C" __declspec(dllexport) void stopRenderJob(int scene_index);

/** ���ܣ�������ͣ������
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 */
extern "C" __declspec(dllexport) void resumeRenderJob(int scene_index);

/** ���ܣ�������������(�൱������ͣ�ټ�������)��
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 */
extern "C" __declspec(dllexport) void restartRenderJob(int scene_index);

/** ���ܣ�ͨ��������Ż��JOB_ID�ţ�
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 *  ע�⣺�����ʱ��Ⱦ��ҵ��δ�ύ����JOB_ID��Ϊ��
 *  ����ֵ��true: �������ڣ�false: ����������
 */
extern "C" __declspec(dllexport) bool getJobID(int scene_index, char *job_id);

/** ���ܣ�ͨ��������Ż����ҵID�ţ�
 *  ������scene_index: skp�г�����ţ������Ĭ�ϳ��������0
 *  ע�⣺�����ʱ��Ⱦ��ҵ��δ�ύ������ҵID��Ϊ��
 *  ����ֵ��true: �������ڣ�false: ����������
 */
extern "C" __declspec(dllexport) bool getJobWorkID(int scene_index, char *job_id);

/** ���ܣ�������ƽ̨��½���û�������
 */
extern "C" __declspec(dllexport) void setUsernamePassword(const char *username, const char *password);

/** ���ܣ������ع⼶��(��ֵԽ������Խ�ߣ�Ĭ��ֵ0)
 */
extern "C" __declspec(dllexport) void setExposureValue(float e);

/** ���ܣ��ر��ع⼶�����ã�ת��ʹ��ÿ������ģʽ��Ĭ���ع�ֵ
 */
extern "C" __declspec(dllexport) void disableExposureValue();

/** ���ܣ����ÿͻ���ID
 */
extern "C" __declspec(dllexport) void setClientID(char *client_id);

/** ���ܣ����û���HDR�ļ�(����ΪHDR�ļ�������"day.hdr")
 */
extern "C" __declspec(dllexport) void setHDRname(char *hdr_name);

/** ���ܣ����û���HDR����(�����õĻ���Ĭ��ֵΪ1)
 */
extern "C" __declspec(dllexport) void setHDRmultipler(float multipler);


#endif
