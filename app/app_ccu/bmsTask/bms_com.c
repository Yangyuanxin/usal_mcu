/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   bms_com
����:   km
˵��:   bms���ݴ���.
***************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
***************************************************************************************************/
#include "bms_task.h"
#include "bms_device.h"
#include "bms_canhandle.h"
#include "bms_com.h"
#include "bms_internal_handle.h"

#include <string.h>
#include <stdio.h>
#include "sys_config.h"
#include "charge_task.h"
#include "time_cur.h"
#include "alarm_task.h"
#include "io_megg.h"
#include "log.h"


/***************************************************************************************************
����:   ȫ�ֱ���.
***************************************************************************************************/
static CanTxMsg m_sendFrame[2];//���ͱ���
struct BmsInfos{
    unsigned short soc100Time;
};
static struct BmsInfos bms_infos[2];

typedef struct
{
	uint8_t u8PtStartSocData;
}sBmsUptateFlg;

sBmsUptateFlg BmsUptateFlg[2];

void bms_StopReason_Judge(int stopCauseSn);//������֤����
typedef struct
{
	uint8_t u8BsmFlg;

}RecvFlg_Kaipu;

RecvFlg_Kaipu sRecvFlg_Kaipu[2];
//static char bms_test_string[64];

static int get_bms_stop_cause(uint8_t gunSn)
{
    int cause = bmsDevice[gunSn].bmsData.bmsStopCause;
    if(cause)
    {
        bmsDevice[gunSn].bmsData.bmsStopCause = 0;
        bmsDevice[gunSn].bmsData.bmsStopCause2015First = 0;
    }  

    return cause;
}

static void set_bms_stop_cause(int cause,int flag,uint8_t gunSn)
{
    if (flag)
    {
        bmsDevice[gunSn].bmsData.bmsStopCause2015First = 0;
        
        if (bmsDevice[gunSn].bmsData.bmsStopCause == 0)
        {
            bmsDevice[gunSn].bmsData.bmsStopCause = cause;
            DLOG("gun%d:BMSֹͣ[%d]", gunSn,cause);
        }
        else if (cause == 0)
        {
            bmsDevice[gunSn].bmsData.bmsStopCause = 0;
        }
    }
    else
    {
        if (cause == 0)
        {
            bmsDevice[gunSn].bmsData.bmsStopCause = 0;
            bmsDevice[gunSn].bmsData.bmsStopCause2015First = 0;
        }
        else if (bmsDevice[gunSn].bmsData.bmsStopCause == 0)
        {
       //     if(bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
            {
                if ((bmsDevice[gunSn].bmsData.chargerCnt+1) > MAX_TIMEOUT_STOP_TRY_CHARGER_CNT)
                {
                    if(BMS_STOP_CAUSE_RECV_BRM_TIMEOUT == cause)
                    {
                        bmsDevice[gunSn].bmsData.bmsStopCause = bmsDevice[gunSn].bmsData.bmsStopCause2015First;
                    }
                    else
                    {
                        bmsDevice[gunSn].bmsData.bmsStopCause = cause;
                    }
                    
                    DLOG("gun%d:2015BMSֹͣ[%d]", gunSn,cause);
                    bmsDevice[gunSn].unBmsStateA.bit.bBmsComFault = 1;                
                }
                else if((0 == bmsDevice[gunSn].bmsData.bmsStopCause2015First)
                    || (BMS_STOP_CAUSE_RECV_BRM_TIMEOUT == bmsDevice[gunSn].bmsData.bmsStopCause2015First))
                {
                    bmsDevice[gunSn].bmsData.bmsStopCause2015First = cause;
                }
            }
        /*    else
            {
                bmsDevice[gunSn].bmsData.bmsStopCause = cause;
                
                DLOG("2011BMSֹͣ[%d]", cause);
                bmsDevice[gunSn].unBmsStateA.bit.bBmsComFault = 1;                
            }
            */
        }
    }
}

/**
  * @brief  BMS���ݴ���
  */
static void charger_sys_bms_data_contact(uint8_t gunSn)
{
    bmsExternData[gunSn].chargerMaxOutputVol = sSysConfig.sInt.uMaxChargeVolt;
    bmsExternData[gunSn].chargerMinOutputVol = sSysConfig.sInt.uDcUnderVolt+50;
    
	int uMaxChargeCurr = sSysConfig.sInt.uMaxChargeCurr;
	if (sSysConfig.sInt.uParallelMode == 0)//��ǹ�������250A
	{
        if (uMaxChargeCurr > 2500)
        {
            uMaxChargeCurr = 2500;
        }
	}
	else
	{
	    uMaxChargeCurr *= 2;
	    if(uMaxChargeCurr > 4000)
	    {
	        uMaxChargeCurr = 4000;
	    }
	}
    bmsExternData[gunSn].chargerMaxOutputCur = uMaxChargeCurr;		
    
    bmsExternData[gunSn].curChargerOutputVol = sSysMegg[gunSn].v_Gun;
    bmsExternData[gunSn].curChargerOutputCur = sSysMegg[gunSn].i_Gun;
    bmsExternData[gunSn].meterElectricEnergy = 0x00;//dlt645Megg.meter.energyPower_1000x;    //needmodify

    if(bmsDevice[gunSn].bmsData.bmsStopCause)
    {
        set_charger_stopcause(get_bms_stop_cause(gunSn),gunSn);
    }

    bmsExternData[gunSn].externStopCause = chargerMegg[gunSn].chargerStopCause;
}

/*static PCanFrame pBmsCanFrame;
const PCanFrame *get_bms_can_frame(BMS_PF_TYPE canIndex,uint8_t clearFlag)
{

    pBmsCanFrame.data = &bmsDevice.frameData.canFrameString[canIndex];
    pBmsCanFrame.len = clearFlag%3;
    
    return &pBmsCanFrame;
}*/

void bms_one_start_charger_init(uint8_t gunSn)
{
    canhandle_recv_init(gunSn);
    
    memset(&bmsDevice[gunSn].frameData.cemFrameSendData,0,sizeof(bmsDevice[gunSn].frameData.cemFrameSendData));
    bmsDevice[gunSn].frameData.broData.broReadyState = 0;
    
    bms_internal_handle_start_charger_init(gunSn);
    
    bmsComInternalData[gunSn].chargerStartPower = bmsExternData[gunSn].meterElectricEnergy;
    bmsComInternalData[gunSn].bmsDisableChargerStarTime = 0;
    bmsComInternalData[gunSn].insdetXieFangCnt = 0;
    bmsDevice[gunSn].bmsData.bsmStopTimeCnt = 0;
}

/**
  * @brief  ÿһ����������ʼ��
  */
void start_bms_handshake_init(uint8_t gunSn)
{
    bms_one_start_charger_init(gunSn);
    
    bmsDevice[gunSn].mainState = SHAKE_HAND;
    bmsDevice[gunSn].subState = SEND_CRM01_00;
    bmsComFrame[gunSn].crm01.sendTime = 0;
    if(bmsDevice[gunSn].bmsType == BMS_GB_2015_TYPE)
    {
        bmsComFrame[gunSn].waitBrm02Time = WAIT_FRAME_TIME_5000MS;
    }
    else
    {
        bmsComFrame[gunSn].waitBrm02Time = WAIT_FRAME_TIME_5000MS*4;
    }
    bms_infos[gunSn].soc100Time = 0;    
}

/**
  * @brief  �û�һ��ȫ�µ���������ʼ��
  */
void bms_start_charger_init(uint8_t gunSn)
{
    if (bmsDevice[gunSn].unBmsStateB.bit.bBmsComState)
    {
        DLOG("gun%d:����ͨѶ��",gunSn);
        return;
    }    

	unionCanhandleMode sMod = {0};
    bmsDevice[gunSn].bmsType = sSysConfig.sInt.bmsType;
    if((bmsDevice[gunSn].bmsType == BMS_GB_2015_TYPE)||(bmsDevice[gunSn].bmsType == BMS_GB_TYPE))
    {
    	sMod.bit.bCheckPriorityEnable=1;//�Ƿ��ⱨ�����ȼ�
    	sMod.bit.bCheckFrameLongEnable=1;//�Ƿ��鱨�ĵĳ��̰�
    	sMod.bit.bCheckSourceAddrEnable=1;//�Ƿ��鱨�ĵ�Դ��ַ 
    }

	set_canhandle_work_mode(sMod,gunSn); 
    
    bmsDevice[gunSn].unBmsStateB.bit.bBmsComState = 1;
    
    bmsExternData[gunSn].unStateA.bit.bGb2015ProtocolFlag = bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag;

    bmsDevice[gunSn].unBmsStateA.all = 0;
    
    memset(&bmsDevice[gunSn].frameData,0,sizeof(bmsDevice[gunSn].frameData));
    bmsComInternalData[gunSn].userStartSecondCnt = 0;
    
    bmsDevice[gunSn].unBmsStateA.bit.bProtocolStable = 1;
    
    bmsDevice[gunSn].bmsData.bcpStartSoc = 0;
    bmsDevice[gunSn].bmsData.cem22ErrorCnt = 0;
    
    set_bms_stop_cause(0,1,gunSn);

//---Ĭ�����¹���    
    bms_one_start_charger_init(gunSn);
    
    bmsDevice[gunSn].mainState = SHAKE_HAND;
    bmsDevice[gunSn].subState = SEND_CHM;
    bmsComFrame[gunSn].chm26.sendTime = 0;
    bmsComFrame[gunSn].waitBhm27Time = WAIT_FRAME_TIME_5000MS;
    DLOG("gun%d:��ʼ����CHM����",gunSn);

}

/**
  * @brief  ��������������
  */
void bms_charger_stop(uint8_t gunSn)
{
    bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
    bmsDevice[gunSn].frameData.broData.broReadyState = 0;
    bmsDevice[gunSn].unBmsStateA.bit.bComChargingStageFlag = 0;
}

void bms_type_judge(uint8_t gunSn)
{
    if (!bmsDevice[gunSn].unBmsStateA.bit.bProtocolStable)
    {
        if ((bmsDevice[gunSn].frameData.brmSa== GB_CHARGER_CAN_ADDR)||(bmsDevice[gunSn].frameData.bcpSa == GB_CHARGER_CAN_ADDR))
        {
            if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
            {
                 bmsDevice[gunSn].bmsType = BMS_GB_2015_TYPE;                    
            }
            else
            {
                bmsDevice[gunSn].bmsType = BMS_GB_TYPE;
            }
        }

        bmsDevice[gunSn].unBmsStateA.bit.bProtocolStable = 1;
    }
}
                     
int send_chm26_gb2015(uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_CH_PF_CHM;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
   
    m_sendFrame[gunSn].Data[0] = 0x01;
    m_sendFrame[gunSn].Data[1] = 0x01;
    m_sendFrame[gunSn].Data[2] = 0x00;
    
    m_sendFrame[gunSn].DLC = 3;
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);  
}

int send_crm01_gb2015(char state,uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_BMS_PF_CRM;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
   
    m_sendFrame[gunSn].Data[0] = state;
    m_sendFrame[gunSn].Data[1] = 0x01;
    
    m_sendFrame[gunSn].Data[2] = 0xff;
    m_sendFrame[gunSn].Data[3] = 0xff;
    m_sendFrame[gunSn].Data[4] = 0xff;
    m_sendFrame[gunSn].Data[5] = 0xff;
    m_sendFrame[gunSn].Data[6] = 0xff;
    m_sendFrame[gunSn].Data[7] = 0xff;
    
    m_sendFrame[gunSn].DLC = 8;
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);  
}

static int send_crm01(char state,uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_TYPE:
        case BMS_GB_2015_TYPE:
        {
           ret = send_crm01_gb2015(state,gunSn);
        }
        break;
    }
    return ret;
}

int send_cts07_gb2015(uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    char timeBuffer[8];
	uint8_t i;
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_BMS_PF_CTS;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
   
    memcpy(timeBuffer,get_time_buffer(),7);
    for(i=0;i<7;i++)
    {
        m_sendFrame[gunSn].Data[i]=timeBuffer[6-i];	
    }
    m_sendFrame[gunSn].Data[6]=timeBuffer[0];
    m_sendFrame[gunSn].Data[5]=timeBuffer[1]; 	
    
    m_sendFrame[gunSn].DLC = 7;
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);
}

