/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   io_megg
����:   km
˵��:   ����IO�����IO��ADC��������.
***************************************************************************************************/

#ifndef IO_MEGG_H
#define IO_MEGG_H

#include "sys_config.h"
#include "os_thread.h"
#include "modyfy.h"
#include "can.h"


#define kCC1VoltScalCst  66 
#define kSysVdcScalCst2 7517//7573//7517         //����ѹ��������ϵ��
#define kSysPortNegOffset 931//924//931        //����ѹ����AD����������

#define kInsPosVdcScalCst 6892       //��Ե����ѹ��������ϵ��
#define kInsPosVdcScalOffset 513     //��Ե����ѹ����ƫ��ϵ��

#define kInsNegVdcScalCst 6892       //��Ե����ѹ��������ϵ��
#define kInsNegVdcScalOffset 513     //��Ե����ѹ����ƫ��ϵ��

#define kSysCurrentScalCst 170        //��������������ϵ��
#define kSysCurrentOffset 2047       //����������AD����������



typedef union 
{
    uint32_t all;
    struct
    {
        uint32_t bBMS_CAN       :1;
        uint32_t bGFD_Pos       :1;
        uint32_t bGFD_Neg       :1;
        uint32_t bGFD_PE        :1; 
        
        uint32_t bACCont_Out    :1;
        uint32_t bFan           :1;
        uint32_t bMidCont_Out0  :1;
        uint32_t bMidCont_Out1  :1;  
        
        uint32_t bMidCont_Out2  :1;
        uint32_t bRsvOut0       :1;
        uint32_t bRsvOut1       :1;
        uint32_t bRsvOut2       :1;
        
        uint32_t bRsvOut3       :1;
        uint32_t bLed           :1;
        uint32_t bBMS_V         :1;
        uint32_t bRsvOut4       :1;
        
        uint32_t bBMS_12V24     :1;
        uint32_t bDcCont_Out    :1;
        uint32_t bGunLock       :1;//�ⲿ����������ǹ�ܱ�־
        uint32_t bGunLock_PorN  :1;

        uint32_t bReserved      :12;
    }bit;
}unionOutputControl;

typedef enum
{
    ENUM_OUT_BMS_CAN= 0,    //BMS CAN ƥ������л�
    ENUM_OUT_GFD_POS,       //��Ե��������
    ENUM_OUT_GFD_NEG,       //��Ե��⸺���
    ENUM_OUT_GFD_PE,        //��Ե���PE
    
    ENUM_OUT_ACCONT,        //����Ӵ���
    ENUM_OUT_FAN,           //����
    ENUM_OUT_MIDCONT1,      //�м�Ӵ���01
    ENUM_OUT_MIDCONT2,      //�м�Ӵ���02

    ENUM_OUT_MIDCONT3,      //�м�Ӵ���03
    ENUM_OUT_RSV_OUT0,      //Ԥ�����00
    ENUM_OUT_RSV_OUT1,      //Ԥ�����01
    ENUM_OUT_RSV_OUT2,      //Ԥ�����02
    
    ENUM_OUT_RSV_OUT3,      //Ԥ�����03
    ENUM_OUT_LED,           //����ָʾ��
    ENUM_OUT_BMS_V,         //BMS A+A- ���
    ENUM_OUT_RSV_OUT4,      //Ԥ�����04
    
    ENUM_OUT_BMS_12V24,     //BMS A+A- 12V24V�л�
    ENUM_OUT_DCCONT,        //����Ӵ���
    ENUM_OUT_GUNLOCK,       //ǹ��12V 
    ENUM_OUT_GUNLOCK_PORN,  //ǹ������

    ENUM_OUT_MAX,
}ENUM_OUPUT_CONTROL;        //���IO

typedef union 
{
    uint32_t all;
    struct
    {
        uint32_t bDoor          :1;    
        uint32_t bFuse          :1;
        uint32_t bCC1           :1;
        uint32_t bSPD           :1;

        uint32_t bMidCont1_In   :1;
        uint32_t bMidCont2_In   :1;
        uint32_t bMidCont3_In   :1;
        uint32_t bRsvIn1        :1;

        uint32_t bRsvIn2        :1;
        uint32_t bRsvIn3        :1;
        uint32_t bRsvIn4        :1;
        uint32_t fan_In         :1; 

        uint32_t bGunLock       :1; //����Ϊ�ߵ�ƽ
        uint32_t bGunBack       :1; //����Ϊ�ߵ�ƽ
        uint32_t bEPO           :1; //����Ϊ�ߵ�ƽ
        uint32_t bBreaker       :1;        
        
        uint32_t bACCont_In     :1;
        uint32_t bDcCont_In     :1;

        uint32_t bReserved      :14;
    }bit;
}unionInputBack;

