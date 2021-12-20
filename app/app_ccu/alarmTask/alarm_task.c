
/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�
�ļ�:   alarm_stopcause
����:   km
˵��: alarm deal.
***************************************************************************************************/

/******************************************************************************************
@note:
�����жϷ�Ϊ��
1�����ݵ�������ɼ�������Ϣ
2�������ն���ǹ���͹�����Ϣ
3������1ǹ���͹�����Ϣ
4�����������������Ϣ(����)

������ã�sSysConfig.sInt.uSysType
******************************************************************************************/

#include<string.h>
#include "alarm_task.h"
#include "charge_task.h"
#include "sys_config.h"
#include "io_megg.h"
#include "log.h"
#include "time_cur.h"
#include "tick_handle.h"         
#include "dcharger_version.h"

StructAlarmMegg sAlarmMegg;
static uint8_t  alarmDataBitValue[ALARM_END_CODE]={0};

const StructStopCodeMap  alarmStopCauseCodeMap[]=
{
    {ALARM_STOP_NULL,       NULL,                       ALARM_TYPE_NULL},      

    //����+����    
    {ALARM_EPO,                "��ͣ����",                         ALARM_TYPE_FAULT},
    {ALARM_BREAKER,            "�����·������",                      ALARM_TYPE_FAULT},
    {ALARM_FUSE,               "����۶�������",                      ALARM_TYPE_FAULT},
    {ALARM_FAN,                "���ȹ���",                         ALARM_TYPE_FAULT},
    {ALARM_SPD,                "����������",                        ALARM_TYPE_FAULT},
    {ALARM_OUTLETTEMPUP,       "���׮���¹���",                      ALARM_TYPE_FAULT},
    {ALARM_VGRIDUP,            "�����ѹ����",                       ALARM_TYPE_FAULT},
    {ALARM_VGRIDLOW,           "����Ƿѹ����",                       ALARM_TYPE_FAULT},

	{ALARM_DOOR,               "�Ž�����",                         ALARM_TYPE_FAULT},
    {ALARM_ACCONT,             "����Ӵ�������",                      ALARM_TYPE_FAULT},
    {ALARM_AX,                 "��Դ����",                         ALARM_TYPE_FAULT},
    {ALARM_MIDCONT,            "�м�Ӵ�������",                      ALARM_TYPE_FAULT},
	{ALARM_TCUCOM,			   "TCUͨ��ʧ��",					   ALARM_TYPE_FAULT},
	{ALARM_MODCOMFAULT, 	   "ģ��ͨ�Ź���",					   ALARM_TYPE_FAULT},
	{ALARM_METER_COM,		   "���ͨ�Ź���",					   ALARM_TYPE_FAULT},
	{ALARM_OUTLETTEMPUP_2,	   "���׮���¸澯", 					   ALARM_TYPE_ABS},
	
    {ALARM_MOD,                "ģ�����",                         ALARM_TYPE_FAULT},
    {ALARM_MOD_AC,             "ģ������澯",                         ALARM_TYPE_ABS},
    {ALARM_MOD_ACUP,           "ģ�������ѹ�澯",                       ALARM_TYPE_ABS},
    {ALARM_MOD_ACLOW,          "ģ������Ƿѹ�澯",                       ALARM_TYPE_ABS},
    {ALARM_MOD_ACLOSE,         "ģ������ȱ��澯",                       ALARM_TYPE_ABS},
    {ALARM_MOD_SHORT,          "ģ�������·�澯",                       ALARM_TYPE_ABS},
    {ALARM_MOD_DCOI,           "ģ����������澯",                       ALARM_TYPE_ABS},
    {ALARM_MOD_DCUP,           "ģ�������ѹ�澯",                       ALARM_TYPE_ABS},
    
    {ALARM_MOD_DCLOW,          "ģ�����Ƿѹ�澯",                       ALARM_TYPE_ABS},
    {ALARM_MOD_OT,             "ģ����¸澯",                         ALARM_TYPE_ABS},
    {ALARM_MOD_COM,            "ģ��ͨ�Ÿ澯",                         ALARM_TYPE_ABS},
    {ALARM_MOD_FAN,            "ģ����ȸ澯",                         ALARM_TYPE_ABS},
    {ALARM_RSV1,               "Ԥ��1",                          ALARM_TYPE_FAULT},
    {ALARM_RSV2,               "Ԥ��2",                          ALARM_TYPE_FAULT},
    {ALARM_RSV3,               "Ԥ��3",                          ALARM_TYPE_FAULT},
	{ALARM_RSV4,			   "Ԥ��4",						   ALARM_TYPE_FAULT},

	//gun1
    {ALARM_DCCONT_GUN1,        "ǹ1ֱ���Ӵ�������",               ALARM_TYPE_GUN1_FAULT},
    {ALARM_GunLock_GUN1,       "ǹ1���ǹ������",                ALARM_TYPE_GUN1_FAULT},

    {ALARM_INSDETFAIL_GUN1,    "ǹ1��Ե����",                  ALARM_TYPE_GUN1_FAULT},
    {ALARM_VDCINV_GUN1,        "ǹ1��ط��ӹ���",                ALARM_TYPE_GUN1_FAULT},
    {ALARM_CC1_GUN1,           "ǹ1���Ƶ�������",                ALARM_TYPE_GUN1_FAULT},
	{ALARM_GUNTEMPUP_GUN1,	   "ǹ1���ǹ���¹���", 			  ALARM_TYPE_GUN1_FAULT},
	{ALARM_GUNNOTBACK_GUN1,    "ǹ1���ǹδ��λ",					     ALARM_TYPE_ABS},
	{ALARM_BMS_COM_GUN1,	   "ǹ1BMSͨ�Ź���",				  ALARM_TYPE_GUN1_FAULT},
	{ALARM_VDCUP_GUN1,		   "ǹ1ֱ����ѹ����",				  ALARM_TYPE_GUN1_FAULT},
	{ALARM_VDCLOW_GUN1, 	   "ǹ1ֱ��Ƿѹ����",				  ALARM_TYPE_GUN1_FAULT},
	{ALARM_IDCUP_GUN1,		   "ǹ1ֱ����������",				  ALARM_TYPE_GUN1_FAULT},
	{ALARM_INSDETABS_GUN1,	   "ǹ1��Ե���澯",					     ALARM_TYPE_ABS},
	{ALARM_GUNTEMPUP_2_GUN1,   "ǹ1���ǹ���¸澯", 					 ALARM_TYPE_ABS},
	{ALARM_SHORT_GUN1,		   "ǹ1�����·����",				  ALARM_TYPE_GUN1_FAULT},
	{ALARM_DISCHARGE_GUN1,	   "ǹ1й�Ż�·����",			      ALARM_TYPE_GUN1_FAULT},
	{ALARM_RSV5,			   "Ԥ��5",						   ALARM_TYPE_FAULT},

	//gun2
	{ALARM_DCCONT_GUN2,        "ǹ2ֱ���Ӵ�������",               ALARM_TYPE_GUN2_FAULT},
    {ALARM_GunLock_GUN2,       "ǹ2���ǹ������",                ALARM_TYPE_GUN2_FAULT},
    {ALARM_INSDETFAIL_GUN2,    "ǹ2��Ե����",                  ALARM_TYPE_GUN2_FAULT},
    {ALARM_VDCINV_GUN2,        "ǹ2��ط��ӹ���",                ALARM_TYPE_GUN2_FAULT},
    {ALARM_CC1_GUN2,           "ǹ2���Ƶ�������",                ALARM_TYPE_GUN2_FAULT},
    {ALARM_GUNTEMPUP_GUN2,     "ǹ2���ǹ���¹���",               ALARM_TYPE_GUN2_FAULT},
    {ALARM_GUNNOTBACK_GUN2,    "ǹ2���ǹδ��λ",                       ALARM_TYPE_ABS},
    {ALARM_BMS_COM_GUN2,       "ǹ2BMSͨ�Ź���",               ALARM_TYPE_GUN2_FAULT},
    {ALARM_VDCUP_GUN2,         "ǹ2ֱ����ѹ����",                ALARM_TYPE_GUN2_FAULT},
    {ALARM_VDCLOW_GUN2,        "ǹ2ֱ��Ƿѹ����",                ALARM_TYPE_GUN2_FAULT},
    {ALARM_IDCUP_GUN2,         "ǹ2ֱ����������",                       ALARM_TYPE_ABS},
    {ALARM_INSDETABS_GUN2,     "ǹ2��Ե���澯",                       ALARM_TYPE_ABS},
    {ALARM_GUNTEMPUP_2_GUN2,   "ǹ2���ǹ���¸澯",                      ALARM_TYPE_ABS},
	{ALARM_SHORT_GUN2,		   "ǹ2�����·����",				  ALARM_TYPE_GUN2_FAULT},
    {ALARM_DISCHARGE_GUN2,	   "ǹ2й�Ż�·����",				  ALARM_TYPE_GUN2_FAULT},
	{ALARM_RSV6,			   "Ԥ��6",						   ALARM_TYPE_FAULT},
    

    //������
    {STOP_UNDEFINE,                      "δֹ֪ͣ",                            ALARM_TYPE_NULL},
    {STOP_BY_CARD,                       "ˢ��ֹͣ",                            ALARM_TYPE_NULL},
    {STOP_BY_USER,                       "�û�����ֹͣ",                          ALARM_TYPE_NULL},
    {STOP_BY_ENERGY,                     "�����ﵽ�趨",                          ALARM_TYPE_NULL},
    {STOP_BY_TIME,                       "ʱ��ﵽ�趨",                          ALARM_TYPE_NULL},
    {STOP_BY_MONEY,                      "���ﵽ�趨",                          ALARM_TYPE_NULL},    
    {STOP_BY_MONEY_NOTENOUGH,            "����",                            ALARM_TYPE_NULL},
    {STOP_BY_BACKGROUP,                  "��ֹ̨ͣ",                            ALARM_TYPE_NULL},
    {STOP_NET_DISCONNECT,                "��̨�Ͽ�����",                          ALARM_TYPE_NULL},
    {STOP_VIN_ERROR,                     "VIN�벻ƥ��",                         ALARM_TYPE_NULL},
    {STOP_BY_ADMIN,                      "����Ա������ֹ",                         ALARM_TYPE_NULL},
    {STOP_INSDET_VBEFORE,                "��Ե���׶Σ�ǹ�ڴ�ѹ",                     ALARM_TYPE_NULL},
    {STOP_INSDET_TO1,                    "��Ե��ʱ��ģ��δ����",                      ALARM_TYPE_NULL},
    {STOP_INSDET_TO2,                    "��Ե��ʱ��ģ�鿪��",                       ALARM_TYPE_NULL},
    {STOP_BMS_VINSDET,                   "BMS��Ե��ѹ��",                        ALARM_TYPE_NULL},
    {STOP_BCP_BATVOLAGE,                 "BCP��ص�ѹ�쳣",                       ALARM_TYPE_NULL},
    {STOP_SOC_PROTECTION,                "SOC�ﵽ�û�����ֵ",                      ALARM_TYPE_NULL},
    {STOP_BMS_SINGLE_VUP,                "BMS�����ѹ����",                       ALARM_TYPE_NULL},
    {STOP_BMS_TEMP_MAX,                  "BMS�¶ȹ���",                         ALARM_TYPE_NULL},
    {STOP_BMS_STAUE,                     "BMS״̬�쳣",                         ALARM_TYPE_NULL},
    {STOP_BMS_BCSVOL,                    "BCS�ϴ���ѹ�쳣",                       ALARM_TYPE_NULL},
    {STOP_BSM_VSINGLE,                   "BMS ���嶯�����ص�ѹ",                   ALARM_TYPE_NULL},
    {STOP_BSM_SOC,                       "BMS �����������غɵ�״̬",                 ALARM_TYPE_NULL},
    {STOP_BSM_OC,                        "BMS �������س�������",                  ALARM_TYPE_NULL},
    {STOP_BSM_TEMPUP,                    "BMS ���������¶ȹ���",                   ALARM_TYPE_NULL},
    {STOP_BSM_INSDET,                    "BMS �������ؾ�Ե״̬",                   ALARM_TYPE_NULL},
    {STOP_BSM_CC,                        "BMS ���������������״̬",                 ALARM_TYPE_NULL},
    {STOP_BSM_BATLOW,                    "BMS ���嶯�����ص�ѹ����",                 ALARM_TYPE_NULL},
    {STOP_BSM_SOCLOW,                    "BMS ������������SOC����",                ALARM_TYPE_NULL},
    {STOP_VOLTAGE_BMSMAX,                "����ѹ����BMS�������ֵ",                  ALARM_TYPE_NULL},
    {STOP_VOLTAGE_BMSDEMAND,             "����ѹ����BMS����ֵ",                    ALARM_TYPE_NULL},
    {STOP_BAT_CIRCUIT,                   "�����ڲ���·�쳣��ֹ",                      ALARM_TYPE_NULL},
    {STOP_BAT_DEMAND_VOLTAGE,            "���������ѹ��������",                      ALARM_TYPE_NULL},
    {STOP_BAT_DEMAND_CURRENT,            "�������������������",                      ALARM_TYPE_NULL},
    {STOP_SOFTFAIL_MOD_START,            "����ʧ��,ģ�鿪��",                       ALARM_TYPE_NULL},
    {STOP_SOFTFAIL_MOD_UNSTART,          "����ʧ��,ģ��δ����",                      ALARM_TYPE_NULL},
    {STOP_SOFTFAIL_TIMEOUT,              "����ʧ��,��ʱ",                         ALARM_TYPE_NULL},
    
    {STOP_TCU_UPDATE,                    "TCU����",                           ALARM_TYPE_NULL},
    {STOP_TCU_CTRL_ABS,                  "TCU ���ư�״̬�쳣",                     ALARM_TYPE_NULL},
    {STOP_TCU_UNKNOWN,                   "TCU δ֪�Ľ���ԭ��",                     ALARM_TYPE_NULL},

    //BMS����ֹͣ
    {BMS_STOP_CAUSE_CHARGE_STARTUP_TIMEOUT_CRM,     "����ʱ�䳬ʱ",               ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BRM_TIMEOUT,               "�յ�BRM���ĳ�ʱ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BCP_TIMEOUT,               "�յ�BCP���ĳ�ʱ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BRO_TIMEOUT,               "�յ�BR0���ĳ�ʱ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BCS_TIMEOUT,               "�յ�BCS���ĳ�ʱ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BCL_TIMEOUT,               "����BCL���ĳ�ʱ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BSM_TIMEOUT,               "����BSM���ĳ�ʱ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BEM_FRAME,                 "�յ�BEM����ֹͣ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_TIMEOUT,               "�յ�BST���ĳ�ʱ",            ALARM_TYPE_NULL},
    
    {BMS_STOP_CAUSE_BSM_CHARGER_ALLOW_TIMEOUT,      "BSM�������쳣",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_BRO_ABS,                        "BRO׼�������쳣",            ALARM_TYPE_NULL},
    
    {BMS_STOP_CAUSE_RECV_BST_0_00,                  "BST��SOCĿ��ֵ",           ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_0_02,                  "BST�ĵ�ѹ�趨ֵ",            ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_0_04,                  "BST�����ѹ��",             ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_0_06,                  "BST_00_6δ����",          ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_1_00,                  "BST��Ե����",              ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_1_02,                  "BST����������",             ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_1_04,                  "BSTԪ������",              ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_1_06,                  "BST����������",             ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_2_00,                  "BST��������",             ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_2_02,                  "BST��������",              ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_2_04,                  "BST_02_4δ����",          ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_2_06,                  "BST_02_6δ����",          ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_3_00,                  "BST��������",              ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_3_02,                  "BST��ѹ�쳣",              ALARM_TYPE_NULL},
    {BMS_STOP_CAUSE_RECV_BST_OHTER,                 "BSTδ֪ͣ��",              ALARM_TYPE_NULL},

};

/**
  * @brief  ֹͣԭ���ӦTip
  */
char* get_alarm_stopcause_tips(uint16_t index)
{    
    int32_t mid = 0;
    int32_t low = 0;
    int32_t high = (sizeof(alarmStopCauseCodeMap)/sizeof(StructStopCodeMap))-1;
    
    while(low <= high)
    {
        mid=(high+low)/2;

        if(alarmStopCauseCodeMap[mid].originalCode > index)
            high = mid - 1;
        else  if(alarmStopCauseCodeMap[mid].originalCode < index)
            low = mid + 1;
        else 
            return (char*)alarmStopCauseCodeMap[mid].tip;
    }
    DLOG("δ�ҵ�alarmtip[%d]",index);
    return  NULL;
}

/**
  * @brief  ��ȡ�澯״̬
  * @param  ENUM_ALARM_STOPǰ�벿�ָ澯����
  * @retval 0 ���� 1 �澯
  */
uint8_t get_alarm_statue(ENUM_ALARM_STOP index)
{
    if((index>=ALARM_END_CODE)||(index==ALARM_STOP_NULL))
    {
        return 0;
    }

    return alarmDataBitValue[index];
}

/**
  * @brief  ���ø澯״̬
  * @param  ENUM_ALARM_STOPǰ�벿�ָ澯����, uset 0 ���� 1 �澯
  */
void set_alarm_statue(ENUM_ALARM_STOP index,uint8_t uset)
{
    if((index>=ALARM_END_CODE)||(index==ALARM_STOP_NULL))
    {
        return;
    }

    int32_t para1 = 0;
    int32_t para2 = 0;
    int32_t para3 = 0;
    
	switch (index)
	{
		case ALARM_OUTLETTEMPUP:     //���׮���¹���
        case ALARM_OUTLETTEMPUP_2:   //���׮���¸澯---�澯
        {
            para1 = sSysMegg[0].t_Outlet;
        }
        break;

		case ALARM_GUNTEMPUP_GUN1:        //���ǹ���¹���
        case ALARM_GUNTEMPUP_2_GUN1:      //���ǹ���¸澯---�澯
		{
            para1 = sSysMegg[0].t_Gun;
		}
		break;

		case ALARM_GUNTEMPUP_GUN2:        //���ǹ���¹���
        case ALARM_GUNTEMPUP_2_GUN2:      //���ǹ���¸澯---�澯
		{
            para1 = sSysMegg[1].t_Gun;
		}
		break;
		
		case ALARM_VGRIDUP:          //�����ѹ����
		case ALARM_VGRIDLOW:         //����Ƿѹ����
        {
            para1 = sSysMegg[0].vab_10x;
            para2 = sSysMegg[0].vbc_10x;
            para3 = sSysMegg[0].vac_10x;
        }
        break;
        
		case ALARM_VDCINV_GUN1:           //��ط��ӹ���
		case ALARM_VDCUP_GUN1:            //ֱ����ѹ����
		case ALARM_VDCLOW_GUN1:           //ֱ��Ƿѹ����
		{
            para1 = sSysMegg[0].v_Gun;
		}
		break;

	    case ALARM_VDCINV_GUN2:           //��ط��ӹ���
		case ALARM_VDCUP_GUN2:            //ֱ����ѹ����
		case ALARM_VDCLOW_GUN2:           //ֱ��Ƿѹ����
		{
            para1 = sSysMegg[1].v_Gun;
		}
		break;
				
		case ALARM_IDCUP_GUN1:            //ֱ����������
		{
            para1 = sSysMegg[0].i_Gun;
		}
		break;

		case ALARM_IDCUP_GUN2:            //ֱ����������
		{
            para1 = sSysMegg[1].i_Gun;
		}
		break;

		default:
			break;
	}
	uint8_t alarm = alarmDataBitValue[index];
    alarmDataBitValue[index] = uset;
    if(alarm != uset)
    {
        DLOG("%s[%d->%d],para[%d,%d,%d]",get_alarm_stopcause_tips(index),alarm,uset,para1,para2,para3);
		
        //DLOG("[%d->%d],para[%d,%d,%d]",alarm,uset,para1,para2,para3);
        if(uset)
        {
            //charg_alarm_add(index);   //������ϼ�¼
        }
        else
        {
            //charg_alarm_del(index);
        }
    }
}

/**
  * @brief  ��Ҫ������Ӧ����.
  * @retval BOOL_TRUE��Ҫ��BOOL_FALSE����Ҫ
  */
BOOL get_alarm_fast_response(void)
{
    if((get_alarm_statue(ALARM_DOOR) == 1)||(get_alarm_statue(ALARM_EPO) == 1))
    {
        return BOOL_TRUE;
    }
    return BOOL_FALSE;
}

static void alarm_sys_epo(void)
{
    uint8_t flag = get_In_IO(ENUM_IN_EPO,GUN_SN_ONE);
    static int32_t epocnt = 0;
    int8_t st = cnt_change_until_threshold(flag,&epocnt,10000,1000,1);
    if(st != 2)
    {
        set_alarm_statue(ALARM_EPO,st);
    }    
}

static void alarm_sys_breaker(void)
{
    static int32_t breakercnt = 0;
    int8_t st = 0;

    uint8_t flag = get_In_IO(ENUM_IN_BREAKER,GUN_SN_ONE);
    st = cnt_change_until_threshold(flag,&breakercnt,10000,10,1);
    if(st != 2)
    {
        set_alarm_statue(ALARM_BREAKER,st);
    } 
}

static void alarm_dccont_gun(void)
{
    uint8_t flag = 0;
    static int32_t dccontcnt[GUN_SN_MAX] = {0};
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;
    
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_DCCONT_GUN1:ALARM_DCCONT_GUN2;
	    flag = get_In_IO(ENUM_IN_DCCONT,gunSn);
	    flag = sSysMegg[gunSn].unOut.bit.bDcCont_Out ^ flag;

	    dccontcnt[gunSn] = (flag!=0)?(dccontcnt[gunSn]+10):(dccontcnt[gunSn]-1);

	    if(dccontcnt[gunSn] >= 10000)
	    {
	        dccontcnt[gunSn] = 10000;
	        set_alarm_statue(index,1);
	    }
	    else if(dccontcnt[gunSn] <= 0)
	    {
	        dccontcnt[gunSn] = 0;
	        set_alarm_statue(index,0);
	    }
	}
}

