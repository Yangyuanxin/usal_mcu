/***************************************************************************************************   
Copyright @ 2021-20xxС�ۿƼ�

�ļ�:   tick_handle
����:   km
˵��:   ϵͳʱ��.
***************************************************************************************************/
#include "tick_handle.h"         
#include <string.h>
#include "time_cur.h"

uint8_t     g_uSys1msFlag = 0;                   //1ms������־.
uint8_t     g_uSys1msEvent = 0;                  //1ms�¼�.0-9.
uint8_t     g_uSys10msEvent = 0;                 //10ms�¼�.0-9.
uint8_t     g_uSys100msEvent = 0;                //100ms�¼�.0-9.

uint32_t    g_uSys10msEventCount = 0;                   //10ms�¼�.�ۻ�����.
uint32_t    g_uSys100msEventCount = 0;                 //100ms�¼�.�ۻ�����.
uint32_t    g_uSys1sEventCount = 0;                    //1s�¼�.�ۻ�����.
uint32_t    g_uSys1sEventCountInt = 0;                 //�ж�1s�¼�.�ۻ�����.

BOOL      g_bSysMsgTimer10ms = BOOL_FALSE;    //10ms��Ϣ.
BOOL      g_bSysMsgTimer100ms = BOOL_FALSE;   //100ms��Ϣ.
BOOL      g_bSysMsgTimer1s = BOOL_FALSE;      //1s��Ϣ.

BOOL      g_bSysTimer10msEventFlag = BOOL_FALSE;    //10ms�¼���־.
BOOL      g_bSysTimer100msEventFlag = BOOL_FALSE;   //100ms�¼���־.
BOOL      g_bSysTimer1sEventFlag = BOOL_FALSE;      //1s�¼���־.

static struct TimerList *timerHead=0;

void sys_timer1ms_int(void);
void sys_timer1ms(void);
void sys_timer1s_int(void);

void sys_timer10ms(void);
void sys_timer100ms(void);
void sys_timer1s(void);

void timer_task_init(void)
{
    
}

void timer_add_task(TimerList *timerList)
{
    TimerList *pTimeList = timerHead;
    timerHead = timerList;
    timerList->next = pTimeList;
}

void timer_init_struct(TimerList *timerList,int32_t time,void(*function)(void))
{
    if ((!timerList)||(!function)||(time < 1))
    {
        return;
    }
    
    timerList->next = 0;
    timerList->runIntervalTime = time;
    timerList->function = function;
}

void timer_task_run(void)
{
    TimerList *pTimeList = timerHead;
    while(pTimeList)
    {
        if (pTimeList->expiresTime > 0)
        {
            pTimeList->expiresTime--;
            if(pTimeList->expiresTime == 0)
            {
                if (pTimeList->function)
                {
                    pTimeList->function();
                }
                
                pTimeList->expiresTime = pTimeList->runIntervalTime;
            }
        }
        else
        {
            if (pTimeList->function)
            {
                pTimeList->function();
            }
            pTimeList->expiresTime = pTimeList->runIntervalTime;
        }
        
        pTimeList = pTimeList->next;
    }
}

/***************************************************************************************************
����:   ϵͳ��ʱ10ms�¼�����.�ж���.
����:   ��.
����:   ��.
˵��:   ���ж���.�������ֶ�ʱ��Ϣ.
***************************************************************************************************/
void sys_timer1ms_int(void)
{
    g_uSys1msEvent++;
    g_uSys1msFlag = BOOL_TRUE;
    if(g_uSys1msEvent >= 10)
    {
        g_uSys1msEvent = 0;
        g_bSysMsgTimer10ms = BOOL_TRUE;                 //��ʱ10ms��Ϣ.
        g_uSys10msEvent++;
        if (g_uSys10msEvent >= 10)
        {
            g_uSys10msEvent = 0;
            g_bSysMsgTimer100ms = BOOL_TRUE;            //��ʱ100ms��Ϣ.
            g_uSys100msEvent++;
            if (g_uSys100msEvent >= 10)
            {
                g_uSys100msEvent = 0;
                g_uSys1sEventCountInt++;
                g_bSysMsgTimer1s = BOOL_TRUE;           //��ʱ1s��Ϣ.
            }
        }
    }
}

/***************************************************************************************************
����:   ϵͳʵʱ�¼�����.
����:   ��
����:   ��
˵��:   ��������main()ѭ������,ʵ�ֳ���ѭ��.
        �����������þ���ȷ����ÿһ��ѭ��ʱҪ������.
        �����жϴ�������,������������������������õı�Դ.
***************************************************************************************************/
void sys_real_time_handle(void)
{
    g_bSysTimer10msEventFlag = BOOL_FALSE;    //10ms�¼���־.
    g_bSysTimer100msEventFlag = BOOL_FALSE;   //100ms�¼���־.
    g_bSysTimer1sEventFlag = BOOL_FALSE;      //1s�¼���־.
    
    if (g_uSys1msFlag == BOOL_TRUE)             //1ms��Ϣ.
    {
         g_uSys1msFlag =  BOOL_FALSE;
         sys_timer1ms();
    }
    if (g_bSysMsgTimer10ms == BOOL_TRUE)        //10ms��Ϣ.
    {
        g_bSysMsgTimer10ms = BOOL_FALSE;
        g_bSysTimer10msEventFlag = BOOL_TRUE;
        sys_timer10ms();
    }   
    if (g_bSysMsgTimer100ms == BOOL_TRUE)       //100ms��Ϣ.
    {
        g_bSysMsgTimer100ms = BOOL_FALSE;
        g_bSysTimer100msEventFlag = BOOL_TRUE;
        sys_timer100ms();
    }
    if (g_bSysMsgTimer1s == BOOL_TRUE)          //1s��Ϣ.
    {
        g_bSysMsgTimer1s = BOOL_FALSE;
        g_bSysTimer1sEventFlag = BOOL_TRUE;
        sys_timer1s();
    }
}

/***************************************************************************************************
����:   ϵͳ��ʱ1ms�¼�����.
����:   ��.
����:   ��.
***************************************************************************************************/
void sys_timer1ms(void)
{
    timer_task_run();
}

/***************************************************************************************************
����:   ϵͳ��ʱ10ms�¼�����.
����:   ��.
����:   ��.
***************************************************************************************************/
void sys_timer10ms(void)
{
    g_uSys10msEventCount++;
}
/***************************************************************************************************
����:   ϵͳ��ʱ100ms�¼�����.
����:   ��.
����:   ��.
***************************************************************************************************/
void sys_timer100ms(void)
{
    g_uSys100msEventCount++;
}

/***************************************************************************************************
����:   ϵͳ��ʱ1s�¼�����.
����:   ��.
����:   ��.
***************************************************************************************************/
void sys_timer1s(void)
{
    g_uSys1sEventCount++;
    timer1s_handle();
}


/***************************************************************************************************
����:   .
����:   ��.
����:   g_uSys1sCount.
***************************************************************************************************/
uint32_t get_poweron_times(void)
{
    return g_uSys1sEventCount;
}
/***************************************************************************************************
�ļ�����.
***************************************************************************************************/


