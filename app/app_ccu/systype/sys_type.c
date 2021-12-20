/***************************************************************************************************   
Copyright @ 2020-20xxС�ۿƼ�

�ļ�:   pro_base
����:   km
˵��:   ��������.
***************************************************************************************************/

#include <string.h>

#include "sys_type.h"
#include "sys_config.h"
#include "log.h"
#include "sys_type_contactor_group.h"

/**
  * @brief  ����ǰ����ʱ�䣬�����л�ģ�顢�Ӵ���
  */
int32_t Project_Type_Delay(void)
{
    int32_t delay = 0;
    
    switch(sSysConfig.sInt.uPorjectType)
    {
        case SYSTYPE_FASTSLOW:
            delay = DELAY_PROJECT_FASTSLOW;
            break;
		case SYSTYPE_CONTACTOR_GROUP:
			
		    delay = DELAY_PROJECT_FASTSLOW;
            break;
        default:
            break;

    
    }
    return delay;
}

void Project_Type_Deal(void)
{
    switch(sSysConfig.sInt.uPorjectType)
    {
        case SYSTYPE_FASTSLOW:
            break;

        case SYSTYPE_CONTACTOR_GROUP:
			//vGh2_contactor_group();
            break;

        default:
            break;

    
    }
}

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