static void alarm_fuse(void)
{
    uint8_t flag = get_In_IO(ENUM_IN_FUSE,GUN_SN_ONE);
    static int32_t fusecnt = 0;
    int8_t st = cnt_change_until_threshold(flag, &fusecnt, 10000,10,1);

    if(st != 2)
    {
        set_alarm_statue(ALARM_FUSE,st);
    }
}

#ifdef GUNLOCK_CHECK_ALWAYS
/**
  * @brief  ����м��ǹ������,Ĭ������,����Ҫ��������ſ���
  */
static void alarm_gunlock(uint8_t gunSn)
{
    static int32_t gunlockcnt_a[GUN_SN_MAX] = {0};
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
    {
	    index = (gunSn == 0)?ALARM_GunLock_GUN1:ALARM_GunLock_GUN2;
	    uint8_t flag = get_In_IO(ENUM_IN_GUNLOCK,gunSn);
	    flag = sSysMegg[gunSn].unOut.bit.bGunLock ^ flag;
	    int8_t st = cnt_change_until_threshold(flag, &gunlockcnt_a[gunSn], 10000,5,1);

	    if(st != 2)
	    {
	        set_alarm_statue(index,st);
	    }
	}
}

/**
  * @brief  ֻ�ڱ�������ʱ���ǹ������,�˴�ֻ��������ϴ��������ж���CTRL_GUNLOCK()
  * @20.04.21 �������������CTRL_GUNLOCK
  */
