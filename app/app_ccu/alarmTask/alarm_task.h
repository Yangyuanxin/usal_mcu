/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   alarm_stopcause
����: km
˵��:   alarm deal.
***************************************************************************************************/

#ifndef __ALARM_TASK_H_
#define __ALARM_TASK_H_

#include"base_type.h"

typedef enum 
{
    ALARM_STOP_NULL = 0,

    //����+����

	//ϵͳ����
    ALARM_EPO,              //��ͣ����
    ALARM_BREAKER,          //�����·������
    ALARM_FUSE,             //����۶�������      
    ALARM_FAN,              //���ȹ���
    ALARM_SPD,              //����������
    ALARM_OUTLETTEMPUP,     //���׮���¹���
    ALARM_VGRIDUP,          //�����ѹ����
    ALARM_VGRIDLOW,         //����Ƿѹ����

	ALARM_DOOR,             //�Ž�����
	ALARM_ACCONT,           //����Ӵ�������
    ALARM_AX,               //��Դ����---���籣��
    ALARM_MIDCONT,          //�м�Ӵ�������
    ALARM_TCUCOM,           //TCUͨ�Ź���
    ALARM_MODCOMFAULT,      //ģ��ͨ�Ź���---����ģ��ʧ��,�б�������
    ALARM_METER_COM,        //���ͨ�Ź���-Ԥ��
    ALARM_OUTLETTEMPUP_2,	//���׮���¸澯---�澯
    
    ALARM_MOD,              //ģ�����
    ALARM_MOD_AC,           //ģ���������---�澯
    ALARM_MOD_ACUP,         //ģ�������ѹ����---�澯
    ALARM_MOD_ACLOW,        //ģ������Ƿѹ����---�澯
    ALARM_MOD_ACLOSE,       //ģ������ȱ�����---�澯
    ALARM_MOD_SHORT,        //ģ�������·����---�澯
    ALARM_MOD_DCOI,         //ģ�������������---�澯
    ALARM_MOD_DCUP,         //ģ�������ѹ����---�澯
    
    ALARM_MOD_DCLOW,        //ģ�����Ƿѹ����---�澯
    ALARM_MOD_OT,           //ģ����¹���---�澯
    ALARM_MOD_COM,          //ģ��ͨ�Ź���---�澯
    ALARM_MOD_FAN,          //ģ����ȹ���---�澯
    ALARM_RSV1, 			//Ԥ��1
	ALARM_RSV2,             //Ԥ��2
    ALARM_RSV3,             //Ԥ��3
	ALARM_RSV4, 			//Ԥ��4

    
	//ǹ1����
    ALARM_DCCONT_GUN1,      //GUN1ֱ���Ӵ�������
    ALARM_GunLock_GUN1, 	//GUN1���ǹ������
	ALARM_INSDETFAIL_GUN1,	//GUN1��Ե����
	ALARM_VDCINV_GUN1,      //GUN1��ط��ӹ���
    ALARM_CC1_GUN1,         //GUN1���Ƶ�������
    ALARM_GUNTEMPUP_GUN1,   //GUN1���ǹ���¹���
    ALARM_GUNNOTBACK_GUN1,  //GUN1���ǹδ��λ---�澯
    ALARM_BMS_COM_GUN1,     //GUN1-BMSͨ�Ź���
    
    ALARM_VDCUP_GUN1,       //GUN1ֱ����ѹ����-
    ALARM_VDCLOW_GUN1,      //GUN1ֱ��Ƿѹ����-
    ALARM_IDCUP_GUN1,       //GUN1ֱ����������-
	ALARM_INSDETABS_GUN1,	//GUN1��Ե���澯---�澯-
    ALARM_GUNTEMPUP_2_GUN1, //GUN1���ǹ���¸澯---�澯-
	ALARM_SHORT_GUN1,		//GUN1�����·����
	ALARM_DISCHARGE_GUN1,   //GUN1й�Ż�·����
	ALARM_RSV5, 			//Ԥ��5

	//ǹ2����
	ALARM_DCCONT_GUN2,		//GUN2ֱ���Ӵ�������
    ALARM_GunLock_GUN2,     //GUN2���ǹ������
	ALARM_INSDETFAIL_GUN2,	//GUN2��Ե����
	ALARM_VDCINV_GUN2,		//GUN2��ط��ӹ���
	ALARM_CC1_GUN2, 		//GUN2���Ƶ�������
	ALARM_GUNTEMPUP_GUN2,	//GUN2���ǹ���¹���
	ALARM_GUNNOTBACK_GUN2,	//GUN2���ǹδ��λ---�澯
	ALARM_BMS_COM_GUN2, 	//GUN2-BMSͨ�Ź���
	ALARM_VDCUP_GUN2,		//GUN2ֱ����ѹ����
    ALARM_VDCLOW_GUN2,      //GUN2ֱ��Ƿѹ����
	ALARM_IDCUP_GUN2,		//GUN2ֱ����������
	ALARM_INSDETABS_GUN2,	//GUN2��Ե���澯---�澯
	ALARM_GUNTEMPUP_2_GUN2, //GUN2���ǹ���¸澯---�澯-
    ALARM_SHORT_GUN2,       //GUN2�����·����
	ALARM_DISCHARGE_GUN2,	//GUN2й�Ż�·����
	ALARM_RSV6, 			//Ԥ��6
	


    ALARM_END_CODE,         //���ϴ��뵽�˽���
    
    //����ǹ����������ڴ�����ALARM_END_CODE֮ǰ������unionAlarm
/**************************************************�����ķָ���**************************************/
    STOP_UNDEFINE = 299,                //δ����
    STOP_BY_CARD = 300,                 //ˢ��ֹͣ
    STOP_BY_USER,                       //�û�ֹͣ
    STOP_BY_ENERGY,                     //�����ﵽ�趨
    STOP_BY_TIME,                       //ʱ��ﵽ�趨
    STOP_BY_MONEY,                      //���ﵽ�趨
    STOP_BY_MONEY_NOTENOUGH,            //����
    STOP_BY_BACKGROUP,                  //��ֹ̨ͣ
    STOP_NET_DISCONNECT,                //��̨�Ͽ�����
    STOP_VIN_ERROR,                     //VIN�벻ƥ��
    STOP_BY_ADMIN,                      //����Ա���Ľ�����ֹ
    STOP_INSDET_VBEFORE,                //��Ե���׶Σ�ǹ�ڴ�ѹ
    STOP_INSDET_TO1,                    //��Ե��ʱ��ģ��δ����
    STOP_INSDET_TO2,                    //��Ե��ʱ��ģ�鿪��
    STOP_BMS_VINSDET,                   //BMS��Ե��ѹ��
    STOP_BCP_BATVOLAGE,                 //BCP��ص�ѹ�쳣
    STOP_SOC_PROTECTION,                //SOC�ﵽ�û�����ֵ
    STOP_BMS_SINGLE_VUP,                //BMS�����ѹ����
    STOP_BMS_TEMP_MAX,                  //BMS�¶ȹ���
    STOP_BMS_STAUE,                     //BMS״̬�쳣-----
    STOP_BMS_BCSVOL,                    //BCS�ϴ���ѹ�쳣-----
    STOP_BSM_VSINGLE,                   //BMS ���嶯�����ص�ѹ
    STOP_BSM_SOC,                       //BMS �����������غɵ�״̬
    STOP_BSM_OC,                        //BMS �������س�������
    STOP_BSM_TEMPUP,                    //BMS ���������¶ȹ���
    STOP_BSM_INSDET,                    //BMS �������ؾ�Ե״̬ 
    STOP_BSM_CC,                        //BMS ���������������״̬
    STOP_BSM_BATLOW,                    //BMS ���嶯�����ص�ѹ����
    STOP_BSM_SOCLOW,                    //BMS ������������SOC����
    STOP_VOLTAGE_BMSMAX,                //����ѹ����BMS�������ֵ
    STOP_VOLTAGE_BMSDEMAND,             //����ѹ����BMS����ֵ
    STOP_BAT_CIRCUIT,                   //�����ڲ���·�쳣��ֹ
    STOP_BAT_DEMAND_VOLTAGE,            //���������ѹ��������
    STOP_BAT_DEMAND_CURRENT,            //�������������������
    STOP_SOFTFAIL_MOD_START,            //����ʧ��,ģ�鿪��
    STOP_SOFTFAIL_MOD_UNSTART,          //����ʧ��,ģ��δ����
    STOP_SOFTFAIL_TIMEOUT,              //����ʧ��,������ʱ

    STOP_TCU_UPDATE,                    //TCU����
    STOP_TCU_CTRL_ABS,                  //TCU ���ư�״̬�쳣
    STOP_TCU_UNKNOWN,                   //TCU δ֪�Ľ���ԭ��

    //BMS����ֹͣ
    BMS_STOP_CAUSE_CHARGE_STARTUP_TIMEOUT_CRM =500,  //����ʱ�䳬ʱ
    BMS_STOP_CAUSE_RECV_BRM_TIMEOUT,            //�յ�BRM���ĳ�ʱ
    BMS_STOP_CAUSE_RECV_BCP_TIMEOUT,            //�յ�BCP���ĳ�ʱ
    BMS_STOP_CAUSE_RECV_BRO_TIMEOUT,            //�յ�BR0���ĳ�ʱ
    BMS_STOP_CAUSE_RECV_BCS_TIMEOUT,            //�յ�BCS���ĳ�ʱ
    BMS_STOP_CAUSE_RECV_BCL_TIMEOUT,            //����BCL���ĳ�ʱ
    BMS_STOP_CAUSE_RECV_BSM_TIMEOUT,            //����BSM���ĳ�ʱ
    BMS_STOP_CAUSE_RECV_BEM_FRAME,              //�յ�BEM����ֹͣ
    BMS_STOP_CAUSE_RECV_BST_TIMEOUT,            //�յ�BST���ĳ�ʱ
    BMS_STOP_CAUSE_BSM_CHARGER_ALLOW_TIMEOUT,   //BSM�������쳣
    BMS_STOP_CAUSE_BRO_ABS,                     //BRO׼�������쳣
    BMS_STOP_CAUSE_RECV_BST_0_00,//BST��SOCĿ��ֵ	
    BMS_STOP_CAUSE_RECV_BST_0_02,//BST�ĵ�ѹ�趨ֵ	
    BMS_STOP_CAUSE_RECV_BST_0_04,//BST�����ѹ��	
    BMS_STOP_CAUSE_RECV_BST_0_06,//BST_00_6δ����	
    BMS_STOP_CAUSE_RECV_BST_1_00,//BST��Ե����		
    BMS_STOP_CAUSE_RECV_BST_1_02,//BST����������	
    BMS_STOP_CAUSE_RECV_BST_1_04,//BSTԪ������		
    BMS_STOP_CAUSE_RECV_BST_1_06,//BST����������	
    BMS_STOP_CAUSE_RECV_BST_2_00,//BST��������	
    BMS_STOP_CAUSE_RECV_BST_2_02,//BST��������	
    BMS_STOP_CAUSE_RECV_BST_2_04,//BST_02_4δ����    
    BMS_STOP_CAUSE_RECV_BST_2_06,//BST_02_6δ����    
    BMS_STOP_CAUSE_RECV_BST_3_00,//BST��������		
    BMS_STOP_CAUSE_RECV_BST_3_02,//BST��ѹ�쳣		
    BMS_STOP_CAUSE_RECV_BST_OHTER,//BSTδ֪ͣ��  
    
    STOP_MAX_CODE,
}ENUM_ALARM_STOP;

