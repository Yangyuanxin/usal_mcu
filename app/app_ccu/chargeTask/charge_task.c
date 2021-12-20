/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   charge_task
����:   km
˵��:   �������.
***************************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "charge_task.h"
#include "io_megg.h"
#include "sys_config.h"
#include "bms_device.h"
#include "alarm_task.h"
#include "sys_type.h"
#include "time_cur.h"
#include "log.h"

structSysClockCnt sChargerTaskClockCnt;
unionSysClockFlag unChargerTaskClockFlag;
static sChargerDevice chargerDevice[2];

sChargerMegg chargerMegg[2];

const static structStateTip sStateTip[] = 
{    
    {CHARGER_STATE_IDLE,"����"},
    {CHARGER_STATE_GUNLOCK,"ǹ���պ�"},
    {CHARGER_STATE_MOD_SWITCH,"ģ�������"},
    {CHARGER_STATE_AXLOCK,"��Դ�պ�"},
    {CHARGER_STATE_GB_CONFIG,"������Ե"},
    {CHARGER_STATE_INSDET,"��Ե�����"},
    {CHARGER_STATE_INS_DISCHARGE,"��Եй�����"},
    {CHARGER_STATE_WAIT_BROAA,"�ȴ���������"},
    {CHARGER_STATE_PRE_CHARGE,"Ԥ���"},
    {CHARGER_STATE_CHARGING,"�����"},
    {CHARGER_STATE_STARTFAULT,"����ʧ��"},
    {CHARGER_STATE_STOP,"ֹͣ���"},
    {CHARGER_STATE_CHG_DISCHARGE,"ֹͣй��"},
    {CHARGER_STATE_AXOPEN,"��Դ�Ͽ�"},
    {CHARGER_STATE_GUNOPEN,"ǹ���Ͽ�"},
    {CHARGER_STATE_END,"��������ˢ��"},
    {CHARGER_STATE_END_SAVE,"�ȴ���¼����"},
};

/**
  * @brief  ���״̬����ת
  */
static void set_charge_state(ENUM_SYS_CHARGER_STATE state,uint8_t gunSn)
{
    chargerMegg[gunSn].record_data.state_last = chargerDevice[gunSn].chargerState;
    chargerDevice[gunSn].chargerState = state;
}

/**
  * @brief  ���״̬����ȡ
  */
ENUM_SYS_CHARGER_STATE get_charge_state(uint8_t gunSn)
{
    return chargerDevice[gunSn].chargerState;
}

char *get_charge_state_tip(ENUM_SYS_CHARGER_STATE state)
{
    int32_t mid = 0;
    int32_t low = 0;
    int32_t high = (sizeof(sStateTip)/sizeof(StructStopCodeMap))-1;
    
    while(low <= high)
    {
        mid=(high+low)/2;

        if(sStateTip[mid].state > state)
            high = mid - 1;
        else  if(sStateTip[mid].state < state)
            low = mid + 1;
        else 
            return sStateTip[mid].tip;
    }
    DLOG("δ�ҵ�statetip[%d]",state);
    return  NULL;
}

/**
  * @brief  ��Ե״̬����ת
  */
static void set_insdet_state(ENUM_SYS_INSDET_STATE state,uint8_t gunSn)
{
    chargerDevice[gunSn].sInsdet.state = state;
}

/**
  * @brief  ��ȡ��Ե״̬����ת
  */
ENUM_SYS_INSDET_STATE get_insdet_state(uint8_t gunSn)
{
    return chargerDevice[gunSn].sInsdet.state;
}

/*
  *@brief ͣ��ԭ����Ϣ�������
  */