static void alarm_gunlock(void)
{
    static int32_t gunlockcnt[GUN_SN_MAX] = {0};
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_GunLock_GUN1:ALARM_GunLock_GUN2;
	    if(get_alarm_statue(index))
	    {
	        uint8_t flag = get_In_IO(ENUM_IN_GUNLOCK,gunSn);
	        flag = sSysMegg[gunSn].unOut.bit.bGunLock^flag;
	        if(flag == 0)
	        {
	            gunlockcnt[gunSn]++;
	            if(gunlockcnt[gunSn] > 10000)
	            {
	                gunlockcnt[gunSn] = 0;
	                set_alarm_statue(index,0);
	            }
	        }
	    }
	    else
	    {
	        gunlockcnt = 0;
	    }
	}
}
#endif

static void alarm_spd(void)
{
    uint8_t flag = get_In_IO(ENUM_IN_SPD,GUN_SN_ONE);
    static int32_t spdcnt = 0;

    int8_t st = cnt_change_until_threshold(flag,&spdcnt,10000,10,1);
    if(st != 2)
    {
        set_alarm_statue(ALARM_SPD,st);
    } 

}

static void alarm_vdc_inv(void)
{
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_VDCINV_GUN1:ALARM_VDCINV_GUN2;

        if(get_alarm_statue(index))
        {
            if(sSysMegg[GUN_SN_ONE].CC1 ==  0)
            {
                set_alarm_statue(index,0);
            }
        }
	}
}

