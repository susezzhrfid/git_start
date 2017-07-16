/*********************************************************************************************************
 * Copyright (c) 2013,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Gpio.c
 * ժ    Ҫ�����ļ���Ҫ��IO����
 * ��ǰ�汾��1.0
 * ��    �ߣ�liyan
 * ������ڣ�2013��1��
 *********************************************************************************************************/
#include "Gpio.h"
#include "pmc.h"
#include "pio_handler.h"

void Gpio_Config(const Pin* pPins, uint32_t dwSize )
{  
    /* Configure pins */
    while ( dwSize > 0 )
    {
        if (( pPins->type ) == PIO_INPUT)
            pmc_enable_periph_clk(pPins->id);   
        pio_configure(pPins->pio, pPins->type, pPins->mask, pPins->attribute);

        pPins++ ;
        dwSize-- ;
    }
}

void Gpio_Clear(const Pin* pPin)
{
    pio_clear(pPin->pio, pPin->mask);
}

void Gpio_Set(const Pin* pPin)
{
    pio_set(pPin->pio, pPin->mask);
}

uint32_t Gpio_Get(const Pin* pPin)
{
    return pio_get(pPin->pio, pPin->type, pPin->mask);
}

void PIO_PinConfigureIt(const Pin *pPin, void (*handler)(uint32_t, uint32_t) )
{
    pio_handler_set(pPin->pio, pPin->id, pPin->mask, pPin->attribute, handler);
}

void PIO_PinEnableIt( const Pin *pPin )
{
    pio_enable_interrupt(pPin->pio, pPin->mask);
}

void PIO_PinDisableIt( const Pin *pPin )
{
    pio_disable_interrupt(pPin->pio, pPin->mask);
}

void PIO_PinSetDebounceFilter( const Pin* pPin, uint32_t dwCutOff )
{
    pio_set_debounce_filter(pPin->pio, pPin->mask, dwCutOff);
}