/*typedef struct 
{
    int32_t Door          ;
    int32_t Fuse          ;
    int32_t CC1           ;
    int32_t SPD           ;
    
    int32_t RsvIn2        ;
    int32_t RsvIn3        ;
    int32_t RsvIn4        ;
    int32_t RsvIn5        ;
    
    int32_t RsvIn6        ;
    int32_t RsvIn7        ;
    int32_t RsvIn8        ;
    int32_t UVP           ;
    
    int32_t GunLock       ;
    int32_t GunBack       ;
    int32_t EPO           ;
    int32_t Breaker       ;
    
    int32_t ACCont_In     ;
    int32_t MidCont_In    ;
    int32_t DcCont_In    ;

}structInputCnt;*/

typedef enum
{    
    ENUM_IN_DOOR,       //�Ž�
    ENUM_IN_FUSE,       //�۶���
    ENUM_IN_CC1,        //CC1
    ENUM_IN_SPD,        //������

    ENUM_IN_MIDCONT1,   //�м�Ӵ���1
	ENUM_IN_MIDCONT2,	//�м�Ӵ���2
	ENUM_IN_MIDCONT3,	//�м�Ӵ���3
    ENUM_IN_RSV1,       //Ԥ��1

    ENUM_IN_RSV2,       //Ԥ��2
    ENUM_IN_RSV3,       //Ԥ��3
    ENUM_IN_RSV4,       //Ԥ��4
    ENUM_IN_FAN,        //���Ʒ��ȼ̵�������

    ENUM_IN_GUNLOCK,    //ǹ��
    ENUM_IN_GUNBACK,    //ǹ��λ��1�ѹ�λ
    ENUM_IN_EPO,        //��ͣ
    ENUM_IN_BREAKER,    //�����·��

    ENUM_IN_ACCONT,     //����Ӵ���
    ENUM_IN_DCCONT,    //����Ӵ���

    ENUM_IN_MAX,
}ENUM_INPUT_BACK;       //����IO

typedef struct 
{
	UNLONG unModPortVdc;
	UNLONG unSysPortVdc;
	UNLONG unGFDPosVdc;
	UNLONG unGFDNegVdc;
	UNLONG unCc1Volt;	
	UNLONG unSysPortIdc;

	UNLONG unInletTemp;
	UNLONG unOutletTemp;
	UNLONG unGunPosTemp;
	UNLONG unGunNegTemp;

	int32_t VModCali;
	int32_t VModCaliZero;
	int32_t VGunCali;
	int32_t VGunCaliZero;
	int32_t IGunCali;
	int32_t IGunCaliZero;
}structCalRms;

typedef struct 
{
    unionInputBack unInput,unInput_b;
    unionOutputControl unOutput,unOutput_b;
    //structInputCnt sInputCnt;
    structCalRms sCalRms;
}structBasicMegg;

typedef struct 
{
    uint8_t gunlock;
    int16_t iGunLockNeg12VCnt;
    int16_t iGunLockNeg12VTime;
    int16_t iGunLockPos12VCnt;
    int16_t iGunLockPos12VTime;
    uint8_t lockst;
    uint8_t openst;    
    int32_t UnLockCnt;

}structGunLock;

typedef struct 
{
    uint8_t BoardAddr; // 1 ~ n
	uint16_t version;//��ǹ�汾
	uint16_t version_gun0;//1ǹ�汾
    uint8_t CC1,CC1_bak;

	int16_t v_Mod;      //10x
	int16_t v_Gun;      //10x -241~1041V
	int16_t v_GFD_P;    //10x
	
	int16_t v_GFD_N;     //10x
    int16_t v_CC1_10x;   // 
	int16_t i_Gun;//��ǹ����
	int16_t i_Gun_All[BOARD_ADDR_MAX];//[1]����1�Ű�
	int16_t t_Inlet;
	
	int16_t t_Outlet;
	int16_t t_Gun;
	int16_t t_GunP;
	int16_t t_GunN;

    uint16_t hz_10x;
    uint16_t vab_10x;
    uint16_t vac_10x;
    uint16_t vbc_10x;

    unionOutputControl unOut;
}structSysBasicMegg;//�ɼ�����10x
extern structSysBasicMegg sSysMegg[2];//������ѹ���������¶ȡ����io���Ƶ�����

uint8_t get_Out_IO(ENUM_OUPUT_CONTROL eOut,uint8_t gunSn);//���IO״̬
uint8_t get_In_IO(ENUM_INPUT_BACK eIn,uint8_t gunSn);//����IO״̬
void vSys_Basic_Init(void);//GPIO��ʼ��
void vCcu_IO_Ctr(void);
BOOL get_DcCont_State(uint8_t gunSn);
BOOL get_accont_statue(void);
void devOpenAllInit(TAppccuTask *self);
int  send_mod_can_frame(T_CAN_MSG *t_msg);
int8_t tcu_com_can_send(CAN_MSG_T *pt_msg,uint8_t gunSn);




#endif

/***************************************************************************************************
�ļ�����.
***************************************************************************************************/
