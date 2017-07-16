/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stdlib.h"
#include "SysDebug.h"
/* BSP includes. */
#include "board.h"
#include "emac.h"
//#include "emacd.h"
//#include "rstc.h"
//#include "macb.h"

/* lwIP includes. */
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#if LWIP_UDP
#include "lwip/udp.h"
#endif
#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif
#include "Gpio.h"
#include "sam3x_ek.h"
#include "rstc.h"
#include "sysclk.h"
#include "ethernet_phy.h"
#include "FaConst.h"

#include "AD.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 't'

#ifdef __ICCARM__          /* IAR */
#define __attribute__(...) /* IAR */
#endif                     /* IAR */

/*----------------------------------------------------------------------------
 *        Variables
 *----------------------------------------------------------------------------*/

struct xEthernetIf
{
	struct eth_addr *ethaddr;
	/* Add whatever per-interface state that is needed here. */
};

/* When a packet is ready to be sent, if it cannot be sent immediately then
 * the task performing the transmit will block for netifTX_BUFFER_FREE_WAIT
 * milliseconds.  It will do this a maximum of netifMAX_TX_ATTEMPTS before
 * giving up.
 */
#define netifTX_BUFFER_FREE_WAIT	( ( portTickType ) 2UL / portTICK_RATE_MS )
#define netifMAX_TX_ATTEMPTS		( 5 )

#define netifMAX_MTU 1500

static emac_options_t g_emac_option;

/** The IP address used for test (ping ...) */
/*
static uint8_t gs_uc_ip_address[] =
		{ ETHERNET_CONF_IPADDR0, ETHERNET_CONF_IPADDR1,
			ETHERNET_CONF_IPADDR2, ETHERNET_CONF_IPADDR3 };
*/
/** The EMAC driver instance */
static emac_device_t gs_emac_dev;

/** Buffer for ethernet packets */
static uint8_t gs_uc_eth_buffer[EMAC_FRAME_LENTGH_MAX];

/* The semaphore used to wake the interrupt handler task.  The peripheral
is processed at the task level to prevent the need to read the entire FIFO from
within the ISR itself. */
static xSemaphoreHandle xMACInterruptSemaphore = NULL;

static xSemaphoreHandle xEmacSendSem = NULL;

/*
 * Copy the received data into a pbuf.
 */
static struct pbuf *prvLowLevelInput( const unsigned char * const pucInputData, unsigned short usDataLength );

/*
 * Send data from a pbuf to the hardware.
 */
static err_t prvLowLevelOutput( struct netif *pxNetIf, struct pbuf *p );

/*
 * Perform any hardware and/or driver initialisation necessary.
 */
static void prvLowLevelInit( struct netif *pxNetIf );

/*
 * Emac RX interrupt callback
*/

static void EmacRxCb(uint32_t status);

/*
 *
*/
static void vMACTask( void *pvParameters );

extern void PhyReset(void);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param pxNetIf the already initialized lwip network interface structure
 *		for this etherpxNetIf
 */
