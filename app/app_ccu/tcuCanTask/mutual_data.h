/***************************************************************************************************   
Copyright @ 2020-20xxС�ۿƼ�

�ļ�:   com_data
����:   km
˵��:   ͨ������.
***************************************************************************************************/


#ifndef __MUTUAL_DATA_H__                     //���ذ���.
#define __MUTUAL_DATA_H__

#include "sys_config.h"
#include "bms_device.h"
#include "alarm_task.h"
#include "charge_task.h"


typedef union 
{
    uint16_t all;
    struct 
    {
        uint16_t    bAcConTSt       :1;       //AC�Ӵ�������״̬
        uint16_t    bDcConTSt       :1;       //DC�Ӵ�������״̬
        uint16_t    bMidConTSt1     :1;       //�м�Ӵ�����һ�����״̬
        uint16_t    bMidConTSt2     :1;       //�м�Ӵ����ڶ������״̬  
		
        uint16_t    bMidConTSt3     :1;       //�м�Ӵ������������״̬  
        uint16_t    bGunLockSt      :1;       //����������״̬  
        uint16_t    bFanSt          :1;       //���ȿ���״̬
        uint16_t    bBmsAxSt        :1;       //bms������Դ����״̬
		
        uint16_t    bSwSt           :1;       //���ػ�״̬��0�ػ� 1����
        uint16_t    bInsDetSt       :2;       //��Ե״̬

        uint16_t    brsv            :5;    
    }bit;
}unionFlagCtrlSt;//����״̬

typedef union 
{
    uint16_t all;
    struct 
    {
        uint16_t    bAcConTSt       :1;         //AC�Ӵ�������
        uint16_t    bDcConTSt       :1;         //DC�Ӵ�������
        uint16_t    bMidConTSt1     :1;         //�м�Ӵ�1����
        uint16_t    bMidConTSt2     :1;         //�м�Ӵ�2����

        uint16_t    bMidConTSt3     :1;         //�м�Ӵ�3����
        uint16_t    bEpoSt          :1;         //��ͣ����       
        uint16_t    bCC1St          :1;         //cc1
        uint16_t    bDoorSt         :1;         //�Ž�����
        uint16_t    bSPDSt          :1;         //���׷���

        uint16_t    bGunToPileSt    :1;         //ǹ��λ������0�ѹ�λ
        uint16_t    bGunLockSt      :1;         //ǹ������
        uint16_t    bFuseSt         :1;         //�۶�������
        uint16_t    bBreakerSt      :1;         //��·������
        
        uint16_t    rsv             :4;                       
    }bit;
}unionFlagBackSt;//����״̬

typedef struct 
{
    unionFlagCtrlSt unFlagCtrlSt;  
    unionFlagBackSt unFlagBackSt;

    int16_t iAcAVolt_10x;           //A���ѹ
    int16_t iAcBVolt_10x;           //B���ѹ
    int16_t iAcCVolt_10x;           //C���ѹ
    int16_t iAcHz;                  //����Ƶ��
    int32_t iMeterE_1000x;          //��ǰ��������tcu�·���ccu��

	int16_t uInsDetRes[2];          //��Ե��������
	int16_t iInsDetPosVolt[2];      //��Ե����Ͷ������ѹ
	int16_t iInsDetNegVolt[2];      //��Ե����Ͷ�и���ѹ
	
	int16_t iCc1Vol_10x;			//������ѹ
    int16_t iModVolt_10x;           //ģ���ѹ
    int16_t iSysVolt_10x;           //ǹ��ѹ
    int16_t iSysCurr_10x;           //��ǹ����  

    int16_t iVolt_Demand_10x;       //�����ѹ
    int16_t iCurr_Demand_10x;       //�������

    int16_t iOutletTemp_10x;        //������¶�
    int16_t iInletTemp_10x;         //������¶�
	int16_t iChGunTempMax_10x;      //��ǰ�������ǹ��	
	int16_t iChGunTempP_10x;        //��ǰ��ǹ��	
	int16_t iChGunTempN_10x;        //��ǰ��ǹ��	

}structSysInfo;//ϵͳ��Ϣ

#if 0
typedef union
{
    uint16_t all;
    struct
    {
        uint16_t bEPO:1;               //��ͣ����
        uint16_t bBREAKER:1;           //�����·������
        uint16_t bFUSE:1;              //����۶�������      
        uint16_t bFAN:1;               //���ȹ���
        uint16_t bSPD:1;               //����������
        uint16_t bOUTLETTEMPUP:1;      //���׮���¹���
        uint16_t bVGRIDUP:1;           //�����ѹ����
        uint16_t bVGRIDLOW:1;          //����Ƿѹ����

        uint16_t bDOOR:1;             //�Ž�����
        uint16_t bACCONT:1;           //����Ӵ�������
        uint16_t bAX:1;               //��Դ����---���籣��
        uint16_t bMIDCONT:1;          //�м�Ӵ�������   
		uint16_t bTCUCOM:1;           //TCUͨ��ʧ��
        uint16_t bMODCOMFAULT:1;      //ģ��ͨ�Ź���---����ģ��ʧ��,�б�������
        uint16_t bMETER_COM:1;        //���ͨ�Ź���--Ԥ��
        uint16_t bOUTLETTEMPUP_2:1;   //���׮���¸澯---�澯
    }bit;
}unionAlarmA;