void charger_record_stopmegg(uint8_t gunSn)
{
    int16_t *stopmegg1 = &chargerMegg[gunSn].record.stopmegg1;
    int16_t *stopmegg2 = &chargerMegg[gunSn].record.stopmegg2;
    int16_t *stopmegg3 = &chargerMegg[gunSn].record.stopmegg3;
    int16_t *stopmegg4 = &chargerMegg[gunSn].record.stopmegg4;
    int16_t *stopmegg5 = &chargerMegg[gunSn].record.stopmegg5;
    int16_t *stopmegg6 = &chargerMegg[gunSn].record.stopmegg6;
    int16_t *stopmegg7 = &chargerMegg[gunSn].record.stopmegg7;
    int16_t *stopmegg8 = &chargerMegg[gunSn].record.stopmegg8;
    *stopmegg1 = 0;
    *stopmegg2 = 0;
    *stopmegg3 = 0;
    *stopmegg4 = 0;
    *stopmegg5 = 0;
    *stopmegg6 = 0;
    *stopmegg7 = 0;
    *stopmegg8 = 0;
    
    switch(chargerMegg[gunSn].record.chargerStopReason)
    {
        case ALARM_DCCONT_GUN1:
		case ALARM_DCCONT_GUN2:
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = sSysMegg[gunSn].unOut.bit.bDcCont_Out;
            *stopmegg3 = get_DcCont_State(gunSn);
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg8 = chargerMegg[gunSn].record_data.t_max; 
        }
        break;

        case ALARM_OUTLETTEMPUP:
        {
            *stopmegg1 = sSysMegg[gunSn].t_GunP;
            *stopmegg2 = sSysMegg[gunSn].t_GunN;
            *stopmegg3 = sSysConfig.sInt.uGunTempProtect;
            *stopmegg4 = chargerMegg[gunSn].i_demand;   
            *stopmegg5 = sSysMegg[gunSn].i_Gun;
            *stopmegg6 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg7 = chargerMegg[gunSn].record_data.i_gun[1];
            *stopmegg8 = chargerMegg[gunSn].record_data.i_bcs[1];
        }
        break;

        case ALARM_GUNTEMPUP_GUN1:
		case ALARM_GUNTEMPUP_GUN2:
        case BMS_STOP_CAUSE_RECV_BST_1_02://BST����������  
        {
            *stopmegg1 = sSysMegg[gunSn].t_GunP;
            *stopmegg2 = sSysMegg[gunSn].t_GunN;
            *stopmegg3 = sSysConfig.sInt.uGunTempProtect;
            *stopmegg4 = chargerMegg[gunSn].i_demand;   
            *stopmegg5 = sSysMegg[gunSn].i_Gun;
            *stopmegg6 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg7 = chargerMegg[gunSn].record_data.i_gun[1];
            *stopmegg8 = chargerMegg[gunSn].record_data.i_bcs[1];
        }
        break;

        case ALARM_INSDETFAIL_GUN1:               //��Ե����
		case ALARM_INSDETFAIL_GUN2: 			  //��Ե����
        case STOP_INSDET_VBEFORE:                //��Ե���׶Σ�ǹ�ڴ�ѹ
        case STOP_INSDET_TO1:                    //��Ե��ʱ��ģ��δ����
        case STOP_INSDET_TO2:                    //��Ե��ʱ��ģ�鿪��
        case STOP_BMS_VINSDET:                   //BMS��Ե��ѹ��
        case STOP_BSM_INSDET:                    //BMS �������ؾ�Ե״̬ 
        case STOP_SOFTFAIL_MOD_START:            //����ʧ��,ģ�鿪��
        case STOP_SOFTFAIL_MOD_UNSTART:          //����ʧ��,ģ��δ����
        case STOP_SOFTFAIL_TIMEOUT:              //����ʧ��,������ʱ
        case BMS_STOP_CAUSE_RECV_BST_1_00://BST��Ե����      
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.v_insdet;
            *stopmegg2 = sSysMegg[gunSn].v_Gun;
            *stopmegg3 = chargerMegg[gunSn].record_data.v_gun[1];            
            *stopmegg4 = sSysMegg[gunSn].v_Mod;
            *stopmegg5 = chargerMegg[gunSn].record_data.v_mod[1];
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg8 = chargerMegg[gunSn].record_data.t_max; 
        }
        break;  
    
        case ALARM_VDCINV_GUN1:       //gun1��ط��ӹ���
		case ALARM_VDCINV_GUN2: 	  //gun2��ط��ӹ���
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].record_data.v_insdet;
            *stopmegg3 = sSysMegg[gunSn].v_Gun;
            *stopmegg4 = chargerMegg[gunSn].record_data.v_gun[1];
        }
        break;  
        
        case ALARM_VGRIDUP:       //�����ѹ����
        case ALARM_VGRIDLOW:      //����Ƿѹ����
        {
            *stopmegg1 = sSysMegg[gunSn].unOut.bit.bACCont_Out;
            *stopmegg2 = sSysMegg[gunSn].vab_10x;
            *stopmegg3 = sSysMegg[gunSn].vac_10x;
            *stopmegg4 = sSysMegg[gunSn].vbc_10x; 
        }
        break;  

        case ALARM_VDCUP_GUN1:       //gun1ֱ����ѹ����
		case ALARM_VDCUP_GUN2:		 //gun2ֱ����ѹ����
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].v_demand;
            *stopmegg3 = sSysMegg[gunSn].v_Gun;
            *stopmegg4 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureVoltage;
            *stopmegg5 = chargerMegg[gunSn].i_demand;
            *stopmegg6 = sSysMegg[gunSn].i_Gun;
            *stopmegg7 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg8 = sSysConfig.sInt.uDcOverVolt;    
        }
        break;  
        
        case ALARM_VDCLOW_GUN1:       //gun1ֱ��Ƿѹ����
		case ALARM_VDCLOW_GUN2: 	  //gun2ֱ��Ƿѹ����
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].v_demand;
            *stopmegg3 = sSysMegg[gunSn].v_Gun;
            *stopmegg4 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureVoltage;
            *stopmegg5 = chargerMegg[gunSn].i_demand;
            *stopmegg6 = sSysMegg[gunSn].i_Gun;
            *stopmegg7 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg8 = sSysConfig.sInt.uDcUnderVolt;    
        }
        break; 
        
        case ALARM_IDCUP_GUN1:       //gun1ֱ����������
		case ALARM_IDCUP_GUN2:		 //gun2ֱ����������
        case STOP_BSM_OC:                        //BMS �������س�������
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].i_demand;
            *stopmegg3 = sSysMegg[gunSn].i_Gun;
            *stopmegg4 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg5 = chargerMegg[gunSn].record_data.i_demand[1];
            *stopmegg6 = chargerMegg[gunSn].record_data.i_gun[1];
            *stopmegg7 = chargerMegg[gunSn].record_data.i_bcs[1];
            *stopmegg8 = sSysConfig.sInt.uDcoverCurr;    
        }
        break; 

        case ALARM_ACCONT:       //����Ӵ�������
        case ALARM_MOD:           //ģ�����
        case ALARM_MODCOMFAULT:   //ģ��ͨ�Ź���
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = sSysMegg[gunSn].unOut.bit.bACCont_Out;
        }
        break;

        case STOP_BCP_BATVOLAGE:       //BCP��ص�ѹ�쳣
        case BMS_STOP_CAUSE_BRO_ABS:                     //BRO׼�������쳣
        {
            *stopmegg1 = bmsDevice[gunSn].frameData.bcpData.bcpCurBatteryVoltage;
            *stopmegg2 = sSysMegg[gunSn].v_Gun;
            *stopmegg3 = chargerMegg[gunSn].record.bcp_vabs2;
        }
        break; 

        case STOP_SOC_PROTECTION:       //SOC�ﵽ�û�����ֵ
        case STOP_BSM_SOC:              //BMS �����������غɵ�״̬
        case STOP_BSM_SOCLOW:                    //BMS ������������SOC����
        case BMS_STOP_CAUSE_RECV_BST_0_00://BST��SOCĿ��ֵ   
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = bmsDevice[gunSn].frameData.bcsData.bcsCurSoc;
            *stopmegg3 = sSysConfig.sInt.uMaxAllowChSOC;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg8 = chargerMegg[gunSn].record_data.t_max; 
        }
        break; 
        
        case STOP_BMS_SINGLE_VUP:       //BMS�����ѹ����
        case STOP_BSM_VSINGLE:                   //BMS ���嶯�����ص�ѹ
        case STOP_BSM_BATLOW:                    //BMS ���嶯�����ص�ѹ����
        case BMS_STOP_CAUSE_RECV_BST_0_04://BST�����ѹ��   
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = bmsDevice[gunSn].frameData.bcsData.bcsSingleInfo.bit.bMaxSingleVol;
            *stopmegg3 = bmsDevice[gunSn].frameData.bcpData.bcpSingleMaxVol;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg8 = chargerMegg[gunSn].record_data.t_max; 
        }
        break; 
        
        case STOP_BMS_TEMP_MAX:       //BMS�¶ȹ���
        case STOP_BSM_TEMPUP:         //BMS���������¶ȹ���
        case BMS_STOP_CAUSE_RECV_BST_1_04://BSTԪ������   
        case BMS_STOP_CAUSE_RECV_BST_2_00://BST��������   
        {
            *stopmegg1 = bmsDevice[gunSn].frameData.bsmData.bsmMaxTemperature;
            *stopmegg2 = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerTemperature;
            *stopmegg3 = chargerMegg[gunSn].i_demand;
            *stopmegg4 = sSysMegg[gunSn].i_Gun;
            *stopmegg5 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_demand[1];
            *stopmegg7 = chargerMegg[gunSn].record_data.i_gun[1];
            *stopmegg8 = chargerMegg[gunSn].record_data.i_bcs[1];
        }
        break; 

        case STOP_VOLTAGE_BMSMAX:       //����ѹ����BMS�������ֵ
        case STOP_VOLTAGE_BMSDEMAND:    //����ѹ����BMS����ֵ
        case STOP_BAT_DEMAND_VOLTAGE:   //���������ѹ��������
        case BMS_STOP_CAUSE_RECV_BST_3_02://BST��ѹ�쳣      
        case BMS_STOP_CAUSE_RECV_BST_0_02://BST�ĵ�ѹ�趨ֵ    
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].v_demand;
            *stopmegg3 = sSysMegg[gunSn].v_Gun;
            *stopmegg4 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureVoltage;
            *stopmegg5 = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerVoltage;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg8 = chargerMegg[gunSn].record_data.t_max; 
        }
        break; 
        
        case STOP_BAT_CIRCUIT:    //�����ڲ���·�쳣��ֹ
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].v_demand;
            *stopmegg3 = sSysMegg[gunSn].v_Gun;
            *stopmegg4 = sSysMegg[gunSn].i_Gun;
            *stopmegg5 = chargerMegg[gunSn].record_data.v_gun[1];
            *stopmegg6 = chargerMegg[gunSn].record_data.i_gun[1];
            //*stopmegg7 = dlt645Megg[gunSn].meter.aPhaseCur_10x;
            *stopmegg8 = bmsDevice[gunSn].frameData.bcsData.bcsCurSoc;
        }
        break; 

        case STOP_BAT_DEMAND_CURRENT:   //�������������������
        case BMS_STOP_CAUSE_RECV_BST_3_00://BST��������      
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.state_last;
            *stopmegg2 = chargerMegg[gunSn].i_demand;
            *stopmegg3 = sSysMegg[gunSn].i_Gun;
            *stopmegg4 = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
            *stopmegg5 = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerCurrent;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg8 = chargerMegg[gunSn].record_data.t_max; 
        }
        break; 

        case ALARM_BMS_COM_GUN1:          //gun1BMSͨ�Ź���,��׼������
		case ALARM_BMS_COM_GUN2:		  //gun2BMSͨ�Ź���,��׼������
        case BMS_STOP_CAUSE_CHARGE_STARTUP_TIMEOUT_CRM:  //����ʱ�䳬ʱ
        case BMS_STOP_CAUSE_RECV_BRM_TIMEOUT:            //�յ�BRM���ĳ�ʱ
        case BMS_STOP_CAUSE_RECV_BCP_TIMEOUT:            //�յ�BCP���ĳ�ʱ
        case BMS_STOP_CAUSE_RECV_BRO_TIMEOUT:            //�յ�BR0���ĳ�ʱ
        case BMS_STOP_CAUSE_RECV_BCS_TIMEOUT:            //�յ�BCS���ĳ�ʱ
        case BMS_STOP_CAUSE_RECV_BCL_TIMEOUT:            //����BCL���ĳ�ʱ
        case BMS_STOP_CAUSE_RECV_BSM_TIMEOUT:            //����BSM���ĳ�ʱ
        case BMS_STOP_CAUSE_RECV_BEM_FRAME:              //�յ�BEM����ֹͣ
        case BMS_STOP_CAUSE_RECV_BST_TIMEOUT:            //�յ�BST���ĳ�ʱ
        {
            //*stopmegg1 = sSysMegg[gunSn].v_BMS_100x;
            *stopmegg2 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg3 = chargerMegg[gunSn].v_demand;
            *stopmegg4 = chargerMegg[gunSn].i_demand;   
            *stopmegg5 = sSysMegg[gunSn].v_Gun;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.t_max; 
            *stopmegg8 = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerTemperature;
        }
        break;

        case ALARM_EPO:
        case ALARM_BREAKER:
        case ALARM_FUSE:
        case ALARM_GunLock_GUN1:
		case ALARM_GunLock_GUN2:
        case ALARM_FAN:
        case ALARM_SPD:
        case ALARM_CC1_GUN1:
		case ALARM_CC1_GUN2:
        case ALARM_DOOR:
        case ALARM_DISCHARGE_GUN1:
		case ALARM_DISCHARGE_GUN2:
        case ALARM_TCUCOM:
        case ALARM_METER_COM:
		case ALARM_SHORT_GUN1:
        case ALARM_SHORT_GUN2:
        case ALARM_AX:                              //��Դ����---���籣��
        case STOP_UNDEFINE:                         //δ����
        case STOP_BY_CARD:                          //ˢ��ֹͣ
        case STOP_BY_USER:                          //�û�ֹͣ
        case STOP_BY_ENERGY:                        //�����ﵽ�趨
        case STOP_BY_TIME:                          //ʱ��ﵽ�趨
        case STOP_BY_MONEY:                         //���ﵽ�趨
        case STOP_BY_MONEY_NOTENOUGH:               //����
        case STOP_BY_BACKGROUP:                     //��ֹ̨ͣ
        case STOP_NET_DISCONNECT:                   //��̨�Ͽ�����
        case STOP_VIN_ERROR:                        //VIN�벻ƥ��
        case STOP_BY_ADMIN:                         //����Ա���Ľ�����ֹ
        case STOP_BMS_STAUE:                        //BMS״̬�쳣-----
        case STOP_BMS_BCSVOL:                       //BCS�ϴ���ѹ�쳣-----
        case STOP_BSM_CC:                           //BMS ���������������״̬
        case STOP_TCU_UPDATE:                       //TCU����
        case STOP_TCU_CTRL_ABS:                     //TCU ���ư�״̬�쳣
        case STOP_TCU_UNKNOWN:                      //TCU δ֪�Ľ���ԭ��
        case BMS_STOP_CAUSE_BSM_CHARGER_ALLOW_TIMEOUT:   //BSM�������쳣
        case BMS_STOP_CAUSE_RECV_BST_0_06:          //BST_00_6δ���� 
        case BMS_STOP_CAUSE_RECV_BST_1_06:          //BST����������    
        case BMS_STOP_CAUSE_RECV_BST_2_02:          //BST��������   
        case BMS_STOP_CAUSE_RECV_BST_2_04:          //BST_02_4δ����    
        case BMS_STOP_CAUSE_RECV_BST_2_06:          //BST_02_6δ����    
        case BMS_STOP_CAUSE_RECV_BST_OHTER:         //BSTδ֪ͣ��
        default:
        {
            *stopmegg1 = chargerMegg[gunSn].record_data.v_max_bat;
            *stopmegg2 = chargerMegg[gunSn].v_demand;
            *stopmegg3 = chargerMegg[gunSn].i_demand;   
            *stopmegg4 = sSysMegg[gunSn].v_Gun;
            *stopmegg5 = sSysMegg[gunSn].i_Gun;
            *stopmegg6 = chargerMegg[gunSn].record_data.i_max;    
            *stopmegg7 = chargerMegg[gunSn].record_data.t_max; 
            *stopmegg8 = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerTemperature;
        }
        break;
    }
    DLOG("gun%d:STOP_MEGG 1.%d 2.%d 3.%d 4.%d 5.%d 6.%d 7.%d 8.%d",gunSn,*stopmegg1,*stopmegg2, \
        *stopmegg3,*stopmegg4,*stopmegg5,*stopmegg6,*stopmegg7,*stopmegg8);
}


/**
  * @brief  �������ӿ�.
  */
void set_charger_stopcause(uint32_t stopCause,uint8_t gunSn)
{
    if (stopCause == 0)
    {
        chargerMegg[gunSn].chargerStopCause = 0;
        //DLOG("���ֹͣԭ��");
        return;
    }
    
    if ((chargerMegg[gunSn].chargerStopCause == 0)&&(chargerMegg[gunSn].unStep.bit.bworking != 0))
    {
        const char *buffer = 0;
        
        chargerMegg[gunSn].chargerStopCause = stopCause;
        chargerMegg[gunSn].record.chargerStopReason = stopCause;

        buffer = get_alarm_stopcause_tips(stopCause);

        DLOG("gun%d:ֹͣԭ��:%d(%s) \n", gunSn,stopCause, buffer);
        //EVENT_ADD(RECORD_COM_INDEX_EVENT_CHARG_END, "�������[%s]",buffer);
        charger_record_stopmegg(gunSn);
    }
 
}

/**
  * @brief  ������ԭ���ȡ
  */
uint32_t get_charger_stopcause(uint8_t gunSn)
{
    return chargerMegg[gunSn].chargerStopCause;
}

/**
  * @brief  ������ʼ��
  */
static void chargering_start_init(ENUM_CHARGER_START_TYPE type,uint8_t gunSn)
{

/*   
    sSysRamIntf.unSysFlag.bit.bWriteErromChargerRecord = 1;
    */
    memset(&chargerMegg[gunSn].record,0,sizeof(chargerMegg[gunSn].record));
    chargerMegg[gunSn].record.startChargeMode = type;
    chargerMegg[gunSn].record.workmode = sSysConfig.sTempInt.WorkMode;
    chargerMegg[gunSn].record.chargerStartTime = get_curtimer_val();
    chargerMegg[gunSn].record.chargerEndTime = chargerMegg[gunSn].record.chargerStartTime;
    //chargerMegg[gunSn].record.chargerStartPower = dlt645Megg.meter.energyPower_1000x;
    //chargerMegg[gunSn].record.chargerEndPower = chargerMegg.record.chargerStartPower;
    chargerMegg[gunSn].record.chargerStopReason = ALARM_AX;
    
    set_charger_stopcause(0,gunSn);

    set_charge_state(CHARGER_STATE_GUNLOCK,gunSn);

}

/**
  * @brief  ���������жϣ�
  * @retval 0 ���Կ��� �����������Կ���
  */