static void alarm_vdc_cc1(void)
{
    static int32_t cc1cnt[GUN_SN_MAX] = {0};
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_CC1_GUN1:ALARM_CC1_GUN2;

		if(chargerMegg[gunSn].unStep.bit.bworking)
        {
            set_alarm_statue(index,(sSysMegg[gunSn].CC1 == 0));
            cc1cnt[gunSn] = 0;
        }

        if(get_charge_state(gunSn) == CHARGER_STATE_IDLE)
        {
            cc1cnt[gunSn]++;
            if(cc1cnt[gunSn] >= 10000)
            {
                cc1cnt[gunSn] = 10000;
                set_alarm_statue(index,0);
            }
        }
    }
}

static void alarm_ot_out(void)
{
    static int32_t ot_outcnt = 0;

    uint8_t flag = (0 != get_alarm_statue(ALARM_OUTLETTEMPUP))?(sSysMegg[GUN_SN_ONE].t_Outlet >= 650):(sSysMegg[GUN_SN_ONE].t_Outlet >= sSysConfig.sInt.uOutletTempUp_10x);
    
    int8_t st = cnt_change_until_threshold(flag,&ot_outcnt,30000,10,1);
    if(st != 2)
    {
        set_alarm_statue(ALARM_OUTLETTEMPUP,st);
    } 
}