/***************************************************************************************************
����:   send_ccm40.HARDWARE_CHECK_MODE
����:   ��
        .
����:   .��
        
˵��:   .���׮��װ�����и�BMS����ccm���ģ��������������ѹ
***************************************************************************************************/
/***************HF add************/
/*
int send_ccm40(void)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame,0,sizeof(m_sendFrame));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_CH_PF_CCM;
    m_canHeader.header.da = CK_BMS_CAN_ADDR;
    m_canHeader.header.sa = CK_CHARGER_CAN_ADDR;
 
    m_sendFrame.ExtId = m_canHeader.all;
#if 0    
    checkprocess.checktimes.VolBMS += 10;//for test suc
    DLOG("send Nend Vol:%d",checkprocess.checktimes.VolBMS);
#endif    
    m_sendFrame.Data[0] = checkprocess.mainState;
    m_sendFrame.Data[1] = checkprocess.checktimes.VolBMS & 0xff;
    m_sendFrame.Data[2] = (checkprocess.checktimes.VolBMS>>8) & 0xff;
    
    m_sendFrame.Data[3] = ((sChBasicMegg.unFlagChIoOutEn.bit.bInsDetUnBalEn << 4) | (sChBasicMegg.unFlagChIoOutEn.bit.bInsDetNegUnBalEn & 0x0f));
    
    m_sendFrame.DLC = 4; 
    return send_bms_can_frame(&m_sendFrame);
}
*/
/***************HF add************/

int send_cts07(uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_TYPE:
        case BMS_GB_2015_TYPE:
        {
           ret = send_cts07_gb2015(gunSn);
        }
        break;
        
    }
    return ret;
}

static int send_cml08_gb2015(int maxVol,int minVol,int maxCur,uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_BMS_PF_CML;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
   
    m_sendFrame[gunSn].Data[0] = (unsigned char)(maxVol&0xff);
    m_sendFrame[gunSn].Data[1] = (unsigned char)((maxVol>>8)&0xff);
    
    m_sendFrame[gunSn].Data[2] = (unsigned char)(minVol&0xff);
    m_sendFrame[gunSn].Data[3] = (unsigned char)((minVol>>8)&0xff);
    
    maxCur = MAX_CHARGER_CURRENT_OFFSETT_GB - maxCur;
    
    if (maxCur < 0)
    {
        maxCur = 0;
    }
    
    m_sendFrame[gunSn].Data[4] = (unsigned char)(maxCur&0xff);
    m_sendFrame[gunSn].Data[5] = (unsigned char)((maxCur>>8)&0xff);
    
    if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
    {
        short minCur = MAX_CHARGER_CURRENT_OFFSETT_GB ;
        m_sendFrame[gunSn].Data[6] = (unsigned char)(minCur&0xff);
        m_sendFrame[gunSn].Data[7] = (unsigned char)((minCur>>8)&0xff);
        m_sendFrame[gunSn].DLC = 8;
    }
    else
    {
        m_sendFrame[gunSn].DLC = 6;
    }
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);
}

static int send_cml08(int maxVol,int minVol,int maxCur,uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           ret = send_cml08_gb2015(maxVol,minVol,maxCur,gunSn);
        }
        break;

    }
    
    return ret;
}

static int send_cro0A_gb2015(char state,uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x10;
    m_canHeader.header.pf = GB_BMS_PF_CRO;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
   
    m_sendFrame[gunSn].Data[0] = state;
    
    m_sendFrame[gunSn].DLC = 1;
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);  
}

static int send_cro0A(char state,uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           ret = send_cro0A_gb2015(state,gunSn);
        }
        break;
    }
    
    return ret;
}

static int send_ccs12_gb2015(char state,uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    int idata = 0;
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_BMS_PF_CCS;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
    
    if (bmsExternData[gunSn].curChargerOutputVol > 0)
    {
        m_sendFrame[gunSn].Data[0] = (unsigned char)(bmsExternData[gunSn].curChargerOutputVol&0xff);
        m_sendFrame[gunSn].Data[1] = (unsigned char)((bmsExternData[gunSn].curChargerOutputVol>>8)&0xff);
    }
    else
    {
        m_sendFrame[gunSn].Data[0] = 0;
        m_sendFrame[gunSn].Data[1] = 0;
    }
    
    idata = MAX_CHARGER_CURRENT_OFFSETT_GB - bmsExternData[gunSn].curChargerOutputCur;
    m_sendFrame[gunSn].Data[2] = (unsigned char)(idata&0xff);
    m_sendFrame[gunSn].Data[3] = (unsigned char)((idata>>8)&0xff);
    
    idata = bmsDevice[gunSn].bmsData.chargerTimeSecondCnt/60;
    
    m_sendFrame[gunSn].Data[4] = (unsigned char)(idata&0xff);
    m_sendFrame[gunSn].Data[5] = (unsigned char)((idata>>8)&0xff);
    
    if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
    {
        m_sendFrame[gunSn].Data[6] = 0xfc|state;
//		m_sendFrame[gunSn].Data[7]=0xff;//��������	
        m_sendFrame[gunSn].DLC = 7;
    }
    else
    {
        m_sendFrame[gunSn].DLC = 6;
    }
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);  
}

static int send_ccs12(char state,uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {

            ret = send_ccs12_gb2015(state,gunSn);

			bmsComFrame[gunSn].ccs12.sendTime = SENDTIMEOUT50_MS;
        }
        break;
    }
    return ret;
}

static int send_cst1A_gb2015(uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x10;
    m_canHeader.header.pf = GB_BMS_PF_CST;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
    
    bmsComInternalData[gunSn].cstIndex = (bmsComInternalData[gunSn].cstIndex & 0x03);
    m_sendFrame[gunSn].Data[bmsComInternalData[gunSn].cstIndex] = bmsComInternalData[gunSn].cstValue;

    m_sendFrame[gunSn].Data[2]= m_sendFrame[gunSn].Data[2]|0xf0;
    m_sendFrame[gunSn].Data[3]= m_sendFrame[gunSn].Data[3]|0xf0;
/*
    if((bmsExternData[gunSn].externStopCause==STOP_CAUSE_CC1_DISCONNECT_1000)||	
	(bmsExternData[gunSn].externStopCause==SYS_ALARM_EPO_BIT_INDEX_1)||
	(bmsExternData[gunSn].externStopCause==SYS_ALARM_DOORRPOTECT_27)||	
	(bmsExternData[gunSn].externStopCause==STOP_CAUSE_CHARGER_VOLTAGE_ABNORMAL_1027))
	*/
	if(bmsComInternalData[gunSn].cstFaultFlag != 0)
    {
		m_sendFrame[gunSn].Data[0]=0x10;//����20170612����
    }    
    m_sendFrame[gunSn].DLC = 4;
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);  
}

static int send_cst1A(uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           ret = send_cst1A_gb2015(gunSn);
        }
        break;
    }
    return ret; 
}

static int send_csd1D_gb2015(uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    int idata = 0;
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = GB_BMS_PF_CSD;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
    
    idata = bmsDevice[gunSn].bmsData.chargerTimeSecondCnt/60;
    
    m_sendFrame[gunSn].Data[0] = (unsigned char)(idata&0xff);
    m_sendFrame[gunSn].Data[1] = (unsigned char)((idata>>8)&0xff);
    
    idata = bmsExternData[gunSn].meterElectricEnergy - bmsComInternalData[gunSn].chargerStartPower;
    idata = idata/10;
    m_sendFrame[gunSn].Data[2] = (unsigned char)(idata&0xff);
    m_sendFrame[gunSn].Data[3] = (unsigned char)((idata>>8)&0xff);
    
    m_sendFrame[gunSn].Data[4] = 1;
    m_sendFrame[gunSn].Data[5] = 0xff;
    m_sendFrame[gunSn].Data[6] = 0xff;
    m_sendFrame[gunSn].Data[7] = 0xff;

	if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
	{
		m_sendFrame[gunSn].DLC = 8;
	}
	else
	{
		m_sendFrame[gunSn].DLC = 5;
	}
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);  
}

static int send_csd1D(uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           ret = send_csd1D_gb2015(gunSn);
        }
        break;
    }
    return ret; 
}


static int send_cem1f_gb2015(int type,uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x08;
    m_canHeader.header.pf = GB_BMS_PF_CEM;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
    
    switch(type)
    {
        case BRM_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[0] = (0x1<<0);
            break;
        case BCP_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[1] = (0x1<<0);
            break;
        case BRO_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[1] = (0x1<<2);
            break;
        case BCL_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[2] = (0x1<<2);
            break;
        case BCS_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[2] = (0x1<<0);
            break;
        case BST_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[2] = (0x1<<4);
            break;
        case BSD_FRAME_TIMEOUT:
            m_sendFrame[gunSn].Data[3] = (0x1<<0);
            break;
        default:
            break;
    }
    
    bmsComInternalData[gunSn].cem1fType = type;
    
    m_sendFrame[gunSn].Data[0] = (unsigned char)(m_sendFrame[gunSn].Data[0]|0xfc);
    m_sendFrame[gunSn].Data[1] = (unsigned char)(m_sendFrame[gunSn].Data[1]|0xf0);
    m_sendFrame[gunSn].Data[2] = (unsigned char)(m_sendFrame[gunSn].Data[2]|0xc0);
    m_sendFrame[gunSn].Data[3] = (unsigned char)(m_sendFrame[gunSn].Data[3]|0xfc);
    
    bmsDevice[gunSn].frameData.cemFrameSendData.Data[0] = m_sendFrame[gunSn].Data[0];
    bmsDevice[gunSn].frameData.cemFrameSendData.Data[1] = m_sendFrame[gunSn].Data[1];
    bmsDevice[gunSn].frameData.cemFrameSendData.Data[2] = m_sendFrame[gunSn].Data[2];
    bmsDevice[gunSn].frameData.cemFrameSendData.Data[3] = m_sendFrame[gunSn].Data[3];
    m_sendFrame[gunSn].DLC = 4;
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);
}

static int send_cem1f(int type,uint8_t gunSn)
{
    int ret = 0;
    
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           ret = send_cem1f_gb2015(type,gunSn);
        }
        break;

    }
    return ret; 
}

int send_cem20(uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = 0x20;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
    
    if(bmsExternData[gunSn].cc1State == 0)//��ǰCC1����״̬
    {
        int pgn = 3522;
        m_sendFrame[gunSn].Data[0] = (unsigned char)(pgn&0xff);
        m_sendFrame[gunSn].Data[1] = (unsigned char)((pgn>>8)&0xff);
        m_sendFrame[gunSn].Data[2] = (unsigned char)((pgn>>16)&0x03);
        m_sendFrame[gunSn].Data[2] = (unsigned char)((7<<3)|m_sendFrame[gunSn].Data[2]);
        m_sendFrame[gunSn].Data[3] = 1;
    }
    else if(bmsExternData[gunSn].externStopCause)
    {
        if(bmsComInternalData[gunSn].cstValue)
        {
            int pgn = 3522;
            m_sendFrame[gunSn].Data[0] = (unsigned char)(pgn&0xff);
            m_sendFrame[gunSn].Data[1] = (unsigned char)((pgn>>8)&0xff);
            m_sendFrame[gunSn].Data[2] = (unsigned char)((pgn>>16)&0x03);
            m_sendFrame[gunSn].Data[2] = (unsigned char)((9<<3)|m_sendFrame[gunSn].Data[2]);
            m_sendFrame[gunSn].Data[3] = 1;
        }
    }
    
    m_sendFrame[gunSn].DLC = 4;

    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);
}

int send_cem22(uint8_t gunSn)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame[gunSn],0,sizeof(m_sendFrame[gunSn]));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = 0x22;
    m_canHeader.header.da = GB_BMS_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame[gunSn].ExtId = m_canHeader.all;
    
   if(bmsExternData[gunSn].cc1State == 0)//��ǰCC1����״̬
    {
        m_sendFrame[gunSn].Data[0] = (unsigned char)(0x01);
        m_sendFrame[gunSn].Data[1] = (unsigned char)(bmsDevice[gunSn].bmsData.cem22ErrorCnt++);
    }
    else if(bmsExternData[gunSn].externStopCause)
    {
        if(bmsComInternalData[gunSn].cstValue)
        {
            m_sendFrame[gunSn].Data[0] = (unsigned char)(0x01);
            m_sendFrame[gunSn].Data[1] = (unsigned char)(bmsDevice[gunSn].bmsData.cem22ErrorCnt++);
        }
    }
    
    m_sendFrame[gunSn].DLC = 2;
    
    return send_bms_can_frame(&m_sendFrame[gunSn],gunSn);
}

