/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   bms_canhandle
����:   km
˵��:   bms���ݴ���.
***************************************************************************************************/

#ifndef __CAN_HANDLE_H__                     //���ذ���.
#define __CAN_HANDLE_H__
/**************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
***************************************************************************************************/
#include "base_type.h"
#include "bms_device.h"
#include "can_tx_rx_list.h"           
#include "kfc_dev.h"
#include "srt_config.h"
#include "can_def.h"

//���ӹ����PF����
#define CAN_TP_CM_RTS  0xec //����ģʽ�µ�������
#define CAN_TP_DT_RTS  0xeb //����Э�飺���ݣ�TP.DT��



typedef struct
{
    //���ڴ���Э��
    uint16_t numberOfBytes; //ȫ�������ֽ���
    uint8_t numberOfPackets; //ȫ�����ݰ���
    uint8_t pgn; //��������
    uint8_t sequenceRx;
    uint16_t recvTickMsCnt;//����ʱ�����
    
    uint8_t data[MAX_CAN_FREAME_DATA_LEN];
}CanLongFrame;



typedef struct
{
    char *data;
    uint8_t len;
    unionBmsFrameFlag flag;
}PCanFrame;


typedef struct
{
     CanFrameHeader sendCanHeader;
     CanFrameHeader recvCanHeader;
     CanLongFrame   canLongFrame;
     //CanTxRxList *p_canHandle;//��ű��Ķ���ָ��
     CanRxMsg m_rxMessage;//���յ�������
     CanTxMsg m_sendFrame;//���ͱ���
}CanHandlerData;

typedef union
{
    uint32_t all;
    struct
    {
        uint32_t  bCheckPriorityEnable:1;//�Ƿ��ⱨ�����ȼ�
        uint32_t  bCheckFrameLongEnable:1;//�Ƿ��鱨�ĵĳ��̰�
        uint32_t  bCheckSourceAddrEnable:1;//�Ƿ��鱨�ĵ�Դ��ַ 
        uint32_t  bCheckFrameOrderEnable:1;//��������Ƿ��鱨��˳��
        
     }bit;
}unionCanhandleMode;
/***************************************************************************************************
����:   ���б���.
***************************************************************************************************/
extern CanHandlerData canHandlerData[2];

/***************************************************************************************************
����:   ���з���.
***************************************************************************************************/
extern int send_bms_can_frame(CanTxMsg *canFrame,uint8_t gunSn);
extern void canhandle_recv_init(uint8_t gunSn);
extern const PCanFrame *get_canheadler_bms_can_frame(BMS_PF_TYPE canIndex,uint8_t clearFlag,uint8_t gunSn);
extern int get_bms_can_frame_exit_flag(BMS_PF_TYPE canIndex,uint8_t gunSn);
extern void canhandle_task_1ms(void);
extern int set_canhandle_work_mode(unionCanhandleMode mode,uint8_t gunSn);

#endif  //__CAN_HANDLE_H__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/

