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
#define BATACT_DISCHARGE       2 //活化放电
#define BATACT_CHARGE          3

#define PTU_NOMINAL     5
#define CTU_NOMINAL     24
#define PTU_LIMIT       90           //90%
#define CTU_LIMIT       62           //62%

static BYTE g_bBatActStep = BATACT_NORMAL;
static bool g_fYxPowerOff = true;     //YX断电，表示PT，CT，蓄电池对YX,GPRS,WL全部断电

bool IsExPowerOff(DWORD dwPTU, DWORD dwCTU)
{    
    if ((dwPTU < PTU_NOMINAL*PTU_LIMIT) && (dwCTU < CTU_NOMINAL*CTU_LIMIT))//PT、CT小于额定值的70%   ,放大了100倍
        return true;
    
    return false;
}

//选择供电电源
void PowerSel(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{    
    bool fYX = g_fYxPowerOff;
    if (dwPTU >= PTU_NOMINAL*PTU_LIMIT)//当PT有电时，关断PB26
    {
        BatYxOff();//PT有电时，开关另外有电给遥信
        g_fYxPowerOff = false;
        if (fYX)
            DTRACE(1, ("PowerMgt : YX power supply by PT!\r\n"));
    }
    else //PT停电
    {
        if (dwCTU >= CTU_NOMINAL*CTU_LIMIT)//CT有电
        {
            BatYxOn(); 
            g_fYxPowerOff = false;
            if (fYX)
                DTRACE(1, ("PowerMgt : YX power supply by CT!\r\n"));
        }
        else//CT停电
        {
            if (dwXdBatU < 1080)//过放保护     //活化时不会进入这里
            {  
                g_fYxPowerOff = true;
                
                if (IsModemPowerOn())
                {
                    DTRACE(DB_TASK, ("PowerMgt : Bat power too low, Cut down GPRS!\r\n"));
                    ModemPowerOff();//关闭GPRS、无线    todotodo:通信线程会不断地拔号重连
                }
                if (CommIsOpen(COMM_WL))
                {
                    DTRACE(DB_TASK, ("PowerMgt : Bat power too low, Cut down Wireless!\r\n"));
                    CommClose(COMM_WL);
                }
                WlModeOff();                    
            
                if (!fYX) //由有电进入断电
                    DTRACE(1, ("PowerMgt : PT&CT is power off, BatV is too Low, Cut down YX&Debug Comm power!\r\n"));
                
                BatYxOff();            //当PT和CT断电且蓄电池小于10.8V时关断PB26
            }
            else if ((dwXdBatU >= 1080) && (dwXdBatU < 1150))
            {//保持不变，防止开关开关开关跳动，导致YX不准             
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
    
    if (IsExPowerOff(dwPTU, dwCTU))//PT、CT小于额定值的70%          
    {        
        //SetLed(false, LED_POWER);   //主电源指示灯灭        该引脚与FLASH ERASE引脚冲突        
    }
    else if ((dwPTU > PTU_NOMINAL*PTU_LIMIT) || (dwCTU > CTU_NOMINAL*CTU_LIMIT))
    {//电源正常，恢复所有功能
        //SetLed(true, LED_POWER);                       
        if (IsWlModeOff())  //无线被切断电
        {                        
            if (!CommIsOpen(COMM_WL))
            {                
                const DWORD dwBaudIndex[] = {300, 600, 1200, 4800, 9600, 19200, 38400, 57600, 115200};                
                CommOpen(COMM_WL, dwBaudIndex[g_tWlPara.bBaud], 8, ONESTOPBIT, NOPARITY);
            }
        }//只要电源管理线程恢复供电，通信线程会自动恢复GPRS通信        
    }
    else//保持不变        //防止振荡，延长电池寿命
    {  
        //SetLed(true, LED_POWER);        
    }
}

void StopCharge(void)
{
    CTChargeOff();
    PTChargeOff();
    ChargeLedOff();     //充电指示灯灭
}

//取PWM浮充充电占空比
WORD GetPwmChargeDuty(DWORD dwXdBatU, WORD wUpLimit) //142
{
    static WORD wDuty = 0;    
    if (dwXdBatU < wUpLimit) 
    {        
        wDuty += 1;                             //步进为1
        if (wDuty > MAX_DUTY_CYCLE)
            wDuty = MAX_DUTY_CYCLE;
    }
    else if (dwXdBatU > wUpLimit) //电压大于142，占空比迅速下降到0
    {                
        if (wDuty > 1)
            wDuty -= 1;
        else
            wDuty = 0;
    }
    return wDuty;
}

//PWM充电
//dwPTU - PT 电压
//dwCTU - CT 电压
//dwXdBatU - 蓄电池电压
//wUpLimit - 充电上限电压
//true 正在充电
bool PwmCharge(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU, WORD wUpLimit)
{
    bool fRet = false;
    WORD wDuty;    
    wDuty = GetPwmChargeDuty(dwXdBatU, wUpLimit);
    if (wDuty)
        fRet = true;
    
    if (dwPTU >= PTU_NOMINAL*PTU_LIMIT) //PT供电正常
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
    
    //PT,CT都不正常不充电
    StopCharge();    
    return false;
}

//活化充电
//充满返回true，
bool BatActCharge(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{            
    
    static DWORD dwClick;
    static BYTE bChargeState = 0;
            
    if (bChargeState == 0) //首次进入活化充电状态
    {
        dwClick = GetClick();
        bChargeState = 1;
    }    
    if ((dwXdBatU >= 1480) || (GetClick()-dwClick >= 24*60*60))//已充满或持续充电24小时
    {
        StopCharge();        
        bChargeState = 0;
        return true;
    }     
    
    //活化充电指示
    if (PwmCharge(dwPTU, dwCTU, dwXdBatU, 1480))
        ChargeLedToggle();
    else
        ChargeLedOff();

    return false;
}

//PT,CT充电控制
//返回1，正在充电,返回0,不在充电
void PtCtChargeCtrl(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{        
    /*if (dwXdBatU >= 1430)//过充
    {
        //StopCharge();
        //return;
    }*/
    
    if (dwXdBatU < 1250)  //恢复充电
    {
    }
    
    //正常使用时的充电指示
    if (PwmCharge(dwPTU, dwCTU, dwXdBatU, 1420))
        ChargeLedToggle();
    else
        ChargeLedOff();
}

//执行电池活化
void DoBatAct(void)
{
    g_bBatActStep = BATACT_START;
}

//取消活化
void CancelBatAct(void)
{
    g_bBatActStep = BATACT_NORMAL;
}

void BatMonitor(DWORD dwPTU, DWORD dwCTU, DWORD dwXdBatU)
{    
    if (IsExPowerOff(dwPTU, dwCTU))//如果外部电源断电不进行电池活化
    {
        g_bBatActStep = BATACT_NORMAL;
        BatActOff();        
    }    
    
    //活化灯指示
    if (g_bBatActStep != BATACT_NORMAL) 
        BatActLedToggle();
    else
        BatActLedOff();
    
    //蓄电池电量指示
    if (dwXdBatU >= 1150)  //电量正常
    {
        SetLed(true, LED_BAT_ENERGY);
    }
    else if ((dwXdBatU < 1150) && (dwXdBatU >= 1080)) //电池将欠压
    {
        BatEgyLedToggle();//电池电量指示灯慢闪
    }
    else
        SetLed(false, LED_BAT_ENERGY);   //关闭蓄电池电量指示
    
    switch (g_bBatActStep)
    {
    case BATACT_START:
        DTRACE(DB_TASK, ("PowerMgt : Start bat active!\r\n"));
        StopCharge();
        BatActOn();
        g_bBatActStep = BATACT_DISCHARGE;
        break;
    case BATACT_DISCHARGE:        
        if (dwXdBatU < 800) //蓄电池电压低于8V
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
    
    ReadItemEx(BN0, PN0, 0xc330, p);  //内部校准        //todo:如果交采还没有准备好
    //todo:读0xc330
    wValue = ByteToWord(p);
    p += 2;    
    wValue = (wValue*300)/4095;     //基准为3V，放大100倍
    dwPTU = wValue*11;             //直实值
    
    wValue = ByteToWord(p);
    p += 2;    
    wValue = (wValue*300)/4095;     //基准为3V，放大100倍
    dwCTU = wValue*11;             //直实值

    wValue = ByteToWord(p);
    p += 2;    
    wValue = (wValue*300)/4095;     //基准为3V，放大100倍
    dwXdBatU = wValue*11;             //直实值

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