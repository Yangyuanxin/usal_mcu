/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   charge_task
����:   km
˵��:   �������.
***************************************************************************************************/

#ifndef __CHARGETASK_H
#define __CHARGETASK_H

#include "fun.h"

typedef enum
{
    CHARGER_STATE_IDLE = 0,         //����
    CHARGER_STATE_GUNLOCK,          //ǹ���պ�
    CHARGER_STATE_MOD_SWITCH,       //ģ�����/�Ӵ�������
    CHARGER_STATE_AXLOCK,           //��Դ�պ�
    CHARGER_STATE_GB_CONFIG,        //�����ж�
    CHARGER_STATE_INSDET,           //��Ե���+й��
    CHARGER_STATE_INS_DISCHARGE,    //��Եй�����
    CHARGER_STATE_WAIT_BROAA,       //BRO
    CHARGER_STATE_PRE_CHARGE,       //Ԥ���
    CHARGER_STATE_CHARGING,         //�����
    CHARGER_STATE_STARTFAULT,       //����ʧ��
    CHARGER_STATE_STOP,             //ֹͣ���
    CHARGER_STATE_CHG_DISCHARGE,    //ֹͣй��
    CHARGER_STATE_AXOPEN,           //��Դ�Ͽ�
    CHARGER_STATE_GUNOPEN,          //ǹ���Ͽ�
    CHARGER_STATE_END,              //����
    CHARGER_STATE_END_SAVE,         //��¼����

    CHARGER_STATE_MAX, //17���Ķ���enum��ע��ChargerSaveEnergyData-chargStTimeֻ�ܷ�16��
}ENUM_SYS_CHARGER_STATE;

typedef struct 
{
    ENUM_SYS_CHARGER_STATE state;   //
    char *tip;                      //��ʾ����¼��
}structStateTip;

/**
  * @brief  ���������ж�
  */
typedef enum
{
    CHARGE_START_LIMIT_NULL = 0,
    CHARGE_START_LIMIT_BUSY,        //���Ǵ���״̬
    CHARGE_START_LIMIT_DISCONNECT,  //ǹδ����
    CHARGE_START_LIMIT_FAULT,       //�������ϲ��ܳ��
}ENUM_CHARGER_START_RESULT;

typedef enum
{
    START_BY_ADMIN = 1,             //�Ӱ�����Ļ����
    START_BY_TCU, 
}ENUM_CHARGER_START_TYPE;          //������ʽ

typedef struct
{
//0*32
    uint16_t chargerIndex;          //��ǰ����¼����
    uint16_t chargerStopReason;     //������ԭ��
    uint32_t chargerTime;           //�ܳ��ʱ�䵥λ��
    uint32_t chargerStartTime;      //
    uint32_t chargerEndTime;        //
//4
    uint8_t startSoc;               //��ʼSOC
    uint8_t endSoc;                 //����SOC
    uint8_t startChargeMode;        //���������ʽ
    uint8_t gbVersion;              //����������
    int32_t chargerPower;           //�ܳ����� 0.001KW
    int32_t chargerStartPower;      //�����ʼ����
    int32_t chargerEndPower;        //����������
//8
    int16_t insdetVdc;              //��Ե�����ѹ
    uint16_t insdetRes;
    uint16_t insdetRes2;
    int16_t GFDPosVdc;
    int16_t GFDNegVdc;
    int16_t GFDPosVdc2;
    int16_t GFDNegVdc2;
    int16_t GFDVmod;                //��Ե��⣬ģ���ѹ�����쳣ʱ�洢
//12    
    int16_t pre_v_gun;
    int16_t pre_v_mod;
    int16_t bcp_vabs;               //BCP��ѹ�쳣 �����������ص�ǰ��ص�ѹ
    int16_t bcp_vabs2;              //BCP��ѹ�쳣 ����BCP��ѹ�쳣��ǰһʱ��ǹͷ��ѹֵ
//14    
    int8_t vin[17];                 //����VIN
    int8_t workmode;                //0 Զ�� 1 ����
    int8_t rsv[2];                  //Ԥ�������롣
//19
    int16_t stopmegg1;
    int16_t stopmegg2;
    int16_t stopmegg3;
    int16_t stopmegg4;
    int16_t stopmegg5;
    int16_t stopmegg6;
    int16_t stopmegg7;
    int16_t stopmegg8;
//23

}ChargerSaveData;//�޸���󲹣���ֹǰ���¼��ͻ

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


typedef union 
{
    uint16_t all;
    struct
    {
        uint16_t bStartCh       :1; // ����״̬ 1���� 0����
        uint16_t bStartMod      :1; // ģ�鿪���ܿ��� 1���� 0����
        uint16_t bRecordSave    :1;	// ��¼����״̬ 1������
        uint16_t bAcConnect     :1; // ��պϽ����Ӵ���
        uint16_t bFan           :1; // ��պϷ���
        uint16_t bInsdet        :1; // ��Ե�嶯��

        uint16_t bRsv           :10;
    }bit;
}unionChargerDeviceFlag;