static int charger_start_condition_judge(uint8_t gunSn)
{
    ENUM_CHARGER_START_RESULT startFailReason = CHARGE_START_LIMIT_NULL;

    char *tip = NULL;
    if (!chargerDevice[gunSn].cc1State)
    {
        tip = "���ǹδ����,�����������";
        startFailReason = CHARGE_START_LIMIT_DISCONNECT;
    }
    else if (chargerDevice[gunSn].chargerState != CHARGER_STATE_IDLE)
    {
        tip = "���ڳ����,�����������";
        startFailReason = CHARGE_START_LIMIT_BUSY;
    }
    else if(chargerMegg[gunSn].unStep.bit.fault)
    {
        tip = "ϵͳ����,�����������";
        startFailReason = CHARGE_START_LIMIT_FAULT;
    }
    DLOG("gun%d:�������[%d][%s]",gunSn,startFailReason,tip);   
    return startFailReason;
}

/**
  * @brief  �����ӿ�
  * @retval BOOL_TRUE ��������
  */
BOOL start_charger_interface(ENUM_CHARGER_START_TYPE type,uint8_t gunSn)
{
    if(charger_start_condition_judge(gunSn))
    {
        return BOOL_FALSE;
    }    

    chargering_start_init(type,gunSn);
    //EVENT_ADD(RECORD_COM_INDEX_EVENT_CHARG_START, "��ʼ���[%d]",sSysConfig.sTempInt.WorkMode);
    return BOOL_TRUE;
}

/**
  * @brief  �����Ӵ������ƣ���ǹ���ϣ������ɺ�Ͽ�����ǹ�����ж�
  */
static void accont_task_handle(void)
{
    static uint8_t acConnect[GUN_SN_MAX] = {0};
	static uint8_t backck_connect_state[GUN_SN_MAX] = {0};
	static uint8_t back_start_state[GUN_SN_MAX] = {0};
	static int32_t accontCount = 0;

	
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    if(backck_connect_state[gunSn] != sSysMegg[gunSn].CC1)
	    {
	        backck_connect_state[gunSn] = sSysMegg[gunSn].CC1;  
	        acConnect[gunSn] = sSysMegg[gunSn].CC1;
	        DLOG("gun%d:cc1�仯[%d]",gunSn,acConnect[gunSn]);
	    }

	    if(back_start_state[gunSn] != chargerMegg[gunSn].flag.bit.bStartCh)
	    {
	        back_start_state[gunSn] = chargerMegg[gunSn].flag.bit.bStartCh;
	        DLOG("�����仯[gunSn%d:%d]",gunSn,back_start_state[gunSn]);
	        acConnect[gunSn] = chargerMegg[gunSn].flag.bit.bStartCh;
	    }

	    if(get_alarm_fast_response()==BOOL_TRUE)
	    {
	        acConnect[gunSn] = 0;
	        accontCount += 15;
	    }

	    chargerMegg[gunSn].flag.bit.bAcConnect = acConnect[gunSn];

    }

    uint8_t temp = 0;
	for(uint8_t i = 0;i < GUN_SN_MAX;i++)
    {
        if(chargerMegg[i].flag.bit.bAcConnect)
        {
            temp = 1;
            break;
        }
    }

    if(temp == 1)
    {
        sSysMegg[GUN_SN_ONE].unOut.bit.bACCont_Out = 1;     
        accontCount = 0;
    }
    else
    {
        if(BOOL_TRUE == cnt_delay(&accontCount, 30000))
        {
            sSysMegg[GUN_SN_ONE].unOut.bit.bACCont_Out = 0;
        }
    }
}

/**
  * @brief  ÿ��������Ҫִ�е�����
  */
static void charger_1ms_task_handle(void)
{
	uint8_t cc1 = 0;
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;
	ENUM_ALARM_STOP index1 = ALARM_STOP_NULL;

	accont_task_handle();

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
    {			    
	    cc1 = sSysMegg[gunSn].CC1;
		
		index = (gunSn == 0)?ALARM_INSDETFAIL_GUN1:ALARM_INSDETFAIL_GUN2;
		index1 = (gunSn == 0)?ALARM_INSDETABS_GUN1:ALARM_INSDETABS_GUN2;
		
	    if (cc1 != chargerDevice[gunSn].cc1State)
	    {    
	        chargerDevice[gunSn].cc1State = cc1;

	        if (cc1)
	        {
	            //DLOG("gun%d:��ǹ>>>",gunSn);
	            set_charger_stopcause(0,gunSn);
	            //EVENT_ADD(RECORD_COM_INDEX_EVENT_GUN_CONNECT, "��ǹ>>>");
	        }
	        else
	        {
	            //DLOG("gun%d:��ǹ<<<",gunSn);
	            chargerMegg[gunSn].i_demand = 0;
				chargerMegg[gunSn].i_demand_mod = 0;
	            set_alarm_statue(index,0);
	            set_alarm_statue(index1,0);
	            //EVENT_ADD(RECORD_COM_INDEX_EVENT_GUN_DISCONNECT, "��ǹ>>>");
	        }
	    }
	    
	}
}

/**
  * @brief  ϵͳ���ȿ��ƣ��������й�����������30S��Ͽ�����ͣ���Ž����������Ͽ�����ǹ�����жϣ�Ȼ��1����ܿ���
  */
#define FAN_START_TEMPRATURE 350 //���ȿ����¶�
#define FAN_MAX_TEMPRATURE 500 //����100%��Ӧ�¶�
static void fan_task_handle(void)
{
    uint8_t fanEn = 0;
	static uint8_t fanCount[2] = {0};
	
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    if(get_charge_state(gunSn)==CHARGER_STATE_CHARGING)
	    {
	        fanEn = 1;
	    }
	    else
	    {
	        fanCount[gunSn] = 0;
	        if(BOOL_TRUE == cnt_delay((int32_t *)&fanCount[gunSn], 30))
	        {
	            fanEn = 0;
	        }
	    }
	    
	    if(get_alarm_fast_response()==BOOL_TRUE)
	    {
	        fanEn = 0;
	        fanCount[gunSn] = 0;
	    }

	    chargerMegg[gunSn].flag.bit.bFan = fanEn;
	}


    uint8_t temp = 0;
    for(uint8_t i=0;i<GUN_SN_MAX;i++)
    {
        if(chargerMegg[i].flag.bit.bFan)
        {
            temp = 1;
            break;
        }
    }

    static int8_t fancnt = 0;
    if(temp != 0)
    {
        if((sSysMegg[0].t_Outlet>FAN_START_TEMPRATURE)||(sSysMegg[0].t_Outlet==-400))
        {
            sSysMegg[0].unOut.bit.bFan = 1;
            fancnt = 60;
        }
        else
        {
            if(fancnt--<=0)
            {
                fancnt = 0;
                sSysMegg[0].unOut.bit.bFan = 0;
            }            
        }
    }
    else
    {
        fancnt -= 4;
        if((fancnt<=0)||(sSysMegg[0].t_Outlet<FAN_START_TEMPRATURE))
        {
            fancnt = 0;        
            sSysMegg[0].unOut.bit.bFan = 0;
        }
    }
    
    static int16_t pwm;
    if(sSysConfig.sTempInt.sysFanspeed != 0)
    {
        if(pwm != sSysConfig.sTempInt.sysFanspeed)
        {
            pwm = sSysConfig.sTempInt.sysFanspeed;
            //set_fan_pwm_duty(pwm);
            DLOG("ϵͳ�ֶ�����%d",pwm);
        }
    }
    else        //�����¶Ƚ��з��ȵ���
    {
        if((sSysMegg[0].unOut.bit.bFan==0)||(sSysMegg[0].t_Outlet<=FAN_START_TEMPRATURE))
        {
            pwm = 0;
            //set_fan_pwm_duty(0);
        }
        else if((pwm%30)==0)//30s
        {
            if(sSysMegg[0].t_Outlet<FAN_MAX_TEMPRATURE)
            {
                //int8_t step = (100-40)*10/(FAN_MAX_TEMPRATURE-FAN_START_TEMPRATURE);
                //int8_t set = (sSysMegg[0].t_Outlet-FAN_START_TEMPRATURE)*step/10+40;
                //set_fan_pwm_duty(set);    //note-�����¶Ƚ���pwm����
            }
            else
            {
                //set_fan_pwm_duty(100);
            }       
        }
        pwm++;            

    }

}

/**
  * @brief  ���״̬�����׶�ռ��ʱ�䣬��λS
  */
static void charger_record_time_handle(void)
{
    uint8_t state = 0;
	
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    state = get_charge_state(gunSn);
	    if(state != CHARGER_STATE_IDLE)
	    {
	        chargerMegg[gunSn].record.chargerTime++;
	        chargerMegg[gunSn].record.chargerEndTime = get_curtimer_val();
	        chargerMegg[gunSn].record.gbVersion = bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag;
	        chargerMegg[gunSn].record.startSoc = bmsDevice[gunSn].frameData.bcpData.bcpStartSoc/10;
	        chargerMegg[gunSn].record.endSoc = (bmsDevice[gunSn].frameData.bsdData.bsdSoc>=bmsDevice[gunSn].frameData.bcsData.bcsCurSoc)?bmsDevice[gunSn].frameData.bsdData.bsdSoc:bmsDevice[gunSn].frameData.bcsData.bcsCurSoc;
	        memcpy(chargerMegg[gunSn].record.vin,bmsDevice[gunSn].frameData.brmData.brmVin,sizeof(chargerMegg[gunSn].record.vin));
	    }
	}
}

/**
  * @brief  ÿ������Ҫִ�е�����
  */
static void charger_1s_task_handle(void)
{
    fan_task_handle();
    charger_record_time_handle();
}

/**
  * @brief  ��ͬ�����񰴲�ͬ�����ڹ���
  */
static void charger_tick_handle(void)
{
    if (unChargerTaskClockFlag.bit.b1ms)
    {
        charger_1ms_task_handle();
    }
    
    if (unChargerTaskClockFlag.bit.b1s)
    {
        charger_1s_task_handle();
    }
    
}

/**
  * @brief  ��Ե��Ϣ�������״̬��IDLEʱִ��
  */
static void insdet_clear(uint8_t gunSn)
{
    memset(&chargerDevice[gunSn].sInsdet,0,sizeof(chargerDevice[gunSn].sInsdet));
    sSysMegg[gunSn].unOut.bit.bGFD_Pos = 0;
    sSysMegg[gunSn].unOut.bit.bGFD_Neg = 0;
    sSysMegg[gunSn].unOut.bit.bGFD_PE = 0;
}

static uint16_t get_insdet_megg(uint8_t posorneg,uint8_t gunSn)
{
    uint16_t voltage = 0;

    if(posorneg)//POS��ѹ
    {
        voltage = _IQsat(sSysMegg[gunSn].v_GFD_P, GUN_MAX_VOLT, 1);
    }
    else//NEG��ѹ
    {
        voltage = _IQsat(sSysMegg[gunSn].v_GFD_N, GUN_MAX_VOLT, 1);
    }

    return voltage;
}


#define kResIntDetMax 40000
/**
  * @brief  ��Ե���������
  */
