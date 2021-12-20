/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   sys_type_contactor_group.h
����:   km
˵��:   �Ӵ���Ⱥ������߼�ͷ�ļ�.
***************************************************************************************************/
#ifndef SYS_TYPE_CONTACTOR_GROUP_H
#define SYS_TYPE_CONTACTOR_GROUP_H

#include "fun.h"


#define GH_TWOGUN_MAX_GROUP 6
#define GH_TWOGUN_GUN_NUM 2

#define GH_MOD_MAX_NUM 32


typedef union
{
    uint8_t all;
    struct{
    uint8_t bCont1:1;
    uint8_t bCont2:1;
    uint8_t bCont3:1;
    uint8_t bCont4:1;
    uint8_t bCont5:1;

    uint8_t bRsv :3;    
    }bit;
}unContCtrlandBack;

typedef struct 
{
    char group_sim[GH_TWOGUN_MAX_GROUP]; //ÿ��ģ��Ӧ����ţ������У�
    char group_now[GH_TWOGUN_MAX_GROUP]; //ÿ��ģ���������
    char startseq[GH_TWOGUN_GUN_NUM];//����˳�� [0]���ȿ���ǹ��
    char group_num[GH_TWOGUN_GUN_NUM]; //ÿǹ����ģ������
    
    char init_flag;
    char mod_num_total;     //ģ������
    char mod_num_unit;      //ÿ����Ԫģ����
    char unit_num;          //��Ԫ��

    char relay_fault[GH_TWOGUN_MAX_GROUP],relay_fault_bak[GH_TWOGUN_MAX_GROUP];//�м�Ӵ����������� 1��һ���ϲ��� 2��������ճ��
    unContCtrlandBack unContst,unContErr;
    unContCtrlandBack unCont,unCont_Deal,unCont_bak; //unΪ�м���㣬DealΪIO����ִ�У��м�����ʱ
    char relay_wait[GH_TWOGUN_MAX_GROUP];//un->deal��ִ��
    int contFaultCnt[GH_TWOGUN_MAX_GROUP];
    char gunStart_bak[GH_TWOGUN_GUN_NUM];

	unsigned char uOrderNum;   //��������
	unsigned char uOrderPar;   //�������
    
    int stepcnt;
}GhModeTwoGunData;

void vGh2_contactor_group(void);
uint8_t vGetMidContCt_Gh2(void);
void vSetMidContSt_Gh2(uint8_t st);
uint8_t vGetMidContSt_Gh2(void);
void vMidContfault_Gh2(void);
uint8_t vGetModGroup_Gh2(uint8_t addr);
uint8_t vGetMidContErr_Gh2(void);
uint8_t vGetModGroupNum_Gh2(uint8_t gun); 


#endif


