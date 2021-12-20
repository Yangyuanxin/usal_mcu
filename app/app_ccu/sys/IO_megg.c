/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   io_megg
����:   km
˵��:   ����IO�����IO��ADC��������.
***************************************************************************************************/

#include <string.h>

#include "io_megg.h"
#include "fun.h"
#include "tick_handle.h"
#include "alarm_task.h"
#include "charge_task.h"
#include "log.h"
#include "kfc_dev.h"
#include "kfc_timer.h"
#include "can_def.h"
#include "sys_type_contactor_group.h"


static structBasicMegg sBasicMegg[2] = {0};
static structGunLock sGunlock[2] = {0};

structSysBasicMegg sSysMegg[2];



static const uint16_t adcTempTAB[156]=    //ÿ����֮����5��;2855:150��;0:150,1:140,2:130,3:120,4:110;5:100
{
	2504,2480,2456,2431,2405,2378,2375,2373,2370,2367,	 
	2364,2362,2359,2356,2353,2350,2348,2345,2342,2339,   //95~86
	2336,2333,2330,2327,2324,2322,2319,2316,2313,2310,   //85~76
	2307,2304,2301,2298,2295,2292,2289,2286,2283,2280,   //75~66
	2276,2273,2270,2267,2264,2261,2258,2255,2252,2249,   //65~56
	2245,2242,2239,2236,2233,2229,2226,2222,2219,2215,   //55~46
	2212,2209,2206,2203,2199,2195,2192,2188,2185,2182,   //25~36
	2178,2175,2171,2167,2164,2160,2157,2154,2150,2146,   //35~26
	2143,2139,2136,2132,2128,2125,2121,2117,2113,2110,   //25~16
	2106,2102,2098,2094,2090,2087,2083,2079,2075,2071,   //15~6
	2068,2064,2060,2056,2052,2048,2044,2040,2036,2032,   //5~-4
	2028,2024,2020,2016,2012,2007,2003,1999,1995,1991,   //-5~-14
	1986,1981,1977,1973,1969,1964,1959,1954,1950,1946,   //-15~-24
	1942,1937,1933,1929,1925,1920,1915,1910,1906,1902,   //-25~-34
	1897,1892,1887,1882,1877,1873,1868,1865,1860,1855,   //-35~-44
	1849,1844,1839,1834,1829,1824                        //-44~-50
	
	
};

int16_t  vRmsAdcTempDeal(int16_t idata,int16_t iTempLen,const uint16_t *table)
{
	int16_t iTemp1=0,iTemp2;
	int16_t i;
	int16_t iTempData;
	idata=_IQsat(idata, table[0], table[156-1]);
	for(i=0;i<iTempLen;i++)
	{
		if(idata>=table[i])
		{
			iTemp1=i-1;		
			break;
		}
	}
	if(iTemp1<=0)
	{
		iTemp1=0;
		iTempData=1500;
	}
	else
	{

		if((iTemp1+1) <= 5)
		{
			iTemp2=(idata-table[iTemp1+1])*100/(table[iTemp1]-table[iTemp1+1]);
			iTempData=1500-(iTemp1+1)*100+iTemp2;
		}
		else
		{
		    iTemp2=(idata-table[iTemp1+1])*10/(table[iTemp1]-table[iTemp1+1]);
		    iTempData=1000-(iTemp1+1-5)*10+iTemp2;
		}
	}
	return(iTempData);
}


int  send_mod_can_frame(T_CAN_MSG *t_msg)
{
    TCanData can_data;

	can_data.length = 8;
	can_data.id_type = 1;
	can_data.frame_type = 0;
	can_data.id = t_msg->id.val;

	for(uint8_t i=0;i<8;i++)
	{
	    can_data.data[i] = t_msg->data[i];
	}

    return KfcDevWrite(kSfDevNumCan2, &can_data, sizeof(can_data));
}

int8_t tcu_com_can_send(CAN_MSG_T *pt_msg,uint8_t gunSn)
{
    TCanData can_data;
	uint16_t canDevIndex = 0;

	can_data.length = pt_msg->len;
	can_data.id_type = 1;
	can_data.frame_type = 0;
	can_data.id = pt_msg->id;

	for(uint8_t i=0;i<8;i++)
	{
	    can_data.data[i] = pt_msg->data[i];
	}
	
    canDevIndex = (gunSn == 0)?kSfDevNumCan0:kSfDevNumCan1;
    return KfcDevWrite(canDevIndex, &can_data, sizeof(can_data));
}

/**
  * @brief  ���GPIO����
  * @param  None
  * @retval None
  */
