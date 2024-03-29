/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief ethernet management for AVR32 UC3.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 *****************************************************************************/

/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 *
 */


#include <string.h>

#include "gpio.h" // Have to include gpio.h before FreeRTOS.h as long as FreeRTOS
                  // redefines the inline keyword to empty.

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo program include files. */
#include "partest.h"
#include "serial.h"
#include "conf_lwip_threads.h"

/* ethernet includes */
#include "ethernet.h"
#include "conf_eth.h"
#include "macb.h"


/* lwIP includes */
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "netif/loopif.h"

/*ZWave TCP server includes */
#include "ZWaveTCP.h"


//_____ M A C R O S ________________________________________________________


//_____ D E F I N I T I O N S ______________________________________________

/* global variable containing MAC Config (hw addr, IP, GW, ...) */
struct netif MACB_if;

//_____ D E C L A R A T I O N S ____________________________________________

/* Initialisation required by lwIP. */
static void prvlwIPInit( void );

/* Initialisation of ethernet interfaces by reading config file */
static void prvEthernetConfigureInterface(void * param);


void vStartEthernetTaskLauncher( unsigned portBASE_TYPE uxPriority )
{
  /* Spawn the Sentinel task. */
  xTaskCreate( vStartEthernetTask, ( const signed portCHAR * )"ETHLAUNCH",
		       configMINIMAL_STACK_SIZE, NULL, uxPriority, ( xTaskHandle * )NULL );
}

/*! \brief create ethernet task, for ethernet management.
 *
 *  \param uxPriority   Input. priority for the task, it should be low
 *
 */
portTASK_FUNCTION( vStartEthernetTask, pvParameters )
{
   static const gpio_map_t MACB_GPIO_MAP =
   {
      {AVR32_MACB_MDC_0_PIN,    AVR32_MACB_MDC_0_FUNCTION   },
      {AVR32_MACB_MDIO_0_PIN,   AVR32_MACB_MDIO_0_FUNCTION  },
      {AVR32_MACB_RXD_0_PIN,    AVR32_MACB_RXD_0_FUNCTION   },
      {AVR32_MACB_TXD_0_PIN,    AVR32_MACB_TXD_0_FUNCTION   },
      {AVR32_MACB_RXD_1_PIN,    AVR32_MACB_RXD_1_FUNCTION   },
      {AVR32_MACB_TXD_1_PIN,    AVR32_MACB_TXD_1_FUNCTION   },
      {AVR32_MACB_TX_EN_0_PIN,  AVR32_MACB_TX_EN_0_FUNCTION },
      {AVR32_MACB_RX_ER_0_PIN,  AVR32_MACB_RX_ER_0_FUNCTION },
      {AVR32_MACB_RX_DV_0_PIN,  AVR32_MACB_RX_DV_0_FUNCTION },
      {AVR32_MACB_TX_CLK_0_PIN, AVR32_MACB_TX_CLK_0_FUNCTION}
   };

   // Assign GPIO to MACB
   gpio_enable_module(MACB_GPIO_MAP, sizeof(MACB_GPIO_MAP) / sizeof(MACB_GPIO_MAP[0]));

   /* Setup lwIP. */
   prvlwIPInit();

//#if (HTTP_USED == 1)
//   /* Create the WEB server task.  This uses the lwIP RTOS abstraction layer.*/
//   sys_thread_new( "WEB", vBasicWEBServer, ( void * ) NULL,
//                   lwipBASIC_WEB_SERVER_STACK_SIZE,
//                   lwipBASIC_WEB_SERVER_PRIORITY );
//#endif
//
//#if (TFTP_USED == 1)
//   /* Create the TFTP server task.  This uses the lwIP RTOS abstraction layer.*/
//   sys_thread_new( "TFTP", vBasicTFTPServer, ( void * ) NULL,
//                   lwipBASIC_TFTP_SERVER_STACK_SIZE,
//                   lwipBASIC_TFTP_SERVER_PRIORITY );
//#endif
//
//#if (SMTP_USED == 1)
//   /* Create the SMTP Client task.  This uses the lwIP RTOS abstraction layer.*/
//   sys_thread_new( "SMTP", vBasicSMTPClient, ( void * ) NULL,
//                   lwipBASIC_SMTP_CLIENT_STACK_SIZE,
//                   lwipBASIC_SMTP_CLIENT_PRIORITY );
//#endif
   sys_thread_new("ZWave", vBasicZwaveServer, ( void *) NULL, 512, 1);
  // Kill this task.
  vTaskDelete(NULL);
}


//! Callback executed when the TCP/IP init is done.
static void tcpip_init_done(void *arg)
{
  sys_sem_t *sem;
  sem = (sys_sem_t *)arg;
  
  /* Set hw and IP parameters, initialize MACB too */
  prvEthernetConfigureInterface(NULL);
  
  sys_sem_signal(*sem); // Signal the waiting thread that the TCP/IP init is done.
}


/*!
 *  \brief start lwIP layer.
 */
static void prvlwIPInit( void )
{
  sys_sem_t sem;


  sem = sys_sem_new(0); // Create a new semaphore.
  tcpip_init(tcpip_init_done, &sem);
  sys_sem_wait(sem);    // Block until the lwIP stack is initialized.
  sys_sem_free(sem);    // Free the semaphore.

}

/*!
 *  \brief set ethernet config
 */
static void prvEthernetConfigureInterface(void * param)
{
   struct ip_addr    xIpAddr, xNetMask, xGateway;
   extern err_t      ethernetif_init( struct netif *netif );
   unsigned portCHAR MacAddress[6];

   /* Default MAC addr. */
   MacAddress[0] = ETHERNET_CONF_ETHADDR0;
   MacAddress[1] = ETHERNET_CONF_ETHADDR1;
   MacAddress[2] = ETHERNET_CONF_ETHADDR2;
   MacAddress[3] = ETHERNET_CONF_ETHADDR3;
   MacAddress[4] = ETHERNET_CONF_ETHADDR4;
   MacAddress[5] = ETHERNET_CONF_ETHADDR5;

   /* pass the MAC address to MACB module */
   vMACBSetMACAddress( MacAddress );

   /* Default ip addr. */
   IP4_ADDR( &xIpAddr,ETHERNET_CONF_IPADDR0,ETHERNET_CONF_IPADDR1,ETHERNET_CONF_IPADDR2,ETHERNET_CONF_IPADDR3 );

   /* Default Subnet mask. */
   IP4_ADDR( &xNetMask,ETHERNET_CONF_NET_MASK0,ETHERNET_CONF_NET_MASK1,ETHERNET_CONF_NET_MASK2,ETHERNET_CONF_NET_MASK3 );

   /* Default Gw addr. */
   IP4_ADDR( &xGateway,ETHERNET_CONF_GATEWAY_ADDR0,ETHERNET_CONF_GATEWAY_ADDR1,ETHERNET_CONF_GATEWAY_ADDR2,ETHERNET_CONF_GATEWAY_ADDR3 );

   /* add data to netif */
   netif_add( &MACB_if, &xIpAddr, &xNetMask, &xGateway, NULL, ethernetif_init, tcpip_input );

   /* make it the default interface */
   netif_set_default( &MACB_if );

   /* bring it up */
   netif_set_up( &MACB_if );
}