static void alarm_ot_gun(void)
{
    static int32_t ot_guncnt[GUN_SN_MAX] = {0};

	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_GUNTEMPUP_GUN1:ALARM_GUNTEMPUP_GUN2;
		
	    if(sSysMegg[gunSn].t_Gun >= sSysConfig.sInt.uGunTempProtect)
	    {
	        ot_guncnt[gunSn] += 10;
	        if(ot_guncnt[gunSn] >= 30000)
	        {
	            ot_guncnt[gunSn] = 30000;
	            set_alarm_statue(index,1);
	        }
	    }
	    else if(sSysMegg[gunSn].t_Gun <= 650)
	    {
	        if(ot_guncnt[gunSn]-- <= 0)
	        {
	            ot_guncnt[gunSn] = 0;
	            set_alarm_statue(index,0);
	        }
	    }
	}
}

static void alarm_vac_up(void)
{
    static int32_t vac_upcnt = 0;
    
    if(get_accont_statue() == BOOL_FALSE)
    {
        vac_upcnt = 0;
        set_alarm_statue(ALARM_VGRIDUP,0);
        return;
    }

    uint16_t vmax = BIGGER(sSysMegg[GUN_SN_ONE].vab_10x,sSysMegg[GUN_SN_ONE].vbc_10x);
    vmax = BIGGER(vmax,sSysMegg[GUN_SN_ONE].vac_10x);

    if(vmax >= sSysConfig.sInt.uAcOverVolt)
    {
        if(vac_upcnt++ >= 10000)
        {
            vac_upcnt = 10000;
            set_alarm_statue(ALARM_VGRIDUP,1);
        }
    }
    else if(vmax <= (sSysConfig.sInt.uAcOverVolt-100))
    {
        if(vac_upcnt-- <= 0)
        {
            vac_upcnt = 0;
            set_alarm_statue(ALARM_VGRIDUP,0);
        }
    }

}