static void prvLowLevelInit( struct netif *pxNetIf )
{
    unsigned portBASE_TYPE uxOriginalPriority, xStatus;
    uint32_t  dwDly;
    BYTE bMac[6];
    BYTE bAddr1[2];
    BYTE bAddr2[2];
    
    memset(bMac, 0, sizeof(bMac));
    ReadItemEx(10, 0, 0xa150, bMac); 
    ReadItemEx(10, 0, 0xa040, bAddr1);
    ReadItemEx(10, 0, 0xa041, bAddr2);

	/* Hardware initialisation can take some time, so temporarily lower the
	task priority to ensure other functionality is not adversely effected.
	The priority will get raised again before this function exits. */
	uxOriginalPriority = uxTaskPriorityGet( NULL );
	vTaskPrioritySet( NULL, tskIDLE_PRIORITY );

	/* set MAC hardware address length */
	pxNetIf->hwaddr_len = ETHARP_HWADDR_LEN;
    
    if (IsAllAByte(bMac, 0, 6))  //全0时自动生成。
    { //bit0-bit23由厂家申请
        //pxNetIf->hwaddr[ 0 ] = ETHERNET_CONF_ETHADDR0;
        //pxNetIf->hwaddr[ 1 ] = ETHERNET_CONF_ETHADDR1;
        //pxNetIf->hwaddr[ 2 ] = ETHERNET_CONF_ETHADDR2;        
        pxNetIf->hwaddr[ 0 ] = 0x36;
        pxNetIf->hwaddr[ 1 ] = 0x35;	    

        //bit24-bit47厂家自定义, bit48组播地址        
        memcpy(&pxNetIf->hwaddr[2], bAddr1, 2);  //取地址	    
        memcpy(&pxNetIf->hwaddr[4], bAddr2, 2);       
    }
    else
    {
        Swap(bMac, 6);
        memcpy(pxNetIf->hwaddr, bMac, 6);
    }
        
    DTRACE(0, ("MAC:%x-%x-%x-%x-%x-%x\r\n", pxNetIf->hwaddr[0], pxNetIf->hwaddr[1],
               pxNetIf->hwaddr[2], pxNetIf->hwaddr[3], pxNetIf->hwaddr[4], pxNetIf->hwaddr[5]));

	pxNetIf->name[0] = 'e';
	pxNetIf->name[1] = 't';

	/* device capabilities */
	pxNetIf->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	/* maximum transfer unit */
	pxNetIf->mtu = netifMAX_MTU;
    
    //MODE 0-1
    pio_configure_pin(PIN_EMAC_ERX0, PIO_TYPE_PIO_OUTPUT_1|PIO_DEFAULT);
	pio_configure_pin(PIN_EMAC_ERX1, PIO_TYPE_PIO_OUTPUT_1|PIO_DEFAULT);
    pio_configure_pin(PIN_EMAC_ECRSDV, PIO_TYPE_PIO_OUTPUT_1|PIO_DEFAULT);
    
    //PHYAD0
    pio_configure_pin(PIN_EMAC_ERXER, PIO_TYPE_PIO_OUTPUT_0|PIO_DEFAULT);
    
    /* Reset PHY */
    //rstc_set_external_reset(RSTC, 13); /* (2^(13+1))/32768 */    //我们的PHY复位引脚没有挂在RSTC的复位脚上
    //rstc_reset_extern(RSTC);
    //while ((rstc_get_status(RSTC) & RSTC_SR_NRSTL) == 0) {};
    PhyReset();
    /* Wait for PHY to be ready (CAT811: Max400ms) */
    dwDly = sysclk_get_cpu_hz() / 1000 / 3 * 400;
    while(dwDly--);
    
    pio_configure_pin(PIN_EEMAC_EREFCK, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETX0, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETX1, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETXEN, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ECRSDV, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERX0, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERX1, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERXER, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_EMDC, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_EMDIO, PIN_EMAC_FLAGS);

    /* Enable EMAC clock */
	pmc_enable_periph_clk(ID_EMAC);

	/* Fill in EMAC options */
	g_emac_option.uc_copy_all_frame = 0;
	g_emac_option.uc_no_boardcast = 0;

	memcpy(g_emac_option.uc_mac_addr, pxNetIf->hwaddr, pxNetIf->hwaddr_len);

	gs_emac_dev.p_hw = EMAC;

	/* Init EMAC driver structure */
	emac_dev_init(EMAC, &gs_emac_dev, &g_emac_option);

    NVIC_SetPriority(EMAC_IRQn, 8);
    
	/* Enable Interrupt */
	NVIC_EnableIRQ(EMAC_IRQn);

    /* Init MAC PHY driver */
    if (ethernet_phy_init(EMAC, BOARD_EMAC_PHY_ADDR, sysclk_get_cpu_hz())
                    != EMAC_OK) 
    {
        Sleep(500);
        if (ethernet_phy_init(EMAC, BOARD_EMAC_PHY_ADDR, sysclk_get_cpu_hz())
                    != EMAC_OK) 
        {
            DTRACE(0, ( "P: PHY Initialize ERROR!\n\r" )) ;
            pxNetIf->flags &= NETIF_FLAG_LINK_UP;
            return;
        }
    }
    else
    {
        DTRACE(0, ( "P: PHY Initialize OK!\n\r" )) ;
    }
    
	/* Reset the task priority back to its original value. */
	vTaskPrioritySet( NULL, uxOriginalPriority );

	//vSemaphoreCreateBinary( xMACInterruptSemaphore );
    xMACInterruptSemaphore = xSemaphoreCreateCounting(255, 0); 
	//xSemaphoreTake( xMACInterruptSemaphore, 0 );
    DTRACE(0, ("prvLowLevelInit ok\n"));
    
    vSemaphoreCreateBinary( xEmacSendSem );

	xStatus = xTaskCreate(vMACTask, ( signed portCHAR * ) "MAC_Task", 224/*192*/, pxNetIf, configMAX_PRIORITIES - 3, NULL );//优先级应该给TCPIP低

	configASSERT( xStatus == pdPASS );
}