static ENUM_SYS_INSDET_STATE insdet_check(uint8_t gunSn)
{
    if(sys_debug_get() == DEBUG1_PROCESS)
    {
        return INSDET_STATE_OVER;
    }

    if(chargerDevice[gunSn].sInsdet.state_bak != chargerDevice[gunSn].sInsdet.state)
    {
        DLOG("gunSn%d:��Ե״̬[%d]->[%d]",gunSn,chargerDevice[gunSn].sInsdet.state_bak,chargerDevice[gunSn].sInsdet.state);
        chargerDevice[gunSn].sInsdet.state_bak = chargerDevice[gunSn].sInsdet.state;
    }

    if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].sInsdet.ModVoltAbsCnt, 30000))
    {
        if(sSysMegg[gunSn].v_Mod > 1000)
        {
            set_charger_stopcause(STOP_INSDET_TO2,gunSn);
        }
        else
        {
            set_charger_stopcause(STOP_INSDET_TO1,gunSn);
        } 
        chargerMegg[gunSn].record.GFDVmod = sSysMegg[gunSn].v_Mod;
        DLOG("gunSn%d:��Ե��ʱ,v_Mod[%d],v_Port[%d],state[%d]",gunSn,sSysMegg[gunSn].v_Mod,sSysMegg[gunSn].v_Gun,chargerDevice[gunSn].sInsdet.state);
        set_insdet_state(INSDET_STATE_OVER,gunSn);
    }    
    
    switch (chargerDevice[gunSn].sInsdet.state)
    {
        case INSDET_STATE_IDLE:
        {
            set_insdet_state(INSDET_STATE_START,gunSn);
        }
        break;
    
        case INSDET_STATE_START:
        {
            int PortVoltAbs = (sSysConfig.sInt.bmsType==BMS_GB_2015_TYPE)?100:1000;
            if (sSysMegg[gunSn].v_Gun > PortVoltAbs)   
            {
                chargerDevice[gunSn].sInsdet.PortVoltAbsCnt++;
            
                if(chargerDevice[gunSn].sInsdet.PortVoltAbsCnt>=3000)
                {
                    chargerDevice[gunSn].sInsdet.PortVoltAbsCnt=3000;
                    set_charger_stopcause(STOP_INSDET_VBEFORE,gunSn);
                    set_insdet_state(INSDET_STATE_OVER,gunSn);
                    break;
                }
            }
            else
            {
                chargerDevice[gunSn].sInsdet.PortVoltAbsCnt -= 2;
            }            

            if(sSysMegg[gunSn].v_Gun <= 0-PortVoltAbs)
            {
                chargerDevice[gunSn].sInsdet.PortVoltInvCnt++;
            
                if(900 == (chargerDevice[gunSn].sInsdet.PortVoltInvCnt%1000))
                {
                    DLOG("gunSn%d:1034:Vport[%d]",gunSn,sSysMegg[gunSn].v_Gun);
                }

                if(chargerDevice[gunSn].sInsdet.PortVoltInvCnt>=3000)
                {
                    chargerDevice[gunSn].sInsdet.PortVoltInvCnt=3000;
                    set_insdet_state(INSDET_STATE_OVER,gunSn);
                    set_charger_stopcause(STOP_INSDET_VBEFORE,gunSn);
                    break;
                }                
            }      
            else
            {
                chargerDevice[gunSn].sInsdet.PortVoltInvCnt -= 2;
            }

            if((chargerDevice[gunSn].sInsdet.PortVoltAbsCnt <= -50) \
                &&(chargerDevice[gunSn].sInsdet.PortVoltInvCnt <= -50))
            {
                set_insdet_state(INSDET_STATE_MULTI,gunSn);
            }            
        }
        break;

        case INSDET_STATE_MULTI:
        {
            chargerMegg[gunSn].v_demand = chargerDevice[gunSn].sInsdet.v_InsDet;
            chargerMegg[gunSn].i_demand = 100;
            chargerMegg[gunSn].v_demand_mod = chargerDevice[gunSn].sInsdet.v_InsDet;
            chargerMegg[gunSn].i_demand_mod = 100;
            sSysMegg[gunSn].unOut.bit.bGFD_PE = 1;
            chargerMegg[gunSn].record.insdetVdc = chargerDevice[gunSn].sInsdet.v_InsDet;
            set_insdet_state(INSDET_STATE_GB15,gunSn);
        }
        break;

        case INSDET_STATE_GB15:
        {
            chargerMegg[gunSn].flag.bit.bStartMod = 1;
            if((sSysMegg[gunSn].v_Mod > chargerDevice[gunSn].sInsdet.v_InsDet-100) \
                &&(sSysMegg[gunSn].v_Mod < chargerDevice[gunSn].sInsdet.v_InsDet+100))

            {
                if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].sInsdet.ModVoltDelayCnt, 100))
                {
                    DLOG("gun%d:��Ե3 Vmod[%d],Vins[%d]",gunSn,sSysMegg[gunSn].v_Mod,chargerDevice[gunSn].sInsdet.v_InsDet);
                    sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                    sSysMegg[gunSn].unOut.bit.bGFD_Pos = 1;
                    sSysMegg[gunSn].unOut.bit.bGFD_Neg = 0;
                    set_insdet_state(INSDET_STATE_INS1,gunSn);
                }
            }       
        }
        break;

        case INSDET_STATE_INS1:
        {
            if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].sInsdet.ins1cnt, 1000))
            {
                chargerMegg[gunSn].record.GFDPosVdc=get_insdet_megg(1,gunSn);
                chargerMegg[gunSn].record.GFDNegVdc=get_insdet_megg(0,gunSn);
                sSysMegg[gunSn].unOut.bit.bGFD_Pos = 0;
                sSysMegg[gunSn].unOut.bit.bGFD_Neg = 1;
                set_insdet_state(INSDET_STATE_INS2,gunSn);
                DLOG("gun%d��Ե4 Vpos[%d],Vneg[%d]",gunSn,chargerMegg[gunSn].record.GFDPosVdc,chargerMegg[gunSn].record.GFDNegVdc);
            }
        }
        break;

        case INSDET_STATE_INS2:
        {
            if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].sInsdet.ins2cnt, 1000))
            {
                chargerMegg[gunSn].record.GFDPosVdc2=get_insdet_megg(1,gunSn);
                chargerMegg[gunSn].record.GFDNegVdc2=get_insdet_megg(0,gunSn);
                DLOG("gun%d:��Ե5 Vpos[%d],Vneg[%d]",gunSn,chargerMegg[gunSn].record.GFDPosVdc2,chargerMegg[gunSn].record.GFDNegVdc2);                
                set_insdet_state(INSDET_STATE_CALCULATE,gunSn);
            }            
        }
        break;

        case INSDET_STATE_CALCULATE:
        {
        	int32_t u32X[4] = {0};
        	int32_t insDetResX0 = 0;			
        	int32_t insDetResX1 = 0;

            chargerMegg[gunSn].flag.bit.bStartMod  = 0;
            u32X[0] = ((long)chargerMegg[gunSn].record.GFDNegVdc*chargerMegg[gunSn].record.GFDPosVdc2);     //U(1-)*U(2+)
            u32X[1] = ((long)chargerMegg[gunSn].record.GFDPosVdc*chargerMegg[gunSn].record.GFDNegVdc2);     //U(1+)*(U2-)
            u32X[2] = (chargerMegg[gunSn].record.GFDPosVdc+chargerMegg[gunSn].record.GFDNegVdc);            //U(1+)+U(1-)
            u32X[3] = (chargerMegg[gunSn].record.GFDPosVdc2+chargerMegg[gunSn].record.GFDNegVdc2);          //U(2+)+(U2-)
            

           if(chargerMegg[gunSn].record.GFDPosVdc<20)//5v
            {
				insDetResX0 = 0;
				insDetResX1 = kResIntDetMax;
            }
            else if(chargerMegg[gunSn].record.GFDNegVdc<20)
            {
				insDetResX0 = kResIntDetMax;
				insDetResX1 = 0;

            }
            else
            {
            	insDetResX0 = (int32_t)((float)(u32X[1]-u32X[0])/((5642.0*chargerMegg[gunSn].record.GFDNegVdc*u32X[3]/630.0/5012000.0)-(5312.0*chargerMegg[gunSn].record.GFDNegVdc2*u32X[2]/300.0/5012000.0)));  //��λ100ŷķ
            	insDetResX1 = (int32_t)((float)(u32X[1]-u32X[0])/((5642.0*chargerMegg[gunSn].record.GFDPosVdc2*u32X[2]/630.0/50120.0)-(5312.0*chargerMegg[gunSn].record.GFDPosVdc*u32X[3]/300.0/50120.0)));
			}

			if(abs(insDetResX0) >= kResIntDetMax)
			{
                insDetResX0 = kResIntDetMax;
			}

			if(abs(insDetResX1) >= kResIntDetMax)
			{
                insDetResX1 = kResIntDetMax;
			}
			
            //DLOG("��Ե6 R1[%d] R2[%d]",insDetResX0,insDetResX1);
            
            chargerMegg[gunSn].record.insdetRes = insDetResX0;
            chargerMegg[gunSn].record.insdetRes2 = insDetResX1;
            uint32_t insdetResToVolt = (chargerMegg[gunSn].record.insdetRes < chargerMegg[gunSn].record.insdetRes2)?chargerMegg[gunSn].record.insdetRes:chargerMegg[gunSn].record.insdetRes2;
            insdetResToVolt = (insdetResToVolt*10000)/chargerDevice[gunSn].sInsdet.v_InsDet;

            DLOG("��Ե6 R1[%d] R2[%d]",chargerMegg[gunSn].record.insdetRes,chargerMegg[gunSn].record.insdetRes2);
            uint8_t fail = 0, abs = 0;
            if(insdetResToVolt<10*sSysConfig.sInt.uInsdetRes)//�жϾ�Ե����ֵ��С    
            {
                fail=1;
                abs=0;
            }
            else if(insdetResToVolt<10*500)
            {
                fail=0;
                abs=1;
            }

			ENUM_ALARM_STOP index = ALARM_STOP_NULL;
			ENUM_ALARM_STOP index1 = ALARM_STOP_NULL;
			index = (gunSn == 0)?ALARM_INSDETFAIL_GUN1:ALARM_INSDETFAIL_GUN2;
			index1 = (gunSn == 0)?ALARM_INSDETABS_GUN1:ALARM_INSDETABS_GUN2;

            set_alarm_statue(index,fail);
            set_alarm_statue(index1,abs);

            set_insdet_state(INSDET_STATE_DISCHARGE,gunSn);
        }
        break;

        case INSDET_STATE_DISCHARGE:
        {
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].sInsdet.dischargecnt,1000))
                ||(sSysMegg[gunSn].v_Gun < 600))
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
                sSysMegg[gunSn].unOut.bit.bGFD_Neg = 0;
                sSysMegg[gunSn].unOut.bit.bGFD_Pos = 0;
                sSysMegg[gunSn].unOut.bit.bGFD_PE = 0;
                set_insdet_state(INSDET_STATE_DISCHARGE_DELAY,gunSn);
            }
        }
        break;

        case INSDET_STATE_DISCHARGE_DELAY:
        {
            set_insdet_state(INSDET_STATE_OVER,gunSn);
        }
        break;
        
        case INSDET_STATE_OVER:
        {
            
        }
        break;

        default:
        {
            set_insdet_state(INSDET_STATE_IDLE,gunSn);
        }
        break;
        
    }

    return chargerDevice[gunSn].sInsdet.state;
}

/**
  * @brief  Ԥ���׶ζ˿ڵ�ѹ��⣬���������˿ڵ�ѹ�Ƿ��ڳ��������Χ
  * @retval BOOL_TRUE ���ͨ��  BOOL_FALSE ���δͨ��(30Sδͨ���ػ�)
  */
