/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�
�ļ�:   can_tx_rx_list
����:   km
˵��:   Can�����ݴ���.
***************************************************************************************************/


#ifndef __CAN_TX_RX_LIST_H__                     //���ذ���.
#define __CAN_TX_RX_LIST_H__

#include "base_type.h"
//#include "stm32f4xx.h"
#include "stm32f4xx_can.h"

typedef struct
{
    uint8_t sa;    //Դ��ַ
    uint8_t da;    //Ŀ���ַ
    uint8_t pf;    //PDU��ʽ
    union 
    {
        struct 
        {
            uint8_t dp:1;    //����ҳ
            uint8_t r:1;    //����λ
            uint8_t p:3;    //���ȼ�
        }bit;
        uint8_t pp;
    }bype1;
}CanHeader;

typedef union
{
    unsigned int all;
    CanHeader header;
}CanFrameHeader;
    
#endif  //__CAN_TX_RX_LIST_H__

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/