typedef enum
{
    ALARM_TYPE_NULL = 0,      //��
    ALARM_TYPE_ABS,           //�澯��ͣ��
    ALARM_TYPE_FAULT,         //ϵͳ���� 
	ALARM_TYPE_GUN1_FAULT,	  //GUN1���� 
	ALARM_TYPE_GUN2_FAULT,	  //GUN2���� 
}ENUM_ALARM_TYPE;

typedef enum
{
    GUN_SN_ONE = 0,
    GUN_SN_TWO,	
    GUN_SN_MAX, 
}ENUM_GUN_SN;



typedef struct 
{
    uint32_t originalCode;      //ֹͣ��ENUM_ALARM_STOP
    char *tip;               //��ʾ����¼��
    ENUM_ALARM_TYPE alarmtype;  //alarm�ȼ�
    
}StructStopCodeMap;

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
    int32_t metercomcnt;
    int32_t modcomcnt;
    int32_t heartcnt;
    uint8_t ver;
    uint8_t alarmsource;		//�Ƿ����Դ 0 �ǣ�1bu��

    unionAlarmA uAlarmA;//ENUM_ALARM_STOP 1-16
    unionAlarmB uAlarmB;//ENUM_ALARM_STOP 17-32
    unionAlarmC uAlarmC[2];//ENUM_ALARM_STOP 33-48:1ǹ  49-64:2ǹ 
}StructAlarmMegg;

extern StructAlarmMegg sAlarmMegg;

char* get_alarm_stopcause_tips(uint16_t index);
uint8_t get_alarm_statue(ENUM_ALARM_STOP index);
void set_alarm_statue(ENUM_ALARM_STOP index,uint8_t uset);
BOOL get_alarm_fast_response(void);

void alarm_init(void);
void alarm_main(void);

#endif

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
