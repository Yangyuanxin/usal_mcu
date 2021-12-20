/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   bms_com
����:   km
˵��:   bms���ݴ���.
***************************************************************************************************/

#ifndef __BMS_COM_H__                     //���ذ���.
#define __BMS_COM_H__
/**************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
***************************************************************************************************/
#include "base_type.h"
#include "bms_device.h"

#define GB_BMS_PF_CRM                             0x01    //������ʶ
#define GB_BMS_PF_BRM                             0x02    //BMS�ͳ�����ʶ (���)

#define GB_BMS_PF_BCP                             0x06    //��س����� (���)
#define GB_BMS_PF_CTS                             0x07    //����ʱ��ͬ��
#define GB_BMS_PF_CML                             0x08    //��������������
#define GB_BMS_PF_BRO                             0x09    //BMS��ɳ��׼��
#define GB_BMS_PF_CRO                             0x0A    //������ɳ��׼��

#define GB_BMS_PF_BCL                             0x10    //��س������
#define GB_BMS_PF_BCS                             0x11    //��س����״̬ (���)
#define GB_BMS_PF_CCS                             0x12    //�������״̬
#define GB_BMS_PF_BSM                             0x13    //���״̬��Ϣ

#define GB_BMS_PF_BMV                             0x15    //���嶯�����ص�ѹ (���)
#define GB_BMS_PF_BMT                             0x16    //���������¶� (���)
#define GB_BMS_PF_BSP                             0x17    //��������Ԥ������ (���)
#define GB_BMS_PF_BST                             0x19    //BMSֹͣ���
#define GB_BMS_PF_CST                             0x1A    //����ֹͣ���

#define GB_BMS_PF_BSD                             0x1C    //BMSͳ��
#define GB_BMS_PF_CSD                             0x1D    //����ͳ��

#define GB_BMS_PF_BEM                             0x1E    //BMS����
#define GB_BMS_PF_CEM                             0x1F    //��������

#define GB_CH_PF_CHM                              0x26    //�������ֱ���
#define GB_BMS_PF_BHM                             0x27    //�������ֱ���

//HARDWARE_CHECK_MODE
#define GB_BMS_PF_BCM                             0x41    //BMS���͵Ĺ�װ��Ϣ����
#define GB_CH_PF_CCM                              0x40    //���׮�ظ��Ĺ�װ������


#define PT_BMS_PF_CRM                             0x01    //������ʶ
#define PT_BMS_PF_BRM                             0x02    //BMS�ͳ�����ʶ (���)

#define PT_BMS_PF_BCP                             0x06    //��س����� (���)
#define PT_BMS_PF_CTS                             0x07    //����ʱ��ͬ��
#define PT_BMS_PF_CML                             0x08    //��������������
#define PT_BMS_PF_BRO                             0x09    //BMS��ɳ��׼��
#define PT_BMS_PF_CRO                             0x0A    //������ɳ��׼��

#define PT_BMS_PF_BCL                             0x10    //��س������
#define PT_BMS_PF_BCS                             0x11    //��س����״̬ (���)
#define PT_BMS_PF_CCS                             0x14    //�������״̬
#define PT_BMS_PF_BSM                             0x12    //���״̬��Ϣ

#define PT_BMS_PF_BMV                             0x15    //���嶯�����ص�ѹ (���)
#define PT_BMS_PF_BMT                             0x16    //���������¶� (���)
#define PT_BMS_PF_BSP                             0x17    //��������Ԥ������ (���)
#define PT_BMS_PF_BST                             0x19    //BMSֹͣ���
#define PT_BMS_PF_CST                             0x1A    //����ֹͣ���

#define PT_BMS_PF_BSD                             0x1C    //BMSͳ��
#define PT_BMS_PF_CSD                             0x1D    //����ͳ��

#define PT_BMS_PF_BEM                             0x1E    //BMS����
#define PT_BMS_PF_CEM                             0x1F    //��������

/***************************************************************************************************
����:   ���б���.
***************************************************************************************************/


/***************************************************************************************************
����:   ���з���.
***************************************************************************************************/
void bms_start_charger_init(uint8_t gunSn);
void bms_charger_scheduler(void);

#endif  //__BMS_COM_H__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/


