/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   tick_handle
����:   km
˵��:   ϵͳʱ��.
***************************************************************************************************/

#ifndef __TICK_HANDLE_H__                     //���ذ���.
#define __TICK_HANDLE_H__

#include "base_type.h"

typedef union
{
    uint16_t all;
    struct
    {    
        uint16_t  bEvent1ms:1;
        uint16_t  bEvent10ms:1;
        uint16_t  bEvent100ms:1;
        uint16_t  bEvent1000ms:1;
        uint16_t  bit4:1;
        uint16_t  bit5:1;
        uint16_t  bit6:1;
        uint16_t  bit7:1;
    }bit;
}TimeFlag;

typedef  struct TimerList
{
    struct TimerList *next;
    int32_t runIntervalTime;    //ִ�м��
    int32_t expiresTime;        //ʣ��ʱ��
    void (*function)(void);
}TimerList;


extern BOOL      g_bSysTimer10msEventFlag;    //10ms�¼���־.
extern BOOL      g_bSysTimer100msEventFlag;   //100ms�¼���־
extern BOOL      g_bSysTimer1sEventFlag;      //1s�¼���־

extern uint32_t    g_uSys10msEventCount;      //10ms�¼�.�ۻ�����.
extern uint32_t    g_uSys100msEventCount;     //100ms�¼�.�ۻ�����.
extern uint32_t    g_uSys1sEventCount;        //100ms�¼�.�ۻ�����.


void sys_real_time_handle(void);
void sys_timer1ms_int(void);
uint32_t get_poweron_times(void);
void timer1ms_handle_sys(void);
void timer_task_init(void);
void timer_add_task(TimerList *timerList);
void timer_init_struct(TimerList *timerList,int32_t time,void(*function)(void));

#endif  //__TICK_HANDLE_H__
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/

