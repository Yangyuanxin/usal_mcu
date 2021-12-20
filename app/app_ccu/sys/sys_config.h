/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   sys_config
����:   km
˵��:   ϵͳ����.
***************************************************************************************************/


#ifndef __SYS_CONFIG_H
#define __SYS_CONFIG_H

#include"base_type.h"

//ϵͳ��������ѹ������
#define GUN_MAX_VOLT         	10400		 //���ǹ��������ѹ
#define GUN_MAX_CURR         	5000		 //���ǹ����������
#define MOD_MAX_ADDR         	32      	 //ϵͳģ��������������ж�Ӧ����[1]����1��ģ��
#define VOLTAGE_FLOOR           500		     //���ǹ��С�����ѹ
#define SYS_MAX_TEMP         	1500		 //ϵͳ�������¶�
#define SYS_MIN_TEMP         	-500		 //ϵͳ�����С�¶�

//У׼��Χ
#define kCaliMax	6144			
#define kCaliMid	4096			
#define kCaliMin	2048			
#define kZeroMax	1024			
#define kZeroMid	0			
#define kZeroMin	-1024			

//�洢��Ч��־
#define SYS_CONFIG_VALID_FLAG 0x5AA52288


typedef struct 
{
    char sysSn[32];//���׮����   
}structSysConfigChar;

#pragma pack(push)
#pragma pack(1)
typedef struct 
{
    int32_t uPorjectType;              //��Ŀ���� ENUM_PROJECTTYPE
    int32_t uSysType;                  //����ԴENUM_SYS_TYPE
    int32_t uModuleType;               //ģ������  
	int32_t uModNum;			       //ģ������-�ܣ����У�
	int32_t unitModuleNum;             //ÿ�鵥Ԫ��ģ������
	int32_t uGhMode;                   //�Ӵ���Ⱥ����ͣ�1����ģʽ 2�ȵ��ȳ�
	int32_t uModLoadValue;             //ģ�鸺����
	int32_t uModLimit;                 //ģ�������
    int32_t uLowNoise;                 //ģ�齵��ģʽ    
    
    int32_t uGunNum;                   //ǹ��
    int32_t uGunType;                  //���ǹ����
    int32_t bmsType;                   //BMS����ENUM_BMSTYPE
    int32_t uBmsAxType;                //��Դ���� 0:12V 1:24V
    int32_t uMeterType;                //�������
    int32_t uRsvIoType;                //Ԥ��IO����
	int32_t uParallelMode;			   //����ģʽ
    int32_t iAGun_ScalSysPortVdcCali[2];  //����˵�ѹУ׼   
    
    int32_t iAGun_ScalSysPortVdcZero[2];   
    int32_t iAGun_ScalModVdcCali[2];      //ģ���ѹУ׼   
    int32_t iAGun_ScalModVdcZero[2];   

    //����
    int32_t uMaxChargeVolt;            	//����������ѹ
    int32_t uMaxChargeCurr;            	//������������
    int32_t uPowerLimit_10x;           	//��������繦�� 
    int32_t uInsdetRes;                	//��Ե��ֵ
    int32_t uMaxAllowChSOC;            	//�������SOC
    int32_t uGunTempProtect;           	//���ǹ����ֵ
    int32_t uEnvTmepHigh;            	//
    int32_t uEnvTmepLow;           	    //
	int32_t uOutletTempUp_10x;			//�ڲ�����
    int32_t uDcOverVolt;               	//��ѹֵ 
    
    int32_t uDcUnderVolt;              	//Ƿѹֵ
    int32_t uDcoverCurr;               	//����ֵ
    int32_t uAcOverVolt;               	//�����ѹֵ
    int32_t uAcUnderVolt;              	//����Ƿѹֵ
	int32_t uDoorProtectEn;            	//�Ž�����
	int32_t uBreakerCheckEn;           	//��·�����
	int32_t uAcContCheckEn;            	//����Ӵ������
	int32_t uBatProtectEn;             	//������ر���
	
	int32_t uBatStatueEn;              	//����״̬����
	//int32_t uBMSAxGuding;              	//�̶�bms ��Դ��ѹ�����ָ�12

	uint32_t validFlag;//��־
    uint32_t crc;
}structSysConfigInt;//�̶�4�ֽ�,MAX->W25_PAGE_SIZE
#pragma pack(pop)


