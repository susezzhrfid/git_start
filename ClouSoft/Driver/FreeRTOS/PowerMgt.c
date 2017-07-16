#include "PowerMgt.h"
#include "Drivers.h"
#include "FaCfg.h"
#include "Sysarch.h"
#include "ComAPI.h"
#include "LibDbConst.h"
#include "SysDebug.h"
#include "Comm.h"
#include "DrvCfg.h"
#include "LibDbAPI.h"
#include "Si4432.h"

#define BATACT_NORMAL          0
#define BATACT_START           1
#define BATACT_DISCHARGE       2 //��ŵ�
#define BATACT_CHARGE          3

#define PTU_NOMINAL     5
#define CTU_NOMINAL     24
#define PTU_LIMIT       90           //90%
#define CTU_LIMIT       62           //62%

static BYTE g_bBatActStep = BATACT_NORMAL;
static bool g_fYxPowerOff = true;     //YX�ϵ磬��ʾPT��CT�����ض�YX,GPRS,WLȫ���ϵ�

bool IsExPowerOff(DWORD dwPTU, DWORD dwCTU)
{    
    if ((dwPTU < PTU_NOMINAL*PTU_LIMIT) && (dwCTU < CTU_NOMINAL*CTU_LIMIT))//PT��CTС�ڶֵ��70%   ,�Ŵ���100��
        return true;
    
    return false;
}