static BOOL voltage_check_before_start(uint8_t gunSn)
{
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;
	ENUM_ALARM_STOP index1 = ALARM_STOP_NULL;

    if((sSysMegg[gunSn].v_Gun > sSysConfig.sInt.uDcUnderVolt)
        &&(sSysMegg[gunSn].v_Gun < sSysConfig.sInt.uDcOverVolt))
    {
        int idata = chargerDevice[gunSn].cnt.waitPortCnt;
        chargerDevice[gunSn].cnt.waitPortCnt = (idata<0)?0:(idata+1);
    }
    else
    {
        chargerDevice[gunSn].cnt.waitPortCnt--;
    }

    if(chargerDevice[gunSn].cnt.waitPortCnt>=3000)
    {
        chargerDevice[gunSn].cnt.waitPortCnt=3000;
        return BOOL_TRUE;
    }
    
    if(chargerDevice[gunSn].cnt.waitPortCnt<=-30000)
    {
        chargerDevice[gunSn].cnt.waitPortCnt=-30000;
        if(9000 == (abs(chargerDevice[gunSn].cnt.waitPortCnt) % 10000))
        {
            DLOG("gun%d:1039 Vbat:[%d]",gunSn,sSysMegg[gunSn].v_Gun);        
        }

		index = (gunSn == 0)?ALARM_VDCINV_GUN1:ALARM_VDCINV_GUN2;
		index1 = (gunSn == 0)?ALARM_VDCLOW_GUN1:ALARM_VDCLOW_GUN2;
		
        if(sSysMegg[gunSn].v_Gun <= -500)
        {
            set_alarm_statue(index,1);
        }
        else
        {
            set_charger_stopcause(index1,gunSn);
        }
        return BOOL_FALSE;
    }
    else
    {
        return BOOL_FALSE;
    }
}

/**
  * @brief  Ԥ���׶ζ˿ڵ�ѹ���
  * @retval BOOL_TRUE ���ͨ��  BOOL_FALSE ���δͨ��
  *			���踺�� ֱ��ͨ��
  *			��ظ��� BMS���� != 5 ֻ����Ƿ��������Χ
  *			��ظ��� BMS���� == 5 ��⣺
  *				1��bcp���ĵ�ѹ�Ƿ��������Χ 
  *				2����ص�ѹ��BCP���ĵ�ѹƫ���Ƿ�С��5%���ǵĻ�3S��ͨ������Ļ�30S��ֹͣ��� 
  */
static BOOL bms_batteryVoltage_check(uint8_t gunSn)
{
    if((LOAD_R == sSysConfig.sTempInt.LoadType)||(sys_debug_get() == DEBUG1_PROCESS))
    {
        return BOOL_TRUE;
    }

    if(BMS_GB_2015_TYPE != sSysConfig.sInt.bmsType)
    {
        return (voltage_check_before_start(gunSn));            
    }
    else if((bmsDevice[gunSn].frameData.bcpData.bcpCurBatteryVoltage < sSysConfig.sInt.uDcUnderVolt)
        ||(bmsDevice[gunSn].frameData.bcpData.bcpCurBatteryVoltage > sSysConfig.sInt.uMaxChargeVolt))
    {
        set_charger_stopcause(STOP_BCP_BATVOLAGE,gunSn);
        return BOOL_TRUE;
    }

    int32_t volBattery = bmsDevice[gunSn].frameData.bcpData.bcpCurBatteryVoltage;
    int32_t volPort = sSysMegg[gunSn].v_Gun; 

    volBattery *= 100;
    volPort *= 100;
    
    int32_t idata = volBattery - volPort;
    if (volBattery < volPort)
    {
        idata = volPort - volBattery;
    }  

    if(idata < (int32_t)(volBattery * 0.05))
    {
        if(chargerDevice[gunSn].cnt.waitPortCnt++ >= 3000)
        {
            chargerDevice[gunSn].cnt.waitPortCnt = 3000;
            return BOOL_TRUE;
        }
        else
        {
            return BOOL_FALSE;
        }
    }

//    SHOW("[Vbcp:%d, Vbat:%d]", volBattery / 100, volPort / 100);

    if(chargerDevice[gunSn].cnt.waitBatVolCnt++ >= 30000)
    {
        chargerDevice[gunSn].cnt.waitBatVolCnt = 30000;
        chargerMegg[gunSn].record.bcp_vabs = sSysMegg[gunSn].v_Gun;

		ENUM_ALARM_STOP index = ALARM_STOP_NULL;
		index = (gunSn == 0)?ALARM_VDCINV_GUN1:ALARM_VDCINV_GUN2;
        if(sSysMegg[gunSn].v_Gun <= -500)
        {
            set_alarm_statue(index,1);
        }
        else
        {
            set_charger_stopcause(STOP_BCP_BATVOLAGE,gunSn);
        }
    }
    else
    {
        if(500 == (chargerDevice[gunSn].cnt.waitBatVolCnt % 5000))
        {
            chargerMegg[gunSn].record.bcp_vabs2 = sSysMegg[gunSn].v_Gun;
            DLOG("gun%d:BCP��ѹ���[%d, %d]", gunSn,volBattery/100, volPort/100);
        }
    }
    
    return BOOL_FALSE;
}

/**
  * @brief  �����BMS��������
  * @retval 
  *			����ģʽ ������
  *			Զ��ģʽ
  *				1�������ѹ������BCS�������BCP����������ѹ������1S����
  *				2��SOC������BCS SOC���ڲ��� �������SOC������1S����
  *				3������¶ȱ�����BSM�������¶ȴ���BCP��������¶ȣ�����1S����
  *				4�����״̬������BSM״̬�쳣�����������ò������״̬����ʹ�ܣ���SOC�ͼ���ص�ѹ�ͽ���BMS==5ʱ��Ч
  *				5������쳣������  ����ѹ����BCP����������ѹ������ѹ����BCL�����ѹ��ֹͣ���
  */
static void bms_protection_handle(uint8_t gunSn)
{
    if(sSysConfig.sTempInt.WorkMode == WORK_LOCAL)
    {
        return;
    }
    
    if(bmsDevice[gunSn].frameData.bcsData.bcsSingleInfo.bit.bMaxSingleVol > bmsDevice[gunSn].frameData.bcpData.bcpSingleMaxVol)
    {
        if(chargerDevice[gunSn].cnt.batSinglecnt ++ > 600)
        {
            set_charger_stopcause(STOP_BMS_SINGLE_VUP,gunSn);
            chargerDevice[gunSn].cnt.batSinglecnt = 600;
        }
    }
    else
    {
        chargerDevice[gunSn].cnt.batSinglecnt = 0;
    }

    if(bmsDevice[gunSn].frameData.bcsData.bcsCurSoc > sSysConfig.sInt.uMaxAllowChSOC)
    {
        if(chargerDevice[gunSn].cnt.batSOCcnt ++ > 1000)
        {
            set_charger_stopcause(STOP_SOC_PROTECTION,gunSn);
            chargerDevice[gunSn].cnt.batSOCcnt = 1000;
        }
    }
    else
    {
        chargerDevice[gunSn].cnt.batSOCcnt = 0;
    }

    if(bmsDevice[gunSn].frameData.bsmData.bsmMaxTemperature > bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerTemperature)
    {
        if(chargerDevice[gunSn].cnt.batTempcnt ++ > 600)
        {
            set_charger_stopcause(STOP_BMS_TEMP_MAX,gunSn);
            chargerDevice[gunSn].cnt.batTempcnt = 600;
        }
    }
    else
    {
        chargerDevice[gunSn].cnt.batTempcnt = 0;
    }

    if (sSysConfig.sInt.uBatStatueEn == 1)
    {
        if ((bmsDevice[gunSn].frameData.bsmData.bsmBatteyState)&0x01)
        {
            set_charger_stopcause(STOP_BSM_VSINGLE,gunSn);
        }
        else if ((bmsDevice[gunSn].frameData.bsmData.bsmBatteyState)&0x04)
        {
            set_charger_stopcause(STOP_BSM_SOC,gunSn);
        }		  
        else if ((bmsDevice[gunSn].frameData.bsmData.bsmBatteyState)&0x10)
        {
            set_charger_stopcause(STOP_BSM_OC,gunSn);
        }		  		  
        else if ((bmsDevice[gunSn].frameData.bsmData.bsmBatteyState)&0x40)
        {
            set_charger_stopcause(STOP_BSM_TEMPUP,gunSn);
        }		  			  
	    else if(bmsDevice[gunSn].frameData.bsmData.bsmByte7.all&0x01)
        {
            set_charger_stopcause(STOP_BSM_INSDET,gunSn);  	  	
        }		  
        else if(bmsDevice[gunSn].frameData.bsmData.bsmByte7.all&0x04)
        {
            set_charger_stopcause(STOP_BSM_CC,gunSn);  	  	
        }
        else if(sSysConfig.sInt.bmsType == BMS_GB_2015_TYPE)
        {
            if ((bmsDevice[gunSn].frameData.bsmData.bsmBatteyState)&0x08)
            {
                set_charger_stopcause(STOP_BSM_SOCLOW,gunSn);
            }
            else if ((bmsDevice[gunSn].frameData.bsmData.bsmBatteyState)&0x02)
            {
                set_charger_stopcause(STOP_BSM_BATLOW,gunSn);
            }
        }
    
    }

    const int16_t maxOutputAbnCnt = 5000;// �����׸Ķ���Զ�̿���ж�� �ױ������ѹ

    int16_t s16bcpMaxChargerVoltage_105=(bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerVoltage*1.01);//
    int16_t s16bclVoltageDemand_10V=(bmsDevice[gunSn].frameData.bclData.bclVoltageDemand+50);	


    if (sSysConfig.sInt.uBatProtectEn == 1)
    {        
        //����ѹ����BCP����������ѹ��ֹͣ���,����1%
        int maxAbnormalCnt = 300;
        //            int maxAbnormalCnt = (sChSysMegg.iRecMode==1)?300:5000;//��̫��ж��ʱ�ᱣ����50ms��Ӧʱ��620ms,Ҫ����1S��
        uint8_t flag = (sSysMegg[gunSn].v_Gun > s16bcpMaxChargerVoltage_105 + 50);
        if(1==cnt_change_until_threshold(flag,&chargerDevice[gunSn].cnt.maxvoltagecnt,maxAbnormalCnt,1,1))
        {
            set_charger_stopcause(STOP_VOLTAGE_BMSMAX,gunSn);
        }

        //����ѹ����BCL�����ѹ��ֹͣ���
        if(bmsDevice[gunSn].frameData.bclData.bclVoltageDemand!=0)
        {
            flag = (sSysMegg[gunSn].v_Gun >s16bclVoltageDemand_10V);
            if(1==cnt_change_until_threshold(flag,&chargerDevice[gunSn].cnt.bclvoltagecnt,maxOutputAbnCnt,1,1))
            {
                set_charger_stopcause(STOP_VOLTAGE_BMSDEMAND,gunSn);
            }
	    }
    }    

}

/**
  * @brief ǹ�½����
  */
#define DERATING_TEMP_ZERO2 950
#define DERATING_TEMP_ZERO1 850
static int16_t gun_temp_derate(uint8_t gunSn)
{    
    float ratio[4] = {0};
    
    float x[2] = {DERATING_TEMP_ZERO1,DERATING_TEMP_ZERO2};
    float y[2] = {sSysConfig.sInt.uMaxChargeCurr, sSysConfig.sInt.uMaxChargeCurr/10};

    if(sSysMegg[gunSn].t_Gun < x[0])
    {
        return 0;
    }
    else if(sSysMegg[gunSn].t_Gun > x[1])
    {
        return y[1];
    }

    ratio[0] = (y[0] - y[1]) / (x[0] - x[1]);
    ratio[1] = y[0] + ratio[0] * (sSysMegg[gunSn].t_Gun - x[0]);  
    
    return (int16_t)ratio[1];
}