void vIO_OUT_ACT(void)
{
	uint32_t gpioState = 0;

	gpioState = sBasicMegg[0].unOutput.bit.bGFD_Pos;
	KfcDevWrite(kSfDevNumPowerSw1Ctr1, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bGFD_Neg;
	KfcDevWrite(kSfDevNumPowerSw1Ctr2, &gpioState, 4);
	gpioState = (1-sBasicMegg[0].unOutput.bit.bGFD_PE);
	KfcDevWrite(kSfDevNumPowerIsoCtrl1, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bACCont_Out;
	KfcDevWrite(kSfDevNumPowerDoCtr10, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bFan;
	KfcDevWrite(kSfDevNumPowerDoCtr12, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bMidCont_Out0;
	KfcDevWrite(kSfDevNumPowerDoCtr7, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bMidCont_Out1;
	KfcDevWrite(kSfDevNumPowerDoCtr8, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bMidCont_Out2;
	KfcDevWrite(kSfDevNumPowerDoCtr9, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bBMS_V;
	KfcDevWrite(kSfDevNumPowerDoCtr1, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bDcCont_Out;
	KfcDevWrite(kSfDevNumPowerDoCtr3, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bGunLock;
	KfcDevWrite(kSfDevNumPowerDoCtr2, &gpioState, 4);
	gpioState = sBasicMegg[0].unOutput.bit.bGunLock_PorN;
	KfcDevWrite(kSfDevNumPowerLockCtrl1, &gpioState, 4);

	gpioState = sBasicMegg[1].unOutput.bit.bGFD_Pos;
	KfcDevWrite(kSfDevNumPowerSw2Ctr1, &gpioState, 4);
	gpioState = sBasicMegg[1].unOutput.bit.bGFD_Neg;
	KfcDevWrite(kSfDevNumPowerSw2Ctr2, &gpioState, 4);
	gpioState = (1-sBasicMegg[1].unOutput.bit.bGFD_PE);
	KfcDevWrite(kSfDevNumPowerIsoCtrl2, &gpioState, 4);
	gpioState = sBasicMegg[1].unOutput.bit.bBMS_V;
	KfcDevWrite(kSfDevNumPowerDoCtr4, &gpioState, 4);
	gpioState = sBasicMegg[1].unOutput.bit.bDcCont_Out;
	KfcDevWrite(kSfDevNumPowerDoCtr6, &gpioState, 4);
	gpioState = sBasicMegg[1].unOutput.bit.bGunLock;
	KfcDevWrite(kSfDevNumPowerDoCtr5, &gpioState, 4);
	gpioState = sBasicMegg[1].unOutput.bit.bGunLock_PorN;
	KfcDevWrite(kSfDevNumPowerLockCtrl2, &gpioState, 4);
	
}

/**
  * @brief  �����������״̬
  * @param  ENUM_OUPUT_CONTROL,flag 0 ��ʹ�� 1ʹ��
  */
void set_Out_IO(ENUM_OUPUT_CONTROL eOut,uint8_t flag,uint8_t gunSn)
{
    if(flag)
    {
        sBasicMegg[gunSn].unOutput.all |= (1<<eOut);
    }
    else
    {
        sBasicMegg[gunSn].unOutput.all &= ~(1<<eOut);
    }
}

/**
  * @brief  �������״̬��ȡ
  * @param  ENUM_OUPUT_CONTROL
  * @retval 0 ��ʹ�� 1ʹ��
  */
uint8_t get_Out_IO(ENUM_OUPUT_CONTROL eOut,uint8_t gunSn)
{
    uint32_t data = sBasicMegg[gunSn].unOutput.all;
    return (uint8_t)(0x01&(data>>eOut));
}

/**
  * @brief  ����GPIO��ȡ
  * @param  None
  * @retval None
  */
void vIO_IN_ACT(void)
{
	uint32_t gpioReadState = 0;

	KfcDevRead(kSfDevNumSwitchDi01, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bEPO = gpioReadState;
	
	KfcDevRead(kSfDevNumSwitchDi02, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bDoor = gpioReadState;
	
	KfcDevRead(kSfDevNumSwitchDi03, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bSPD = gpioReadState;
	
	KfcDevRead(kSfDevNumSwitchDi04, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bDcCont_In = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi05, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bFuse = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi06, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bGunLock = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi07, &gpioReadState, 4);
	sBasicMegg[1].unInput.bit.bDcCont_In = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi08, &gpioReadState, 4);
	sBasicMegg[1].unInput.bit.bFuse = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi09, &gpioReadState, 4);
	sBasicMegg[1].unInput.bit.bGunLock = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi10, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bMidCont1_In = gpioReadState;
	
	KfcDevRead(kSfDevNumSwitchDi11, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bMidCont2_In = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi12, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bMidCont3_In = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi13, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bBreaker = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi14, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.bACCont_In = gpioReadState;

	KfcDevRead(kSfDevNumSwitchDi15, &gpioReadState, 4);
	sBasicMegg[0].unInput.bit.fan_In = gpioReadState;
	
}

uint8_t get_In_IO(ENUM_INPUT_BACK eIn,uint8_t gunSn)
{
    return ((sBasicMegg[gunSn].unInput.all>>eIn)&0x01);
}

BOOL get_DcCont_State(uint8_t gunSn)
{
    BOOL state = BOOL_FALSE;

	state = (get_In_IO(ENUM_IN_DCCONT,gunSn))?BOOL_TRUE:BOOL_FALSE;

    state = (sys_debug_get() == DEBUG1_PROCESS)?BOOL_TRUE:state;

	return state;
}

/**
  * @brief  ��ȡ�����Ӵ�������״̬
  * @retval false �Ͽ� true �պ� 
  */
BOOL get_accont_statue(void)
{
    if(sSysMegg[GUN_SN_ONE].unOut.bit.bACCont_Out)
    {
        return BOOL_TRUE;
    }
	
    return BOOL_FALSE;
}


static void ccu_set_cc1_state(int16_t cc1Volt,uint8_t gunSn)
{
	sSysMegg[gunSn].CC1_bak = sSysMegg[gunSn].CC1;
	if(((cc1Volt >= 52)&&(cc1Volt <= 68))||((cc1Volt >= 32)&&(cc1Volt <= 48)))   //3.2-4.8(4)||5.2-6.8(6)
	{
		sBasicMegg[gunSn].unInput.bit.bCC1 = 1;
		sSysMegg[gunSn].CC1 = sBasicMegg[gunSn].unInput.bit.bCC1;
	}
	else
	{
		sBasicMegg[gunSn].unInput.bit.bCC1 = 0;
		sSysMegg[gunSn].CC1 = sBasicMegg[gunSn].unInput.bit.bCC1;
	}
}

/**
  * @brief  ADC���ݶ�ȡ
  * @param  None
  * @retval None
  */
void vIO_ADC_ACT(void)
{
    structCalRms *p1 = &sBasicMegg[0].sCalRms;
    structCalRms *p2 = &sBasicMegg[1].sCalRms;
    structSysBasicMegg *sys1 = &sSysMegg[0];
    structSysBasicMegg *sys2 = &sSysMegg[1];

    int32_t sysVdcScalCstVar = kSysVdcScalCst2;
    int32_t sysPortNegOffset = kSysPortNegOffset;
	int32_t insPosVdcScalCstVar = kInsPosVdcScalCst;
	int32_t insPosVdcScalOffsetVar = kInsPosVdcScalOffset;
    int32_t insNegVdcScalCstVar = kInsNegVdcScalCst;
	int32_t insNegVdcScalOffsetVar = kInsNegVdcScalOffset;
	int32_t iSysCurrentVar=kSysCurrentScalCst;
	int32_t iSysCurrentOffsetVar=kSysCurrentOffset;

    p1->VGunCali = sSysConfig.sInt.iAGun_ScalSysPortVdcCali[0];    
    p1->VGunCaliZero = sSysConfig.sInt.iAGun_ScalSysPortVdcZero[0];   
	p2->VGunCali = sSysConfig.sInt.iAGun_ScalSysPortVdcCali[1];    
    p2->VGunCaliZero = sSysConfig.sInt.iAGun_ScalSysPortVdcZero[1]; 

	p1->VGunCali = 4096;    
    p1->VGunCaliZero = 0;   
	p2->VGunCali = 4096;    
    p2->VGunCaliZero = 0; 

    //gun1-�Ӵ�����������ѹ
    int32_t iScalSysPortVdcAdc1=0;
    KfcDevRead(kSfDevNumAdcAOutVol, &iScalSysPortVdcAdc1, 4);
    int32_t iScalSysPortVdc1=sysVdcScalCstVar*p1->VGunCali>>12;
    int32_t iSysPortVdc1=(((iScalSysPortVdcAdc1-sysPortNegOffset)*iScalSysPortVdc1)>>11)+p1->VGunCaliZero;
    p1->unSysPortVdc.dword += ((iSysPortVdc1<<16) - p1->unSysPortVdc.dword)>>5;    
    sys1->v_Gun=_IQsat(p1->unSysPortVdc.half.hword, GUN_MAX_VOLT, p1->unSysPortVdc.half.hword);

	//gun2-�Ӵ�����������ѹ
	 int32_t iScalSysPortVdcAdc2=0;
	 KfcDevRead(kSfDevNumAdcBOutVol, &iScalSysPortVdcAdc2, 4);
	 int32_t iScalSysPortVdc2=sysVdcScalCstVar*p2->VGunCali>>12;
	 int32_t iSysPortVdc2=(((iScalSysPortVdcAdc2-sysPortNegOffset)*iScalSysPortVdc2)>>11)+p2->VGunCaliZero;
	 p2->unSysPortVdc.dword += ((iSysPortVdc2<<16) - p2->unSysPortVdc.dword)>>5;	
	 sys2->v_Gun=_IQsat(p2->unSysPortVdc.half.hword, GUN_MAX_VOLT, p2->unSysPortVdc.half.hword);

	 //gun1��Ե����ߵ�ѹ
	 int32_t iGFDPosVdcAdc1 = 0;
	 KfcDevRead(kSfDevNumAdcAPosIso, &iGFDPosVdcAdc1, 4);
	 int32_t iGFDPosVdc1=(((iGFDPosVdcAdc1-insPosVdcScalOffsetVar)*insPosVdcScalCstVar)>>11);
	 p1->unGFDPosVdc.dword += ((iGFDPosVdc1<<16)-p1->unGFDPosVdc.dword)>>5;
	 sys1->v_GFD_P = _IQsat(p1->unGFDPosVdc.half.hword, GUN_MAX_VOLT, p1->unGFDPosVdc.half.hword);
	 
	 //gun1��Ե����ߵ�ѹ
	 int32_t iGFDNegVdcAdc1 = 0;
	 KfcDevRead(kSfDevNumAdcANegIso, &iGFDNegVdcAdc1, 4);
	 int32_t iGFDNegVdc1=(((iGFDNegVdcAdc1-insNegVdcScalOffsetVar)*insNegVdcScalCstVar)>>11);
	 p1->unGFDNegVdc.dword += ((iGFDNegVdc1<<16)-p1->unGFDNegVdc.dword)>>5;
	 sys1->v_GFD_N = _IQsat(p1->unGFDNegVdc.half.hword, GUN_MAX_VOLT, p1->unGFDNegVdc.half.hword);

	 //gun2��Ե����ߵ�ѹ
	 int32_t iGFDPosVdcAdc2 = 0;
	 KfcDevRead(kSfDevNumAdcBPosIso, &iGFDPosVdcAdc2, 4);
	 int32_t iGFDPosVdc2=(((iGFDPosVdcAdc2-insPosVdcScalOffsetVar)*insPosVdcScalCstVar)>>11);
	 p2->unGFDPosVdc.dword += ((iGFDPosVdc2<<16)-p2->unGFDPosVdc.dword)>>5;
	 sys2->v_GFD_P = _IQsat(p2->unGFDPosVdc.half.hword, GUN_MAX_VOLT, p2->unGFDPosVdc.half.hword);
	 
	 //gun2��Ե����ߵ�ѹ
	 int32_t iGFDNegVdcAdc2 = 0;
	 KfcDevRead(kSfDevNumAdcBNegIso, &iGFDNegVdcAdc2, 4);
	 int32_t iGFDNegVdc2=(((iGFDNegVdcAdc2-insNegVdcScalOffsetVar)*insNegVdcScalCstVar)>>11);
	 p2->unGFDNegVdc.dword += ((iGFDNegVdc2<<16)-p2->unGFDNegVdc.dword)>>5;
	 sys2->v_GFD_N = _IQsat(p2->unGFDNegVdc.half.hword, GUN_MAX_VOLT, p2->unGFDNegVdc.half.hword);

	 //gun1-�������
	 int32_t iSysCurrentAdc1=0;
	 KfcDevRead(kSfDevNumAdcACurr, &iSysCurrentAdc1, 4);
	 int32_t iSysCurrent1=(((iSysCurrentAdc1-iSysCurrentOffsetVar)*iSysCurrentVar)>>12);	 
	 p1->unSysPortIdc.dword += ((iSysCurrent1<<16) - p1->unSysPortIdc.dword)>>5;
	 //sys1->i_Gun = _IQsat(p1->unSysPortIdc.half.hword, GUN_MAX_CURR, p1->unSysPortIdc.half.hword);;

	 //gun2-�������
	 int32_t iSysCurrentAdc2=0;
	 KfcDevRead(kSfDevNumAdcBCurr, &iSysCurrentAdc2, 4);
	 int32_t iSysCurrent2=(((iSysCurrentAdc2-iSysCurrentOffsetVar)*iSysCurrentVar)>>12);	 
	 p2->unSysPortIdc.dword += ((iSysCurrent2<<16) - p2->unSysPortIdc.dword)>>5;
	 //sys2->i_Gun = _IQsat(p2->unSysPortIdc.half.hword, GUN_MAX_CURR, p2->unSysPortIdc.half.hword);;


    //gun1-cc1��ѹ
    uint32_t icc1VoltAdc1=0;
    KfcDevRead(kSfDevNumAdcACc1Vol, &icc1VoltAdc1, 4);
    int32_t iScalCC1Volt1=kCC1VoltScalCst;
    int32_t icc1Volt1=((icc1VoltAdc1*iScalCC1Volt1)>>11);   
    p1->unCc1Volt.dword += ((icc1Volt1<<16) - p1->unCc1Volt.dword)>>5;
    sys1->v_CC1_10x = p1->unCc1Volt.half.hword;
	ccu_set_cc1_state(sys1->v_CC1_10x,0);
    //gun2-cc1��ѹ
    uint32_t icc1VoltAdc2=0;
    KfcDevRead(kSfDevNumAdcBCc1Vol, &icc1VoltAdc2, 4);
    int32_t iScalCC1Volt2=kCC1VoltScalCst;
    int32_t icc1Volt2=((icc1VoltAdc2*iScalCC1Volt2)>>11);   
    p2->unCc1Volt.dword += ((icc1Volt2<<16) - p2->unCc1Volt.dword)>>5;
    sys2->v_CC1_10x = p2->unCc1Volt.half.hword;
	ccu_set_cc1_state(sys2->v_CC1_10x,1);

    //������¶�
	int32_t iInletTemp=0; 
    KfcDevRead(kSfDevNumAdcSysTempInput, &iInletTemp, 4);
	sys1->hz_10x = iInletTemp;
    p1->unInletTemp.dword+=((iInletTemp<<16)-p1->unInletTemp.dword)>>5;  
    int16_t iInletTempValue=vRmsAdcTempDeal(p1->unInletTemp.half.hword,156,adcTempTAB);
    sys1->t_Inlet=_IQsat(iInletTempValue, SYS_MAX_TEMP, SYS_MIN_TEMP);

    //������¶�
    int32_t iOutletTemp=0;
    KfcDevRead(kSfDevNumAdcSysTempOutput, &iOutletTemp, 4);
    p1->unOutletTemp.dword+=((iOutletTemp<<16)-p1->unOutletTemp.dword)>>5;
    int16_t iOutletTempValue=vRmsAdcTempDeal(p1->unOutletTemp.half.hword,156,adcTempTAB);
    sys1->t_Outlet=_IQsat(iOutletTempValue, SYS_MAX_TEMP, SYS_MIN_TEMP);

	//gun1-ǹ�¶�+
    int32_t iGunPosTemp1=0;	
    KfcDevRead(kSfDevNumAdcAPosTemp, &iGunPosTemp1, 4);
    p1->unGunPosTemp.dword+=((iGunPosTemp1<<16)-p1->unGunPosTemp.dword)>>5;
    int16_t iGunPosTempValue1=vRmsAdcTempDeal(p1->unGunPosTemp.half.hword,156,adcTempTAB);
    sys1->t_GunP=_IQsat(iGunPosTempValue1, SYS_MAX_TEMP, SYS_MIN_TEMP);
    //gun1-ǹ�¶�-
    int32_t iGunNegTemp1=0;
    KfcDevRead(kSfDevNumAdcANegTemp, &iGunNegTemp1, 4);
    p1->unGunNegTemp.dword+=((iGunNegTemp1<<16)-p1->unGunNegTemp.dword)>>5;
    int16_t iGunNegTempValue1=vRmsAdcTempDeal(p1->unGunNegTemp.half.hword,156,adcTempTAB);
    sys1->t_GunN=_IQsat(iGunNegTempValue1, SYS_MAX_TEMP, SYS_MIN_TEMP);
    sys1->t_Gun= _IQsat(iGunPosTempValue1, SYS_MAX_TEMP, iGunNegTempValue1);

	//gun2-ǹ�¶�+
    int32_t iGunPosTemp2=0;
    KfcDevRead(kSfDevNumAdcBPosTemp, &iGunPosTemp2, 4);
    p2->unGunPosTemp.dword+=((iGunPosTemp2<<16)-p2->unGunPosTemp.dword)>>5;
    int16_t iGunPosTempValue2=vRmsAdcTempDeal(p2->unGunPosTemp.half.hword,156,adcTempTAB);
    sys2->t_GunP=_IQsat(iGunPosTempValue2, SYS_MAX_TEMP, SYS_MIN_TEMP);

    //gun2-ǹ�¶�-
    int32_t iGunNegTemp2=0;
    KfcDevRead(kSfDevNumAdcBNegTemp, &iGunNegTemp2, 4);
    p2->unGunNegTemp.dword+=((iGunNegTemp2<<16)-p2->unGunNegTemp.dword)>>5;
    int16_t iGunNegTempValue2=vRmsAdcTempDeal(p2->unGunNegTemp.half.hword,156,adcTempTAB);
    sys2->t_GunN=_IQsat(iGunNegTempValue2, SYS_MAX_TEMP, SYS_MIN_TEMP);
	
    sys2->t_Gun=_IQsat(iGunPosTempValue2, SYS_MAX_TEMP, iGunNegTempValue2);


	get_yfymod_phase_data(&sys1->vab_10x,&sys1->vbc_10x,&sys1->vac_10x); //��ģ���ȡ�����ѹ
    
}

#define kLockIoCloseTime	25
#define kLockIoOpenTime		275
#define kLockIoBackTime		800
#define kLockIoEndTime		805
#define kLockIoOPTime		2

#define kLockIoTime_0   500
#define kLockEndTime 1050

void gunlock_type2(uint8_t gunSn)
{

	ENUM_ALARM_STOP index = ALARM_STOP_NULL;
	index = (gunSn == 0)?ALARM_GunLock_GUN1:ALARM_GunLock_GUN2;

	if(sSysConfig.sTempInt.GunUnLock[gunSn] != 0)//Ӧ������
    {
        set_Out_IO(ENUM_OUT_GUNLOCK_PORN, 0,gunSn);

        sGunlock[gunSn].UnLockCnt++;
        if(sGunlock[gunSn].UnLockCnt > kLockIoOpenTime)
        {               
            sGunlock[gunSn].UnLockCnt = 0;
            sSysConfig.sTempInt.GunUnLock[gunSn] = 0;
            if(get_In_IO(ENUM_IN_GUNLOCK,gunSn) != 1)//�����û�н���-
            {
                set_alarm_statue(index,0);            
            }
            set_Out_IO(ENUM_OUT_GUNLOCK, 0, gunSn);
        }
        else if(sGunlock[gunSn].UnLockCnt > kLockIoCloseTime)
        {
            set_Out_IO(ENUM_OUT_GUNLOCK, 1, gunSn);
        }
        return;
    }

    if(sGunlock[gunSn].gunlock==1)//����
    {
        set_Out_IO(ENUM_OUT_GUNLOCK_PORN, 1,gunSn);
        sGunlock[gunSn].iGunLockNeg12VCnt=0;
        sGunlock[gunSn].iGunLockNeg12VTime=0;

        sGunlock[gunSn].iGunLockPos12VCnt++;
        
        if(sGunlock[gunSn].iGunLockPos12VCnt>=kLockIoEndTime)
        {
            sGunlock[gunSn].iGunLockPos12VTime=kLockIoOPTime;
            sGunlock[gunSn].iGunLockPos12VCnt=kLockIoEndTime;
        }
        else if(sGunlock[gunSn].iGunLockPos12VCnt>kLockIoBackTime)
        {
            sGunlock[gunSn].iGunLockPos12VTime++;
            if(get_In_IO(ENUM_IN_GUNLOCK,gunSn) == 0)//����û�ɹ�
            {
//                if(sGunlock[gunSn].iGunLockPos12VTime >= kLockIoOPTime)//����2���˳�
                {
                    sGunlock[gunSn].iGunLockPos12VTime=kLockIoOPTime;
                    sGunlock[gunSn].iGunLockPos12VCnt=kLockIoEndTime;
                    set_charger_stopcause(index,gunSn);
                }
//                else
                {
//                    sGunlock[gunSn].iGunLockPos12VCnt=0;//ִ�д�����������,���¿�ʼ��������
                }
            }
            else
            {
                sGunlock[gunSn].iGunLockPos12VTime=kLockIoOPTime;
                sGunlock[gunSn].iGunLockPos12VCnt=kLockIoEndTime;
            }
        }
        else if(sGunlock[gunSn].iGunLockPos12VCnt>kLockIoOpenTime)//step2:lock free for  1ms *250 = 250ms
        {
            set_Out_IO(ENUM_OUT_GUNLOCK, 0,gunSn);

            if(get_In_IO(ENUM_IN_GUNLOCK,gunSn) == 1)//���ճɹ�
            {
                sGunlock[gunSn].iGunLockPos12VTime=kLockIoOPTime;
                sGunlock[gunSn].iGunLockPos12VCnt=kLockIoEndTime;
            }
        }
        else if(sGunlock[gunSn].iGunLockPos12VCnt>kLockIoCloseTime)//step1:lock close
        {
            set_Out_IO(ENUM_OUT_GUNLOCK, 1,gunSn);
        }
    }
    else//����
    {
        sGunlock[gunSn].iGunLockPos12VCnt=0;
        sGunlock[gunSn].iGunLockPos12VTime=0;
        set_Out_IO(ENUM_OUT_GUNLOCK_PORN, 0,gunSn);
        
        sGunlock[gunSn].iGunLockNeg12VCnt++;

        if(sGunlock[gunSn].iGunLockNeg12VCnt>=kLockIoEndTime)
        {
            sGunlock[gunSn].iGunLockNeg12VTime=kLockIoOPTime;
            sGunlock[gunSn].iGunLockNeg12VCnt=kLockIoEndTime;
        }
        else if(sGunlock[gunSn].iGunLockNeg12VCnt>kLockIoBackTime)
        {
            sGunlock[gunSn].iGunLockNeg12VTime++;
            if(get_In_IO(ENUM_IN_GUNLOCK,gunSn) == 1)       //�����û�н���-
            {
                if(sGunlock[gunSn].iGunLockNeg12VTime >= kLockIoOPTime)//�������ξ��˳�
                {
                    sGunlock[gunSn].iGunLockNeg12VTime = kLockIoOPTime;
                    sGunlock[gunSn].iGunLockNeg12VCnt = kLockIoEndTime;
                    set_alarm_statue(index,1);
                }
                else
                {
                    sGunlock[gunSn].iGunLockNeg12VCnt = 0;
                }
            }
            else
            {
                sGunlock[gunSn].iGunLockNeg12VTime=kLockIoOPTime;
                sGunlock[gunSn].iGunLockNeg12VCnt=kLockIoEndTime;
            }
        }
        else if(sGunlock[gunSn].iGunLockNeg12VCnt>kLockIoOpenTime)
        {
            set_Out_IO(ENUM_OUT_GUNLOCK, 0,gunSn);
            if(get_In_IO(ENUM_IN_GUNLOCK,gunSn) == 0)//�������          
            {
                set_alarm_statue(index,0);            
                sGunlock[gunSn].iGunLockNeg12VTime=kLockIoOPTime;
                sGunlock[gunSn].iGunLockNeg12VCnt=kLockIoEndTime;
            }
        }
        else if(sGunlock[gunSn].iGunLockNeg12VCnt>kLockIoCloseTime)
        {
            set_Out_IO(ENUM_OUT_GUNLOCK, 1,gunSn);
        }
    }
}

void CTRL_BMS_V(uint8_t gunSn)
{
    set_Out_IO(ENUM_OUT_BMS_V, sSysMegg[gunSn].unOut.bit.bBMS_V,gunSn);
}

void CTRL_GUNLOCK(uint8_t gunSn)
{
    if(get_poweron_times() < 5)//�ϵ����ʱ5��
    {
        return;
    }

    if(g_bSysTimer1sEventFlag)
    {
        sGunlock[gunSn].gunlock = sSysMegg[gunSn].unOut.bit.bGunLock;
    }

	ENUM_ALARM_STOP index = ALARM_STOP_NULL;
	index = (gunSn == 0)?ALARM_GunLock_GUN1:ALARM_GunLock_GUN2;
	if(sSysConfig.sInt.uGunType == ENUM_GUN_NONE)
	{
		set_alarm_statue(index,0);
		return;
	}

    /*static int16_t opencnt = 0;
    static int16_t opencycle = 0;
    if(get_alarm_statue(ALARM_LOCK_OPEN)==1)
    {
        if(opencycle<10)
        {
            if(opencnt++ >= 5000)
            {
                opencycle++;
                opencnt = 0;
                sSysConfig.sTempInt.GunUnLock = 1;
            }
        }
    }
    else
    {
        opencnt = 0;
        opencycle = 0;
    }*/

    switch (sSysConfig.sInt.uGunType)
    {
        case ENUM_GUN_TYPE2:
        {
            gunlock_type2(gunSn);
        }
        break;        

        default:
        {
            sGunlock[gunSn].gunlock = 0;
        }
        break;

    }	

}

void CTRL_GFD(uint8_t gunSn)
{    
    set_Out_IO(ENUM_OUT_GFD_POS, sSysMegg[gunSn].unOut.bit.bGFD_Pos,gunSn);
    set_Out_IO(ENUM_OUT_GFD_NEG, sSysMegg[gunSn].unOut.bit.bGFD_Neg,gunSn);
    set_Out_IO(ENUM_OUT_GFD_PE, sSysMegg[gunSn].unOut.bit.bGFD_PE,gunSn);
}

void CTRL_DCCONT(uint8_t gunSn)
{
    set_Out_IO(ENUM_OUT_DCCONT, sSysMegg[gunSn].unOut.bit.bDcCont_Out,gunSn);
}

void CTRL_ACCONT(uint8_t gunSn)
{    
    set_Out_IO(ENUM_OUT_ACCONT, sSysMegg[gunSn].unOut.bit.bACCont_Out,gunSn);
}

void CTRL_MIDCONT(uint8_t gunSn)
{    
	uint8_t gh2_cont = 0;//vGetMidContCt_Gh2();
	sSysMegg[gunSn].unOut.bit.bMidCont_Out0 = (gh2_cont>>0)&0x01;
	sSysMegg[gunSn].unOut.bit.bMidCont_Out1 = (gh2_cont>>1)&0x01;
	sSysMegg[gunSn].unOut.bit.bMidCont_Out2 = (gh2_cont>>2)&0x01;

    set_Out_IO(ENUM_OUT_MIDCONT1, sSysMegg[gunSn].unOut.bit.bMidCont_Out0,gunSn);
    set_Out_IO(ENUM_OUT_MIDCONT2, sSysMegg[gunSn].unOut.bit.bMidCont_Out1,gunSn);
    set_Out_IO(ENUM_OUT_MIDCONT3, sSysMegg[gunSn].unOut.bit.bMidCont_Out2,gunSn);
}

void CTRL_FAN(uint8_t gunSn)
{
    set_Out_IO(ENUM_OUT_FAN, sSysMegg[gunSn].unOut.bit.bFan,gunSn);
}

void CTRL_RSV(uint8_t gunSn)
{
}

void CTRL_HEART_LED(void)  //�����ƿ��ƣ�1ms����
{
    static uint16_t time_1s_cnt = 0;
	static uint32_t ledState = 0;

	time_1s_cnt++;
	if(time_1s_cnt >= 1000)
	{
	    time_1s_cnt = 0;
		ledState = !ledState;
		KfcDevWrite(kSfDevNumPowerRunLed, &ledState, 4);
	}
	
}

void vIO_OUT_CTRL(void)
{
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    CTRL_BMS_V(gunSn);           //BMS A+A- ���
	    CTRL_GUNLOCK(gunSn);         //ǹ��
	    CTRL_GFD(gunSn);             //��Ե���
	    CTRL_DCCONT(gunSn);          //����Ӵ���
	    CTRL_ACCONT(gunSn);          //����Ӵ���
	    CTRL_MIDCONT(gunSn);         //�м�Ӵ���3��
	    CTRL_FAN(gunSn);             //����
	    CTRL_RSV(gunSn);             //Ԥ�����
	}
}

/**
  * @brief  ϵͳ�ײ����ݳ�ʼ��
  * @param  None
  * @retval None
  */
void vSys_Basic_Init(void)
{
	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    memset(&sBasicMegg[gunSn],0,sizeof(sBasicMegg[gunSn]));
	    memset(&sSysMegg[gunSn],0,sizeof(sSysMegg[gunSn]));
	}
    sSysMegg[0].version = DCHARGER_VERSION*100+DCHARGER_VERSION_BRANCH;
}

/**
  * @brief  ϵͳ�ײ��ܽӿ�
  * @param  None
  * @retval None
  */
void vCcu_IO_Ctr(void)
{
    vIO_OUT_CTRL();
    vIO_OUT_ACT();
    vIO_IN_ACT();
    vIO_ADC_ACT();    

	for(uint8_t gunSn = 0;gunSn < GUN_SN_MAX;gunSn++)
	{
	    if(sBasicMegg[gunSn].unInput_b.all != sBasicMegg[gunSn].unInput.all)
	    {
	//        DLOG("�����źű仯 0x%x -> 0x%x",sBasicMegg[gunSn].unInput_b.all,sBasicMegg[gunSn].unInput.all);
	        sBasicMegg[gunSn].unInput_b.all = sBasicMegg[gunSn].unInput.all;
	    }
	    sBasicMegg[gunSn].unOutput_b.bit.bLed = sBasicMegg[gunSn].unOutput.bit.bLed;
	    if(sBasicMegg[gunSn].unOutput_b.all != sBasicMegg[gunSn].unOutput.all)
	    {
	//        DLOG("�����źű仯 0x%x -> 0x%x",sBasicMegg[gunSn].unOutput_b.all,sBasicMegg[gunSn].unOutput.all);
	        sBasicMegg[gunSn].unOutput_b.all = sBasicMegg[gunSn].unOutput.all;
	    }
	}
}

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
/*��ʱ���ж�*/
static void ccuTimerIrq(void *receiver)
{
	CTRL_HEART_LED();
}

/*��ʱ����ʼ��*/
static void ccuTimerStartInit(TAppccuTask *self)
{
    TKfcTimer *os_timer = TKfcTimerSrvCreateTimer(&g_kfc_timer_srv);
    os_timer->receiver = self;
    os_timer->MethodOnTimerIrq = ccuTimerIrq;
    TKfcTimerSetInterval(os_timer, 1);
    TKfcTimerStart(os_timer);    
}

void devOpenAllInit(TAppccuTask *self)         //��������IO���豸���ȵĳ�ʼ������
{
	

    KfcDevOpen(kSfDevNumLog);                    //232����
    KfcDevOpen(kSfDevNumPowerRunLed);            //������
	ccuTimerStartInit(self);                    //1ms��ʱ��
	
    //KfcDevOpen(kSfDevNumPowerOutWdogFeedDog);    //���Ź�
    //KfcDevOpen(kSfDevNumPowerBeep);              //������
    KfcDevOpen(kSfDevNumPowerLockCtrl1);           //gun1��������Դ�������
    KfcDevOpen(kSfDevNumPowerLockCtrl2);           //gun2��������Դ�������
    
    KfcDevOpen(kSfDevNumPowerIsoCtrl1);            //gun1-PEͶ�п���-�����߶�
    KfcDevOpen(kSfDevNumPowerIsoCtrl2);            //gun2-PEͶ�п���-�����߶�
    KfcDevOpen(kSfDevNumPowerSw1Ctr1);             //gun1-��Ͷ�п���
    KfcDevOpen(kSfDevNumPowerSw1Ctr2);             //gun1-��Ͷ�п���
    KfcDevOpen(kSfDevNumPowerSw2Ctr1);             //gun2-��Ͷ�п���
    KfcDevOpen(kSfDevNumPowerSw2Ctr2);             //gun2-��Ͷ�п���

    KfcDevOpen(kSfDevNumPowerDoCtr1);              //gun1-bms������Դ����
    KfcDevOpen(kSfDevNumPowerDoCtr2);              //gun1��������Դ����
    KfcDevOpen(kSfDevNumPowerDoCtr3);              //gun1-DC�Ӵ������ƣ�K1��K2��
    KfcDevOpen(kSfDevNumPowerDoCtr4);              //gun2-bms������Դ����
    KfcDevOpen(kSfDevNumPowerDoCtr5);              //gun2��������Դ����
    KfcDevOpen(kSfDevNumPowerDoCtr6);              //gun2-DC�Ӵ������ƣ�K1��K2��

    KfcDevOpen(kSfDevNumPowerDoCtr7);              //�м�Ӵ���1����
    KfcDevOpen(kSfDevNumPowerDoCtr8);              //�м�Ӵ���2����
    KfcDevOpen(kSfDevNumPowerDoCtr9);              //�м�Ӵ���3����
    KfcDevOpen(kSfDevNumPowerDoCtr10);             //AC�Ӵ���1����
    //KfcDevOpen(kSfDevNumPowerDoCtr11);           //AC�Ӵ���2����
    
    KfcDevOpen(kSfDevNumPowerDoCtr12);             //ϵͳ���ȿ���
    KfcDevOpen(kSfDevNumPowerDoCtr13);             //ϵͳָʾ��1����-����
    KfcDevOpen(kSfDevNumPowerDoCtr14);             //ϵͳָʾ��2����-���

	//KfcDevOpen(kSfDevNumPowerDoCtr15);           //Ԥ��1
	//KfcDevOpen(kSfDevNumPowerDoCtr16);           //Ԥ��2


	
    KfcDevOpen(kSfDevNumSwitchDi01);               //��ͣ����״̬
    KfcDevOpen(kSfDevNumSwitchDi02);               //�Ž�����״̬
    KfcDevOpen(kSfDevNumSwitchDi03);               //����������״̬
    KfcDevOpen(kSfDevNumSwitchDi04);               //gun1-DC�Ӵ���״̬
    KfcDevOpen(kSfDevNumSwitchDi05);               //gun1ֱ���۶���״̬
    KfcDevOpen(kSfDevNumSwitchDi06);               //gun1���ǹ������״̬
    KfcDevOpen(kSfDevNumSwitchDi07);               //gun2-DC�Ӵ���״̬
    KfcDevOpen(kSfDevNumSwitchDi08);               //gun2ֱ���۶���״̬
    KfcDevOpen(kSfDevNumSwitchDi09);               //gun2���ǹ������״̬
    KfcDevOpen(kSfDevNumSwitchDi10);               //ĸ���Ӵ���1״̬
    KfcDevOpen(kSfDevNumSwitchDi11);               //ĸ���Ӵ���2״̬
    KfcDevOpen(kSfDevNumSwitchDi12);               //ĸ���Ӵ���3״̬
    KfcDevOpen(kSfDevNumSwitchDi13);               //�������߶�·����բ����
    KfcDevOpen(kSfDevNumSwitchDi14);               //�����Ӵ���״̬
    KfcDevOpen(kSfDevNumSwitchDi15);               //����ϵͳ���ȽӴ���״̬
    //KfcDevOpen(kSfDevNumSwitchDi16);               //(ϵͳ���ȹ���״̬FG)Ԥ��
    //KfcDevOpen(kSfDevNumSwitchDi17);               //Ԥ��
    //KfcDevOpen(kSfDevNumSwitchDi18);               //Ԥ��
    //KfcDevOpen(kSfDevNumSwitchDi19);               //Ԥ��
    //KfcDevOpen(kSfDevNumSwitchDi20);               //Ԥ��

	

	KfcDevOpen(kSfDevNumAdcSysTempInput);          //ϵͳ�¶Ȳ���(�����)
    KfcDevOpen(kSfDevNumAdcSysTempOutput);         //ϵͳ�¶Ȳ���(�����)
    KfcDevOpen(kSfDevNumAdcAPosTemp);              //ǹ1�����¶Ȳ���
    KfcDevOpen(kSfDevNumAdcANegTemp);              //ǹ1�����¶Ȳ���
    KfcDevOpen(kSfDevNumAdcBPosTemp);              //ǹ2�����¶Ȳ���
    KfcDevOpen(kSfDevNumAdcBNegTemp);              //ǹ2�����¶Ȳ���
    KfcDevOpen(kSfDevNumAdcAPosIso);               //ǹ1��Ե����ѹ����
    KfcDevOpen(kSfDevNumAdcANegIso);               //ǹ1��Ե����ѹ����
    KfcDevOpen(kSfDevNumAdcBPosIso);               //ǹ2��Ե����ѹ����
    KfcDevOpen(kSfDevNumAdcBNegIso);               //ǹ2��Ե����ѹ����
    KfcDevOpen(kSfDevNumAdcAOutVol);               //ǹ1��ѹ����
    KfcDevOpen(kSfDevNumAdcBOutVol);               //ǹ2��ѹ����
    KfcDevOpen(kSfDevNumAdcACurr);                 //ǹ1��������
    KfcDevOpen(kSfDevNumAdcBCurr);                 //ǹ2��������
	KfcDevOpen(kSfDevNumAdcACc1Vol);				  //ǹ1-cc1d��ѹ����
	KfcDevOpen(kSfDevNumAdcBCc1Vol); 			  //ǹ2-cc1d��ѹ����

    KfcDevOpen(kSfDevNumCan0);                   //
    KfcDevOpen(kSfDevNumCan1);
    KfcDevOpen(kSfDevNumCan2);
	
}

