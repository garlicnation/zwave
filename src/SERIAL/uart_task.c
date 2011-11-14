/* This source file is part of the ATMEL AVR-UC3-SoftwareFramework-1.7.0 Release */

/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief USART example application for AVR32 USART driver.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices with a USART module can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/*! \page License
 * Copyright (c) 2009 Atmel Corporation. All rights reserved.
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

/*! \mainpage
 * \section intro Introduction
 * This is the documentation for the data structures, functions, variables,
 * defines, enums, and typedefs for the USART software driver.\n It also comes
 * bundled with an example. This example is a basic Hello-World example.\n
 * <b>Example's operating mode: </b>
 * -# A message is displayed on the PC terminal ("Hello, this is AT32UC3 saying hello! (press enter)")
 * -# You may then type any character other than CR(Carriage Return) and it will
 * be echoed back to the PC terminal.
 * -# If you type a CR, "'CRLF'Goodbye." is echoed back to the PC terminal and
 * the application ends.
 *
 * \section files Main Files
 * - usart.c: USART driver;
 * - usart.h: USART driver header file;
 * - usart_example.c: USART example application.
 *
 * \section compilinfo Compilation Information
 * This software is written for GNU GCC for AVR32 and for IAR Embedded Workbench
 * for Atmel AVR32. Other compilers may or may not work.
 *
 * \section deviceinfo Device Information
 * All AVR32 devices with a USART module can be used.
 *
 * \section configinfo Configuration Information
 * This example has been tested with the following configuration:
 * - EVK1100, EVK1101, UC3C_EK, EVK1104, EVK1105 evaluation kits; STK600+RCUC3L routing card; AT32UC3L-EK
 * - CPU clock:
 *        -- 12 MHz : EVK1100, EVK1101, EVK1104, EVK1105, AT32UC3L-EK evaluation kits; STK600+RCUC3L routing card
 *        -- 16 Mhz : AT32UC3C-EK
 * - USART1 (on EVK1100 or EVK1101) connected to a PC serial port via a standard
 *   RS232 DB9 cable, or USART0 (on EVK1105) or USART2 (on AT32UC3C-EK) or USART1 (on EVK1104)
 *   or USART3 (on AT32UC3L-EK) abstracted with a USB CDC connection to a PC;
 *   STK600 usart port for the STK600+RCUC3L setup (connect STK600.PE2 to STK600.RS232 SPARE.TXD
 *   and STK600.PE3 to STK600.RS232 SPARE.RXD)
 * - PC terminal settings:
 *   - 57600 bps,
 *   - 8 data bits,
 *   - no parity bit,
 *   - 1 stop bit,
 *   - no flow control.
 *
 * \section contactinfo Contact Information
 * For further information, visit
 * <A href="http://www.atmel.com/products/AVR32/">Atmel AVR32</A>.\n
 * Support and FAQ: http://support.atmel.no/
 */
// DEBUG HEADERS


#include <avr32/io.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "compiler.h"
#include "board.h"
#include "power_clocks_lib.h"
#include "gpio.h"
#include "usart.h"
#include "portmacro.h"
#include "partest.h"
#include "task.h"
#include "queue.h"
#include "lwip/api.h"


#include <string.h>
#include "ipc.h"
/*! \name USART Settings
 */
//! @{

#  define EXAMPLE_TARGET_PBACLK_FREQ_HZ configPBA_CLOCK_HZ  // PBA clock target frequency, in Hz

#if BOARD == EVK1100
#  define EXAMPLE_USART               (&AVR32_USART1)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART1_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART1_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART1_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART1_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK    AVR32_USART1_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBA
#elif BOARD == EVK1101
#  define EXAMPLE_USART               (&AVR32_USART1)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART1_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART1_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART1_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART1_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK    AVR32_USART1_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBA
#elif BOARD == UC3C_EK
#  define EXAMPLE_USART               (&AVR32_USART2)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART2_RXD_0_1_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART2_RXD_0_1_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART2_TXD_0_1_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART2_TXD_0_1_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK    AVR32_USART2_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBB
#elif BOARD == EVK1104
#  define EXAMPLE_USART               (&AVR32_USART1)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART1_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART1_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART1_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART1_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK    AVR32_USART1_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBA
#elif BOARD == EVK1105
#  define EXAMPLE_USART               (&AVR32_USART0)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART0_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART0_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN        AVR32_USART0_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART0_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK    AVR32_USART0_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBA
#elif BOARD == STK600_RCUC3L0
#  define EXAMPLE_USART               (&AVR32_USART1)
#  define EXAMPLE_USART_RX_PIN        AVR32_USART1_RXD_0_1_PIN
#  define EXAMPLE_USART_RX_FUNCTION   AVR32_USART1_RXD_0_1_FUNCTION
// For the RX pin, connect STK600.PORTE.PE3 to STK600.RS232 SPARE.RXD
#  define EXAMPLE_USART_TX_PIN        AVR32_USART1_TXD_0_1_PIN
#  define EXAMPLE_USART_TX_FUNCTION   AVR32_USART1_TXD_0_1_FUNCTION
// For the TX pin, connect STK600.PORTE.PE2 to STK600.RS232 SPARE.TXD
#  define EXAMPLE_USART_CLOCK_MASK    AVR32_USART1_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBA
#elif BOARD == UC3L_EK
#  define EXAMPLE_USART                 (&AVR32_USART3)
#  define EXAMPLE_USART_RX_PIN          AVR32_USART3_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION     AVR32_USART3_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN          AVR32_USART3_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION     AVR32_USART3_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK      AVR32_USART3_CLK_PBA
#  define EXAMPLE_TARGET_DFLL_FREQ_HZ   96000000  // DFLL target frequency, in Hz
#  define EXAMPLE_TARGET_MCUCLK_FREQ_HZ 12000000  // MCU clock target frequency, in Hz
#  undef  EXAMPLE_TARGET_PBACLK_FREQ_HZ
#  define EXAMPLE_TARGET_PBACLK_FREQ_HZ 12000000  // PBA clock target frequency, in Hz
#  define EXAMPLE_PDCA_CLOCK_HSB      AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB       AVR32_PDCA_CLK_PBA
#endif

