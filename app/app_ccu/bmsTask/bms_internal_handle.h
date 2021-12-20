/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   bms_internal_handle
����:   km
˵��:   bms���ݴ���.
***************************************************************************************************/

#ifndef __BMS_INTERNAL_HANDLE__                     //���ذ���.
#define __BMS_INTERNAL_HANDLE__
/**************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
***************************************************************************************************/
#include "base_type.h"

#define WAIT_FRAME_TIME_1000MS  1008
#define WAIT_FRAME_TIME_5000MS  5030
#define WAIT_FRAME_TIME_10000MS  10060
#define WAIT_FRAME_TIME_60000MS  60300

#define SENDTIMEOUT10_MS          10
#define SENDTIMEOUT20_MS          20
#define SENDTIMEOUT50_MS          50
#define SENDTIMEOUT250_MS         250
#define SENDTIMEOUT500_MS         500
#define SENDTIMEOUT1000_MS        1000
#define SENDTIMEOUT5000_MS        5100
#define SENDTIMEOUT10000_MS       10000
#define SENDTIMEOUT60000_MS       60000


typedef enum    //��������.
{
    BRM_FRAME_TIMEOUT = 1,
    BCP_FRAME_TIMEOUT,
    BRO_FRAME_TIMEOUT,
    BCL_FRAME_TIMEOUT,
    BCS_FRAME_TIMEOUT,
    BSM_FRAME_TIMEOUT,
    BST_FRAME_TIMEOUT,
    BSD_FRAME_TIMEOUT,
    BEM_FRAME_TIMEOUT,
}BMS_FRAME_TIMEOUT_TYPE;


#define MAX_TIMEOUT_STOP_TRY_CHARGER_CNT 3 //���ʱ���Գ�����

typedef union
{
    uint8_t all;
    struct
    {
        uint8_t  bEnalbeSend:1;//����ʹ��
        uint8_t  bLock:1;//������־
    }bit;
}unionFrameFlag;

typedef struct 
{
    unionFrameFlag unFrameFlag; 
    uint16_t sendTime;//����ʱ�䣬Ϊ0ʱ����
    uint16_t maxSendTime;//�����ʱ��
}StructChargerSendCanFrame;

typedef struct 
{
    StructChargerSendCanFrame chm26;
    StructChargerSendCanFrame crm01;
    StructChargerSendCanFrame cts07;
    StructChargerSendCanFrame ccm40;//HARDWARE_CHECK_MODE
    StructChargerSendCanFrame cml08;
    StructChargerSendCanFrame cro0A;
    StructChargerSendCanFrame ccs12;
    StructChargerSendCanFrame cst1A;
    StructChargerSendCanFrame csd1D;
    StructChargerSendCanFrame cem1F;
    
    uint16_t waitBhm27Time;
    uint16_t waitBrm02Time;
    uint16_t waitBcp06Time;
    uint16_t waitBro09Time;
    uint16_t waitBro09OkTime;
    uint16_t waitBcl10Time;
    uint16_t waitBcs11Time;
    uint16_t waitBsm13Time;
    uint16_t waitBst19Time;
    uint16_t waitBsd1DTime;
    uint16_t waitBcm41Time;//HARDWARE_CHECK_MODE
}StructBmsComFrameInfo;


typedef struct 
{
    uint32_t chargerStartPower;//������������ 0.01kwh
    int32_t userStartSecondCnt;//�û��������ʱ�䣨�룩
    int32_t bmsDisableChargerStarTime;//BMS��ֹ��翪ʼʱ��
    uint8_t cem1fType;//CEM�쳣����
    uint8_t cstIndex;
    uint8_t cstValue;
    uint8_t cstFaultFlag; // CST����ж��Ƿ����
    int32_t insdetXieFangCnt;
}BmsComInternalData;//BMSͨ���ڲ�ʹ������

/***************************************************************************************************
����:   ���б���.
***************************************************************************************************/
extern StructBmsComFrameInfo bmsComFrame[2];
extern BmsComInternalData bmsComInternalData[2];

/***************************************************************************************************
����:   ���з���.
***************************************************************************************************/
extern void bms_internal_handle_init(void);
extern void bms_internal_handle_start_charger_init(uint8_t gunSn);
extern void bms_internal_handle_task_1ms(uint8_t gunSn);

#endif  //__BMS_INTERNAL_HANDLE__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/