static void alarm_vac_low(void)
{
    static int32_t vac_lowcnt = 0;
    uint8_t flag2 = 0;

	#if 0       
    for(uint8_t u8ModCnt = 1; u8ModCnt <MOD_MAX_ADDR;u8ModCnt ++)    //��ģ��Э��ȷ��
    {
        if(sModMegg_SH.sModRxMegg.unFlagModSt[u8ModCnt].bit.bOnLine==1)
        {
            flag2 = 1;
            break;
        }
    }
	#else     //��ǰĬ��ģ�����ߣ�����ģ��Э�����������޸�
	flag2 = 1;
	#endif 

    if((get_accont_statue() == BOOL_FALSE)||(flag2 == 0))
    {
        vac_lowcnt = 0;
        set_alarm_statue(ALARM_VGRIDLOW,0);
        return;
    }
    uint16_t vmin = SMALLER(sSysMegg[GUN_SN_ONE].vab_10x,sSysMegg[GUN_SN_ONE].vbc_10x);
    vmin = SMALLER(vmin,sSysMegg[GUN_SN_ONE].vac_10x);

    if(vmin <= sSysConfig.sInt.uAcUnderVolt)
    {
        if(vac_lowcnt++ >= 10000)
        {
            vac_lowcnt = 10000;
            set_alarm_statue(ALARM_VGRIDLOW,1);
        }
    }
    else if(vmin >= (sSysConfig.sInt.uAcUnderVolt+100))
    {
        if(vac_lowcnt-- <= 0)
        {
            vac_lowcnt = 0;
            set_alarm_statue(ALARM_VGRIDLOW,0);
        }
    }    

}

static void alarm_vdc_up(void)
{    
    static int32_t vdc_upcnt[GUN_SN_MAX] = {0};
	uint8_t flag = 0;
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
		index = (gunSn == 0)?ALARM_VDCUP_GUN1:ALARM_VDCUP_GUN2;
	    if(chargerMegg[gunSn].unStep.bit.bworking)
	    {
	        flag = (sSysMegg[gunSn].v_Gun >= sSysConfig.sInt.uDcOverVolt);

	        if(1 == cnt_change_until_threshold(flag, &vdc_upcnt[gunSn], 10000, 1, 1))
	        {
	            set_alarm_statue(index,1);
	        }
	    }

	    if(get_alarm_statue(index))
	    {
	        if(sSysMegg[gunSn].CC1 ==  0)
	        {
	            vdc_upcnt[gunSn] = 0;
	            set_alarm_statue(index,0);
	        }
	    }    
	}
}

static void alarm_vdc_low(void)
{
    static int32_t vdc_lowcnt[GUN_SN_MAX] = {0};
	uint8_t flag = 0;
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_VDCLOW_GUN1:ALARM_VDCLOW_GUN2;
	    if((chargerMegg[gunSn].unStep.bit.charging)&&(sSysConfig.sTempInt.LoadType == LOAD_BAT))
	    {
	        flag = (sSysMegg[gunSn].v_Gun <= sSysConfig.sInt.uDcUnderVolt);

	        if(1 == cnt_change_until_threshold(flag, &vdc_lowcnt[gunSn], 60000, 1, 1))
	        {
	            set_alarm_statue(index,1);
	        }
	    }

	    if(get_alarm_statue(index))
	    {
	        if(sSysMegg[gunSn].CC1 ==  0)
	        {
	            vdc_lowcnt[gunSn]= 0;
	            set_alarm_statue(index,0);
	        }
	    } 
	}
}