/*void send_debug_info60(void)
{
    CanFrameHeader m_canHeader = {0};
    
    memset(&m_sendFrame,0,sizeof(m_sendFrame));
    
    m_canHeader.header.bype1.pp = 0x18;
    m_canHeader.header.pf = 0x60;
    m_canHeader.header.da = GB_CHARGER_CAN_ADDR;
    m_canHeader.header.sa = GB_CHARGER_CAN_ADDR;
    
    m_sendFrame.ExtId = m_canHeader.all;
    
    m_sendFrame.Data[0]= (bmsExternData.externStopCause&0xff);
    m_sendFrame.Data[1]= (bmsExternData.externStopCause>>8)&0xff;
    m_sendFrame.Data[2]= (bmsExternData.externStopCause>>16)&0xff;
    m_sendFrame.Data[3]= (bmsExternData.externStopCause>>24)&0xff;
    
    m_sendFrame.Data[4]= (bmsExternData.iChVdcRef&0xff);
    m_sendFrame.Data[5]= (bmsExternData.iChVdcRef>>8)&0xff;
    m_sendFrame.Data[6]= (bmsExternData.iChIdcRef&0xff);
    m_sendFrame.Data[7]= (bmsExternData.iChIdcRef>>8)&0xff;
    
    m_sendFrame.DLC = 8;
    
    send_bms_can_frame(&m_sendFrame);  
}*/

void brm_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        memcpy(bmsDevice[gunSn].frameData.brmData.brmBmsVersion,get_string_form_string(0,sizeof(bmsDevice[gunSn].frameData.brmData.brmBmsVersion),pCanFrame->len,pCanFrame->data),3);
        if ((pCanFrame->data[2] == 0) && (pCanFrame->data[1] == 1) && (pCanFrame->data[0] == 1))
        {
            bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag = 1;
        }

        bmsDevice[gunSn].frameData.brmData.brmBatteryType = get_data_form_string(DATA_TYPE_CHAR,3,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.brmData.brmBatteryCapacity = get_data_form_string(DATA_TYPE_INT16U,4,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.brmData.brmBatteryTotalVol = get_data_form_string(DATA_TYPE_INT16U,6,pCanFrame->len,pCanFrame->data);
        memcpy(bmsDevice[gunSn].frameData.brmData.brmBatteryFactory,get_string_form_string(8,sizeof(bmsDevice[gunSn].frameData.brmData.brmBatteryFactory),pCanFrame->len,pCanFrame->data),4);
        memcpy(bmsDevice[gunSn].frameData.brmData.brmBatterySn,get_string_form_string(12,sizeof(bmsDevice[gunSn].frameData.brmData.brmBatterySn),pCanFrame->len,pCanFrame->data),4);
        bmsDevice[gunSn].frameData.brmData.brmBatteryDateYear = get_data_form_string(DATA_TYPE_CHAR,16,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.brmData.brmBatteryDateMonth = get_data_form_string(DATA_TYPE_CHAR,17,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.brmData.brmBatteryDateDay = get_data_form_string(DATA_TYPE_CHAR,18,pCanFrame->len,pCanFrame->data);
        memcpy(bmsDevice[gunSn].frameData.brmData.brmBatteryChargeCycles,get_string_form_string(19,sizeof(bmsDevice[gunSn].frameData.brmData.brmBatteryChargeCycles),pCanFrame->len,pCanFrame->data),sizeof(bmsDevice[gunSn].frameData.brmData.brmBatteryChargeCycles));
        bmsDevice[gunSn].frameData.brmData.brmBatteryProperty = get_data_form_string(DATA_TYPE_CHAR,23,pCanFrame->len,pCanFrame->data);
        memcpy(bmsDevice[gunSn].frameData.brmData.brmVin,get_string_form_string(24,sizeof(bmsDevice[gunSn].frameData.brmData.brmVin),pCanFrame->len,pCanFrame->data),sizeof(bmsDevice[gunSn].frameData.brmData.brmVin));
        if(bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
        {
            memcpy(bmsDevice[gunSn].frameData.brmData.brmSoftVersion,get_string_form_string(41,sizeof(bmsDevice[gunSn].frameData.brmData.brmSoftVersion),pCanFrame->len,pCanFrame->data),sizeof(bmsDevice[gunSn].frameData.brmData.brmSoftVersion));
        }
    }
}

void bcp_data_handle_gb2015(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        unsigned short bcpStartSoc;
        unsigned short macChargerCurr = get_data_form_string(DATA_TYPE_INT16U,2,pCanFrame->len,pCanFrame->data);
        
        if (macChargerCurr > MAX_CHARGER_CURRENT_OFFSETT_GB)
        {
           // DLOG("BCP ��������������������",60);  �����п��ܴ���400
        }
        
        //����BCP��������
        bmsDevice[gunSn].frameData.bcpData.bcpSingleMaxVol = get_data_form_string(DATA_TYPE_INT16U,0,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerCurrent = MAX_CHARGER_CURRENT_OFFSETT_GB - get_data_form_string(DATA_TYPE_INT16U,2,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.bcpData.bcpBatteryTotalEnergy = get_data_form_string(DATA_TYPE_INT16U,4,pCanFrame->len,pCanFrame->data);//�������ر��������  0.1kWh/λ
        bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerVoltage = get_data_form_string(DATA_TYPE_INT16U,6,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerTemperature = get_data_form_string(DATA_TYPE_UCHAR,8,pCanFrame->len,pCanFrame->data);//��������¶� ���ݷֱ���:1 ��/λ��ƫ����:-50
        bcpStartSoc = get_data_form_string(DATA_TYPE_INT16U,9,pCanFrame->len,pCanFrame->data);//�������SOC
        
        if (bmsDevice[gunSn].bmsData.chargerCnt == 0)
        {
            bmsDevice[gunSn].frameData.bcpData.bcpStartSoc = bcpStartSoc;//�������SOC
            bmsDevice[gunSn].bmsData.bcpStartSoc = bmsDevice[gunSn].frameData.bcpData.bcpStartSoc;
        }
        
        bmsDevice[gunSn].frameData.bcpData.bcpCurBatteryVoltage = get_data_form_string(DATA_TYPE_INT16U,11,pCanFrame->len,pCanFrame->data);//��ǰ��ص�ѹ �Ŵ�10�� 0.1V/bit
    }
}

static void bcp_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
            bcp_data_handle_gb2015(pCanFrame,gunSn);
        }
        break;
    }
    
}

static void bro_data_handle_gb2015(const PCanFrame *pCanFrame,uint8_t gunSn)
{
     if (pCanFrame)
     {
         int data = get_data_form_string(DATA_TYPE_CHAR,0,pCanFrame->len,pCanFrame->data);
         
         if (data == 0xaa)
         {
            bmsDevice[gunSn].frameData.broData.broReadyState = 1;
         }
         else
         {
            bmsDevice[gunSn].frameData.broData.broReadyState = 0;
         }
     }
}

static void bro_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           bro_data_handle_gb2015(pCanFrame,gunSn);
        }
        break;
        
    }
}

static void bcs_data_handle_gb2015(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureVoltage = get_data_form_string(DATA_TYPE_INT16U,0,pCanFrame->len,pCanFrame->data);//����ѹ����ֵ �Ŵ�10�� 0.1V/bit
        bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent = MAX_CHARGER_CURRENT_OFFSETT_GB - get_data_form_string(DATA_TYPE_INT16U,2,pCanFrame->len,pCanFrame->data);//����������ֵ �Ŵ�10�� 0.1A/bit
        bmsDevice[gunSn].frameData.bcsData.bcsSingleInfo.all = get_data_form_string(DATA_TYPE_INT16U,4,pCanFrame->len,pCanFrame->data);//�����ѹ��Ϣ
        bmsDevice[gunSn].frameData.bcsData.bcsCurSoc = get_data_form_string(DATA_TYPE_UCHAR,6,pCanFrame->len,pCanFrame->data);//��ǰSOC
        int idata = get_data_form_string(DATA_TYPE_INT16U,7,pCanFrame->len,pCanFrame->data);//ʣ����ʱ�� min
        if((idata>600)||(idata<0))
            idata = 600;
        bmsDevice[gunSn].frameData.bcsData.bcsChargerLeftTime = idata;

    }
}

static void bcs_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           bcs_data_handle_gb2015(pCanFrame,gunSn);
        }
        break;
    }
}

static void bcl_data_handle_gb2015(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        unsigned short maxChargerCurr = get_data_form_string(DATA_TYPE_INT16S,2,pCanFrame->len,pCanFrame->data);
        
        bmsDevice[gunSn].frameData.bclData.bclVoltageDemand = get_data_form_string(DATA_TYPE_INT16U,0,pCanFrame->len,pCanFrame->data);;//BMS�����ѹ �Ŵ�10�� 0.1V/bit
        
        if (maxChargerCurr <= MAX_CHARGER_CURRENT_OFFSETT_GB)
        {
            bmsDevice[gunSn].frameData.bclData.bclCurrentDemand = MAX_CHARGER_CURRENT_OFFSETT_GB - maxChargerCurr;//BMS������� �Ŵ�10�� 0.1A/bit
        }
        else
        {
            bmsDevice[gunSn].frameData.bclData.bclCurrentDemand = 0;
            DLOG("gun%d:BCL ����������������Ϲ���",gunSn);
        }
        
        bmsDevice[gunSn].frameData.bclData.bclChargerMode = get_data_form_string(DATA_TYPE_UCHAR,4,pCanFrame->len,pCanFrame->data);//���ģʽ(Ox01: ��ѹ��� ;Ox02: �������)
    }
}

static void bcl_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           bcl_data_handle_gb2015(pCanFrame,gunSn); 
        }
        break;
        
    }
}

/**
  * @brief  BMS���ģ�������͹�����bcm���ģ��˱���ר���ڹ�װ���ԣ�����BMS�˶���׮�˸���������
  */
/*
static void bcm_data_handle_check(const PCanFrame *pCanFrame)
{
    if (pCanFrame)
    {
        bmsDevice.frameData.bcmData.ms_state.all = get_data_form_string(DATA_TYPE_UCHAR,0,pCanFrame->len,pCanFrame->data);//״̬��������״̬
        bmsDevice.frameData.bcmData.bsmvol = get_data_form_string(DATA_TYPE_UCHAR,1,pCanFrame->len,pCanFrame->data);//��Դ��ѹ
        bmsDevice.frameData.bcmData.gunvol = get_data_form_string(DATA_TYPE_INT16U,2,pCanFrame->len,pCanFrame->data);//����ѹ
        bmsDevice.frameData.bcmData.meterenerge = get_data_form_string(DATA_TYPE_INT,4,pCanFrame->len,pCanFrame->data);//����ѹ
        
        DLOG("recv st:%d-bmsvol:%d-gunvol:%d-meter:%d",bmsDevice.frameData.bcmData.ms_state.all,bmsDevice.frameData.bcmData.bsmvol,
        bmsDevice.frameData.bcmData.gunvol,bmsDevice.frameData.bcmData.meterenerge);
        
        //SHOW("rev Bvol:%d",bmsDevice.frameData.bcmData.bsmvol);
    }
}
*/

/**
  * @brief  ֱ�ӽ���װ���ķ���һ���ӿ���,�Ǹ��׶���Ҫ���;ͷ����ĸ��׶�,�������������ص�
  */
/*
void hardware_check_msg_deal(void)
{
    static uint8_t mainStateback = 0;
    
    if(checkprocess.checkflag.bit.cProcess == 1)//ֻ���ڹ�װʱ����CCM����,ʹ��CAN��ץһ�±��Ŀ���
    {
        const PCanFrame *pCanFrame;
        
        pCanFrame = get_canheadler_bms_can_frame(BMS_BCM_TYPE,1);
        if (pCanFrame)//����bmsDevice.frameData.bcmData
        {
            bcm_data_handle_check(pCanFrame);
            bmsComFrame.waitBcm41Time = WAIT_FRAME_TIME_5000MS; 
        }

        if(mainStateback != checkprocess.mainState)//��װ״̬�����仯,��������һ��
        {
            bmsComFrame.ccm40.sendTime = 0;
            mainStateback = checkprocess.mainState;
        }
        
        if (bmsComFrame.ccm40.sendTime == 0)//if (unBmsTaskClockFlag.bit.b250ms)
        {
            send_ccm40();
            bmsComFrame.ccm40.sendTime = SENDTIMEOUT250_MS;
        }
    }
}
*/