typedef struct 
{
    int32_t WorkMode;                  //����ģʽ ENUM_WORKMODE��Զ��ΪIDLEʱ�����л�set_workmode
    int32_t LoadType;                  //�������� ENUM_LOADTYPE
    int32_t v_local;                   //���������ѹ
    int32_t i_local;                   //�����������

    int32_t GunUnLock[2];                 //Ӧ������
    int32_t uRenZhengEn;               //��֤ģʽʹ��
    
    int32_t Debug1;
    int32_t Debug2;
    int32_t getModMegg;                //��ȡģ����Ϣָ��
    int32_t modFanspeed;               //ģ������ٶ�
    int32_t sysFanspeed;               //ϵͳ�����ٶ�
}structSysConfigTempInt;//vClientSinexcelConfigData

typedef struct 
{
    int32_t para[32];
    char str1[64];                 
    uint8_t str2[64];                 
}structSysConfigUnstandard;//�Ǳ걣�ֲ���

typedef struct 
{
    structSysConfigInt sInt;            //����--����
    structSysConfigTempInt sTempInt;    //����--������
    structSysConfigChar sChar;          //�ַ���-����
    structSysConfigUnstandard sUnstandard;//�Ǳ�-����
}structSysConfig;
extern structSysConfig sSysConfig;

typedef enum
{
    DEBUG1_TCU = 1,             //��TCUͨ��
    DEBUG1_NOERROR,             //��������
    DEBUG1_PROCESS,             //����ģ�⣬�޹���


    DEBUG1_FLASH_CLEAR = 1024,         //���FLASH    
    DEBUG1_MAX,
}ENUM_DEBUG1;

typedef enum
{
    SYSTYPE_FASTSLOW = 0,             //������
	SYSTYPE_CONTACTOR_GROUP,	  //�Ӵ���Ⱥ��
    
    SYSTYPE_MAX,
}ENUM_PROJECTTYPE;

typedef enum
{
    BMS_GB_TYPE = 0,
    BMS_GB_2015_TYPE = 5,
    BMS_MAX_TYPE,
}ENUM_BMSTYPE;

typedef enum
{
    WORK_REMOTE = 0,
    WORK_LOCAL,
}ENUM_WORKMODE;

typedef enum
{
    LOAD_BAT = 0,
    LOAD_R,
}ENUM_LOADTYPE;

typedef enum
{
    MOD_TYPE_0 = 0,
		
	MOD_TYPE_INFY_30KW,      //Ӣ��Դ30kwģ��
	MOD_TYPE_WINLINE_30KW,	 //����30kwģ��
}ENUM_MODULETYPE; //ģ������

typedef enum
{
    ENUM_GUN_TYPE2 = 2,    
    ENUM_GUN_NONE  = 4,     //����ǹ    

}ENUM_GUN_TYPE;             //���ǹ����

typedef enum
{
    BOARD_MAIN = 1,         //����

    BOARD_ADDR_MAX  = 13,   //���12ǹ�����ж�Ӧ����[1]����1ǹ    
}ENUM_BOARD_ADDR;           //���ӵ�ַ

void sys_config_init(uint32_t sysConfigValidFlag);
void set_sys_config_save(void);
void record_manage_sys_config_init(uint32_t sysConfigValidFlag);
void voltage_cali(int32_t idata,uint8_t gunSn);

void sys_debug_set(uint16_t de);
uint16_t sys_debug_get(void);

#endif


/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