static void alarm_idc_up(void)
{
    static int32_t idc_upcnt[GUN_SN_MAX] = {0};
	uint8_t flag = 0;
	ENUM_ALARM_STOP index = ALARM_STOP_NULL;

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    index = (gunSn == 0)?ALARM_IDCUP_GUN1:ALARM_IDCUP_GUN2;
		
	    if(chargerMegg[gunSn].unStep.bit.charging)
	    {
	        flag = (sSysMegg[gunSn].i_Gun >= sSysConfig.sInt.uDcoverCurr);

	        if(1 == cnt_change_until_threshold(flag, &idc_upcnt[gunSn], 10000, 1, 1))
	        {
	            set_alarm_statue(index,1);
	        }
	    }

	    if(get_alarm_statue(index))
	    {
	        if(sSysMegg[gunSn].CC1 ==  0)
	        {
	            idc_upcnt[gunSn] = 0;
	            set_alarm_statue(index,0);
	        }
	    }  
	}

}

static void alarm_door(void)
{
    static int32_t doorcnt = 0;
    if(sSysConfig.sInt.uDoorProtectEn)
    {
        uint8_t flag = get_In_IO(ENUM_IN_DOOR,GUN_SN_ONE);
        int8_t st = cnt_change_until_threshold(flag,&doorcnt,10000,1000,1);
        if(st != 2)
        {
            set_alarm_statue(ALARM_DOOR,st);
        }  
    }
    else
    {
        set_alarm_statue(ALARM_DOOR,0);
        doorcnt = 0;
    }
}

static void alarm_accont(void)
{
    static int32_t accontcnt = 0;
	uint8_t flag;
    
    if(sSysConfig.sInt.uAcContCheckEn)
    {
        flag = get_In_IO(ENUM_IN_ACCONT,GUN_SN_ONE);
        flag = sSysMegg[0].unOut.bit.bACCont_Out ^ flag;
        int8_t st = cnt_change_until_threshold(flag,&accontcnt,10000, 2, 1);
        if(st != 2)
        {
            set_alarm_statue(ALARM_ACCONT,st);
        } 

    }
    else
    {
        set_alarm_statue(ALARM_ACCONT,0);
        accontcnt = 0;
    }    
}

#if 0

static void alarm_ax(void)//�����¼�������ڻ����ϲ��ԣ������ϲ�����
{
    uint8_t flag = get_In_IO(ENUM_IN_UVP,GUN_SN_ONE);
    static int32_t axcnt = 0;
    int8_t st = cnt_change_until_threshold(flag, &axcnt, 10000, 2000, 1);

    if(st != 2)
    {
        set_alarm_statue(ALARM_AX,st);
    }
}

static void alarm_midcont(void)
{
    if(sSysConfig.sInt.uPorjectType != PROJECT_FASTSLOW)
    {
        set_alarm_statue(ALARM_MIDCONT,0);
        return;
    }
    uint8_t flag = get_In_IO(ENUM_IN_MIDCONT,GUN_SN_ONE);
    flag = sSysMegg[GUN_SN_ONE].unOut.bit.bMidCont_Out ^ flag;
    static int32_t midcont1cnt = 0;
    int8_t st = cnt_change_until_threshold(flag, &midcont1cnt, 10000, 10, 1);

    if(st != 2)
    {
        set_alarm_statue(ALARM_MIDCONT,st);
    }
}
#endif

static void alarm_mod_com(void)
{
    if(!unChargerTaskClockFlag.bit.b1s)
    {
        return;
    }

    if((chargerMegg[GUN_SN_ONE].flag.bit.bStartCh == 0)&&(chargerMegg[GUN_SN_TWO].flag.bit.bStartCh == 0))
    {
        sAlarmMegg.modcomcnt = 0;
    }

    if(sAlarmMegg.modcomcnt >= 30)
    {
        sAlarmMegg.modcomcnt = 30;
        set_alarm_statue(ALARM_MODCOMFAULT,1);
    }
    else if(sAlarmMegg.modcomcnt <= 0)
    {
        sAlarmMegg.modcomcnt = 0;
        set_alarm_statue(ALARM_MODCOMFAULT,0);
    }
    
    sAlarmMegg.modcomcnt++;
}

#if 0
static void alarm_mod_fault(void)
{
    static int32_t modfaultcnt = 0;

    uint8_t online = 0;
    uint8_t fault = 0;    
    uint8_t flag = 0;    
    
    for (uint8_t u8ModCnt = 1; u8ModCnt <MOD_MAX_ADDR;u8ModCnt ++)
    {
        if(sSysMegg.BoardAddr == sModMegg_SH.sModRxMegg.uModGroupSave[u8ModCnt])
        {
            if(sModMegg_SH.sModRxMegg.unFlagModSt[u8ModCnt].bit.bOnLine==1)
            {
                online++;
                
                if(sModMegg_SH.sModRxMegg.unFlagModSt[u8ModCnt].bit.bAlarmSt)
                {
                    fault++;
                }
            }
        }
    }

    if((online != 0)&&(fault == online))
    {
        flag = 1;
    }

    if((chargerMegg[GUN_SN_ONE].flag.bit.bStartCh == 0)&&(chargerMegg[GUN_SN_TWO].flag.bit.bStartCh == 0))
    {
        flag = 0;
        modfaultcnt = 0;
    }

    if(flag)
    {
        modfaultcnt += 2;    
        if(modfaultcnt >= 20000)
        {
            modfaultcnt = 20000;
            set_alarm_statue(ALARM_MOD,1);
        }
    }
    else
    {
        if(modfaultcnt-- <= 0)
        {
            modfaultcnt = 0;
            set_alarm_statue(ALARM_MOD,0);
        }
    }

}
#endif