static void bsm_data_handle_gb2015(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        bmsDevice[gunSn].frameData.bsmData.bsmMaxSingleVolGroup = get_data_form_string(DATA_TYPE_UCHAR,0,pCanFrame->len,pCanFrame->data);//��ߵ��嶯�����ص�ѹ���ڱ��
        bmsDevice[gunSn].frameData.bsmData.bsmMaxTemperature = get_data_form_string(DATA_TYPE_UCHAR,1,pCanFrame->len,pCanFrame->data);//��߶��������¶�  ���ݷֱ���:1 ��/λ,ƫ����-50
        bmsDevice[gunSn].frameData.bsmData.bsmMaxTemperatureGroup = get_data_form_string(DATA_TYPE_UCHAR,2,pCanFrame->len,pCanFrame->data);//����¶ȼ�����
        bmsDevice[gunSn].frameData.bsmData.bsmMinTemperature = get_data_form_string(DATA_TYPE_UCHAR,3,pCanFrame->len,pCanFrame->data);//��Ͷ��������¶�
        bmsDevice[gunSn].frameData.bsmData.bsmMinTemperatureGroup = get_data_form_string(DATA_TYPE_UCHAR,4,pCanFrame->len,pCanFrame->data);//��Ͷ��������¶ȼ�����
        bmsDevice[gunSn].frameData.bsmData.bsmBatteyState = get_data_form_string(DATA_TYPE_UCHAR,5,pCanFrame->len,pCanFrame->data);//���״̬��Ϣ
        bmsDevice[gunSn].frameData.bsmData.bsmByte7.all = get_data_form_string(DATA_TYPE_UCHAR,6,pCanFrame->len,pCanFrame->data);//����״̬�ͳ������

        bmsDevice[gunSn].unBmsStateA.bit.bPowerState = (0 ==bmsDevice[gunSn].frameData.bsmData.bsmByte7.bit.bChargeAllow)?0:1; 
        bmsDevice[gunSn].unBmsStateA.bit.bBSMPause = (0 ==bmsDevice[gunSn].frameData.bsmData.bsmByte7.bit.bChargeAllow);
    }
}

static void bsm_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    switch(bmsDevice[gunSn].bmsType)
    {
        case BMS_GB_2015_TYPE:
        case BMS_GB_TYPE:
        {
           bsm_data_handle_gb2015(pCanFrame,gunSn); 
        }
        break;
        
    }
}

void bst_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        if (pCanFrame->data[0])
        {
            bmsDevice[gunSn].unBmsStateA.bit.bBmsChargerStopNormal = 1;
        }
        else if (pCanFrame->data[1]||pCanFrame->data[2])
        {
            bmsDevice[gunSn].unBmsStateA.bit.bBmsFault = 1;
        }
    }
}

void bsd_data_handle(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    if (pCanFrame)
    {
        bmsDevice[gunSn].frameData.bsdData.bsdSoc = get_data_form_string(DATA_TYPE_UCHAR,0,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.bsdData.bsdMinSingleVol = get_data_form_string(DATA_TYPE_INT16U,1,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.bsdData.bsdMaxSingleVol = get_data_form_string(DATA_TYPE_INT16U,3,pCanFrame->len,pCanFrame->data);
        bmsDevice[gunSn].frameData.bsdData.bsdMinTemp = get_data_form_string(DATA_TYPE_UCHAR,5,pCanFrame->len,pCanFrame->data);//��Ͷ��������¶�
        bmsDevice[gunSn].frameData.bsdData.bsdMaxTemp = get_data_form_string(DATA_TYPE_UCHAR,6,pCanFrame->len,pCanFrame->data);//���ض��������¶�
    }
}

static void recv_bcs_state_handle(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCS_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcs_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCL_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcl_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BSM_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bsm_data_handle(pCanFrame,gunSn);
    }
    
    if (get_bms_can_frame_exit_flag(BMS_BCL_TYPE,gunSn) || get_bms_can_frame_exit_flag(BMS_BCS_TYPE,gunSn))
    {
        bmsComFrame[gunSn].ccs12.sendTime = SENDTIMEOUT20_MS;
        bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
        bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
        
        bmsDevice[gunSn].subState = RECV_BCL10;
        bmsDevice[gunSn].mainState = CHARGING;
        get_canheadler_bms_can_frame(BMS_BEM_TYPE,1,gunSn);//������ǰ���BEM
        DLOG("gun%d:������׶�",gunSn);
    }
    
    if (bmsComFrame[gunSn].cro0A.sendTime == 0)
    {
        send_cro0A(0xaa,gunSn);
        bmsComFrame[gunSn].cro0A.sendTime = SENDTIMEOUT250_MS;
    }
    
    if (bmsComFrame[gunSn].waitBcl10Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCL_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCL_TIMEOUT,0,gunSn);
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
    
    //��ʱû���յ�BCS����
    if (bmsComFrame[gunSn].waitBcs11Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCS_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCS_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
}

static void recv_bcl_state_handle(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;

    pCanFrame = get_canheadler_bms_can_frame(BMS_BCS_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcs_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCL_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcl_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BSM_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bsm_data_handle(pCanFrame,gunSn);
    }
    
    if (bmsComFrame[gunSn].ccs12.sendTime == 0)
    {
        send_ccs12(0x01,gunSn);
    }
    
    if (bmsComFrame[gunSn].waitBcl10Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCL_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCL_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
    
    //��ʱû���յ�BCS����
    if (bmsComFrame[gunSn].waitBcs11Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCS_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCS_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
}

void bms_extern_charger_stop_cst_fill(int stopCauseSn,uint8_t gunSn)
{
    if (bmsComFrame[gunSn].chm26.unFrameFlag.bit.bLock == 0)
    {
        bmsComFrame[gunSn].chm26.unFrameFlag.bit.bLock = 1;
        bmsComInternalData[gunSn].cstFaultFlag = 1;
        switch(stopCauseSn)
        {
            case STOP_BY_ENERGY:
            case STOP_BY_TIME:
            case STOP_BY_MONEY:
            case STOP_BY_MONEY_NOTENOUGH://�ﵽ�趨����ֹͣ
            {
                bmsComInternalData[gunSn].cstIndex = 0;
                bmsComInternalData[gunSn].cstValue = (1<<0);
                bmsComInternalData[gunSn].cstFaultFlag = 0;
            }
            break;
            
            case STOP_BY_ADMIN:
            case STOP_BY_BACKGROUP:
            case STOP_BY_USER:
            case STOP_BY_CARD://�˹�ֹͣ
            {
                bmsComInternalData[gunSn].cstIndex = 0;
                bmsComInternalData[gunSn].cstValue = (1<<2);
                bmsComInternalData[gunSn].cstFaultFlag = 0;
            }
            break;

            case BMS_STOP_CAUSE_RECV_BST_0_00:
            case BMS_STOP_CAUSE_RECV_BST_0_02:	
            case BMS_STOP_CAUSE_RECV_BST_0_04:
            case BMS_STOP_CAUSE_RECV_BST_0_06:
            case BMS_STOP_CAUSE_RECV_BST_1_00:
            case BMS_STOP_CAUSE_RECV_BST_1_02:
            case BMS_STOP_CAUSE_RECV_BST_1_04:		
            case BMS_STOP_CAUSE_RECV_BST_1_06:
            case BMS_STOP_CAUSE_RECV_BST_2_00:
            case BMS_STOP_CAUSE_RECV_BST_2_02:	
            case BMS_STOP_CAUSE_RECV_BST_2_04:   
            case BMS_STOP_CAUSE_RECV_BST_2_06:   
            case BMS_STOP_CAUSE_RECV_BST_3_00:
            case BMS_STOP_CAUSE_RECV_BST_3_02:		
            case BMS_STOP_CAUSE_RECV_BST_OHTER://BSTֹͣ
            {
                bmsComInternalData[gunSn].cstIndex = 0;
                bmsComInternalData[gunSn].cstValue = (1<<6);
                bmsComInternalData[gunSn].cstFaultFlag = 0;
            }
            break;
            
            case ALARM_CC1_GUN1://�Ͽ�����
			case ALARM_CC1_GUN2://�Ͽ�����
            {
                bmsComInternalData[gunSn].cstIndex = 1;
                bmsComInternalData[gunSn].cstValue = (1<<2);
            }
            break;
 
            case ALARM_OUTLETTEMPUP://����ڹ��¡������ڲ�����
            {
                 bmsComInternalData[gunSn].cstIndex = 1;
                 bmsComInternalData[gunSn].cstValue = (1<<4);
            }
            break;             
            
            case ALARM_METER_COM:
            {
                 bmsComInternalData[gunSn].cstIndex = 1;
                 bmsComInternalData[gunSn].cstValue = (1<<6);
            }
            break;      

            case ALARM_EPO://��ͣ
            {
                 bmsComInternalData[gunSn].cstIndex = 2;
                 bmsComInternalData[gunSn].cstValue = (1<<0);
            }
            break;
       
			case ALARM_VDCUP_GUN1:
			case ALARM_VDCUP_GUN2:
			case ALARM_VDCLOW_GUN1:
			case ALARM_VDCLOW_GUN2:
			case STOP_BCP_BATVOLAGE:
			case STOP_BMS_SINGLE_VUP:
			case STOP_INSDET_VBEFORE:
			case STOP_VOLTAGE_BMSMAX:
			case STOP_VOLTAGE_BMSDEMAND:
			case STOP_BAT_DEMAND_VOLTAGE:
			case STOP_BSM_BATLOW:
			case STOP_BSM_VSINGLE:
			case STOP_BMS_BCSVOL:
			{
                 bmsComInternalData[gunSn].cstIndex = 3;
                 bmsComInternalData[gunSn].cstValue = (1<<2);
            }
	        break;		

            case ALARM_IDCUP_GUN1://�������
            case STOP_BAT_DEMAND_CURRENT:
            case STOP_BSM_OC:
            {
                 bmsComInternalData[gunSn].cstIndex = 3;
                 bmsComInternalData[gunSn].cstValue = (1<<0);
            }
            break;
			
            default://��������
                bmsComInternalData[gunSn].cstIndex = 2;
                bmsComInternalData[gunSn].cstValue = (1<<2);
                break;
        }
    }
}

void  bms_internal_charger_stop_cst_fill(const PCanFrame *pCanFrame,uint8_t gunSn)
{
    short i;
    int idata = 0;
    
    if (bmsComFrame[gunSn].chm26.unFrameFlag.bit.bLock == 0)
    {
        bmsComFrame[gunSn].chm26.unFrameFlag.bit.bLock = 1;
        
        bmsComInternalData[gunSn].cstIndex = 0;
        bmsComInternalData[gunSn].cstValue = 0;
        
        idata = (pCanFrame->data[3]<<24)|(pCanFrame->data[2]<<16)|(pCanFrame->data[1]<<8)|(pCanFrame->data[0]);
        
        for(i=0;i<16;i++)
        {
            if(idata & 0x01)
            {
                set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BST_0_00 + i, 1,gunSn);
                break;
            }
            
            idata = idata >> 2;
        }
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BST_OHTER,1,gunSn);
    }
}

/**
  * @brief  
  */
static void bms_wait_stage(uint8_t gunSn)
{
    bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
    sRecvFlg_Kaipu[gunSn].u8BsmFlg=0;	
    
    bmsDevice[gunSn].bmsData.chargerCnt++;
	
   // send_debug_info60();
    
    if (sSysMegg[gunSn].CC1) //&& (bmsDevice.unBmsStateA.bit.bGb2015ProtocolFlag))
    {
        if ((bmsDevice[gunSn].bmsData.chargerCnt <= MAX_TIMEOUT_STOP_TRY_CHARGER_CNT)
            && (0 == bmsExternData[gunSn].externStopCause)
            && (0 == bmsDevice[gunSn].bmsData.bmsStopCause))
        {
            start_bms_handshake_init(gunSn);
            bmsDevice[gunSn].frameData.bcpData.bcpStartSoc = bmsDevice[gunSn].bmsData.bcpStartSoc;
            bmsDevice[gunSn].unBmsStateA.bit.bReChargerFlag = 1;

            DLOG("gun%d:BMS���Լ���ֵ = %d", gunSn,bmsDevice[gunSn].bmsData.chargerCnt);
            return;
        }
    }

    bmsDevice[gunSn].bmsData.chargerCnt = 0;
    bmsDevice[gunSn].mainState = IDLE;
}
            
static void bms_handshake_stage(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    //�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
    if (bmsExternData[gunSn].externStopCause)
    {
        bms_charger_stop(gunSn);


        bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
        
        if (bmsDevice[gunSn].subState != RECV_BST19)
        {
            send_cst1A(gunSn);
            send_cem20(gunSn);
            send_cem22(gunSn);
            
            bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
            bmsDevice[gunSn].subState = RECV_BST19;
        }
    }
    
    switch (bmsDevice[gunSn].subState)
    {
        case SEND_CHM:
            {
                //if (!bmsExternData.unStateA.bit.bChargerIsFlag)
                {
                    if (bmsComFrame[gunSn].chm26.sendTime == 0)
                    {
                        send_chm26_gb2015(gunSn);
                        bmsComFrame[gunSn].chm26.sendTime = SENDTIMEOUT250_MS;
                    }
                }
                    
                if (bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag == 0)
                {
                    pCanFrame = get_canheadler_bms_can_frame(BMS_BHM_TYPE,0,gunSn);
                    if (pCanFrame)
                    {
                         bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag = 1;
                         bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol = get_data_form_string(DATA_TYPE_INT16U,0,pCanFrame->len,pCanFrame->data);
                         
                         bmsDevice[gunSn].bmsData.insulationVol = bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol;
                         bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag = 1;//������Ե���
                         //bmsExternData[gunSn].unStateA.bit.bChargerIsFlag = 1;
                         
                         DLOG("gun%d:�յ�BHM[%d],������Ե",gunSn,bmsDevice[gunSn].bmsData.insulationVol);
                    }
                    else if (bmsExternData[gunSn].unStateA.bit.bChargerIsFlag)
                    {
                        bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol = bmsDevice[gunSn].bmsData.insulationVol;
                        bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag = bmsExternData[gunSn].unStateA.bit.bGb2015ProtocolFlag;
                        bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag = 1;//������Ե���
                        DLOG("gun%d:û�����²�ǹ���γ�磬������Ե[%d]",gunSn,bmsDevice[gunSn].bmsData.insulationVol);
                    }
                    
                    if (bmsComFrame[gunSn].waitBhm27Time == 0)
                    {
                        bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag = 1;//������Ե���
                        DLOG("gun%d:����BHM���ĳ�ʱ,������Ե",gunSn);
                    }
                }

                if (get_charge_state(gunSn) > CHARGER_STATE_INS_DISCHARGE)
                {
                    uint16_t max_cnt = (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)?9000:3000;

                    if(bmsComInternalData[gunSn].insdetXieFangCnt++>=max_cnt)
                    {
                        DLOG("gun%d:��ʼ����CRM����",gunSn);
                        bmsExternData[gunSn].unStateA.bit.bChargerIsFlag = 1;
                        start_bms_handshake_init(gunSn);
                    }
                }
                
                //���������ʱ
                if (bmsComInternalData[gunSn].userStartSecondCnt > 120)
                {
                    bmsDevice[gunSn].mainState = IDLE;
                    set_bms_stop_cause(BMS_STOP_CAUSE_CHARGE_STARTUP_TIMEOUT_CRM,1,gunSn);
                }
            }
            break;
            
         case SEND_CRM01_00:
            {
                if (bmsComFrame[gunSn].crm01.sendTime == 0)
                {
                    send_crm01(0,gunSn);
                    bmsComFrame[gunSn].crm01.sendTime = SENDTIMEOUT250_MS;
//                    DLOG("gun%d:����CRM 00����",gunSn);
                }
             
                //�յ�BRM����
                pCanFrame = get_canheadler_bms_can_frame(BMS_BRM_TYPE,0,gunSn);
                if (pCanFrame)
                {
                    brm_data_handle(pCanFrame,gunSn);
                    bmsComFrame[gunSn].crm01.sendTime = 0;
                    bmsComFrame[gunSn].waitBcp06Time = WAIT_FRAME_TIME_5000MS;
                    bmsDevice[gunSn].subState = SEND_CRM01_01;
                }
                
                if (bmsComFrame[gunSn].waitBrm02Time == 0)
                {
                    bmsDevice[gunSn].bmsData.cemType = BRM_FRAME_TIMEOUT;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                    
                    set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BRM_TIMEOUT,0,gunSn);
                    DLOG("BRM���Ļظ���ʱ,�˳�SEND_CRM01_00״̬",30);
                    
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                    bmsDevice[gunSn].subState = SEND_CEM1F;
                    bmsDevice[gunSn].mainState = CHARG_OVER;
                }
            }
            break;
         case SEND_CRM01_01:
            {
                 bms_type_judge(gunSn);
                 if (bmsComFrame[gunSn].crm01.sendTime == 0)
                 {
                     send_crm01(0xaa,gunSn);
                     
                     bmsComFrame[gunSn].crm01.sendTime = SENDTIMEOUT250_MS;
                     bmsDevice[gunSn].subState = RECV_BCP06;
                     bmsDevice[gunSn].mainState = SETTING;
                     
                     DLOG("gun%d:�������ý׶�",gunSn);
                 }
                 
                 //��ʱû���յ�BCP����
                 if (bmsComFrame[gunSn].waitBcp06Time == 0)
                 {
                    bmsDevice[gunSn].bmsData.cemType = BCP_FRAME_TIMEOUT;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                    set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCP_TIMEOUT,0,gunSn);
                     
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                    bmsDevice[gunSn].subState = SEND_CEM1F;
                    bmsDevice[gunSn].mainState = CHARG_OVER;
                 }
             }
             break;
         default:
            {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
            }
             break;
    }
}