//ѡ�񹩵��Դ
void PowerSel(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{    
    bool fYX = g_fYxPowerOff;
    if (dwPTU >= PTU_NOMINAL*PTU_LIMIT)//��PT�е�ʱ���ض�PB26
    {
        BatYxOff();//PT�е�ʱ�����������е��ң��
        g_fYxPowerOff = false;
        if (fYX)
            DTRACE(1, ("PowerMgt : YX power supply by PT!\r\n"));
    }
    else //PTͣ��
    {
        if (dwCTU >= CTU_NOMINAL*CTU_LIMIT)//CT�е�
        {
            BatYxOn(); 
            g_fYxPowerOff = false;
            if (fYX)
                DTRACE(1, ("PowerMgt : YX power supply by CT!\r\n"));
        }
        else//CTͣ��
        {
            if (dwXdBatU < 1080)//���ű���     //�ʱ�����������
            {  
                g_fYxPowerOff = true;
                
                if (IsModemPowerOn())
                {
                    DTRACE(DB_TASK, ("PowerMgt : Bat power too low, Cut down GPRS!\r\n"));
                    ModemPowerOff();//�ر�GPRS������    todotodo:ͨ���̻߳᲻�ϵذκ�����
                }
                if (CommIsOpen(COMM_WL))
                {
                    DTRACE(DB_TASK, ("PowerMgt : Bat power too low, Cut down Wireless!\r\n"));
                    CommClose(COMM_WL);
                }
                WlModeOff();                    
            
                if (!fYX) //���е����ϵ�
                    DTRACE(1, ("PowerMgt : PT&CT is power off, BatV is too Low, Cut down YX&Debug Comm power!\r\n"));
                
                BatYxOff();            //��PT��CT�ϵ�������С��10.8Vʱ�ض�PB26
            }
            else if ((dwXdBatU >= 1080) && (dwXdBatU < 1150))
            {//���ֲ��䣬��ֹ���ؿ��ؿ�������������YX��׼             
            }
            else //dwXdBatU >= 1150
            {
                BatYxOn(); 
                g_fYxPowerOff = false;
                if (fYX)
                    DTRACE(1, ("PowerMgt : YX power supply by Bat!\r\n"));
            }
        }        
    }
    
    if (IsExPowerOff(dwPTU, dwCTU))//PT��CTС�ڶֵ��70%          
    {        
        //SetLed(false, LED_POWER);   //����Դָʾ����        ��������FLASH ERASE���ų�ͻ        
    }
    else if ((dwPTU > PTU_NOMINAL*PTU_LIMIT) || (dwCTU > CTU_NOMINAL*CTU_LIMIT))
    {//��Դ�������ָ����й���
        //SetLed(true, LED_POWER);                       
        if (IsWlModeOff())  //���߱��жϵ�
        {                        
            if (!CommIsOpen(COMM_WL))
            {                
                const DWORD dwBaudIndex[] = {300, 600, 1200, 4800, 9600, 19200, 38400, 57600, 115200};                
                CommOpen(COMM_WL, dwBaudIndex[g_tWlPara.bBaud], 8, ONESTOPBIT, NOPARITY);
            }
        }//ֻҪ��Դ�����ָ̻߳����磬ͨ���̻߳��Զ��ָ�GPRSͨ��        
    }
    else//���ֲ���        //��ֹ�񵴣��ӳ��������
    {  
        //SetLed(true, LED_POWER);        
    }
}

void StopCharge(void)
{
    CTChargeOff();
    PTChargeOff();
    ChargeLedOff();     //���ָʾ����
}

//ȡPWM������ռ�ձ�
WORD GetPwmChargeDuty(DWORD dwXdBatU, WORD wUpLimit) //142
{
    static WORD wDuty = 0;    
    if (dwXdBatU < wUpLimit) 
    {        
        wDuty += 1;                             //����Ϊ1
        if (wDuty > MAX_DUTY_CYCLE)
            wDuty = MAX_DUTY_CYCLE;
    }
    else if (dwXdBatU > wUpLimit) //��ѹ����142��ռ�ձ�Ѹ���½���0
    {                
        if (wDuty > 1)
            wDuty -= 1;
        else
            wDuty = 0;
    }
    return wDuty;
}

//PWM���
//dwPTU - PT ��ѹ
//dwCTU - CT ��ѹ
//dwXdBatU - ���ص�ѹ
//wUpLimit - ������޵�ѹ
//true ���ڳ��
bool PwmCharge(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU, WORD wUpLimit)
{
    bool fRet = false;
    WORD wDuty;    
    wDuty = GetPwmChargeDuty(dwXdBatU, wUpLimit);
    if (wDuty)
        fRet = true;
    
    if (dwPTU >= PTU_NOMINAL*PTU_LIMIT) //PT��������
    {
        CTChargeOff();
        PTChargeOn(wDuty);
        return fRet;
    }
    
    if (dwCTU >= CTU_NOMINAL*CTU_LIMIT)
    {
        PTChargeOff();
        CTChargeOn(wDuty);
        return fRet;
    }   
    
    //PT,CT�������������
    StopCharge();    
    return false;
}

//����
//��������true��
bool BatActCharge(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{            
    
    static DWORD dwClick;
    static BYTE bChargeState = 0;
            
    if (bChargeState == 0) //�״ν������״̬
    {
        dwClick = GetClick();
        bChargeState = 1;
    }    
    if ((dwXdBatU >= 1480) || (GetClick()-dwClick >= 24*60*60))//�ѳ�����������24Сʱ
    {
        StopCharge();        
        bChargeState = 0;
        return true;
    }     
    
    //����ָʾ
    if (PwmCharge(dwPTU, dwCTU, dwXdBatU, 1480))
        ChargeLedToggle();
    else
        ChargeLedOff();

    return false;
}

//PT,CT������
//����1�����ڳ��,����0,���ڳ��
void PtCtChargeCtrl(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{        
    /*if (dwXdBatU >= 1430)//����
    {
        //StopCharge();
        //return;
    }*/
    
    if (dwXdBatU < 1250)  //�ָ����
    {
    }
    
    //����ʹ��ʱ�ĳ��ָʾ
    if (PwmCharge(dwPTU, dwCTU, dwXdBatU, 1420))
        ChargeLedToggle();
    else
        ChargeLedOff();
}

//ִ�е�ػ
void DoBatAct(void)
{
    g_bBatActStep = BATACT_START;
}

//ȡ���
void CancelBatAct(void)
{
    g_bBatActStep = BATACT_NORMAL;
}

void BatMonitor(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{    
    if (IsExPowerOff(dwPTU, dwCTU))//����ⲿ��Դ�ϵ粻���е�ػ
    {
        g_bBatActStep = BATACT_NORMAL;
        BatActOff();        
    }    
    
    //���ָʾ
    if (g_bBatActStep != BATACT_NORMAL) 
        BatActLedToggle();
    else
        BatActLedOff();
    
    //���ص���ָʾ
    if (dwXdBatU >= 1150)  //��������
    {
        SetLed(true, LED_BAT_ENERGY);
    }
    else if ((dwXdBatU < 1150) && (dwXdBatU >= 1080)) //��ؽ�Ƿѹ
    {
        BatEgyLedToggle();//��ص���ָʾ������
    }
    else
        SetLed(false, LED_BAT_ENERGY);   //�ر����ص���ָʾ
    
    switch (g_bBatActStep)
    {
    case BATACT_START:
        DTRACE(DB_TASK, ("PowerMgt : Start bat active!\r\n"));
        StopCharge();
        BatActOn();
        g_bBatActStep = BATACT_DISCHARGE;
        break;
    case BATACT_DISCHARGE:        
        if (dwXdBatU < 800) //���ص�ѹ����8V
        {
            BatActOff();
            g_bBatActStep = BATACT_CHARGE;
        }        
        break;
    case BATACT_CHARGE:
        if (BatActCharge(dwPTU, dwCTU, dwXdBatU))
        {
            g_bBatActStep = BATACT_NORMAL; 
            DTRACE(DB_TASK, ("PowerMgt : Bat active over\r\n"));
        }
        break;
    case BATACT_NORMAL:
        PtCtChargeCtrl(dwPTU, dwCTU, dwXdBatU);
        break;
    default:
        break;
    }    
}

void DoPowerMgt(void)
{       
    BYTE bBuf[16];
    DWORD dwPTU;
    DWORD dwCTU;    
    DWORD dwXdBatU; 
    BYTE *p = bBuf;
    WORD wValue;
    
    ReadItemEx(BN0, PN0, 0xc330, p);  //�ڲ�У׼        //todo:������ɻ�û��׼����
    //todo:��0xc330
    wValue = ByteToWord(p);
    p += 2;    
    wValue = (wValue*300)/4095;     //��׼Ϊ3V���Ŵ�100��
    dwPTU = wValue*11;             //ֱʵֵ
    
    wValue = ByteToWord(p);
    p += 2;    
    wValue = (wValue*300)/4095;     //��׼Ϊ3V���Ŵ�100��
    dwCTU = wValue*11;             //ֱʵֵ

    wValue = ByteToWord(p);
    p += 2;    
    wValue = (wValue*300)/4095;     //��׼Ϊ3V���Ŵ�100��
    dwXdBatU = wValue*11;             //ֱʵֵ

    //DTRACE(1, ("PowerMgt : PT = %d, CT = %d, XD = %d\r\n", dwPTU, dwCTU, dwXdBatU));
        
    //dwPTU = 500;//500
    //dwCTU = 2400;      
    //dwXdBatU = 0;//1425;
    
#if 0    //for Test BatAct
    static DWORD dwBatU = 1425;
    static BYTE bStep = 0;
    
    switch (bStep)
    {
    case 0:
        dwBatU = 1425;
        DoBatAct();
        bStep = 1;
        break;
    case 1:
        dwBatU -= 10;
        if (dwBatU < 750)
            bStep = 2;
        break;
    case 2:
        dwBatU += 10;
        if (dwBatU >= 1490)
            bStep = 3;
        break;
    case 3:
        break;
    default:
        break;
    }    
    dwXdBatU = dwBatU;
#endif     
            
    PowerSel(dwPTU, dwCTU, dwXdBatU);
    BatMonitor(dwPTU, dwCTU, dwXdBatU); 
}

bool IsPowerOff(void)
{
    //return g_fYxPowerOff;
    
    //todo:for test!
    return false;
}