typedef union
{
    uint16_t all;
    struct
    {
        uint16_t bMOD:1;              //ģ�����
        uint16_t bMOD_AC:1;           //ģ���������---�澯
        uint16_t bMOD_ACUP:1;         //ģ�������ѹ����---�澯
        uint16_t bMOD_ACLOW:1;        //ģ������Ƿѹ����---�澯
        uint16_t bMOD_ACLOSE:1;       //ģ������ȱ�����---�澯
        uint16_t bMOD_SHORT:1;        //ģ�������·����---�澯
        uint16_t bMOD_DCOI:1;         //ģ�������������---�澯
        uint16_t bMOD_DCUP:1;         //ģ�������ѹ����---�澯
        
        uint16_t bMOD_DCLOW:1;        //ģ�����Ƿѹ����---�澯
        uint16_t bMOD_OT:1;           //ģ����¹���---�澯
        uint16_t bMOD_COM:1;          //ģ��ͨ�Ź���---�澯
        uint16_t bMOD_FAN:1;          //ģ����ȹ���---�澯
        uint16_t bRSV0:1;             //Ԥ��
        uint16_t bRSV1:1;             //Ԥ��
        uint16_t bRSV2:1;             //Ԥ��
        uint16_t bRSV3:1;             //Ԥ��
    }bit;
}unionAlarmB;

typedef union
{
    uint16_t all;
    struct
    {
        uint16_t bDCCONT:1;            //ֱ���Ӵ�������
        uint16_t bGunLock:1;           //���ǹ�����ϣ��������м��ǹ��״̬�ô�,��׼������
        uint16_t bINSDETFAIL:1;        //��Ե����
        uint16_t bVDCINV:1;            //��ط��ӹ���
        uint16_t bCC1:1;               //���Ƶ�������
        uint16_t bGUNTEMPUP:1;         //���ǹ���¹���
        uint16_t bGUNNOTBACK:1;        //���ǹδ��λ---�澯
        uint16_t bBMS_COM:1;           //BMSͨ�Ź���,��׼������
        
        uint16_t bVDCUP:1;            //ֱ����ѹ����
        uint16_t bVDCLOW:1;           //ֱ��Ƿѹ����
        uint16_t bIDCUP:1;            //ֱ����������
        uint16_t bINSDETABS:1;        //��Ե���澯---�澯
        uint16_t bGUNTEMPUP_2:1;      //���ǹ���¸澯---�澯
        uint16_t bShort:1;             //�����·����
        uint16_t bDISCHARGE:1;        //й�Ż�·����,��׼������
		uint16_t bRSV0:1;             //Ԥ��
    }bit;
}unionAlarmC;

typedef struct 
{
    uint32_t originalCode;      //ֹͣ��ENUM_ALARM_STOP
    char *tip;               //��ʾ����¼��
    ENUM_ALARM_TYPE alarmtype;  //alarm�ȼ�
    
}StructStopCodeMap;

typedef enum
{ 
    BOOL_FALSE = 0,
    BOOL_TRUE = 1,
}BOOL;

typedef union 
{
    uint16_t all;
    struct
    {
        uint16_t idle           :1;  //����
        uint16_t starting       :1;  //������    
        uint16_t startok        :1;  //�����ɹ�
        uint16_t startfail      :1;  //����ʧ��
        uint16_t charging       :1;  //�����  
        uint16_t over           :1;  //������,���״̬������5s������
        uint16_t fault          :1;  //����
        uint16_t bworking       :1;  //����������

        uint16_t insdet         :1;  //1 ��Ե�����
    }bit;
}UNION_SYS_CHARGE_STEP;

#endif

typedef struct 
{    
    structSysInfo sSysInfo;
    //ChargerSaveData *sChargeInfo;
    BmsFrameData *sBmsInfo;
	
    unionAlarmA uAlarmA;     //ENUM_ALARM_STOP 1-16
    unionAlarmB uAlarmB;     //ENUM_ALARM_STOP 17-32
    unionAlarmC uAlarmC;      //ENUM_ALARM_STOP 33-48:1ǹ  49-64:2ǹ 
	UNION_SYS_CHARGE_STEP charge_stage;
    unionChargerDeviceFlag flag;
    uint16_t sofewareVer;   
    
}structComData;


extern structComData sMutualData[2];//�����������ʾ����

uint32_t mutual_get_current_stopcause(uint8_t gunSn); 
BOOL tcu_start_charger_interface(uint8_t gunSn);
void tcu_set_charger_stopcause(uint32_t stopCause,uint8_t gunSn);




void mutual_data_init(void);
void mutual_data_deal(void);

#endif  //__DWIN_DATA_H__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