/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param pxNetIf the lwip network interface structure for this etherpxNetIf
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *		 an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *	   strange results. You might consider waiting for space in the DMA queue
 *	   to become availale since the stack doesn't retry to send a packet
 *	   dropped because of memory failure (except for the TCP timers).
 */

static err_t prvLowLevelOutput( struct netif *pxNetIf, struct pbuf *p )  //有MAC线程与TCPIP线程调用这个函数
{

	/* This is taken from lwIP example code and therefore does not conform
	to the FreeRTOS coding standard. */

    struct pbuf *q;
    static unsigned char ucBuffer[ EMAC_FRAME_LENTGH_MAX ];   //这里共用了缓存
    unsigned char *pucBuffer = ucBuffer;
    unsigned char *pucChar;
    struct eth_hdr *pxHeader;
    err_t xReturn = ERR_OK;
    long x;

	( void ) pxNetIf;
    
    xSemaphoreTake( xEmacSendSem, (portTickType)(portMAX_DELAY/portTICK_RATE_MS));  //排队发送

	#if defined(LWIP_DEBUG) && LWIP_NETIF_TX_SINGLE_PBUF
		LWIP_ASSERT("p->next == NULL && p->len == p->tot_len", p->next == NULL && p->len == p->tot_len);
	#endif

	/* Initiate transfer. */
	if( p->len == p->tot_len ) 
	{
		/* No pbuf chain, don't have to copy -> faster. */
		pucBuffer = &( ( unsigned char * ) p->payload )[ ETH_PAD_SIZE ];
	} 
	else 
	{
		/* pbuf chain, copy into contiguous ucBuffer. */
		if( p->tot_len >= sizeof( ucBuffer ) )
		{
			LINK_STATS_INC( link.lenerr );
			LINK_STATS_INC( link.drop );
			//snmp_inc_ifoutdiscards( pxNetIf );
			xReturn = ERR_BUF;
		}
		else
		{
			pucChar = ucBuffer;

			for( q = p; q != NULL; q = q->next )
			{
				/* Send the data from the pbuf to the interface, one pbuf at a
				time. The size of the data in each pbuf is kept in the ->len
				variable. */
				/* send data from(q->payload, q->len); */
				LWIP_DEBUGF( NETIF_DEBUG, ( "NETIF: send pucChar %p q->payload %p q->len %i q->next %p\n", pucChar, q->payload, ( int ) q->len, ( void* ) q->next ) );
				if( q == p )
				{
					memcpy( pucChar, &( ( char * ) q->payload )[ ETH_PAD_SIZE ], q->len - ETH_PAD_SIZE );
					pucChar += q->len - ETH_PAD_SIZE;
				}
				else
				{
					memcpy( pucChar, q->payload, q->len );
					pucChar += q->len;
				}
			}
		}
	}

	if( xReturn == ERR_OK )
	{
		for( x = 0; x < netifMAX_TX_ATTEMPTS; x++ )
		{
			//NOTE:aynsc,when EMACD_Send return, tx may not finished
			//we don't know if tx success or not.
			//TODO:callback 
            xReturn = emac_dev_write(&gs_emac_dev, pucBuffer, p->tot_len, NULL);
			if( xReturn == EMAC_OK )
			{
				break;
			}
			else
			{
				vTaskDelay( netifTX_BUFFER_FREE_WAIT );
			}
		}

		if( xReturn != EMAC_OK )
		{
			LINK_STATS_INC( link.memerr );
			LINK_STATS_INC( link.drop );
			//snmp_inc_ifoutdiscards( pxNetIf );
			xReturn = ERR_BUF;
		}
		else
		{
			LINK_STATS_INC( link.xmit );
			//snmp_add_ifoutoctets( pxNetIf, usTotalLength );
			pxHeader = ( struct eth_hdr * )p->payload;
			if( ( pxHeader->dest.addr[ 0 ] & 1 ) != 0 ) 
			{
				/* broadcast or multicast packet*/
				//snmp_inc_ifoutnucastpkts( pxNetIf );
			} 
			else 
			{
				/* unicast packet */
				//snmp_inc_ifoutucastpkts( pxNetIf );
			}

			xReturn = ERR_OK;
		}
	}
    
    xSemaphoreGive( xEmacSendSem );

	return xReturn;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param pxNetIf the lwip network interface structure for this etherpxNetIf
 * @return a pbuf filled with the received packet (including MAC header)
 *		 NULL on memory error
 */
static struct pbuf *prvLowLevelInput( const unsigned char * const pucInputData, unsigned short usDataLength )
{
    struct pbuf *p = NULL, *q;

	if( usDataLength > 0U )
	{
		#if ETH_PAD_SIZE
			len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
		#endif

		/* We allocate a pbuf chain of pbufs from the pool. */
		p = pbuf_alloc( PBUF_RAW, usDataLength, PBUF_POOL );
  
		if( p != NULL ) 
		{
			#if ETH_PAD_SIZE
				pbuf_header( p, -ETH_PAD_SIZE ); /* drop the padding word */
			#endif

			/* We iterate over the pbuf chain until we have read the entire
			* packet into the pbuf. */
			usDataLength = 0U;
			for( q = p; q != NULL; q = q->next ) 
			{
				/* Read enough bytes to fill this pbuf in the chain. The
				* available data in the pbuf is given by the q->len
				* variable.
				* This does not necessarily have to be a memcpy, you can also preallocate
				* pbufs for a DMA-enabled MAC and after receiving truncate it to the
				* actually received size. In this case, ensure the usTotalLength member of the
				* pbuf is the sum of the chained pbuf len members.
				*/
				memcpy( q->payload, &( pucInputData[ usDataLength ] ), q->len );
				usDataLength += q->len;
			}

			#if ETH_PAD_SIZE
				pbuf_header( p, ETH_PAD_SIZE ); /* reclaim the padding word */
			#endif

			LINK_STATS_INC(link.recv);
		}
	}

	return p;  
}

/*
 * NOTE: we are in interrupt context!!!
*/
static void EmacRxCb(uint32_t status)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR( xMACInterruptSemaphore, &xHigherPriorityTaskWoken );
	//emac_dev_set_rx_callback(&gs_emac_dev, NULL); //set rx callback null disable RX interrupt???
    /* Switch to the uIP task. */
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

/**
 * \brief EMAC interrupt handler.
 */
void EMAC_Handler(void)
{
    __disable_interrupt();
	emac_handler(&gs_emac_dev);
    __enable_interrupt();
}

bool g_fDhcpStart = false;
//bool g_fDhcpRelease = false;
bool g_fDhcpStop = false;
//TODO:
extern bool   g_fEnDHCP;

void ReInitMac(struct netif *pxNetIf);

static void vMACTask( void *pvParameters )
{
    uint32_t frmlen;
    //uint32_t dwRstClick = 0;
    struct pbuf *p;
	struct netif *pxNetIf = ( struct netif * ) pvParameters;
	uint8_t rc;
    static bool fLinkUp = false;
    BYTE i/*, bRstCnt = 0*/;

	for( ;; )
	{
        if (GetInfo(INFO_MAC_RST))
        {
            //g_fDhcpStop = true;  //先释放之前DHCP的空间
            //Sleep(300); //最少250MS
            ReInitMac(pxNetIf);  //会导致重新UP/DOWN
            //dwRstClick = GetClick();
        }
		/* Wait for something to do. */
		if (xSemaphoreTake( xMACInterruptSemaphore, (portTickType)(1000/portTICK_RATE_MS) ) == pdTRUE)
		{
            for (i=0; i<EMAC_RX_BUFFERS; i++)
            {
                rc = emac_dev_read(&gs_emac_dev, (uint8_t *) gs_uc_eth_buffer, sizeof(gs_uc_eth_buffer), &frmlen);
	    		if (rc ==  EMAC_OK)
		    	{
                    //DTRACE(DRIVER_DEBUG, ("ET recv a frm!\n"));
    				if (frmlen >0 && (p = prvLowLevelInput(gs_uc_eth_buffer, frmlen)))
    				{
                        //TraceFrm("<-- ET Recv:", gs_uc_eth_buffer, frmlen);	
    					ethernet_input(p, pxNetIf);
                        //pxNetIf->input(p, pxNetIf);
    				}
    			}
                else  //读完为止
                    break;
            }

			//emac_dev_set_rx_callback(&gs_emac_dev, EmacRxCb); /*Re-enable RX*/
		}
		else /*no RX then we check link status*/
		{
			/*link status change into up satate*/
            rc = ethernet_phy_set_link(EMAC, BOARD_EMAC_PHY_ADDR, 0);
		    if ( rc == EMAC_OK )
		    {     
               //dwRstClick = GetClick();
               //bRstCnt = 0;

			    if (fLinkUp)
				    continue;
			    
                /* Auto Negotiate, work in RMII mode */
	            if (ethernet_phy_auto_negotiate(EMAC, BOARD_EMAC_PHY_ADDR) != EMAC_OK) 
                {
                    DTRACE(0,( "P: Auto Negotiate ERROR!\n\r" )) ;
    		        continue;
    	        }
                                
			    DTRACE(0, ("Link Up\n"));
			    pxNetIf->flags |= NETIF_FLAG_LINK_UP;
			    emac_dev_set_rx_callback(&gs_emac_dev, EmacRxCb);  /*Enable RX */
			    fLinkUp = true;

			    if (g_fEnDHCP)
				    //dhcp_start(pxNetIf);  //todo:这个线程不能调，频繁掉线是不是有问题
                    g_fDhcpStart = true;
			    else   //固定IP应该 netif_set_up, PPPOE也应该 netif_set_up。因为PPPOE会自动协商IP
				    netif_set_up(pxNetIf);
			}
		    else if (EMAC_INVALID == rc) //网线掉了
		    {
			    if (fLinkUp)
			    {
				    fLinkUp = false;
				    DTRACE(0, ("Link Down\n"));
				    pxNetIf->flags &= ~NETIF_FLAG_LINK_UP;
				    /*Enable RX */
				    emac_dev_set_rx_callback(&gs_emac_dev, NULL);

				    if (g_fEnDHCP)
					{
						//dhcp_release(pxNetIf);
                        //g_fDhcpRelease = true;
						//dhcp_stop(pxNetIf);
                        g_fDhcpStop = true;
					}
				    else
				    {
					    netif_set_down(pxNetIf);
				    }
			    }
                /*else if ((bRstCnt<5 && GetClick()-dwRstClick >= 60) ||
                         (bRstCnt>=5 && GetClick()-dwRstClick >= 60*60))
                {
                    dwRstClick = GetClick();
                    ReInitMac(pxNetIf);  //会导致重新UP/DOWN
                    bRstCnt++;
                }*/
		    }
            else //EMAC_TIMEOUT 超时   
            {
                DTRACE(DRIVER_DEBUG, ("EMAC_TIMEOUT!\n"));
            }
		}        
	}//for
}
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function prvLowLevelInit() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to pxNetIf_add().
 *
 * @param pxNetIf the lwip network interface structure for this etherpxNetIf
 * @return ERR_OK if the loopif is initialized
 *		 ERR_MEM if private data couldn't be allocated
 *		 any other err_t on error
 */
err_t ethernetif_init( struct netif *pxNetIf )
{
    err_t xReturn = ERR_OK;

	/* This is taken from lwIP example code and therefore does not conform
	to the FreeRTOS coding standard. */
	
    struct xEthernetIf *pxEthernetIf;

	LWIP_ASSERT( "pxNetIf != NULL", ( pxNetIf != NULL ) );
	
	pxEthernetIf = mem_malloc( sizeof( struct xEthernetIf ) );
	if( pxEthernetIf == NULL ) 
	{
		LWIP_DEBUGF(NETIF_DEBUG, ( "ethernetif_init: out of memory\n" ) );
		xReturn = ERR_MEM;
	}
	else
	{
		#if LWIP_NETIF_HOSTNAME
		{
			/* Initialize interface hostname */
			pxNetIf->hostname = "lwip";
		}
		#endif /* LWIP_NETIF_HOSTNAME */

		pxNetIf->state = pxEthernetIf;

		/* We directly use etharp_output() here to save a function call.
		* You can instead declare your own function an call etharp_output()
		* from it if you have to do some checks before sending (e.g. if link
		* is available...) */
		pxNetIf->output = etharp_output;  //todo:走PPPOE这里不应该挂到 etharp    pppifNetifInit
		
		pxNetIf->linkoutput = prvLowLevelOutput;

		pxEthernetIf->ethaddr = ( struct eth_addr * ) &( pxNetIf->hwaddr[ 0 ] );

		/* initialize the hardware */
		prvLowLevelInit( pxNetIf );
	}

	return xReturn;
}
/*
void ethernet_set_mac(struct netif *pxNetIf, uint8_t *pMacAddr)
{
	memcpy(pxNetIf->hwaddr, pMacAddr, 6);
	emac_set_address(gs_emac_dev.p_hw, 0, pxNetIf->hwaddr);
}*/

/**
 * Ask if an interface is linkup
 */ 
u8_t netif_is_linkup(struct netif *netif)
{
  return (netif->flags & NETIF_FLAG_LINK_UP)?1:0;
}

void ReInitMac(struct netif *pxNetIf)
{    
    uint32_t  dwDly;
    BYTE bMac[6];
    BYTE bAddr1[2];
    BYTE bAddr2[2];
    
    if (IsPowerOff()) //当前是停电状态
        return;
    
    ADStop();
    
    memset(bMac, 0, sizeof(bMac));
    ReadItemEx(10, 0, 0xa150, bMac); 
    ReadItemEx(10, 0, 0xa040, bAddr1);
    ReadItemEx(10, 0, 0xa041, bAddr2);
    
    xSemaphoreTake( xEmacSendSem, (portTickType)(portMAX_DELAY/portTICK_RATE_MS));  //禁止发送
    OsEnterCritical(); //只能禁止调度，不能关总中断 ,以下内容将有可能等不到信号量
    /* set MAC hardware address length */
	pxNetIf->hwaddr_len = ETHARP_HWADDR_LEN;    
    
    if (IsAllAByte(bMac, 0, 6))  //全0时自动生成。
    { //bit0-bit23由厂家申请
        //pxNetIf->hwaddr[ 0 ] = ETHERNET_CONF_ETHADDR0;
        //pxNetIf->hwaddr[ 1 ] = ETHERNET_CONF_ETHADDR1;
        //pxNetIf->hwaddr[ 2 ] = ETHERNET_CONF_ETHADDR2;
        pxNetIf->hwaddr[ 0 ] = 0x36;
        pxNetIf->hwaddr[ 1 ] = 0x35;	    

        //bit24-bit47厂家自定义, bit48组播地址        
        memcpy(&pxNetIf->hwaddr[2], bAddr1, 2);  //取地址	    
        memcpy(&pxNetIf->hwaddr[4], bAddr2, 2);          
    }
    else
        memcpy(pxNetIf->hwaddr, bMac, 6);

	pxNetIf->name[0] = 'e';
	pxNetIf->name[1] = 't';

	/* device capabilities */
	pxNetIf->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;  //todo:这里还是有问题，因为别的线程会改这个标志

	/* maximum transfer unit */
	pxNetIf->mtu = netifMAX_MTU;
      
/*    pio_configure_pin(PIN_EEMAC_EREFCK, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETX0, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETX1, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETXEN, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ECRSDV, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERX0, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERX1, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERXER, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_EMDC, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_EMDIO, PIN_EMAC_FLAGS);*/
    
    EnterCritical();  //emac_handler中断里会使用 gs_emac_dev
    //MODE 0-1
    pio_configure_pin(PIN_EMAC_ERX0, PIO_TYPE_PIO_OUTPUT_1|PIO_DEFAULT);
	pio_configure_pin(PIN_EMAC_ERX1, PIO_TYPE_PIO_OUTPUT_1|PIO_DEFAULT);
    pio_configure_pin(PIN_EMAC_ECRSDV, PIO_TYPE_PIO_OUTPUT_1|PIO_DEFAULT);
    
    //PHYAD0
    pio_configure_pin(PIN_EMAC_ERXER, PIO_TYPE_PIO_OUTPUT_0|PIO_DEFAULT);
    
    ExitCritical();
    
    /* Reset PHY */
    //rstc_set_external_reset(RSTC, 13); /* (2^(13+1))/32768 */
    //rstc_reset_extern(RSTC);
    //while ((rstc_get_status(RSTC) & RSTC_SR_NRSTL) == 0) {};
    PhyReset();  //这里有SLEEP，但不会引起任务切换
    /* Wait for PHY to be ready (CAT811: Max400ms) */
    dwDly = sysclk_get_cpu_hz() / 1000 / 3 * 400;
    while(dwDly--);
    
    EnterCritical();  //emac_handler中断里会使用 gs_emac_dev
    
    pio_configure_pin(PIN_EEMAC_EREFCK, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETX0, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETX1, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ETXEN, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ECRSDV, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERX0, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERX1, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_ERXER, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_EMDC, PIN_EMAC_FLAGS);
	pio_configure_pin(PIN_EMAC_EMDIO, PIN_EMAC_FLAGS);

    /* Enable EMAC clock */
	pmc_enable_periph_clk(ID_EMAC);    

	/* Fill in EMAC options */
	g_emac_option.uc_copy_all_frame = 0;
	g_emac_option.uc_no_boardcast = 0;

	memcpy(g_emac_option.uc_mac_addr, pxNetIf->hwaddr, pxNetIf->hwaddr_len);

	gs_emac_dev.p_hw = EMAC;

	/* Init EMAC driver structure */
	emac_dev_init(EMAC, &gs_emac_dev, &g_emac_option);

	/* Init MAC PHY driver */
	if (ethernet_phy_init(EMAC, BOARD_EMAC_PHY_ADDR, sysclk_get_cpu_hz())
					!= EMAC_OK) 
    {             
        if (ethernet_phy_init(EMAC, BOARD_EMAC_PHY_ADDR, sysclk_get_cpu_hz())  //重复一次，以免出错
					!= EMAC_OK) 
        {
            ExitCritical();
            pxNetIf->flags &= NETIF_FLAG_LINK_UP;
            OsExitCritical();   
            xSemaphoreGive( xEmacSendSem );
            DTRACE(0, ( "P: PHY Initialize ERROR!\n\r" )) ;
            
            g_bJumpCyc = 1;

            ADStart();
    		return;
        }
	}
    ExitCritical();
    OsExitCritical();  
    xSemaphoreGive( xEmacSendSem );
    DTRACE(0, ( "ReInitMac&PHY OK!\n\r" )) ;
    
    g_bJumpCyc = 1;

    ADStart();
}