/**
  * @brief  ���ϴ���ֹͣ���
  */
void alarm_deal(void)
{
    uint8_t curLevel = 0;
    for(uint16_t i=1;i<ALARM_END_CODE;i++)
    {
        if(get_alarm_statue((ENUM_ALARM_STOP)i) != 0)
        {
            if(curLevel < alarmStopCauseCodeMap[i].alarmtype)
            {
                curLevel = alarmStopCauseCodeMap[i].alarmtype;
                if(curLevel>=ALARM_TYPE_FAULT)
                {

					if(ALARM_TYPE_FAULT == curLevel)
					{
                    	set_charger_stopcause(alarmStopCauseCodeMap[i].originalCode,0);  //1ǹͣ��
                    	set_charger_stopcause(alarmStopCauseCodeMap[i].originalCode,1);  //2ǹͣ��
						chargerMegg[0].unStep.bit.fault = 1;
						chargerMegg[1].unStep.bit.fault = 1;
					}
					else if(ALARM_TYPE_GUN1_FAULT == curLevel)
					{
                    	set_charger_stopcause(alarmStopCauseCodeMap[i].originalCode,0);  //1ǹͣ��
						chargerMegg[0].unStep.bit.fault = 1;
					}
					else if(ALARM_TYPE_GUN2_FAULT == curLevel)
					{
                    	set_charger_stopcause(alarmStopCauseCodeMap[i].originalCode,1);   //2ǹͣ��
						chargerMegg[1].unStep.bit.fault = 1;
					}
                }
            }
        }
    }
    
    uint8_t i = 0;
    uint16_t udata = 0;
    for(i=1;i<=16;i++)
    {
        if(get_alarm_statue((ENUM_ALARM_STOP) i))
        {
            udata |= (1<<(i-1));
        }
    }
    sAlarmMegg.uAlarmA.all = udata;
    
    udata = 0;
    for(i=1;i<=16;i++)
    {
        if(get_alarm_statue((ENUM_ALARM_STOP)(16 + i)))
        {
            udata |= (1<<(i-1));
        }
    }
    sAlarmMegg.uAlarmB.all = udata;

    udata = 0;
    for(i=1;i<=16;i++)
    {
        if(get_alarm_statue((ENUM_ALARM_STOP)(32 + i)))
        {
            udata |= (1<<(i-1));
        }
    }
    sAlarmMegg.uAlarmC[0].all = udata;    
    
    udata = 0;
    for(i=1;i<=16;i++)
    {
        if(get_alarm_statue((ENUM_ALARM_STOP)(48 + i)))
        {
            udata |= (1<<(i-1));
        }
    }
    sAlarmMegg.uAlarmC[1].all = udata;    

    static uint16_t a_bak,b_bak,c_bak,d_bak;
    if(a_bak != sAlarmMegg.uAlarmA.all)
    {
        DLOG("A[%x->%x]",a_bak,sAlarmMegg.uAlarmA.all);
        a_bak = sAlarmMegg.uAlarmA.all;
    }

    if(b_bak != sAlarmMegg.uAlarmB.all)
    {
        DLOG("B[%x->%x]",b_bak,sAlarmMegg.uAlarmB.all);
        b_bak = sAlarmMegg.uAlarmB.all;
    }    

    if(c_bak != sAlarmMegg.uAlarmC[0].all)
    {
        DLOG("C[%x->%x]",c_bak,sAlarmMegg.uAlarmC[0].all);
        c_bak = sAlarmMegg.uAlarmC[0].all;
    }

    if(d_bak != sAlarmMegg.uAlarmC[1].all)
    {
        DLOG("D[%x->%x]",d_bak,sAlarmMegg.uAlarmC[1].all);
        d_bak = sAlarmMegg.uAlarmC[1].all;
    }    
}

void alarm_init(void)
{
    memset(&sAlarmMegg,0,sizeof(sAlarmMegg));
    memset(alarmDataBitValue,0,sizeof(alarmDataBitValue));    
}

void alarm_main(void)
{
    if((sys_debug_get() == DEBUG1_NOERROR)
      ||(sys_debug_get() == DEBUG1_PROCESS))
    {
        chargerMegg[0].unStep.bit.fault = 0;
        chargerMegg[1].unStep.bit.fault = 0;
        alarm_init();
        return;
    }

    alarm_sys_epo();
    alarm_sys_breaker();
	alarm_dccont_gun();
    alarm_fuse();
//    alarm_gunlock();
    alarm_spd();
    alarm_vdc_inv();
    alarm_vdc_cc1();
    alarm_ot_out();
	alarm_ot_gun();
    alarm_vac_up();
    alarm_vac_low();
	alarm_vdc_up();
	alarm_vdc_low();
    alarm_idc_up();
    alarm_door();
    alarm_accont();
    //alarm_ax();
    //alarm_midcont();
    alarm_mod_com();
    //alarm_mod_fault();    //���ڿ�����ģ��Э������
    
    alarm_deal();
}

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/

