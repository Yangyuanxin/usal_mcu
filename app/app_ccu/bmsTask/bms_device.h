/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   bms_device
����:   km
˵��:   bms���ݴ���.
***************************************************************************************************/

#ifndef __BMS_DEVICE_H__                     //���ذ���.
#define __BMS_DEVICE_H__
/**************************************************************************************************/

/***************************************************************************************************
����:   ͷ�ļ�.
***************************************************************************************************/
#include "base_type.h"
#include "fun.h"

#define MAX_CHARGER_CURRENT_OFFSETT_GB   4000//������ƫ����
#define MAX_CHARGER_CURRENT_OFFSETT_PT   8000//������ƫ����

//BMSͨ��Э���е�CAN��ַ����
#define GB_CHARGER_CAN_ADDR 0x56 //�������CAN��ַ
#define GB_BMS_CAN_ADDR     0xF4 //����BMS��CAN��ַ
#define PT_BMS_CAN_ADDR     0xF4 //���ڵر�BMS��CAN��ַ
#define PT_CHARGER_CAN_ADDR 0xE5 //���ڵر����CAN��ַ

#define CK_BMS_CAN_ADDR     0xF5 //���Թ�װBMSĿ���ַ
#define CK_CHARGER_CAN_ADDR 0x55 //���Թ�װ���׮Ŀ���ַ

typedef enum    //bms���͵ĸ���PF������������.
{
    BMS_BRM_TYPE = 0,
    BMS_BCP_TYPE,
    BMS_BRO_TYPE,
    BMS_BCL_TYPE,
    BMS_BCS_TYPE,
    BMS_BSM_TYPE,
    BMS_BMV_TYPE,
    BMS_BMT_TYPE,
    BMS_BSP_TYPE,
    BMS_BST_TYPE,
    BMS_BSD_TYPE,
    BMS_BEM_TYPE,
    BMS_BHM_TYPE,

    BMS_BCM_TYPE = 23,
}BMS_PF_TYPE;

//�����յ�BMS���ĵĸ���֡����
#define BMS_BRM_DATA_LEN                             64    //BMS�ͳ�����ʶ (���)
#define BMS_BCP_DATA_LEN                             16    //��س����� (���)
#define BMS_BRO_DATA_LEN                             8     //BMS��ɳ��׼��
#define BMS_BCL_DATA_LEN                             8     //��س������
#define BMS_BCS_DATA_LEN                             16    //��س����״̬ (���)
#define BMS_BSM_DATA_LEN                             8     //���״̬��Ϣ
#define BMS_BMV_DATA_LEN                             8     //���嶯�����ص�ѹ(��������Ȳ���)
#define BMS_BMT_DATA_LEN                             8     //���������¶�(��������Ȳ���)
#define BMS_BSP_DATA_LEN                             8     //��������Ԥ������(��������Ȳ���)
#define BMS_BST_DATA_LEN                             8     //BMS ��ֹ���
#define BMS_BSD_DATA_LEN                             8     //BMS ͳ������
#define BMS_BEM_DATA_LEN                             8     //BMS ������
#define BMS_BHM_DATA_LEN                             8     //BHM ���ֱ���
#define MAX_CAN_FREAME_DATA_LEN                      64    //������������ֽڳ���


//BMS�ĸ���֡���ݴ���������е�����
#define K_CAN_FRAME_BRM_02_START_INDEX 0
#define K_CAN_FRAME_BCP_06_START_INDEX (K_CAN_FRAME_BRM_02_START_INDEX+BMS_BRM_DATA_LEN)
#define K_CAN_FRAME_BRO_09_START_INDEX (K_CAN_FRAME_BCP_06_START_INDEX+BMS_BCP_DATA_LEN)
#define K_CAN_FRAME_BCL_10_START_INDEX (K_CAN_FRAME_BRO_09_START_INDEX+BMS_BRO_DATA_LEN)
#define K_CAN_FRAME_BCS_11_START_INDEX (K_CAN_FRAME_BCL_10_START_INDEX+BMS_BCL_DATA_LEN)
#define K_CAN_FRAME_BSM_13_START_INDEX (K_CAN_FRAME_BCS_11_START_INDEX+BMS_BCS_DATA_LEN)
#define K_CAN_FRAME_BST_19_START_INDEX (K_CAN_FRAME_BSM_13_START_INDEX+BMS_BSM_DATA_LEN)
#define K_CAN_FRAME_BSD_1C_START_INDEX (K_CAN_FRAME_BST_19_START_INDEX+BMS_BST_DATA_LEN)
#define K_CAN_FRAME_BEM_1E_START_INDEX (K_CAN_FRAME_BSD_1C_START_INDEX+BMS_BSD_DATA_LEN)
#define K_CAN_FRAME_BHM_27_START_INDEX (K_CAN_FRAME_BEM_1E_START_INDEX+BMS_BHM_DATA_LEN)