static void charge_voltage_current_deal(uint8_t gunSn)    
{
    uint16_t volt,curr = 0;
    
    if(sSysConfig.sTempInt.WorkMode == WORK_LOCAL)
    {
        chargerMegg[gunSn].v_demand = sSysConfig.sTempInt.v_local;
        chargerMegg[gunSn].i_demand = sSysConfig.sTempInt.i_local;
    }
    else
    {
        chargerMegg[gunSn].v_demand = bmsDevice[gunSn].frameData.bclData.bclVoltageDemand;
		if(chargerMegg[gunSn].v_demand > bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerVoltage)
		{
			if(sSysConfig.sInt.bmsType == BMS_GB_2015_TYPE)
			{
				set_charger_stopcause(STOP_BAT_DEMAND_VOLTAGE,gunSn);
			}
			else
			{
				chargerMegg[gunSn].v_demand = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerVoltage;
			}
		}

        chargerMegg[gunSn].i_demand = bmsDevice[gunSn].frameData.bclData.bclCurrentDemand;
        if(chargerMegg[gunSn].i_demand > bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerCurrent)
        {
			if(sSysConfig.sInt.bmsType == BMS_GB_2015_TYPE)
			{
				set_charger_stopcause(STOP_BAT_DEMAND_CURRENT,gunSn);
			}
			else
			{
				chargerMegg[gunSn].i_demand = bmsDevice[gunSn].frameData.bcpData.bcpMaxChargerCurrent;
			}
        }
    }

    volt = (chargerMegg[gunSn].v_demand < sSysConfig.sInt.uMaxChargeVolt)  \
            ? chargerMegg[gunSn].v_demand : sSysConfig.sInt.uMaxChargeVolt;
    curr = (chargerMegg[gunSn].i_demand < sSysConfig.sInt.uMaxChargeCurr)  \
            ? chargerMegg[gunSn].i_demand : sSysConfig.sInt.uMaxChargeCurr;

    if(sSysConfig.sInt.uPowerLimit_10x != 0)//�޹���
    {
        uint16_t v_limit = _IQsat(sSysMegg[gunSn].v_Gun,sSysConfig.sInt.uMaxChargeVolt,sSysConfig.sInt.uDcUnderVolt);
        uint16_t c_limit = ((double)sSysConfig.sInt.uPowerLimit_10x * 10000)/v_limit;
        curr = (curr < c_limit) ? curr : c_limit;
    }
    
    if(sSysConfig.sInt.uParallelMode != 0)//���䣬ע�����λ�ò����ƶ�
    {
        curr /= 2;
    }

    curr = (curr>GUN_MAX_CURR)?GUN_MAX_CURR:curr;//��ǹ����250

    int16_t gun_t = gun_temp_derate(gunSn);//ǹ�½���
    if(gun_t != 0)
    {
        curr = (curr>gun_t)?gun_t:curr;
    }
    
    chargerMegg[gunSn].v_demand_mod = volt;
    chargerMegg[gunSn].i_demand_mod = curr;
    
}

static void charger_data_handle(uint8_t gunSn)
{
    charge_voltage_current_deal(gunSn);
//    charger_output_handle();
    bms_protection_handle(gunSn);
}

static void charge_off_clear(uint8_t gunSn)
{
    memset(&chargerDevice[gunSn].cnt,0,sizeof(chargerDevice[gunSn].cnt));
    sSysMegg[gunSn].unOut.bit.bBMS_V = 0;
    sSysMegg[gunSn].unOut.bit.bGunLock = 0;
    sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
    chargerMegg[gunSn].flag.bit.bStartCh = 0;
}

static void charge_logic_off(uint8_t gunSn)
{
    chargerMegg[gunSn].flag.bit.bStartMod = 0;
    chargerMegg[gunSn].v_demand_mod = 0;
    chargerMegg[gunSn].i_demand_mod = 0;
    chargerMegg[gunSn].v_demand = 0;
    chargerMegg[gunSn].i_demand = 0;
//    sChCan1Megg.iBSMPauseFlag = 0;
}

static void charger_state_logic_all(uint8_t gunSn)
{
    switch(chargerDevice[gunSn].chargerState)
    {
        case CHARGER_STATE_IDLE:
        {
            charge_logic_off(gunSn);
            charge_off_clear(gunSn);
            insdet_clear(gunSn);
        }
        break;
        
        case CHARGER_STATE_GUNLOCK:
        {
            chargerMegg[gunSn].flag.bit.bStartCh = 1;
    	    sSysMegg[gunSn].unOut.bit.bGunLock = 1;
            set_charge_state(CHARGER_STATE_MOD_SWITCH,gunSn);
        }
        break;

        case CHARGER_STATE_MOD_SWITCH:
        {
            int32_t delay = Project_Type_Delay();
    	    if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.modswitchcnt,delay))
    	    {
                set_charge_state(CHARGER_STATE_AXLOCK,gunSn);
    	    }
        }
        break;

        case CHARGER_STATE_AXLOCK:
        {
            chargerDevice[gunSn].sInsdet.v_InsDet = _IQsat(4000, sSysConfig.sInt.uMaxChargeVolt,sSysConfig.sInt.uDcUnderVolt+50);
            DLOG("gun%d:v_InsDet:%d",gunSn,chargerDevice[gunSn].sInsdet.v_InsDet);
            set_charge_state(CHARGER_STATE_INSDET,gunSn);
        }
        break;

        case CHARGER_STATE_INSDET:
        {            
            if(insdet_check(gunSn) == INSDET_STATE_OVER)
            {
                set_charge_state(CHARGER_STATE_INS_DISCHARGE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_INS_DISCHARGE:
        {
            sSysMegg[gunSn].unOut.bit.bBMS_V = 1;        
            bms_charger_start(gunSn);
            set_charge_state(CHARGER_STATE_WAIT_BROAA,gunSn);
        }
        break;

        case CHARGER_STATE_WAIT_BROAA:
        {
            if(sSysMegg[gunSn].i_Gun < 50)
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;//bms����ʱ�ڴ˵ȴ�
            }
            chargerMegg[gunSn].flag.bit.bStartMod  = 0;
            chargerDevice[gunSn].cnt.waitPortCnt = 0;
            chargerDevice[gunSn].cnt.waitBatVolCnt = 0;
            chargerDevice[gunSn].cnt.prechgcnt = 0;
            if(0 != bmsDevice[gunSn].frameData.broData.broReadyState)
            {
                set_charge_state(CHARGER_STATE_PRE_CHARGE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_PRE_CHARGE:
        {
            if ((bmsDevice[gunSn].mainState == IDLE)||(bmsDevice[gunSn].mainState == WAIT_START)) 
            {
                set_charge_state(CHARGER_STATE_WAIT_BROAA,gunSn);
                break;
            }
            
            if(BOOL_FALSE == bms_batteryVoltage_check(gunSn))
            {
                chargerMegg[gunSn].record.pre_v_gun = sSysMegg[gunSn].v_Gun;
                break;
            }

            if(LOAD_R == sSysConfig.sTempInt.LoadType)
            {
                chargerMegg[gunSn].flag.bit.bStartMod  = 1;
                chargerMegg[gunSn].v_demand_mod = 2000;
                chargerMegg[gunSn].i_demand_mod = 50;
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                set_charge_state(CHARGER_STATE_CHARGING,gunSn);
            }
            else
            {
                chargerMegg[gunSn].flag.bit.bStartMod  = 1;
                chargerMegg[gunSn].v_demand_mod = sSysMegg[gunSn].v_Gun - 50;
                chargerMegg[gunSn].i_demand_mod = 50;
                chargerMegg[gunSn].record.pre_v_mod = sSysMegg[gunSn].v_Mod;
                
                if(sSysMegg[gunSn].unOut.bit.bDcCont_Out == 0)
                {
                    if((sSysMegg[gunSn].v_Mod >= sSysMegg[gunSn].v_Gun - 100) \
                        &&(sSysMegg[gunSn].v_Mod <= sSysMegg[gunSn].v_Gun))
                    {
                        sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                    }
                }
                
                if ((BOOL_TRUE == get_DcCont_State(gunSn))&&(bmsDevice[gunSn].unBmsStateA.bit.bComChargingStageFlag))
                {
                    chargerMegg[gunSn].v_demand_mod = sSysMegg[gunSn].v_Gun;
                    set_charge_state(CHARGER_STATE_CHARGING,gunSn);
                }
                
                if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.prechgcnt,20000)) 
                {
                    if(sSysMegg[gunSn].v_Mod > 1500)
                    {
                        set_charger_stopcause(STOP_SOFTFAIL_MOD_START,gunSn);
                    }
                    else
                    {
                        set_charger_stopcause(STOP_SOFTFAIL_MOD_UNSTART,gunSn);
                    }
                }            
            }
        }
        break;        

        case CHARGER_STATE_CHARGING:
        {     
            if ((bmsDevice[gunSn].mainState == IDLE)||(bmsDevice[gunSn].mainState == WAIT_START)) 
            {
                set_charge_state(CHARGER_STATE_WAIT_BROAA,gunSn);
                break;
            }        

            if(bmsDevice[gunSn].unBmsStateA.bit.bBSMPause)
            {
                chargerMegg[gunSn].flag.bit.bStartMod  = 0;
                if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.bsmPausecnt,500)) \
                    && (sSysMegg[gunSn].i_Gun < 50))      
                {
                    sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
                }
            }
            else
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                chargerMegg[gunSn].flag.bit.bStartMod  = 1;
                chargerDevice[gunSn].cnt.bsmPausecnt = 0;
            }
            
            charger_data_handle(gunSn);      
        }
        break;

        case CHARGER_STATE_STARTFAULT:
        {
            set_charge_state(CHARGER_STATE_STOP,gunSn);
        }
        break;       

        case CHARGER_STATE_STOP:
        {
            charge_logic_off(gunSn);
            sSysMegg[gunSn].unOut.bit.bGunLock = 0;
            if(get_alarm_statue(ALARM_AX))
            {
                set_charge_state(CHARGER_STATE_END,gunSn);
                break;
            }
            uint16_t contdelay = 150;

            if((get_alarm_fast_response()==BOOL_TRUE)||(sSysMegg[gunSn].CC1 == 0))
            {
                contdelay = 20;
            }
            
            if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.dcContCutcnt,contdelay)) 
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
                set_charge_state(CHARGER_STATE_CHG_DISCHARGE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_CHG_DISCHARGE:
        {
            set_charge_state(CHARGER_STATE_AXOPEN,gunSn);
        }
        break;
        
        case CHARGER_STATE_AXOPEN:
        {
            sSysMegg[gunSn].unOut.bit.bBMS_V = 0;
            set_charge_state(CHARGER_STATE_GUNOPEN,gunSn);
        }
        break;

        case CHARGER_STATE_GUNOPEN:
        {
            set_charge_state(CHARGER_STATE_END,gunSn);
        }
        break;

        case CHARGER_STATE_END:
        {
            charge_logic_off(gunSn);
            sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
            sSysMegg[gunSn].unOut.bit.bGunLock = 0;
            sSysMegg[gunSn].unOut.bit.bBMS_V = 0;
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.updaterecordcnt,1000))||(get_alarm_statue(ALARM_AX)!=0))//���µ���
            {                
                chargerMegg[gunSn].flag.bit.bRecordSave = 1;
                DLOG("gun%d:EEEEND %d ms",gunSn,get_curtimer_ms());
                set_charge_state(CHARGER_STATE_END_SAVE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_END_SAVE:
        {
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.recordcnt,5000))//��¼�洢��ʱ
                ||(chargerMegg[gunSn].flag.bit.bRecordSave == 0))
            {
                DLOG("gun%d:�洢����%d",gunSn,chargerDevice[gunSn].cnt.recordcnt);
                set_charge_state(CHARGER_STATE_IDLE,gunSn);
                charge_off_clear(gunSn);
            }
        }
        break;        

        default:
        {
            set_charge_state(CHARGER_STATE_IDLE,gunSn);
        }
        break;
    }

}

