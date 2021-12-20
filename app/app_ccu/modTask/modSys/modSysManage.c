/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   modManage
����:   km
˵��:   ģ�����.
***************************************************************************************************/

#include<string.h>

#include "tick_handle.h"
#include "modSysManage.h"
#include "charge_task.h"
#include "log.h"
#include "alarm_task.h"


structModMegg ModMegg[2];
struct structParallel sParallel[BOARD_ADDR_MAX];
extern structSysClockCnt sSysComClockCnt;
extern unionSysClockFlag unSysComClockFlag;



/**
  * @brief  ��ģ���������ͨ����������������TCU�·�ģ������ʣ�ģ������������������ðٷֱ�
  */
static int16_t mod_out_current_limit(int16_t curr)
{
    curr = (int16_t)((float)(sSysConfig.sInt.uModLimit*curr)/100);
    return curr;    
}

/**
  * @brief  ���㵱ǰ�˿ڵ�ѹ����£�����ģ�鹦�����ߣ���ģ������������
  */
static int16_t mod_out_current(int16_t iVdcPort)
{    
    int16_t i_Limit = 0,iVdcLimit = 0;

    switch(sSysConfig.sInt.uModuleType)
    {
    	case MOD_TYPE_INFY_30KW:
        {

            iVdcLimit=_IQsat(iVdcPort, kMod_VMAX_1KV, VOLTAGE_FLOOR);
            if(iVdcLimit >= 3000)
            {
                i_Limit = (int16_t)((float)kMod2U_P30K/iVdcLimit);
            }
            else                    
            {
                i_Limit = (int16_t)((float)kMod2U_P30K/3000);
            }

            i_Limit += 50;//�ȹ������߷ſ�5A
        }
        break;
    	case MOD_TYPE_WINLINE_30KW:

		break;
        default:
            return 0;
    
    }
    
    return mod_out_current_limit(i_Limit);
}

/**
  * @brief  ��ȡ����ģ����������ͬ��Ŀ���ͣ�����ģ���������㲻һ��
  */
static uint8_t get_modnum_systype(uint8_t gunSn)
{
    uint8_t modnum = 0;         // ���μ�������ģ������ ����С����

    switch (sSysConfig.sInt.uPorjectType)
    {
        case 10:
            break;
        default:
        {
			ModMegg[gunSn].i_max = (ModMegg[gunSn].i_max==0)?1:ModMegg[gunSn].i_max;
			modnum = (0==(chargerMegg[gunSn].i_demand_mod%ModMegg[gunSn].i_max))?(chargerMegg[gunSn].i_demand_mod/ModMegg[gunSn].i_max):(chargerMegg[gunSn].i_demand_mod/ModMegg[gunSn].i_max+1);
        }
        break;
    }

    modnum = _IQsat(modnum,32,1);
    return modnum;
}

/**
  * @brief  ��ǹ���㱾ǹ��Ҫ����ģ������
  */
static void mod_num_demand(uint8_t gunSn)
{
    ModMegg[gunSn].i_max = mod_out_current(sSysMegg[gunSn].v_Gun);

    if(!chargerMegg[gunSn].flag.bit.bStartCh)
    {
        ModMegg[gunSn].numdemand_cnt = 0;
        ModMegg[gunSn].numdemand = 0;
        ModMegg[gunSn].modnum_last = 0;
    }    
    else
    {
        if(sSysConfig.sTempInt.WorkMode == WORK_LOCAL)
        {
            ModMegg[gunSn].numdemand = get_modnum_systype(gunSn);
        }
        else
        {
            if(CHARGER_STATE_CHARGING != get_charge_state(gunSn))
            {
                ModMegg[gunSn].numdemand_cnt = 0;    
                ModMegg[gunSn].numdemand = 4;
            }
            else if(ModMegg[gunSn].numdemand_cnt++ >= 60000)//�����һ���Ӽ���һ������ģ������
            {
            	DLOG("need[%d~%d]",sSysMegg[gunSn].v_Gun,ModMegg[gunSn].i_max);
                ModMegg[gunSn].numdemand_cnt = 0;
                ModMegg[gunSn].numdemand = get_modnum_systype(gunSn);
            }
        }
    }
}


#if 0
/**
  * @brief  �����ѹ����
  */
static uint16_t v_voltage_compare(uint8_t group,int16_t v_demand)  
{
    return (uint16_t)v_demand;
}

/**
  * @brief  ˢ�·��͸�ģ��ĵ�ѹ����ֵ
  */
static void v_mod_Iout_limit(uint8_t group)
{
	switch(sSysConfig.sInt.uModuleType)
	{
		case MOD_TYPE_INFY_30KW:

			break;

		case MOD_TYPE_WINLINE_30KW:
			
			break;
		
		default:
			break;
	}
}
#endif

void mod_manage_init(void)
{
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
        memset(&ModMegg[gunSn],0x00,sizeof(ModMegg[gunSn]));
	}
    memset(sParallel,0,sizeof(sParallel));
}

void mod_manage_main(void)
{
    //��ǹ��ؼ���
    
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
    {
        mod_num_demand(gunSn);
    }



}

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