#define K_CAN_FRAME_BCM_41_START_INDEX (K_CAN_FRAME_BHM_27_START_INDEX+BMS_BHM_DATA_LEN)

#define K_CAN_FRAME_LEN_END (K_CAN_FRAME_BCM_41_START_INDEX+BMS_BEM_DATA_LEN)


typedef union
{
    uint8_t all;
    struct
    {
        uint8_t  bexit:1;//�����������յ����˱���
        uint8_t  brecv:1;//��ǰ�׶��յ��˱��ģ����Ĵ�����ɺ�ᱻ����
        uint8_t  bit2:1;
        uint8_t  bit3:1;
        uint8_t  bit4:1;
        uint8_t  bit5:1;
        uint8_t  bit6:1;
        uint8_t  bit7:1;
     }bit;
}unionBmsFrameFlag;

typedef union 
{
    uint16_t all;
    struct
    {
        uint16_t bMaxSingleVol:12;//������ѹ
        uint16_t  bGroup:4;//������ѹ���
     }bit;
}unionBatteryInfo1;

typedef union 
{
    uint8_t all;
    struct
    {
        uint8_t  bInsDetFail:2;//����Ե״̬
        uint8_t  bcontactorStatus:2;//������״̬
        uint8_t  bChargeAllow:2;//�������λ
        uint8_t  bit6:1;
        uint8_t  bit7:1;
     }bit;
}unionBsmByte7Info;

typedef struct 
{
    const uint8_t pfTypeID;
    const uint8_t gbPf;
    const uint8_t ptPf;
    const uint16_t dataStartIndex;
    const uint8_t maxLen;
    unionBmsFrameFlag flag[2];  //˫ǹ
}BmsFrameStruct;



typedef struct 
{
    char Data[4];
}BmsCemFrameStruct;

typedef struct 
{
    char brmBmsVersion[3];//BMSͨ��Э��汾��
    uint8_t brmBatteryType;//�������
    int16_t brmBatteryCapacity;//������������ϵͳ�����/A. h ,O.l A  1/λ��
    int16_t brmBatteryTotalVol;//������������ϵͳ��ܵ�ѹ/V�� O.l V/  λ
    uint8_t brmBatteryFactory[4];//��س�������
    uint8_t brmBatterySn[4];//��������
    uint8_t brmBatteryDateYear;//��������������꣬-1985
    uint8_t brmBatteryDateMonth;//���������������
    uint8_t brmBatteryDateDay;//���������������    
    uint8_t brmBatteryChargeCycles[3];//����������
    uint8_t brmBatteryProperty;//������Ȩ 0���� 1������
    char brmVin[17];//����ʶ����(V1N)
    uint8_t brmSoftVersion[8];//BMS����汾
}BrmFrameData;

typedef struct 
{
    int16_t bcpSingleMaxVol;//���嶯����������������ѹ  �Ŵ�100�� 0.01V/bit
    int16_t bcpMaxChargerCurrent; //������������ �����Ѵ���� �Ŵ�10�� 0.1A/bit
    int16_t bcpMaxChargerVoltage;//����������ѹ �Ŵ�10�� 0.1V/bit
    int16_t bcpBatteryTotalEnergy;//�������ر��������  0.1kWh/λ
    uint8_t  bcpMaxChargerTemperature;//��������¶� ���ݷֱ���:1 ��/λ,ƫ����-50
    uint16_t bcpStartSoc;//�������SOC 0.1%
    int16_t bcpCurBatteryVoltage;//��ǰ��ص�ѹ �Ŵ�10�� 0.1V/bit
}BcpFrameData;

typedef struct 
{
    uint8_t broReadyState;//0-δ���� 1-����
}BroFrameData;

typedef struct 
{
    int16_t bclVoltageDemand;//BMS�����ѹ �Ŵ�10�� 0.1V/bit         
    int16_t bclCurrentDemand;//BMS������� �Ŵ�10�� 0.1A/bit         
    uint8_t  bclChargerMode;//���ģʽ(Ox01: ��ѹ��� ;Ox02: �������)
}BclFrameData;


