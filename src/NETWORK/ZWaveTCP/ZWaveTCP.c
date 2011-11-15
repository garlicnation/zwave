/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief Basic WEB Server for AVR32 UC3.
 *
 * - Compiler:           GNU GCC for AVR32
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

/*
  Implements a simplistic WEB server.  Every time a connection is made and
  data is received a dynamic page that shows the current FreeRTOS.org kernel
  statistics is generated and returned.  The connection is then closed.

  This file was adapted from a FreeRTOS lwIP slip demo supplied by a third
  party.
 */



/* Standard includes. */
#include <stdio.h>
#include <string.h>

#include "conf_eth.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "partest.h"
#include "serial.h"

/* Demo includes. */
/* Demo app includes. */
#include "portmacro.h"

/* lwIP includes. */
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "netif/loopif.h"

/* ethernet includes */
#include "ethernet.h"

#include "usart.h"

#include "ipc.h"


/*! The port on which we listen. */
#define zwavePORT		( 23 )


struct netbuf * pxRxBuffer;
xSemaphoreHandle xRxSem;

/*! Function to process the current connection */
static void prvweb_HandleZwaveSession( struct netconn *pxNetCon );


/*! \brief WEB server main task
 *         check for incoming connection and process it
 *
 *  \param pvParameters   Input. Not Used.
 *
 */
portTASK_FUNCTION( vBasicZwaveServer, pvParameters )
{
	struct netconn *pxZwaveListener, *pxNewConnection;

	/*We create FreeRTOS tools for ipc and locking*/
	zw_tcp_recv_queue = xQueueCreate(100, 1);
	xRxSem = xSemaphoreCreateMutex();

	/* Create a new tcp connection handle */
	vParTestToggleLED(1);
	vTaskDelay(500*portTICK_RATE_MS);
	vParTestToggleLED(1);
	pxZwaveListener = netconn_new( NETCONN_TCP );
	netconn_bind(pxZwaveListener, NULL, zwavePORT );
	netconn_listen( pxZwaveListener );
	vTaskDelay(500*portTICK_RATE_MS);
	vParTestToggleLED(2);
	vTaskDelay(1000*portTICK_RATE_MS);
	vParTestToggleLED(2);
	/* Loop forever */
	for( ;; )
	{
		/* Wait for a first connection. */
		pxNewConnection = netconn_accept(pxZwaveListener);

		if(pxNewConnection != NULL)
		{
			prvweb_HandleZwaveSession(pxNewConnection);
		}/* end if new connection */
		vTaskDelay(500*portTICK_RATE_MS);

	} /* end infinite loop */
}


/*! \brief parse the incoming request
 *         parse the HTML request and send file
 *
 *  \param pxNetCon   Input. The netconn to use to send and receive data.
 *
 */
static void prvweb_HandleZwaveSession( struct netconn *pxNetCon )
{
	portCHAR *pcRxString;
	unsigned portSHORT usLength;
	char to_send[100];
	size_t to_send_idx = 0;
	unsigned short i;


	while((pxNetCon->err & ERR_CLSD) == 0){
		pxRxBuffer = netconn_recv(pxNetCon);
		if (pxRxBuffer != NULL){
			netbuf_data(pxRxBuffer, (void*)&pcRxString, &usLength);
			vParTestToggleLED(5);
			for(i = 0; i < usLength; i++){
				xQueueSend(zw_tcp_recv_queue, pcRxString+i, 100);
			}
			netbuf_delete(pxRxBuffer);
		}
		if (usart_recv_queue && uxQueueMessagesWaiting(usart_recv_queue)){
			vParTestToggleLED(4);
			while(uxQueueMessagesWaiting(usart_recv_queue)>0 && to_send_idx < 100){
				vParTestToggleLED(3);
				xQueueReceive(usart_recv_queue, (to_send+to_send_idx), 100);
				to_send_idx++;
			}
			if (to_send_idx>0){
				netconn_write(pxNetCon, to_send, to_send_idx, NETCONN_COPY);
				to_send_idx = 0;
			}
		}
		vTaskDelay(100/portTICK_RATE_MS);
	}

	netconn_close( pxNetCon );
	netconn_delete( pxNetCon );
}

portTASK_FUNCTION( vWaitDataTask , pvParameters ){
	struct netcon * pxNetCon;


	pxNetCon = pvParameters;
	while((pxNetCon->err & ERR_CLSD) == 0){

	}
}