static void charger_state_logic_gb2015(uint8_t gunSn)
{
    switch(chargerDevice[gunSn].chargerState)
    {
        case CHARGER_STATE_IDLE:
        {
            charge_logic_off(gunSn);
            charge_off_clear(gunSn);
            insdet_clear(gunSn);
        }
        break;
        
        case CHARGER_STATE_GUNLOCK:
        {
            chargerMegg[gunSn].flag.bit.bStartCh = 1;
    	    sSysMegg[gunSn].unOut.bit.bGunLock = 1;
            set_charge_state(CHARGER_STATE_MOD_SWITCH,gunSn);
        }
        break;

        case CHARGER_STATE_MOD_SWITCH:
        {
            int32_t delay = Project_Type_Delay();
    	    if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.modswitchcnt,delay))
    	    {
                set_charge_state(CHARGER_STATE_AXLOCK,gunSn);
    	    }
        }
        break;

        case CHARGER_STATE_AXLOCK:
        {
    	    sSysMegg[gunSn].unOut.bit.bBMS_V = 1;
    	    if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.axdelaycnt,50))
    	    {
                bms_charger_start(gunSn);
                set_charge_state(CHARGER_STATE_GB_CONFIG,gunSn);
    	    }
        }
        break;

        case CHARGER_STATE_GB_CONFIG:
        {
            if(!bmsDevice[gunSn].unBmsStateA.bit.bInsulationCmdFlag)
            {
                break;
            }
            
            set_charge_state(CHARGER_STATE_INSDET,gunSn);
            if (bmsDevice[gunSn].unBmsStateA.bit.bGb2015ProtocolFlag == 0)
            {
                sSysMegg[gunSn].unOut.bit.bBMS_V = 0; //GB2011
                chargerDevice[gunSn].sInsdet.v_InsDet = _IQsat(4000, sSysConfig.sInt.uMaxChargeVolt,sSysConfig.sInt.uDcUnderVolt+50);
            }
            else
            {
                chargerDevice[gunSn].sInsdet.v_InsDet = (bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol > sSysConfig.sInt.uMaxChargeVolt) \
                                               ?sSysConfig.sInt.uMaxChargeVolt:bmsDevice[gunSn].frameData.bhmData.bhmMaxChargerVol;
                if(chargerDevice[gunSn].sInsdet.v_InsDet < sSysConfig.sInt.uDcUnderVolt)
                {
                    set_charger_stopcause(STOP_BMS_VINSDET,gunSn);
                }
            }                
            DLOG("gun%d:v_InsDet:%d",gunSn,chargerDevice[gunSn].sInsdet.v_InsDet);
        }
        break;

        case CHARGER_STATE_INSDET:
        {            
            if(insdet_check(gunSn) == INSDET_STATE_OVER)
            {
                set_charge_state(CHARGER_STATE_INS_DISCHARGE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_INS_DISCHARGE:
        {
            if(sSysMegg[gunSn].unOut.bit.bBMS_V == 0)//GB2011
            {
                sSysMegg[gunSn].unOut.bit.bBMS_V = 1;        
                bms_charger_start(gunSn);
            }
            set_charge_state(CHARGER_STATE_WAIT_BROAA,gunSn);
        }
        break;

        case CHARGER_STATE_WAIT_BROAA:
        {
            if(sSysMegg[gunSn].i_Gun < 50)
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;//bms����ʱ�ڴ˵ȴ�
            }
            chargerMegg[gunSn].flag.bit.bStartMod  = 0;
			chargerDevice[gunSn].cnt.waitPortCnt = 0;
            chargerDevice[gunSn].cnt.waitBatVolCnt = 0;
			chargerDevice[gunSn].cnt.prechgcnt = 0;
            if(0 != bmsDevice[gunSn].frameData.broData.broReadyState)
            {
                set_charge_state(CHARGER_STATE_PRE_CHARGE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_PRE_CHARGE:
        {
            if ((bmsDevice[gunSn].mainState == IDLE)||(bmsDevice[gunSn].mainState == WAIT_START)) 
            {
                set_charge_state(CHARGER_STATE_WAIT_BROAA,gunSn);
                break;
            }
            
            if(BOOL_FALSE == bms_batteryVoltage_check(gunSn))
            {
                chargerMegg[gunSn].record.pre_v_gun = sSysMegg[gunSn].v_Gun;
                break;
            }

            if(LOAD_R == sSysConfig.sTempInt.LoadType)
            {
                chargerMegg[gunSn].flag.bit.bStartMod  = 1;
                chargerMegg[gunSn].v_demand_mod = 2000;
                chargerMegg[gunSn].i_demand_mod = 50;
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                set_charge_state(CHARGER_STATE_CHARGING,gunSn);
            }
            else
            {
                chargerMegg[gunSn].flag.bit.bStartMod  = 1;
                chargerMegg[gunSn].v_demand_mod = sSysMegg[gunSn].v_Gun - 50;
                chargerMegg[gunSn].i_demand_mod = 50;
                chargerMegg[gunSn].record.pre_v_mod = sSysMegg[gunSn].v_Mod;
                
                if(sSysMegg[gunSn].unOut.bit.bDcCont_Out == 0)
                {
                    if((sSysMegg[gunSn].v_Mod >= sSysMegg[gunSn].v_Gun - 100) \
                        &&(sSysMegg[gunSn].v_Mod <= sSysMegg[gunSn].v_Gun))
                    {
                        sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                    }
                }
                
                if ((BOOL_TRUE == get_DcCont_State(gunSn))&&(bmsDevice[gunSn].unBmsStateA.bit.bComChargingStageFlag))
                {
                    chargerMegg[gunSn].v_demand_mod = sSysMegg[gunSn].v_Gun;
                    set_charge_state(CHARGER_STATE_CHARGING,gunSn);
                }
                
                if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.prechgcnt,30000)) 
                {
                    if(sSysMegg[gunSn].v_Mod > 1500)
                    {
                        set_charger_stopcause(STOP_SOFTFAIL_MOD_START,gunSn);
                    }
                    else
                    {
                        set_charger_stopcause(STOP_SOFTFAIL_MOD_UNSTART,gunSn);
                    }
                }            
            }
        }
        break;        

        case CHARGER_STATE_CHARGING:
        {     
            if ((bmsDevice[gunSn].mainState == IDLE)||(bmsDevice[gunSn].mainState == WAIT_START)) 
            {
                set_charge_state(CHARGER_STATE_WAIT_BROAA,gunSn);
                break;
            }        

            if(bmsDevice[gunSn].unBmsStateA.bit.bBSMPause)
            {
                chargerMegg[gunSn].flag.bit.bStartMod  = 0;
                if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.bsmPausecnt,500)) \
                    && (sSysMegg[gunSn].i_Gun < 50))      
                {
                    sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
                }
            }
            else
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                chargerMegg[gunSn].flag.bit.bStartMod  = 1;
                chargerDevice[gunSn].cnt.bsmPausecnt = 0;
            }
            
            charger_data_handle(gunSn);      
        }
        break;

        case CHARGER_STATE_STARTFAULT:
        {
            //����ʧ��ֹͣ��ֹͣԭ����ࣺ��������硣����
            set_charge_state(CHARGER_STATE_STOP,gunSn);
        }
        break;       

        case CHARGER_STATE_STOP:
        {
            charge_logic_off(gunSn);
            if(get_alarm_statue(ALARM_AX))
            {
                set_charge_state(CHARGER_STATE_END,gunSn);
                break;
            }

            uint16_t contdelay = 150;

            if((get_alarm_fast_response()==BOOL_TRUE)||(sSysMegg[gunSn].CC1 == 0))
            {
                contdelay = 20;
            }
            
            if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.dcContCutcnt,contdelay)) 
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
                set_charge_state(CHARGER_STATE_CHG_DISCHARGE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_CHG_DISCHARGE:
        {
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.chargeDiscnt,1000))
                ||(sSysMegg[gunSn].v_Mod <= 600))
            {
                set_charge_state(CHARGER_STATE_AXOPEN,gunSn);
            }
        }
        break;
        
        case CHARGER_STATE_AXOPEN:
        {
            if(!bmsDevice[gunSn].unBmsStateB.bit.bBmsComState)
            {
                sSysMegg[gunSn].unOut.bit.bBMS_V = 0;
                set_charge_state(CHARGER_STATE_GUNOPEN,gunSn);
            }
        }
        break;

        case CHARGER_STATE_GUNOPEN:
        {
            if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.gunopencnt,200))
            {
                sSysMegg[gunSn].unOut.bit.bGunLock = 0;
                set_charge_state(CHARGER_STATE_END,gunSn);
            }
        }
        break;

        case CHARGER_STATE_END:
        {
            charge_logic_off(gunSn);
            sSysMegg[gunSn].unOut.bit.bDcCont_Out = 0;
            sSysMegg[gunSn].unOut.bit.bGunLock = 0;
            sSysMegg[gunSn].unOut.bit.bBMS_V = 0;
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.updaterecordcnt,1000))||(get_alarm_statue(ALARM_AX)!=0))//���µ���
            {                
                chargerMegg[gunSn].flag.bit.bRecordSave = 1;
                set_charge_state(CHARGER_STATE_END_SAVE,gunSn);
            }
        }
        break;

        case CHARGER_STATE_END_SAVE:
        {
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.recordcnt,5000))
                ||(chargerMegg[gunSn].flag.bit.bRecordSave == 0))
            {
                DLOG("gun%d:�洢����%d",gunSn,chargerDevice[gunSn].cnt.recordcnt);
                set_charge_state(CHARGER_STATE_IDLE,gunSn);
                charge_off_clear(gunSn);
            }
        }
        break;        

        default:
        {
            set_charge_state(CHARGER_STATE_IDLE,gunSn);
        }
        break;
    }


                   

}


/**
  * @brief  ���س������
  */