#if 0
static void bms_handshake_stageEx(void)
{
    const PCanFrame * pCanFrame = 0;
    
    switch (bmsDevice.subState)
    {
        case SEND_CHM:
            if(bmsComFrame.chm26.sendTime == 0)
            {
                send_chm26_gb2015();
                bmsComFrame.chm26.sendTime = SENDTIMEOUT250_MS;
            }

            pCanFrame = get_canheadler_bms_can_frame(BMS_BHM_TYPE,0);
            if(0 == pCanFrame)
            {
                if(0 != bmsComFrame.waitBhm27Time)//����ģʽ���ɹ���͵ȴ���ʱ5��
                {
                    break;
                }
            }
            else
            {
                bmsDevice.unBmsStateA.bit.bGb2015ProtocolFlag = 1;
                bmsDevice.frameData.bhmData.bhmMaxChargerVol = get_data_form_string(DATA_TYPE_INT16U,0,pCanFrame->len,pCanFrame->data);
                bmsDevice.bmsData.insulationVol = bmsDevice.frameData.bhmData.bhmMaxChargerVol;
                bmsExternData.unStateA.bit.bChargerIsFlag=1;
            }
            
            start_bms_handshake_init();
            break;
        case SEND_CRM01_00:
            if(bmsComFrame.crm01.sendTime == 0)
            {
                send_crm01(0);
                bmsComFrame.crm01.sendTime = SENDTIMEOUT250_MS;
            }
         
            pCanFrame = get_canheadler_bms_can_frame(BMS_BRM_TYPE,0);
            if(0 == pCanFrame)
            {
                if(0 != bmsComFrame.waitBrm02Time)
                {
                    break;
                }
            }
            else
            {
                brm_data_handle(pCanFrame);
                bmsDevice.mainState = IDLE;                
            }
        default:
            bms_charger_stop();
            bmsDevice.mainState = IDLE;
            break;
    }
}
#endif

static void bms_config_stage(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    //�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
    if (bmsExternData[gunSn].externStopCause)
    {
        bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;

        bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
        
        if (bmsDevice[gunSn].subState != RECV_BST19)
        {
            send_cst1A(gunSn);
            send_cem20(gunSn);
            send_cem22(gunSn);
            
            bmsDevice[gunSn].mainState = IDLE;
   /*         bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
            bmsDevice[gunSn].subState = RECV_BST19;*/
        }
    }
    
    switch (bmsDevice[gunSn].subState)
    {       
        case RECV_BCP06:
            {
                pCanFrame = get_canheadler_bms_can_frame(BMS_BCP_TYPE,0,gunSn);
                if (pCanFrame)
                {
                     bcp_data_handle(pCanFrame,gunSn);
                     
                     bmsDevice[gunSn].frameData.broData.broReadyState = 0;
                     
                     bmsComFrame[gunSn].cts07.sendTime = 0;
                     bmsComFrame[gunSn].cml08.sendTime = 0;
                     bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                     bmsComFrame[gunSn].waitBro09OkTime = WAIT_FRAME_TIME_60000MS;
                     
                     bmsDevice[gunSn].subState = SEND_CTS07;
                }
                else
                {
                    bmsDevice[gunSn].mainState = SHAKE_HAND;
                    bmsDevice[gunSn].subState = SEND_CRM01_01;
                }
            }
            break;
         case SEND_CTS07:
            {
                if (bmsComFrame[gunSn].cts07.sendTime == 0)
                {
                    send_cts07(gunSn);
                    bmsComFrame[gunSn].cts07.sendTime = SENDTIMEOUT500_MS;
                }
                
                if (bmsComFrame[gunSn].cml08.sendTime == 0)
                {
                    send_cml08(bmsExternData[gunSn].chargerMaxOutputVol,bmsExternData[gunSn].chargerMinOutputVol,bmsExternData[gunSn].chargerMaxOutputCur,gunSn);
                    bmsComFrame[gunSn].cml08.sendTime = SENDTIMEOUT250_MS;
                }
                
                bmsDevice[gunSn].subState = RECV_BRO09;
            }
            break;
         case RECV_BRO09:
             pCanFrame = get_canheadler_bms_can_frame(BMS_BRO_TYPE,1,gunSn);
             if (pCanFrame)
             {
                bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                bro_data_handle(pCanFrame,gunSn);
             }
             
             if (bmsDevice[gunSn].frameData.broData.broReadyState)
             {
                 bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                 bmsComFrame[gunSn].cro0A.maxSendTime = SENDTIMEOUT60000_MS;
                 bmsDevice[gunSn].bmsData.croAASendCnt = SENDTIMEOUT10000_MS;
                 bmsDevice[gunSn].subState = SEND_CRO0A;
                 DLOG("gun%d:bms���׼����",gunSn);
             }
             else if ((bmsComFrame[gunSn].waitBro09Time == 0) || (bmsComFrame[gunSn].waitBro09OkTime == 0))
             {
                 bmsDevice[gunSn].bmsData.cemType = BRO_FRAME_TIMEOUT;
                 send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                 set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BRO_TIMEOUT,0,gunSn);
                 bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                 bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                 bmsDevice[gunSn].subState = SEND_CEM1F;
                 bmsDevice[gunSn].mainState = CHARG_OVER;
             }
             else
             {
                if((bmsComFrame[gunSn].cts07.sendTime == 0)
                || (bmsComFrame[gunSn].cml08.sendTime == 0))
                {
                    bmsDevice[gunSn].subState = SEND_CTS07;
                }
             }
             
             break;
         case SEND_CRO0A:
            {
                //if (1)//��Ϊʵ���Ϻܶ೵��û�а��¹���ִ�У���Ҫ�����ȷ���׼�������󣬵�ص�ѹ�Źҹ����������������Ԥ��
                //#warning for test ��֤��
                char en = 0;
//                if((0 != sChSysMegg.unFlagChSysStA.bit.bDcConTOk)
//                    ||(0 == bmsDevice[gunSn].bmsData.croAASendCnt))
                {
                    en = 1;
                }
                
                if (en)
                {
                    send_cro0A(0xaa,gunSn);
                    
                    bmsDevice[gunSn].unBmsStateA.bit.bComChargingStageFlag = 1;
                    bmsDevice[gunSn].bmsData.chargerTimeSecondCnt = 0;//����ʱ����
 /*                   DLOG("bmsԤ������Cnt[%d] DcConTOk[%d] V-port[%d] V-mod[%d]", \
                        bmsDevice[gunSn].bmsData.croAASendCnt,sChSysMegg.unFlagChSysStA.bit.bDcConTOk, \
                        sChSysMegg.sCalRms.uAbsSysPortVdc,sChSysMegg.sCalRms.uAbsModPortVdc);
  */                  
                    bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_5000MS;
                    bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
                    bmsComFrame[gunSn].cro0A.sendTime = SENDTIMEOUT250_MS;
                    bmsDevice[gunSn].subState = RECV_BCS11;
                }
                else if (bmsComFrame[gunSn].cro0A.sendTime == 0)
                {
                    send_cro0A(0x00,gunSn);
                    bmsComFrame[gunSn].cro0A.sendTime = SENDTIMEOUT250_MS;
                }
                
                //��ʱ
                if (bmsComFrame[gunSn].cro0A.maxSendTime == 0)
                {
                    bmsDevice[gunSn].mainState = WAIT_START;
                }
            }
            break;
         case RECV_BCS11:
            bmsDevice[gunSn].bmsData.chargerCnt = 0; //������׶������������
            recv_bcs_state_handle(gunSn);
            break;
         default:
             {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
             }
             break;
    }
}