#if !defined(EXAMPLE_USART)             || \
		!defined(EXAMPLE_USART_RX_PIN)      || \
		!defined(EXAMPLE_USART_RX_FUNCTION) || \
		!defined(EXAMPLE_USART_TX_PIN)      || \
		!defined(EXAMPLE_USART_TX_FUNCTION)
#  error The USART configuration to use in this example is missing.
#endif

//! @}


#if BOARD == UC3L_EK
/*! \name Parameters to pcl_configure_clocks().
 */
//! @{
static scif_gclk_opt_t gc_dfllif_ref_opt = { SCIF_GCCTRL_SLOWCLOCK, 0, OFF };
static pcl_freq_param_t pcl_dfll_freq_param =
{
		.main_clk_src = PCL_MC_DFLL0,
		.cpu_f        = EXAMPLE_TARGET_MCUCLK_FREQ_HZ,
		.pba_f        = EXAMPLE_TARGET_PBACLK_FREQ_HZ,
		.pbb_f        = EXAMPLE_TARGET_PBACLK_FREQ_HZ,
		.dfll_f       = EXAMPLE_TARGET_DFLL_FREQ_HZ,
		.pextra_params = &gc_dfllif_ref_opt
};
//! @}
#endif

#include "portmacro.h"
static const gpio_map_t USART_GPIO_MAP =
{
		{EXAMPLE_USART_RX_PIN, EXAMPLE_USART_RX_FUNCTION},
		{EXAMPLE_USART_TX_PIN, EXAMPLE_USART_TX_FUNCTION}
};

// USART options.
static const usart_options_t USART_OPTIONS =
{
		.baudrate     = 57600,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE
};


portTASK_FUNCTION(vBasicSerialServer, pvParameters)
{
	int recieved;
	int i;
	struct netbuf * tcp_netbuf;
	portCHAR * netbufdata;
	unsigned portSHORT len;
	char debug[100];
	// Configure Osc0 in crystal mode (i.e. use of an external crystal source, with
	// frequency FOSC0) with an appropriate startup time then switch the main clock
	// source to Osc0.

	// Assign GPIO to USART.
	gpio_enable_module(USART_GPIO_MAP,
			sizeof(USART_GPIO_MAP) / sizeof(USART_GPIO_MAP[0]));

	// Initialize USART in RS232 mode.
	usart_init_rs232(EXAMPLE_USART, &USART_OPTIONS, EXAMPLE_TARGET_PBACLK_FREQ_HZ);

	usart_recv_queue = (xQueueHandle)xQueueCreate(1000, 1);

	// Hello world!
	for(;;)
	{


		vParTestToggleLED(2);
		vTaskDelay(100/portTICK_RATE_MS);
		vParTestToggleLED(2);
		vTaskDelay(100/portTICK_RATE_MS);
		usart_write_line(EXAMPLE_USART, "Waiting to recv chars\n");


		// Press enter to continue.
		vParTestToggleLED(0);
		while (!usart_test_hit(EXAMPLE_USART)){
			if(zw_tcp_recv_queue && uxQueueMessagesWaiting(zw_tcp_recv_queue)){
				sprintf(debug, "we have %d messages in the queue\n", (int)uxQueueMessagesWaiting(zw_tcp_recv_queue));
				usart_write_line(EXAMPLE_USART, debug);
				vParTestToggleLED(1);
				xQueueReceive(zw_tcp_recv_queue, &tcp_netbuf, 100);
				usart_write_line(EXAMPLE_USART, "recvd data, now calling netbuf_data\n");
				if(tcp_netbuf!=NULL){
					netbuf_data(tcp_netbuf, &netbufdata, &len);
				usart_write_line(EXAMPLE_USART, "netbuf_data called, now writing chars to usart\n");
				}else{
					usart_write_line(EXAMPLE_USART, "netbuf_data not called, something null in the queue\n");
				}
				for(i = 0; i < len; i++){
					usart_write_char(EXAMPLE_USART, netbufdata[i]);
				}
				usart_write_line(EXAMPLE_USART, "Finished with the chars\n");
			}
			vTaskDelay(100/portTICK_RATE_MS);// Block while there is nothing to .
		}
		vParTestToggleLED(0);

		while(usart_test_hit(EXAMPLE_USART)){
			recieved = usart_getchar(EXAMPLE_USART);
			usart_recv_queue_spoiled = xQueueSend(usart_recv_queue, &recieved, 100);
		}

		usart_write_line(EXAMPLE_USART, "Finished receiving chars.\n");
	}

	//*** Sleep mode
	// This program won't be doing anything else from now on, so it might as well
	// sleep.
	// Modules communicating with external circuits should normally be disabled
	// before entering a sleep mode that will stop the module operation.
	// Make sure the USART dumps the last message completely before turning it off.
	while(!usart_tx_empty(EXAMPLE_USART));
	vTaskDelete(NULL);
}