typedef struct 
{
    int16_t bcsChargerMeasureVoltage;//����ѹ����ֵ �Ŵ�10�� 0.1V/bit
    int16_t bcsChargerMeasureCurrent;//����������ֵ �Ŵ�10�� 0.1A/bit
    unionBatteryInfo1 bcsSingleInfo;//�����ѹ��Ϣ
    uint8_t  bcsCurSoc;//��ǰSOC 1%
    uint16_t bcsChargerLeftTime;//ʣ����ʱ�� min
}BcsFrameData;

typedef struct 
{
    uint8_t bsmMaxSingleVolGroup;//��ߵ��嶯�����ص�ѹ���ڱ��
    uint8_t bsmMaxTemperature;//��߶��������¶�  ���ݷֱ���:1 ��/λ,ƫ����-50
    uint8_t bsmMaxTemperatureGroup;//����¶ȼ�����
    uint8_t bsmMinTemperature;//��Ͷ��������¶�
    uint8_t bsmMinTemperatureGroup;//��Ͷ��������¶ȼ�����
    uint8_t bsmBatteyState;//���״̬��Ϣ
    unionBsmByte7Info bsmByte7;//����״̬�ͳ������
}BsmFrameData;

//HARDWARE_CHECK_MODE
typedef union check_states
{
    uint8_t all;
    struct 
    { 
        uint8_t substate               :1;    // BIT0-3 ��״̬
        uint8_t insul110kres               :1;    //
        uint8_t res2               :1;    //
        uint8_t bmsFansEn               :1;    //

        
        uint8_t bmsLoardInContactEn                   :1;    // �ڲ�����
        uint8_t bmsLoardOutContactEn                  :1;    // �ⲿ����
        uint8_t bmsGunContactEn                       :1;    // K5K6
        uint8_t bmsModeOn                             :1;    // ����ģ��
    }bit;
}check_states;

typedef struct 
{
    check_states ms_state;//����״̬
    uint8_t bsmvol;         //��Դ��ѹ
    uint16_t gunvol;         //����¶ȼ�����
    uint32_t meterenerge;    //������
}BcmFrameData;


typedef struct 
{
    uint8_t bsdSoc;//��ֹSOC 1%
    uint16_t bsdMinSingleVol;
    uint16_t bsdMaxSingleVol;
    uint8_t bsdMinTemp;
    uint8_t bsdMaxTemp;
}BsdFrameData;

typedef struct 
{
    uint16_t bhmMaxChargerVol;//BMS����������ѹ �Ŵ�10�� 0.1V/bit
}BhmFrameData;



typedef struct 
{
    BrmFrameData brmData;
    BcpFrameData bcpData;
    BroFrameData broData;
    BclFrameData bclData;
    BcsFrameData bcsData;
    BsmFrameData bsmData;
    BsdFrameData bsdData;
    BhmFrameData bhmData;

    BcmFrameData bcmData;
    
    uint8_t brmSa;
    uint8_t bcpSa;
    
    BmsCemFrameStruct cemFrameSendData;
    char canFrameString[K_CAN_FRAME_LEN_END];
}BmsFrameData;


typedef struct 
{
    uint32_t chargerTimeSecondCnt;//���������ʱ
    uint32_t bmsStopCause;//�ڲ�ֹͣԭ�� 0-���� ����BMSͨ��ģ��ֹͣ���
    uint32_t bmsStopCause2015First;//2015����������ԣ������¼����ǰ��ֹͣԭ��
    uint16_t bcpStartSoc;//�����ʼSOC
    uint16_t insulationVol;//��Ե����ѹ
    uint16_t bsmStopTimeCnt;//BSM״̬���������ʱ
    uint8_t  chargerCnt;//�����������
    uint8_t  cemType;//CEM���ĳ�ʱ����
    uint8_t  cem22ErrorCnt;
    uint8_t csd1DSendCnt;
    uint16_t croAASendCnt;//5s��ŷ���0xAA
}BmsData;