typedef struct
{
    int32_t modswitchcnt;
    int32_t axdelaycnt;
    int32_t gunopencnt;
    int32_t insdetDiscnt;
    int32_t waitBatVolCnt;
    int32_t waitPortCnt;
    int32_t bsmPausecnt;
    int32_t dcContCutcnt;
    int32_t chargeDiscnt;
    int32_t updaterecordcnt;    
    int32_t recordcnt;
    
    int32_t maxvoltagecnt;
    int32_t bclvoltagecnt;
    int32_t batSinglecnt;
    int32_t batSOCcnt;
    int32_t batTempcnt;
    int32_t prechgcnt; //��������ʱ

}structChargerDeviceCnt;


typedef enum
{
    INSDET_STATE_IDLE = 0,          //����
    INSDET_STATE_START,             //������Ե
    INSDET_STATE_MULTI,             //���丱ǹ
    INSDET_STATE_GB15,              //ģ���ѹ�ȿ�������Ե��ѹ���ٱպϽӴ���  
    INSDET_STATE_INS1,              //�̵����л�1
    INSDET_STATE_INS2,              //�̵����л�2
    INSDET_STATE_CALCULATE,           
    INSDET_STATE_DISCHARGE,         //й��
    INSDET_STATE_DISCHARGE_DELAY,   //�ȴ��Ͽ�
    INSDET_STATE_OVER,              //������
}ENUM_SYS_INSDET_STATE;//��Ե״̬��

typedef struct
{
    int32_t v_InsDet;               			//��Ե����ѹ
    ENUM_SYS_INSDET_STATE state,state_bak;    	//��Ե����״̬��

    int16_t PortVoltAbsCnt;         			//��Եǰ���˿ڵ�ѹ����ĳֵ���
    int16_t PortVoltInvCnt;         			//��Եǰ���˿ڵ�ѹ���Ӽ��
    int32_t ModVoltDelayCnt;       				//GB15��Եʱ�ȿ�ģ���ٱպϽӴ������м���ʱ
    int32_t ModVoltAbsCnt;         				//ģ���ѹδ�ﵽ��Ե��ѹ�쳣
    int32_t ins1cnt;
    int32_t ins2cnt;
    int32_t dischargecnt;          				//й��

}structInsdetDeal; 

typedef struct
{
    int8_t cc1StateStableCount;
    int8_t fanCount;
    int32_t accontCount;

}structOtherCnt; 

typedef struct
{
    ENUM_SYS_CHARGER_STATE chargerState,chargerStatebak;    //���״̬
    structChargerDeviceCnt cnt;             				//���״̬����cnt��IDLE���
    structInsdetDeal sInsdet;               				//��Ե���ݣ�IDLE���

    uint8_t cc1State;
}sChargerDevice;    //��̬��������Ӧ.cʹ��

typedef struct
{
    ENUM_SYS_CHARGER_STATE state_last;  //��һ�׶Σ�������¼
    int16_t v_insdet;                   //��Ե��������ѹ
    int16_t v_mod[2];                   //ģ���ѹ�������ݣ�[0]��һ�� [1]���ϸ�
    int16_t v_gun[2];                   //ǹ�ڵ�ѹ�������ݣ�[0]��һ�� [1]���ϸ�
    int16_t i_gun[2];                   //ǹ�ڵ�ѹ�������ݣ�[0]��һ�� [1]���ϸ�
    int16_t v_demand[2];                //�����ѹ�������ݣ�[0]��һ�� [1]���ϸ�
    int16_t i_demand[2];                //��������������ݣ�[0]��һ�� [1]���ϸ�
    int16_t v_bcs[2];                   //BCS��ѹ�������ݣ�[0]��һ�� [1]���ϸ�
    int16_t i_bcs[2];                   //BCS�����������ݣ�[0]��һ�� [1]���ϸ�
    int16_t i_max;                      //��������������������ڵ��
    int16_t t_max;                      //����е������¶ȣ�����BSM
    int16_t v_max_bat;                  //����е��������ѹ *10��BYTE0����BCS���壬BYTE1����BCP���������
}structRecordData;  //������¼����

typedef struct
{
    uint16_t v_demand; 					//BMS����򱾵������ѹ
    uint16_t i_demand; 					//BMS����򱾵��������
    uint16_t v_demand_mod; 				//������ǹģ����ܵ�ѹ������ʱ�������𣬴�ǹ�·�ģ��������ѹ
    uint16_t i_demand_mod; 				//������ǹģ����ܵ��������ƻ򲢳�ʱ�������𣬴�ǹ�·�ģ����������
    uint32_t  chargerStopCause;			//������ԭ�򣬿������ǹʱ���

    unionChargerDeviceFlag flag;
    ChargerSaveData record;         	//�����ʱ��¼
    UNION_SYS_CHARGE_STEP unStep;   	//���״̬��ֶ�
    structRecordData record_data;       //��¼����
}sChargerMegg;

extern sChargerMegg chargerMegg[2];
















extern structSysClockCnt sChargerTaskClockCnt;
extern unionSysClockFlag unChargerTaskClockFlag;



void chargeTaskInit(void);
void charger_task_main(void);
ENUM_SYS_INSDET_STATE get_insdet_state(uint8_t gunSn);
ENUM_SYS_CHARGER_STATE get_charge_state(uint8_t gunSn);
BOOL start_charger_interface(ENUM_CHARGER_START_TYPE type,uint8_t gunSn);//�����ӿ�
void set_charger_stopcause(uint32_t stopCause,uint8_t gunSn);//�ػ��ӿ�
uint32_t get_charger_stopcause(uint8_t gunSn);


#endif