static void bms_charging_stage(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BST_TYPE,0,gunSn);
    if (pCanFrame)
    {
        bms_charger_stop(gunSn);
        
        bst_data_handle(pCanFrame,gunSn);
        bms_internal_charger_stop_cst_fill(pCanFrame,gunSn);
        bmsComInternalData[gunSn].cstFaultFlag = 0;
        
        bmsComFrame[gunSn].cst1A.sendTime = 0;
        bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
        bmsDevice[gunSn].subState = RECV_BST19;
    }
    
    //�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
    if (bmsExternData[gunSn].externStopCause)
    {
        bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;

        bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
        
        if (bmsDevice[gunSn].subState != RECV_BST19)
        {
            send_cst1A(gunSn);
            send_cem20(gunSn);
            send_cem22(gunSn);
            
            bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
            bmsDevice[gunSn].subState = RECV_BST19;
        }
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BEM_TYPE,1,gunSn);
    if (pCanFrame)
    {
        uint32_t errorData = get_data_form_string(DATA_TYPE_INT,0,pCanFrame->len,pCanFrame->data);
        if(0 != errorData)
        {
            bms_charger_stop(gunSn);
            set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BEM_FRAME,0,gunSn);
            bmsDevice[gunSn].mainState = WAIT_START;
        }
        else
        {
            //DLOG("û�д��󷢸�ë��");
        }
    }
    
    switch (bmsDevice[gunSn].subState)
    {
        case RECV_BCL10:
            recv_bcl_state_handle(gunSn);
            break;
        case RECV_BST19:
            pCanFrame = get_canheadler_bms_can_frame(BMS_BST_TYPE,1,gunSn);
            if (pCanFrame)
            {
                bst_data_handle(pCanFrame,gunSn);

                bms_charger_stop(gunSn);
                
                bmsComFrame[gunSn].waitBsd1DTime = WAIT_FRAME_TIME_10000MS;
                bmsDevice[gunSn].subState = RECV_BSD1C;
                bmsDevice[gunSn].mainState = CHARG_OVER;
                DLOG("gun%d:����������׶�",gunSn);
            }
            
            if (bmsComFrame[gunSn].cst1A.sendTime == 0)
            {
                send_cst1A(gunSn);
                bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            }
            
            if (bmsComFrame[gunSn].waitBst19Time == 0)
            {
                bmsDevice[gunSn].bmsData.cemType = BST_FRAME_TIMEOUT;
                send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BST_TIMEOUT,0,gunSn);
                
                bmsComFrame[gunSn].waitBsd1DTime = WAIT_FRAME_TIME_10000MS;
                bmsDevice[gunSn].subState = RECV_BSD1C;
                bmsDevice[gunSn].mainState = CHARG_OVER;;
            }
            break;
        default:
            {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
            }
            break;
    }
    
    if (unBmsTaskClockFlag.bit.b1s)
    {

        if (!bmsDevice[gunSn].unBmsStateA.bit.bPowerState)
        {
              bmsDevice[gunSn].bmsData.bsmStopTimeCnt++;
        }
        else
        {
            bmsDevice[gunSn].bmsData.bsmStopTimeCnt = 0;
        }
        
        if (bmsDevice[gunSn].bmsData.bsmStopTimeCnt > 600)
        {
            if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
            {
                set_bms_stop_cause(BMS_STOP_CAUSE_BSM_CHARGER_ALLOW_TIMEOUT,1,gunSn);
                bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
                bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
                bmsDevice[gunSn].subState = RECV_BST19;
            }
        }
    }
    
    /*if (unBmsTaskClockFlag.bit.b5s)
    {
        send_debug_info60();
    }*/
}

void bms_charger_over_frame_handle(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCS_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcs_data_handle(pCanFrame,gunSn);
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BSD_TYPE,0,gunSn);
    if (pCanFrame)
    {
        bsd_data_handle(pCanFrame,gunSn);
        bmsDevice[gunSn].frameData.bcsData.bcsCurSoc = bmsDevice[gunSn].frameData.bsdData.bsdSoc;
        
        if (bmsDevice[gunSn].frameData.bcsData.bcsCurSoc > 100)
        {
            bmsDevice[gunSn].frameData.bcsData.bcsCurSoc = 100;
        }
    }
}

void bms_charger_over_stage(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
    
    bms_charger_over_frame_handle(gunSn);
    
    switch (bmsDevice[gunSn].subState)
    {
        case RECV_BSD1C:
            if (bmsComFrame[gunSn].cst1A.sendTime == 0)
            {
                send_cst1A(gunSn);
                bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            }
            
            pCanFrame = get_canheadler_bms_can_frame(BMS_BSD_TYPE,1,gunSn);
            if ((pCanFrame) ||(bmsComFrame[gunSn].waitBsd1DTime == 0))
            {
                bsd_data_handle(pCanFrame,gunSn);
                DLOG("gun%d:���յ�BMSͳ�Ʊ���",gunSn);
                
                bmsComFrame[gunSn].csd1D.sendTime = 0;
                bmsComFrame[gunSn].csd1D.maxSendTime = SENDTIMEOUT5000_MS;
                
                bmsDevice[gunSn].subState = SEND_CSD1D;
            }
            
            break;
        case SEND_CSD1D:
            if (bmsComFrame[gunSn].csd1D.sendTime == 0)
            {
                send_csd1D(gunSn);
                bmsComFrame[gunSn].csd1D.sendTime = SENDTIMEOUT250_MS;
            }
            
            if (bmsComFrame[gunSn].csd1D.maxSendTime == 0)
            {
                bmsDevice[gunSn].mainState = WAIT_START;
            }
            break;
        case SEND_CEM1F:
            {
                bmsDevice[gunSn].frameData.broData.broReadyState = 0;
            
                if (bmsComFrame[gunSn].cem1F.maxSendTime == 0)
                {
                    bmsDevice[gunSn].mainState = WAIT_START;
                    break;
                }
                
                if (bmsComFrame[gunSn].cem1F.sendTime == 0)
                {
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                }
            }
            break;
        default:
            {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
            }
            break;
    }
    
}

void bms_task_handle(void)
{	
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
        charger_sys_bms_data_contact(gunSn);

        if (unBmsTaskClockFlag.bit.b1s)
        {
            bmsDevice[gunSn].bmsData.chargerTimeSecondCnt++;
            bmsComInternalData[gunSn].userStartSecondCnt++;
		    if(IDLE != bmsDevice[gunSn].mainState)
		    {
	        //		send_debug_info61();
		    }
        }

        if((!sSysMegg[gunSn].CC1)&&(sSysMegg[gunSn].CC1_bak))
        {
            bmsDevice[gunSn].unBmsStateA.bit.bBmsChargerStopNormal = 0;
        }     

        #if 1 //����
        static uint8_t bmsMainState[2] = {0};
        static uint8_t bmsSubState[2] = {0};
        if(bmsDevice[gunSn].mainState != bmsMainState[gunSn])
        {
            bmsMainState[gunSn] = bmsDevice[gunSn].mainState;
            DLOG("gun%d:mainState = %d", gunSn,bmsMainState[gunSn]);
        }
        if(bmsDevice[gunSn].subState != bmsSubState[gunSn])
        {
            bmsSubState[gunSn] = bmsDevice[gunSn].subState;
            DLOG("gun%d:subState---%d", gunSn,bmsSubState[gunSn]);
        }
        #endif
	}
}

void bms_charger_scheduler_old(uint8_t gunSn)
{
    switch (bmsDevice[gunSn].mainState)
    {
        case IDLE:
            bmsDevice[gunSn].unBmsStateB.bit.bBmsComState = 0;
            bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
            bmsComInternalData[gunSn].cstFaultFlag = 0;
            BmsUptateFlg[gunSn].u8PtStartSocData=0;
            break;
        case WAIT_START://�ȴ����ָ��
            bms_wait_stage(gunSn);
            break;
        case SHAKE_HAND://���ֽ׶�
            bms_handshake_stage(gunSn);
            break;
        case SETTING://���ý׶�
            bms_config_stage(gunSn);
            break;
        case CHARGING://���׶�
            bms_charging_stage(gunSn);
            break;
        case CHARG_OVER://�������׶�
            bms_charger_over_stage(gunSn);
            break;
        case ERROR_STAGE://����׶�
            
            break;
        default:
            break;
    }
}


static void recv_bcs_state_handle_new(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCS_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcs_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCL_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcl_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BSM_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bsm_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBsm13Time = WAIT_FRAME_TIME_5000MS;	
		sRecvFlg_Kaipu[gunSn].u8BsmFlg= (sSysConfig.sInt.bmsType == BMS_GB_2015_TYPE)?1:0;	
    }
    
    if (get_bms_can_frame_exit_flag(BMS_BCL_TYPE,gunSn) && get_bms_can_frame_exit_flag(BMS_BCS_TYPE,gunSn))
    {
        bmsComFrame[gunSn].ccs12.sendTime = SENDTIMEOUT20_MS; // ��巢��
        bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
        bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
        bmsComFrame[gunSn].waitBsm13Time = WAIT_FRAME_TIME_5000MS;						  
        bmsDevice[gunSn].subState = RECV_BCL10;
        bmsDevice[gunSn].mainState = CHARGING;
        get_canheadler_bms_can_frame(BMS_BEM_TYPE,1,gunSn);//������ǰ���BEM
        DLOG("gun%d:������׶�",gunSn);
    }
    
    if (bmsComFrame[gunSn].cro0A.sendTime == 0)
    {
        send_cro0A(0xaa,gunSn);
        bmsComFrame[gunSn].cro0A.sendTime = SENDTIMEOUT250_MS;
    }
    
    if (bmsComFrame[gunSn].waitBcl10Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCL_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCL_TIMEOUT,0,gunSn);
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
    
    //��ʱû���յ�BCS����
    if (bmsComFrame[gunSn].waitBcs11Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCS_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCS_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }

}

static void recv_bcl_state_handle_new(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;

    pCanFrame = get_canheadler_bms_can_frame(BMS_BCS_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcs_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BCL_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bcl_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BSM_TYPE,1,gunSn);
    if (pCanFrame)
    {
        bsm_data_handle(pCanFrame,gunSn);
        bmsComFrame[gunSn].waitBsm13Time = WAIT_FRAME_TIME_5000MS;	
		sRecvFlg_Kaipu[gunSn].u8BsmFlg= (sSysConfig.sInt.bmsType == BMS_GB_2015_TYPE)?1:0;	
    }
    
    if (bmsComFrame[gunSn].ccs12.sendTime == 0)
    {
		if(sRecvFlg_Kaipu[gunSn].u8BsmFlg==1)
		{
			if(bmsDevice[gunSn].unBmsStateA.bit.bPowerState==1)
			{ 
				 send_ccs12(0x01,gunSn);
			}
			else
			{
				send_ccs12(0,gunSn);
			}	
		}
		else
		{
			send_ccs12(0x01,gunSn);
		}
    }
    
    if (bmsComFrame[gunSn].waitBcl10Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCL_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCL_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
    
    //��ʱû���յ�BCS����
    if (bmsComFrame[gunSn].waitBcs11Time == 0)
    {
        bmsDevice[gunSn].bmsData.cemType = BCS_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCS_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }

	     //��ʱû���յ�BSM����
    if((bmsComFrame[gunSn].waitBsm13Time == 0)&&(sRecvFlg_Kaipu[gunSn].u8BsmFlg==1))
    {
        bmsDevice[gunSn].bmsData.cemType = BSM_FRAME_TIMEOUT;
        send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
        set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BSM_TIMEOUT,0,gunSn);
        
        bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
        bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
        bmsDevice[gunSn].subState = SEND_CEM1F;
        bmsDevice[gunSn].mainState = CHARG_OVER;
    }
}

