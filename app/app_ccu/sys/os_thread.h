/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�: fun
����:   km
˵��:   ���ܺ���.
***************************************************************************************************/
#ifndef __OS_THREAD_H__                     
#define __OS_THREAD_H__
#include <ofc_object.h>
#include <kfc_task.h>

typedef struct TAppccuTaskTag TAppccuTask;
struct TAppccuTaskTag
{
PROTECTED
    TKfcTask parent_;                ///< �̳еĸ���
    TOfVirtualMethod Destroy_;      ///< ��������   
};

extern TAppccuTask bmsTaskHandle;
extern TAppccuTask chargeTaskHandle;
extern TAppccuTask printTaskHandle;
extern TAppccuTask modTaskHandle;
extern TAppccuTask can3TaskHandle;
//extern TAppccuTask recordTaskHandle;
//extern TAppccuTask meterTaskHandle;
//extern TAppccuTask usbTaskHandle;

void StartBMSTask(TOfObject *object);
void StartChargeTask(TOfObject *object);
void StartPrintTask(TOfObject *object);
void StartModTask(TOfObject *object);
void StartCan3Task(TOfObject *object);
//void StartFlashTask(void const * argument);
//void StartMeterTask(void const * argument);
//void StartUsbTask(void const * argument);

void StartBMSTaskCreate(TAppccuTask *self);
void StartChargeTaskCreate(TAppccuTask *self);
void StartPrintTaskCreate(TAppccuTask *self);
void StartModTaskCreate(TAppccuTask *self);
void StartTcuCanTaskCreate(TAppccuTask *self);


#endif  //__CFUN_H__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/


