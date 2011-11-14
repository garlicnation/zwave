/*
 * ipc.h
 *
 *  Created on: Nov 13, 2011
 *      Author: User
 */

#ifndef IPC_H_
#define IPC_H_


xQueueHandle usart_recv_queue;
unsigned short int usart_recv_queue_spoiled;

xQueueHandle zw_tcp_recv_queue;
unsigned short int zw_tcp_recv_queue_spoiled;

unsigned short int connection_active;

#endif /* IPC_H_ */