void bms_StopReason_Judge(int stopCauseSn)
{/*
	if(stopCauseSn>0)
	{
		if((stopCauseSn<=SYS_ALARM_CAN3COMABS_28)
            &&(stopCauseSn!=SYS_ALARM_SYSVDCUP_BIT_INDEX_3)
            &&(stopCauseSn!=SYS_ALARM_SYSVDCLOW_BIT_INDEX_4))
		{
			send_cem20();
		    send_cem22();
		}
		else if((stopCauseSn>=STOP_CAUSE_INSDET_FAIL_1014)&&(stopCauseSn<=STOP_CAUSE_REMAIN_MONEY_NOT_ENOUGH_1029)
			&&(stopCauseSn!=STOP_CAUSE_BMS_STAUE_ABNORMAL_1017))
		{
			// send_cem20();
            		// send_cem22();
		}
	}*/
}

static void bms_handshake_stage_new(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    //�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
    if (bmsExternData[gunSn].externStopCause)
    {
        bms_charger_stop(gunSn);
        
        bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
        
        if (bmsDevice[gunSn].subState != RECV_BST19)
        {
			if((bmsDevice[gunSn].subState!=SEND_CHM))
		    {
                 send_cst1A(gunSn);

                 bms_StopReason_Judge(bmsExternData[gunSn].externStopCause);

		    }
            bmsDevice[gunSn].mainState = IDLE;
        }
    }
    
    switch (bmsDevice[gunSn].subState)
    {
        case SEND_CHM:
            {
                //if (!bmsExternData.unStateA.bit.bChargerIsFlag)
                {
                    if (bmsComFrame[gunSn].chm26.sendTime == 0)
                    {
                        send_chm26_gb2015(gunSn);
                        bmsComFrame[gunSn].chm26.sendTime = SENDTIMEOUT250_MS;
                    }
                }
                    
                if (bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag == 0)
                {
                    pCanFrame = get_canheadler_bms_can_frame(BMS_BHM_TYPE,0,gunSn);
                    if (pCanFrame)
                    {
                        bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag = 1;
                        bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol = get_data_form_string(DATA_TYPE_INT16U,0,pCanFrame->len,pCanFrame->data);
                        bmsDevice[gunSn].bmsData.insulationVol = bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol;

                        bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag = 1;//������Ե���
                        bmsExternData[gunSn].unStateA.bit.bChargerIsFlag = 1;

                        bmsComFrame[gunSn].waitBhm27Time = WAIT_FRAME_TIME_5000MS;			 
                        //����CHM����ʱ��
                        //bmsComFrame.chm26.sendTime =0;
                        DLOG("gun%d:�յ�BHM����,����������Ե���",gunSn);                        
                        DLOG("gun%d:2015�¹��� ",gunSn);
                    }
                    else if (bmsExternData[gunSn].unStateA.bit.bChargerIsFlag)
                    {
                        bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol = bmsDevice[gunSn].bmsData.insulationVol;
                        bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag = bmsExternData[gunSn].unStateA.bit.bGb2015ProtocolFlag;
                        bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag = 1;//������Ե���
                        DLOG("gun%d:û�����²�ǹ�������˵ڶ��γ�磬ֱ��������Ե���",gunSn);                        
                    }
                    
                    if (bmsComFrame[gunSn].waitBhm27Time == 0)
                    {
                        bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag = 1;//������Ե���
                        bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag=0;
                        
                        DLOG("gun%d:����BHM��ʱ��ֱ��������Ե���",gunSn);  
                        DLOG("gun%d:2011�ɹ��� ",gunSn);
                    }
                }

                if (get_charge_state(gunSn) > CHARGER_STATE_INS_DISCHARGE)
                {
                    DLOG("gun%d:��ʼ����CRM����",gunSn);
                    bmsExternData[gunSn].unStateA.bit.bChargerIsFlag = 1;
                    start_bms_handshake_init(gunSn);
                }
                
                //���������ʱ
                if (bmsComInternalData[gunSn].userStartSecondCnt > 120)
                {
                    bmsDevice[gunSn].mainState = IDLE;
                    set_bms_stop_cause(BMS_STOP_CAUSE_CHARGE_STARTUP_TIMEOUT_CRM,1,gunSn);
                }
            }
            break;
            
         case SEND_CRM01_00:
            {
                if (bmsComFrame[gunSn].crm01.sendTime == 0)
                {
                    send_crm01(0,gunSn);
                    bmsComFrame[gunSn].crm01.sendTime = SENDTIMEOUT250_MS;
//                    DLOG("����CRM 00����",3);
                }
             
                //�յ�BRM����
                pCanFrame = get_canheadler_bms_can_frame(BMS_BRM_TYPE,0,gunSn);
                if (pCanFrame)
                {
                    brm_data_handle(pCanFrame,gunSn);
                    bmsComFrame[gunSn].crm01.sendTime = 0;
                    bmsComFrame[gunSn].waitBcp06Time = WAIT_FRAME_TIME_5000MS;
                    bmsDevice[gunSn].subState = SEND_CRM01_01;
                }
                
                if (bmsComFrame[gunSn].waitBrm02Time == 0)
                {
                    bmsDevice[gunSn].bmsData.cemType = BRM_FRAME_TIMEOUT;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                    
                    set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BRM_TIMEOUT,0,gunSn);
                    DLOG("gun%d:BRM���Ļظ���ʱ,�˳�SEND_CRM01_00״̬",gunSn);
                    
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                    bmsDevice[gunSn].subState = SEND_CEM1F;
                    bmsDevice[gunSn].mainState = CHARG_OVER;
                }
            }
            break;
         case SEND_CRM01_01:
            {
                 bms_type_judge(gunSn);

				pCanFrame = get_canheadler_bms_can_frame(BMS_BCP_TYPE,0,gunSn);//������֤����
				if (pCanFrame)
				{
					bmsDevice[gunSn].subState = RECV_BCP06;
					bmsDevice[gunSn].mainState = SETTING;		
					break;
				}
				 
                 if (bmsComFrame[gunSn].crm01.sendTime == 0)
                 {
                     send_crm01(0xaa,gunSn);
                     
                     bmsComFrame[gunSn].crm01.sendTime = SENDTIMEOUT250_MS;
                     bmsDevice[gunSn].subState = RECV_BCP06;
                     bmsDevice[gunSn].mainState = SETTING;
                     
                     DLOG("gun%d:�������ý׶�",gunSn);
                 }
                 
                 //��ʱû���յ�BCP����
                 if (bmsComFrame[gunSn].waitBcp06Time == 0)
                 {
                    bmsDevice[gunSn].bmsData.cemType = BCP_FRAME_TIMEOUT;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                    set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BCP_TIMEOUT,0,gunSn);
                     
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                    bmsDevice[gunSn].subState = SEND_CEM1F;
                    bmsDevice[gunSn].mainState = CHARG_OVER;
                 }
             }
             break;
         default:
            {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
            }
             break;
    }
}

static void bms_config_stage_new(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    //�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
    if (bmsExternData[gunSn].externStopCause)
    {
        bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
        
        bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
        
        if (bmsDevice[gunSn].subState != RECV_BST19)
        {
			bms_StopReason_Judge(bmsExternData[gunSn].externStopCause);
            send_cst1A(gunSn);

            bmsDevice[gunSn].mainState = IDLE;
        }
    }
    
    switch (bmsDevice[gunSn].subState)
    {       
        case RECV_BCP06:
            {
                pCanFrame = get_canheadler_bms_can_frame(BMS_BCP_TYPE,0,gunSn);
                if (pCanFrame)
                {
                     bcp_data_handle(pCanFrame,gunSn);
                     
                     bmsDevice[gunSn].frameData.broData.broReadyState = 0;
                     
                     bmsComFrame[gunSn].cts07.sendTime = 0;
                     bmsComFrame[gunSn].cml08.sendTime = 0;
                     bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                     bmsComFrame[gunSn].waitBro09OkTime = WAIT_FRAME_TIME_60000MS;
					 
					 //sCan3TaskData.unData.sGWSysCanData.sProcessFlag[0].bPF2D_SendFlag = 1;   //���弴�䱨�ķ��ͱ�ʶ
					 
                     bmsDevice[gunSn].subState = SEND_CTS07;
                }
                else
                {
                    bmsDevice[gunSn].mainState = SHAKE_HAND;
                    bmsDevice[gunSn].subState = SEND_CRM01_01;
                }
            }
            break;
         case SEND_CTS07:
            {
                if (bmsComFrame[gunSn].cts07.sendTime == 0)
                {
                    send_cts07(gunSn);
                    bmsComFrame[gunSn].cts07.sendTime = SENDTIMEOUT500_MS;
                }
                
                if (bmsComFrame[gunSn].cml08.sendTime == 0)
                {
                    send_cml08(bmsExternData[gunSn].chargerMaxOutputVol,bmsExternData[gunSn].chargerMinOutputVol,bmsExternData[gunSn].chargerMaxOutputCur,gunSn);
                    bmsComFrame[gunSn].cml08.sendTime = SENDTIMEOUT250_MS;
                }
                
                bmsDevice[gunSn].subState = RECV_BRO09;
            }
            break;
         case RECV_BRO09:
             pCanFrame = get_canheadler_bms_can_frame(BMS_BRO_TYPE,1,gunSn);
             if (pCanFrame)
             {
                bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                bro_data_handle(pCanFrame,gunSn);
             }
             
             if (bmsDevice[gunSn].frameData.broData.broReadyState)
             {
                 bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                 bmsComFrame[gunSn].cro0A.maxSendTime = SENDTIMEOUT60000_MS;
                 
                 bmsDevice[gunSn].subState = SEND_CRO0A;
                 DLOG("gun%d:bms���׼����",gunSn);
             }
             else if ((bmsComFrame[gunSn].waitBro09Time == 0) || (bmsComFrame[gunSn].waitBro09OkTime == 0))
             {
                 bmsDevice[gunSn].bmsData.cemType = BRO_FRAME_TIMEOUT;
                 send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                 set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BRO_TIMEOUT,0,gunSn);
                 bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                 bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                 bmsDevice[gunSn].subState = SEND_CEM1F;
                 bmsDevice[gunSn].mainState = CHARG_OVER;
             }
             else
             {
                if((bmsComFrame[gunSn].cts07.sendTime == 0)
                || (bmsComFrame[gunSn].cml08.sendTime == 0))
                {
                    bmsDevice[gunSn].subState = SEND_CTS07;
                }
             }
             
             break;
         case SEND_CRO0A:
            {
	            pCanFrame = get_canheadler_bms_can_frame(BMS_BRO_TYPE,1,gunSn);
                if (pCanFrame)
                {
                   bmsComFrame[gunSn].waitBro09Time = WAIT_FRAME_TIME_5000MS;
                   bro_data_handle(pCanFrame,gunSn);
                }

                if(!bmsDevice[gunSn].frameData.broData.broReadyState)
                {
        	        bms_charger_stop(gunSn);
        		
        	        set_bms_stop_cause(BMS_STOP_CAUSE_BRO_ABS,1,gunSn);
                    bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
                    send_cst1A(gunSn);                    
                    bmsDevice[gunSn].mainState = IDLE;
                    
                    DLOG("gun%d:BRO��AA��Ϊ00",gunSn);
                    break;
                }
                
                if (bmsComFrame[gunSn].waitBro09Time == 0)//|| (bmsComFrame.waitBro09OkTime == 0))
                {
                    bmsDevice[gunSn].bmsData.cemType = BRO_FRAME_TIMEOUT;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                    set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BRO_TIMEOUT,0,gunSn);
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    bmsComFrame[gunSn].cem1F.maxSendTime = SENDTIMEOUT5000_MS;
                    bmsDevice[gunSn].subState = SEND_CEM1F;
                    bmsDevice[gunSn].mainState = CHARG_OVER;
                    break;
                }
                
                if((get_DcCont_State(gunSn)==BOOL_TRUE)||
                (get_charge_state(gunSn)==CHARGER_STATE_CHARGING)||
                (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag==0))//��֤��,�����޸�������������
                {
                    send_cro0A(0xaa,gunSn);
                    
                    bmsDevice[gunSn].unBmsStateA.bit.bComChargingStageFlag = 1;
                    bmsDevice[gunSn].bmsData.chargerTimeSecondCnt = 0;//����ʱ����
                    DLOG("gun%d:������ɳ��׼����BMS�ۼƳ��ʱ������",gunSn);
                    
                    //��֤��
                    bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_1000MS;
                    //bmsComFrame[gunSn].waitBcl10Time = WAIT_FRAME_TIME_5000MS;
                    bmsComFrame[gunSn].waitBcs11Time = WAIT_FRAME_TIME_5000MS;
                    bmsComFrame[gunSn].cro0A.sendTime = SENDTIMEOUT250_MS;
                    bmsDevice[gunSn].subState = RECV_BCS11;
                }
                else if (bmsComFrame[gunSn].cro0A.sendTime == 0)
                {
                    send_cro0A(0x00,gunSn);
                    bmsComFrame[gunSn].cro0A.sendTime = SENDTIMEOUT250_MS;
                }
                
                //��ʱ
                if (bmsComFrame[gunSn].cro0A.maxSendTime == 0)
                {
                    bmsDevice[gunSn].mainState = WAIT_START;
                }
            }
            break;
         case RECV_BCS11:
            recv_bcs_state_handle_new(gunSn);
            break;
         default:
             {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
             }
             break;
    }
}