static void charger_state_logic_local(uint8_t gunSn)
{    
    switch(chargerDevice[gunSn].chargerState)
    {
        case CHARGER_STATE_IDLE:
        {
            charge_logic_off(gunSn);
            charge_off_clear(gunSn);
            insdet_clear(gunSn);
        }
        break;    
        
        case CHARGER_STATE_GUNLOCK:
        {
            chargerMegg[gunSn].flag.bit.bStartCh = 1;
    	    sSysMegg[gunSn].unOut.bit.bGunLock = 1;
    	    set_charge_state(CHARGER_STATE_MOD_SWITCH,gunSn);
        }
        break;   

        case CHARGER_STATE_MOD_SWITCH:
        {
            int32_t delay = Project_Type_Delay();
    	    if(BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.modswitchcnt,delay))
    	    {
                set_charge_state(CHARGER_STATE_PRE_CHARGE,gunSn);
                sSysMegg[gunSn].unOut.bit.bBMS_V = 1;
    	    }
        }
        break;

        case CHARGER_STATE_PRE_CHARGE:
        {
            if(sSysConfig.sTempInt.LoadType == LOAD_BAT)
            {
                chargerMegg[gunSn].flag.bit.bStartMod = 1;
                chargerMegg[gunSn].v_demand_mod = sSysMegg[gunSn].v_Gun - 50;
                chargerMegg[gunSn].i_demand_mod = 50;                
                int16_t v_err = (sSysMegg[gunSn].v_Gun > sSysMegg[gunSn].v_Mod)?(sSysMegg[gunSn].v_Gun - sSysMegg[gunSn].v_Mod) \
                                :(sSysMegg[gunSn].v_Mod - sSysMegg[gunSn].v_Gun);
                if(v_err < 250)
                {
                    sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                    chargerMegg[gunSn].v_demand_mod = sSysMegg[gunSn].v_Gun;
                    set_charge_state(CHARGER_STATE_CHARGING,gunSn);
                }
            }
            else
            {
                sSysMegg[gunSn].unOut.bit.bDcCont_Out = 1;
                chargerMegg[gunSn].flag.bit.bStartMod = 1;
                set_charge_state(CHARGER_STATE_CHARGING,gunSn);
            }
        }
        break;

        case CHARGER_STATE_CHARGING:
        {
            charger_data_handle(gunSn);
        }
        break;        

        case CHARGER_STATE_STARTFAULT:
        {
            set_charge_state(CHARGER_STATE_END,gunSn);
        }
        break;   

        case CHARGER_STATE_STOP:
        case CHARGER_STATE_END:
        {
            charge_logic_off(gunSn);
            if((BOOL_TRUE == cnt_delay(&chargerDevice[gunSn].cnt.dcContCutcnt,500)) \
                && (sSysMegg[gunSn].i_Gun < 50))
            {
                charge_off_clear(gunSn);
                set_charge_state(CHARGER_STATE_IDLE,gunSn);
    	    }
        }
        break;               

        default:
        break;
    }
        
}

/**
  * @brief  step״̬ȷ��
  */
static void charger_step_handle(uint8_t gunSn)
{
    chargerMegg[gunSn].unStep.bit.idle = (chargerDevice[gunSn].chargerState == CHARGER_STATE_IDLE);
    chargerMegg[gunSn].unStep.bit.starting = ((chargerDevice[gunSn].chargerState > CHARGER_STATE_IDLE) \
                                        &&(chargerDevice[gunSn].chargerState < CHARGER_STATE_CHARGING));
    chargerMegg[gunSn].unStep.bit.charging = (chargerDevice[gunSn].chargerState == CHARGER_STATE_CHARGING);

    static int16_t overcnt[2] = {0};
    if(chargerDevice[gunSn].chargerState >= CHARGER_STATE_END_SAVE)
    {
        chargerMegg[gunSn].unStep.bit.over = 1;
        overcnt[gunSn] = 0;        
    }
    else
    {
        if(overcnt[gunSn]++ >= 5000)        
        {
            overcnt[gunSn] = 5000;
            chargerMegg[gunSn].unStep.bit.over = 0;
        }
    }

    if(chargerDevice[gunSn].chargerState == CHARGER_STATE_CHARGING)
    {
        chargerMegg[gunSn].unStep.bit.startok = 1;
    }
    else if(chargerDevice[gunSn].chargerState < CHARGER_STATE_CHARGING)
    {
        chargerMegg[gunSn].unStep.bit.startok = 0;
        chargerMegg[gunSn].unStep.bit.startfail = 0;
    }

    chargerMegg[gunSn].unStep.bit.bworking = chargerMegg[gunSn].unStep.bit.starting | chargerMegg[gunSn].unStep.bit.charging;
    chargerMegg[gunSn].unStep.bit.insdet = ((get_insdet_state(gunSn) != INSDET_STATE_IDLE)&&(get_insdet_state(gunSn) != INSDET_STATE_OVER));

    static uint16_t bakset[2] = {0};
    if(bakset[gunSn] != chargerMegg[gunSn].unStep.all)
    {
        DLOG("gun%d:CHstep:[%x->%x]",gunSn,bakset[gunSn],chargerMegg[gunSn].unStep.all);
        bakset[gunSn] = chargerMegg[gunSn].unStep.all;
    }
}

/**
  * @brief  ���״̬���仯����
  */
static void charger_state_common(uint8_t gunSn)
{
    if(chargerDevice[gunSn].chargerStatebak != chargerDevice[gunSn].chargerState)
    {
        DLOG("gun%d:chargerState change [%d]->[%d]",gunSn,chargerDevice[gunSn].chargerStatebak,chargerDevice[gunSn].chargerState);
        chargerDevice[gunSn].chargerStatebak = chargerDevice[gunSn].chargerState;
        char *buffer;
        buffer = get_charge_state_tip(chargerDevice[gunSn].chargerState);
        if(buffer)
        {
            DLOG("gun%d:%s",gunSn,buffer);        
        }
    }
}

/**
  * @brief  Զ�̡�����������
  */
static void charger_logic_handle(void)
{
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{

	    if(get_alarm_statue(ALARM_AX) != 0)
	    {
	        if((chargerDevice[gunSn].chargerState > CHARGER_STATE_STARTFAULT) \
	            &&(chargerDevice[gunSn].chargerState < CHARGER_STATE_END))
	        {
	            set_charge_state(CHARGER_STATE_END,gunSn);
	        }
	    }

	    if(chargerMegg[gunSn].chargerStopCause > 0)
	    {
	        if((chargerDevice[gunSn].chargerState > CHARGER_STATE_IDLE) \
	            &&(chargerDevice[gunSn].chargerState < CHARGER_STATE_CHARGING))
	        {
	            set_charge_state(CHARGER_STATE_STARTFAULT,gunSn);
	            chargerMegg[gunSn].unStep.bit.startfail = 1;
	        }
	        else if(chargerDevice[gunSn].chargerState == CHARGER_STATE_CHARGING)
	        {
	            set_charge_state(CHARGER_STATE_STOP,gunSn);
	        }
	    }     

	    if(sSysConfig.sTempInt.WorkMode==WORK_REMOTE)
	    {
	        switch (sSysConfig.sInt.bmsType)
	        {
	          case BMS_GB_2015_TYPE:
	              charger_state_logic_gb2015(gunSn);
	              break;
	          default:
	              charger_state_logic_all(gunSn);
	              break;
	        }
	    }
	    else
	    {
	        charger_state_logic_local(gunSn);    
	    }
	    
	    charger_step_handle(gunSn);
	    charger_state_common(gunSn);
	}
    
}

/**
  * @brief  �ϵ��ʼ��
  */
void chargeTaskInit(void)
{
    vSys_Basic_Init();
    alarm_init();
	sys_config_init(SYS_CONFIG_VALID_FLAG);
    memset(&sChargerTaskClockCnt,0,sizeof(sChargerTaskClockCnt));
	
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
    	memset(&chargerDevice[gunSn],0,sizeof(chargerDevice[gunSn]));
    	memset(&chargerMegg[gunSn],0,sizeof(chargerMegg[gunSn]));
	}
    unChargerTaskClockFlag.all = 0;
}

/**
  * @brief  ��¼���ݴ���
  */
void record_data_deal(void)
{
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    chargerMegg[gunSn].record_data.v_insdet = chargerDevice[gunSn].sInsdet.v_InsDet;
	    if(unChargerTaskClockFlag.bit.b1s)
	    {
	        chargerMegg[gunSn].record_data.v_mod[1] = chargerMegg[gunSn].record_data.v_mod[0];
	        chargerMegg[gunSn].record_data.v_mod[0] = sSysMegg[gunSn].v_Mod;
	        chargerMegg[gunSn].record_data.v_gun[1] = chargerMegg[gunSn].record_data.v_gun[0];
	        chargerMegg[gunSn].record_data.v_gun[0] = sSysMegg[gunSn].v_Gun;
	        chargerMegg[gunSn].record_data.i_gun[1] = chargerMegg[gunSn].record_data.i_gun[0];
	        chargerMegg[gunSn].record_data.i_gun[0] = sSysMegg[gunSn].i_Gun;
	        chargerMegg[gunSn].record_data.v_demand[1] = chargerMegg[gunSn].record_data.v_demand[0];
	        chargerMegg[gunSn].record_data.v_demand[0] = chargerMegg[gunSn].v_demand;
	        chargerMegg[gunSn].record_data.i_demand[1] = chargerMegg[gunSn].record_data.i_demand[0];
	        chargerMegg[gunSn].record_data.i_demand[0] = chargerMegg[gunSn].i_demand;
	        chargerMegg[gunSn].record_data.v_bcs[1] = chargerMegg[gunSn].record_data.v_bcs[0];
	        chargerMegg[gunSn].record_data.v_bcs[0] = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureVoltage;
	        chargerMegg[gunSn].record_data.i_bcs[1] = chargerMegg[gunSn].record_data.i_bcs[0];
	        chargerMegg[gunSn].record_data.i_bcs[0] = bmsDevice[gunSn].frameData.bcsData.bcsChargerMeasureCurrent;
	    }

	    if(get_charge_state(gunSn)>=CHARGER_STATE_CHARGING)
	    {    
	        uint8_t v_single = chargerMegg[gunSn].record_data.v_max_bat&0xff;
	        v_single = BIGGER(v_single,bmsDevice[gunSn].frameData.bcsData.bcsSingleInfo.bit.bMaxSingleVol/10);
	        chargerMegg[gunSn].record_data.v_max_bat = ((bmsDevice[gunSn].frameData.bcpData.bcpSingleMaxVol/10)<<8)+v_single;
	        
	        //chargerMegg[gunSn].record_data.i_max = BIGGER(chargerMegg[gunSn].record_data.i_max,dlt645Megg.meter.aPhaseCur_10x);    
	        chargerMegg[gunSn].record_data.t_max = BIGGER(chargerMegg[gunSn].record_data.t_max,bmsDevice[gunSn].frameData.bsmData.bsmMaxTemperature);    
	    }
	    else
	    {
	        chargerMegg[gunSn].record_data.v_max_bat = 0;
	        chargerMegg[gunSn].record_data.i_max = 0;    
	        chargerMegg[gunSn].record_data.t_max = 0;
	    }
	}
}

void record_debug(void)
{
    sys_debug_set(DEBUG1_PROCESS);

    static uint32_t cycle_total = 0;

    static uint32_t cycle = 0;

    if(cycle_total > 5130)//MAX_CHARGER_RECORD_NUM
    {
        return;
    }
    
    cycle++;
    if(cycle==20)
    {
        start_charger_interface(START_BY_ADMIN,0);
    
    }
    else if(cycle == 50000)    //5000
    {
        static uint32_t stopCause = 0;
        stopCause++;
        if(stopCause==ALARM_END_CODE)
        {
            stopCause = STOP_UNDEFINE;
        }
        else if(stopCause==STOP_TCU_UNKNOWN)
        {
            stopCause = BMS_STOP_CAUSE_CHARGE_STARTUP_TIMEOUT_CRM;
        }
        else if(stopCause>BMS_STOP_CAUSE_RECV_BST_OHTER)
        {
            stopCause = ALARM_EPO;
        }        
        set_charger_stopcause(stopCause,0);
    }
    else if(cycle > 100000)
    {
        cycle = 0;
        cycle_total++;
    }
    
}

void charger_task_main(void)
{
    sys_clock_handle(&sChargerTaskClockCnt,&unChargerTaskClockFlag);
    alarm_main();
    charger_tick_handle();
    charger_logic_handle();
    record_data_deal();
    //record_debug();
	sys_debug_set(DEBUG1_PROCESS);   //��������ʱ  �˴���Ҫ����
	vCcu_IO_Ctr();
}