typedef struct 
{
    union
    {
        uint32_t all;
        struct
        {
            uint32_t  bStartChargerCmdFlag:1;
            uint32_t  bit1:1;//�Ӵ���״̬
            uint32_t  bInsulationResultFault:1;//��Ե����� 0-���� 1-�쳣������쳣�Ͳ�����CRM�����˲���λBMS״̬Ϊ����
            uint32_t  bChargerIsFlag:1;//CC1û�Ͽ��³��������
            uint32_t  bGb2015ProtocolFlag:1;          //GB2015Э���־
            uint32_t  bit6:1;
            uint32_t  bit7:1;
            uint32_t  bit8_31:25;
         }bit;
     }unStateA;
     
     uint8_t cc1State,cc1bak;
     
     int16_t chargerMaxOutputVol;
     int16_t chargerMinOutputVol;
     int16_t chargerMaxOutputCur;
     
     int16_t curChargerOutputVol;//��ǰ�����ѹ
     int16_t curChargerOutputCur;//��ǰ�������
     
     int16_t iChVdcRef;//��ǰ�·�ģ���ѹ
     int16_t iChIdcRef;//��ǰ�·�ģ�����
     
     uint32_t meterElectricEnergy;//��ǰ����ܵ���
     
     uint32_t externStopCause;//�ⲿֹͣԭ�� 0-���� ����Ϊ�˹�ֹͣ���������
}BmsExternalData;//�ⲿģ���ṩ��BMSʹ�õ�����

enum SysMianState//ϵͳ��״̬
{
    IDLE=0, //����
    WAIT_START,//�ȴ����ָ��
    SHAKE_HAND,//���ֽ׶�
    SETTING,//���ý׶�
    CHARGING,//���׶�
    CHARG_OVER,//�������׶�
    ERROR_STAGE,//����׶�
    BMS_MAIN_STATE_NUM_NULL
};

enum SysSubState//ϵͳ��״̬
{
    SEND_CHM = 0,
    SEND_CRM01_00,
    SEND_CRM01_01,
    RECV_BCP06,
    SEND_CTS07,
    SEND_CCM40,//HARDWARE_CHECK_MODE
    SEND_CML08,
    RECV_BRO09,
    SEND_CRO0A,
    
    RECV_BCL10,
    RECV_BCS11,
    SEND_CCS12,
    RECV_BSM13,
    RECV_BMV15,
    RECV_BMT16,
    RECV_BSP17,
    RECV_BST19,
    SEND_CST1A,
    
    RECV_BSD1C,
    SEND_CSD1D,
    RECV_BEM1E,
    SEND_CEM1F,
    SEND_CEM1F_END,
};


typedef union
{
    uint32_t all;
    struct
    {
        uint32_t  bPowerState:1;//���ػ�״̬ 0-BMS����  1-���  //bsm�������
        uint32_t  bComChargingStageFlag:1;//����BMS������׶�
        uint32_t  bProtocolStable:1; //Э������ȷ����־
        uint32_t  bBmsFault:1;//BMS����
        uint32_t  bBmsChargerStopNormal:1;//�������ֹͣ
        uint32_t  bBmsComFault:1;//BMSͨ���쳣,����
        uint32_t  bGb2015ProtocolFlag:1;          //1 GB2015Э���־
        uint32_t  bInsulationCmdFlag:1;           //��Ե����������Ч��־
        
        uint32_t  bReChargerFlag:1;//�س���־
        uint32_t  bBSMPause:1;
        uint32_t  bitA:1;
        uint32_t  bitB:1;
        uint32_t  bitC:1;
        uint32_t  bitD:1;
        uint32_t  bitE:1;
        uint32_t  bitF:1;
        
        uint32_t  bit10_1F:16;
     }bit;
}unionBmsStateA;
     
typedef union
{
    uint32_t all;
    struct
    {
        uint32_t  bBmsComState:1;//BMSͨ��״̬ 0-���� 1-����ͨ����
        uint32_t  bit1_F:15;
        uint32_t  bit10_1F:16;
     }bit;
}unionBmsStateB;

typedef struct 
{
    uint8_t mainState;//��״̬
    uint8_t subState;//��״̬
    
    uint8_t bmsType;
    
    unionBmsStateA  unBmsStateA;
    unionBmsStateB  unBmsStateB;
    
    BmsFrameData frameData;
    BmsData bmsData;
}BmsDevice;




/***************************************************************************************************
����:   ���б���.
***************************************************************************************************/
extern BmsDevice bmsDevice[2];
extern BmsExternalData bmsExternData[2];
extern unionSysClockFlag unBmsTaskClockFlag;
extern structSysClockCnt sBmsTaskClockCnt;

/***************************************************************************************************
����:   ���з���.
***************************************************************************************************/
void bms_device_init(uint8_t gunSn);
void bms_device_charger_handle(void);
void bms_charger_start(uint8_t gunSn);

/**
  * @brief  �ⲿ��ģ�飨����������ô˺�������ȡBMSģ�������Ϣ������
  */
extern const BmsFrameData* getBmsFrameDataHandle(uint8_t gunSn);
extern const BmsDevice* getBmsDeviceHandle(uint8_t gunSn);

#endif  //__BMS_H__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/