static void bms_charging_stage_new(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BST_TYPE,0,gunSn);
    if (pCanFrame)
    {
    	if(bmsDevice[gunSn].subState != RECV_BST19)
    	{
	        bms_charger_stop(gunSn);
	        bst_data_handle(pCanFrame,gunSn);
            
            bms_internal_charger_stop_cst_fill(pCanFrame,gunSn);
	        bmsComInternalData[gunSn].cstIndex =0;
	        bmsComInternalData[gunSn].cstValue = (1<<6);	
            bmsComInternalData[gunSn].cstFaultFlag = 0;
            send_cst1A(gunSn);
	        bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
	        bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
            bmsComFrame[gunSn].waitBsd1DTime = WAIT_FRAME_TIME_10000MS;                        
	        bmsDevice[gunSn].subState = RECV_BST19;
    	 }		
    }
    
    //�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
    if (bmsExternData[gunSn].externStopCause)
    {
        bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
        
        bms_extern_charger_stop_cst_fill(bmsExternData[gunSn].externStopCause,gunSn);
        
        if (bmsDevice[gunSn].subState != RECV_BST19)
        {
            send_cst1A(gunSn);
		    bms_StopReason_Judge(bmsExternData[gunSn].externStopCause);
            
            bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
            bmsComFrame[gunSn].waitBsd1DTime = WAIT_FRAME_TIME_10000MS;            
            bmsDevice[gunSn].subState = RECV_BST19;
        }
    }
    
    pCanFrame = get_canheadler_bms_can_frame(BMS_BEM_TYPE,1,gunSn);
    if (pCanFrame)
    {
        uint32_t errorData = get_data_form_string(DATA_TYPE_INT,0,pCanFrame->len,pCanFrame->data);
        if(0 != errorData)
        {
            bms_charger_stop(gunSn);
            set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BEM_FRAME,0,gunSn);
            bmsDevice[gunSn].mainState = WAIT_START;
        }
        else
        {
            //DLOG("û�д��󷢸�ë��");
        }
    }

    switch (bmsDevice[gunSn].subState)
    {
        case RECV_BCL10:
            recv_bcl_state_handle_new(gunSn);
            break;
        case RECV_BST19:
        {
            pCanFrame = get_canheadler_bms_can_frame(BMS_BST_TYPE,1,gunSn);
            if (pCanFrame)
            {
                bst_data_handle(pCanFrame,gunSn);
                bms_charger_stop(gunSn);
       //         bmsComFrame[gunSn].waitBsd1DTime = WAIT_FRAME_TIME_10000MS;/*add by km 20190319*/
                bmsDevice[gunSn].subState = RECV_BSD1C;
                bmsDevice[gunSn].mainState = CHARG_OVER;
                DLOG("gun%d:����������׶�",gunSn);                  
            }

            if (bmsComFrame[gunSn].cst1A.sendTime == 0)
            {
                send_cst1A(gunSn);
                bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            }
            
            if (bmsComFrame[gunSn].waitBst19Time == 0)
            {
                bmsDevice[gunSn].bmsData.cemType = BST_FRAME_TIMEOUT;
                send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                set_bms_stop_cause(BMS_STOP_CAUSE_RECV_BST_TIMEOUT,0,gunSn);
				bmsComFrame[gunSn].cem1F.maxSendTime = WAIT_FRAME_TIME_5000MS;	
				bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;	
                bmsDevice[gunSn].subState = SEND_CEM1F_END;               
                bmsDevice[gunSn].mainState = CHARG_OVER;
            }
            
            break;
        }
        default:
            {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
            }
            break;
    }
    
    if (unBmsTaskClockFlag.bit.b1s)
    {
        if (!bmsDevice[gunSn].unBmsStateA.bit.bPowerState)
        {
              bmsDevice[gunSn].bmsData.bsmStopTimeCnt++;
        }
        else
        {
            bmsDevice[gunSn].bmsData.bsmStopTimeCnt = 0;
        }
        
        if (bmsDevice[gunSn].bmsData.bsmStopTimeCnt > 600)
        {
            if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
            {
                set_bms_stop_cause(BMS_STOP_CAUSE_BSM_CHARGER_ALLOW_TIMEOUT,1,gunSn);
                send_cst1A(gunSn);
                bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
                bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
    	        bmsComFrame[gunSn].waitBst19Time = WAIT_FRAME_TIME_5000MS;
                bmsComFrame[gunSn].waitBsd1DTime = WAIT_FRAME_TIME_10000MS;                  
                bmsDevice[gunSn].subState = RECV_BST19;
            }
        }
    }
    
    if (unBmsTaskClockFlag.bit.b5s)
    {
        switch(bmsDevice[gunSn].bmsType)
        {
            case BMS_GB_2015_TYPE:
            case BMS_GB_TYPE:
            {
                if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag)
                {
                    DLOG("gun%d:GB2015 ���ڳ����",gunSn);
                }
                else
                {
                    DLOG("gun%d:GB2011 ���ڳ����",gunSn);
                }
            }
            break;
        }
       // send_debug_info60();
    }
}

void bms_charger_over_stage_new(uint8_t gunSn)
{
    const PCanFrame *pCanFrame;
    
    bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
    sRecvFlg_Kaipu[gunSn].u8BsmFlg=0;	
    bms_charger_over_frame_handle(gunSn);

    switch (bmsDevice[gunSn].subState)
    {
        case RECV_BSD1C:
            pCanFrame = get_canheadler_bms_can_frame(BMS_BSD_TYPE,1,gunSn);
            if ((pCanFrame))//||(sChSysMegg.i8EmergencyOff)) 
            // 34756,K3K4�ڱ��ķ�����ɺ��ٶϿ������ùܼ�ͣ����CC1
            {
                bsd_data_handle(pCanFrame,gunSn);
                send_csd1D(gunSn);
                bmsComFrame[gunSn].csd1D.maxSendTime = WAIT_FRAME_TIME_5000MS;
                bmsComFrame[gunSn].csd1D.sendTime = SENDTIMEOUT250_MS;                
                bmsDevice[gunSn].subState = SEND_CSD1D;	
                bmsDevice[gunSn].bmsData.csd1DSendCnt = 0;
                break;
            }
                
            if (bmsComFrame[gunSn].waitBsd1DTime == 0)
            {
                bmsDevice[gunSn].bmsData.cemType = BSD_FRAME_TIMEOUT;
                send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                bmsComFrame[gunSn].cem1F.maxSendTime=SENDTIMEOUT5000_MS;
                bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS; 
                bmsDevice[gunSn].subState = SEND_CEM1F_END;	
                break;
            }            
            
            if (bmsComFrame[gunSn].cst1A.sendTime == 0)
            {
                send_cst1A(gunSn);
                bmsComFrame[gunSn].cst1A.sendTime = SENDTIMEOUT10_MS;
            }   
            break;
            
        case SEND_CSD1D:
            if (bmsComFrame[gunSn].csd1D.sendTime == 0)
            {
                send_csd1D(gunSn);
                bmsComFrame[gunSn].csd1D.sendTime = SENDTIMEOUT250_MS;
                bmsDevice[gunSn].bmsData.csd1DSendCnt++;           
            }
            
            if (((bmsComFrame[gunSn].csd1D.maxSendTime == 0)||(get_DcCont_State(gunSn) == BOOL_FALSE))
                &&(bmsDevice[gunSn].bmsData.csd1DSendCnt >= 1)) //��ֹ���Ͳ���CSD ,������̫�࣬����bBmsCutFlag��1S
            {
                bmsDevice[gunSn].mainState = WAIT_START;
                bmsDevice[gunSn].bmsData.csd1DSendCnt = 0;
            }
            break;
        case SEND_CEM1F:
            {
                bmsDevice[gunSn].frameData.broData.broReadyState = 0;
            
                if ((bmsComFrame[gunSn].cem1F.maxSendTime == 0)||(bmsDevice[gunSn].unBmsStateA.bit.bBmsComFault != 0))
                {
                    bmsDevice[gunSn].mainState = WAIT_START;
                    break;
                }
                
                if (bmsComFrame[gunSn].cem1F.sendTime == 0)
                {
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                }
            }
            break;
            
        case SEND_CEM1F_END:
            {
                if (bmsComFrame[gunSn].cem1F.maxSendTime == 0)
                {
                    bmsDevice[gunSn].bmsData.chargerCnt = 0;
                    bmsDevice[gunSn].mainState = IDLE;
                    break;
                }
                
                if (bmsComFrame[gunSn].cem1F.sendTime == 0)
                {
                    bmsComFrame[gunSn].cem1F.sendTime = SENDTIMEOUT250_MS;
                    send_cem1f(bmsDevice[gunSn].bmsData.cemType,gunSn);
                }
            }
            break;            
        default:
            {
                 bms_charger_stop(gunSn);
                 bmsDevice[gunSn].mainState = WAIT_START;
            }
            break;
    }
    
    
}

void bms_charger_scheduler_new(uint8_t gunSn)
{
    switch (bmsDevice[gunSn].mainState)
    {
        case IDLE:
            bmsDevice[gunSn].unBmsStateB.bit.bBmsComState = 0;
            bmsDevice[gunSn].unBmsStateA.bit.bPowerState = 0;
            bmsComInternalData[gunSn].cstFaultFlag = 0;
		    sRecvFlg_Kaipu[gunSn].u8BsmFlg=0;	
            bmsDevice[gunSn].bmsData.csd1DSendCnt = 0;
            break;
        case WAIT_START://�ȴ����ָ��
            bms_wait_stage(gunSn);
            break;
        case SHAKE_HAND://���ֽ׶�
            bms_handshake_stage_new(gunSn);
            break;
        case SETTING://���ý׶�
            bms_config_stage_new(gunSn);
            break;
        case CHARGING://���׶�
            bms_charging_stage_new(gunSn);
            break;
        case CHARG_OVER://�������׶�
            bms_charger_over_stage_new(gunSn);
            break;
        case ERROR_STAGE://����׶�
            
            break;
        default:
            break;
    }
    
}

void bms_charger_scheduler(void)
{
    bms_task_handle();
//    hardware_check_msg_deal();//���¹�װ����һֱ����CCM����
    
    if(sSysConfig.sInt.bmsType == BMS_GB_2015_TYPE)
    {
    
	    for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	    {
            bms_charger_scheduler_new(gunSn);
	    }
    }
    else
    {
    	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	    {
			bms_charger_scheduler_old(gunSn);
	    }
    }
}



