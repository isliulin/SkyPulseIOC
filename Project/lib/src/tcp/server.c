/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable echo example.
 *
 **/

 /* This file was modified by ST */


#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "string.h"

typedef 			void *func(void*);
int						_buffer_push(void *, void *,int),
							_buffer_pull(void *, void *,int),
							_buffer_count(void *);
							
void					_io_close(void *),
							_tcp_flush(void *),
							_thread_add(void *,void *,char *,int),
							_thread_remove(void *,void *);
							
extern	volatile int __time__;

#if LWIP_TCP
/* ECHO protocol states */
enum server_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

/* structure for maintaing connection infos to be passed as argument 
   to LwIP callbacks*/
typedef struct 
{
  u8_t state;             /* current connection state */
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *tx;        /* pointer on the received/to be transmitted pbuf */
	struct pbuf *rx;				//~~~ pointer na rx pakete
	void	**io;							//~~~ pointer na _io iz aplikacije, mora bitna zacetku strukture
	func	*f;
} TCP;


static err_t	TCP_accept(	void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t	TCP_recv(		void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void		TCP_error(	void *arg, err_t err);
static err_t	TCP_poll(		void *arg, struct tcp_pcb *tpcb);
static err_t	TCP_sent(		void *arg, struct tcp_pcb *tpcb, u16_t len);
static void		TCP_send(		struct tcp_pcb *tpcb, TCP *es);
static void		TCP_close(	struct tcp_pcb *tpcb, TCP *es);

/**
  * @brief  Initializes the tcp echo server
  * @param  None
  * @retval None
  */
struct tcp_pcb 
	*TCP_init(void *f, struct ip_addr *DestIPaddr, int port) {
		struct tcp_pcb	*pcb = tcp_new();							/* create new tcp pcb */
		if(pcb != NULL) {
			err_t err;		
			if(DestIPaddr != NULL) {
				err = tcp_connect(pcb,DestIPaddr,port,TCP_accept);
				if (err == ERR_OK)
					printf("Can not connect pcb\n");
			} else {
				err = tcp_bind(pcb, IP_ADDR_ANY, port);
				if (err == ERR_OK) {
					pcb = tcp_listen(pcb);
					tcp_arg(pcb, f);
					tcp_accept(pcb, TCP_accept);
				} else
				printf("Can not bind pcb\n");
			}
		} else {
			printf("Can not create new pcb\n");
		}
		return pcb;
	}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used 
  * @retval err_t: error status
  */
static err_t	
	TCP_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
		err_t ret_err;
		TCP *es;
		
		LWIP_UNUSED_ARG(arg);
		LWIP_UNUSED_ARG(err);
		tcp_setprio(newpcb, TCP_PRIO_MIN);					/* set priority for the newly accepted tcp connection newpcb */
		es = (TCP *)mem_malloc(sizeof(TCP));				/* allocate structure es to maintain tcp connection informations */
		if (es != NULL) {
			es->state = ES_ACCEPTED;
			es->pcb = newpcb;
			es->tx = NULL;
			es->rx = NULL;
			tcp_arg(newpcb, es);											/* pass newly allocated es structure as argument to newpcb */
			tcp_recv(newpcb, TCP_recv);								/* initialize lwip tcp_recv callback function for newpcb  */ 
			tcp_err(newpcb, TCP_error);								/* initialize lwip tcp_err callback function for newpcb  */
			tcp_poll(newpcb, TCP_poll, 1);						/* initialize lwip tcp_poll callback function for newpcb */

			es->io=((func*)arg)(NULL);								// prvi klic
			if(es->io) {															// tole je lahko tut drugace
				es->f=(func *)arg;
				_thread_add(es->f,es->io,"user app.",0);
				_thread_add(_tcp_flush,es,"tcp flush",0);
			}
			
			ret_err = ERR_OK;
		} else {
			ret_err = ERR_MEM;												/* return memory error */
		}
		return ret_err;  
	}

/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the reveived pbuf
  * @retval err_t: error code
  */
static err_t 
	TCP_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
		err_t ret_err;
		TCP *es;

		LWIP_ASSERT("arg != NULL",arg != NULL);
		
		es = (TCP *)arg;
		
		if (p == NULL) {															/* if we receive an empty tcp frame from client => close connection */  
			es->state = ES_CLOSING;											/* remote host closed connection */
			if (es->tx == NULL) {
				TCP_close(tpcb, es); 											/* we're done sending, close connection */
			} else {																		/* we're not done yet */
				tcp_sent(tpcb, TCP_sent); 								/* send remaining data*/
				TCP_send(tpcb, es);												/* acknowledge received packet */
			}
			ret_err = ERR_OK;
		} else if(err != ERR_OK) { 										/* else : a non empty frame was received from client but for some reason err != ERR_OK */
			if (p != NULL) {
				es->tx = NULL;
				pbuf_free(p); /* free received pbuf*/
			}
			ret_err = err;
		} else if(es->state == ES_ACCEPTED) { 				/* first data chunk in p->payload */
			es->state = ES_RECEIVED;
			es->rx = p;																	/* store reference to incoming pbuf (chain) */
			ret_err = ERR_OK;
		} else if (es->state == ES_RECEIVED) {
			if (es->rx)
				pbuf_chain(es->rx,p);
			else
				es->rx = p;
			ret_err = ERR_OK;
		} else { 																			/* data received when connection already closed */
			tcp_recved(tpcb, p->tot_len);								/* Acknowledge data reception */
			es->tx = NULL;
			pbuf_free(p);																/* free pbuf and do nothing */
			ret_err = ERR_OK;
		}
		return ret_err;
	}
/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs. 
  * @param  arg: pointer on argument parameter 
  * @param  err: not used
  * @retval None
  */
static void 
	TCP_error(void *arg, err_t err) {
		TCP *es;

		LWIP_UNUSED_ARG(err);
		
		es = (TCP *)arg;
		if (es != NULL) {   
			mem_free(es);																/*  free es structure */
		}
	}
/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
static err_t 
	TCP_poll(void *arg, struct tcp_pcb *tpcb) {
		err_t ret_err;
		TCP *es;

		es = (TCP *)arg;
		if (es != NULL) {
			if (es->tx != NULL) { 											/* there is a remaining pbuf (chain) , try to send data */
				TCP_send(tpcb, es);
			} else {																		/* no remaining pbuf (chain)  */
				if (es->state == ES_CLOSING) {
					TCP_close(tpcb, es);										/*  close tcp connection */
				}
			}
			ret_err = ERR_OK;
		} else {
			tcp_abort(tpcb);														/* nothing to be done */
			ret_err = ERR_ABRT;
		}
		return ret_err;
	}
/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * @param  None
  * @retval None
  */
static err_t 
	TCP_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
		TCP *es;

		LWIP_UNUSED_ARG(len);

		es = (TCP *)arg;
		
		if (es->tx != NULL) {													/* still got pbufs to send */
				tcp_sent(es->pcb, TCP_sent);
				TCP_send(es->pcb, es);
		} else {																			/* if no more data to send and client closed connection*/
			if (es->state == ES_CLOSING) {
				TCP_close(tpcb, es);
			}
		}
		return ERR_OK;
	}

/**
  * @brief  This function is used to send data for tcp connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */

static void 
	TCP_send(struct tcp_pcb *tpcb, TCP *es) {
		struct pbuf *ptr;
		err_t wr_err = ERR_OK;	 
		while ((wr_err == ERR_OK) &&
					 (es->tx != NULL) && 
					 (es->tx->len <= tcp_sndbuf(tpcb)))
		{
			ptr=es->tx;																	/* get pointer on pbuf from es structure & enqueue data for transmission */
			wr_err=tcp_write(tpcb,ptr->payload,ptr->len, 1);
			if (wr_err == ERR_OK) {
				es->tx = ptr->next;
				if (es->tx != NULL) {
					pbuf_ref(es->tx); 											/* increment reference count for es->p */
				}
				pbuf_free(ptr);														/* free pbuf: will free pbufs up to es->p (because es->p has a reference count > 0) */
			} else if(wr_err == ERR_MEM) { 							/* we are low on memory, try later / harder, defer to poll */
				es->tx = ptr;
		 } else { }																		/* other problem ?? */
		}
	}

/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */

static void
	TCP_close(struct tcp_pcb *tpcb, TCP *es) {
		tcp_arg(tpcb, es->io);												/* remove all callbacks */
		tcp_sent(tpcb, NULL);
		tcp_recv(tpcb, NULL);
		tcp_err(tpcb, NULL);
		tcp_poll(tpcb, NULL, 0);

		if (es != NULL) {															/* delete es structure */
			_thread_remove(es->f,es->io);
			_thread_remove(_tcp_flush,es);
			if(es->io)
				_io_close(es->io);
			mem_free(es);
		}  
		tcp_close(tpcb);															/* close tcp connection */
	}

/**
  * @brief  interface na app. layer; push/pull v io buffer od funkcije, 
	*					ki jo tcp podstavi v accept procesu 
  * @param  v je pointer na tcp strukturo
  * @param  es: pointer on echo_state structure
  * @retval None
  */
	
void 	_tcp_flush(void *v) {
		TCP *es=v;
		if(es->pcb->snd_queuelen > TCP_SND_QUEUELEN-1)				  	// !!!! pred vpisom preveri, ce ni queue ze poln  
			tcp_output(es->pcb);																		// kratkih blokov (Nagle algoritem bo javil MEM error....)
		else if(es->io) {																					// sicer nadaljevanje...
			char	c[256];
			int k,n=0;
			if(es->rx) {																						// a je kaj za sprejem ???
				struct pbuf	*q;
				for(q=es->rx; q != NULL; q=es->rx) {									// preskanirat je treba celo verigo pbuf 
					n+=k=_buffer_push(es->io[0],q->payload, q->len);		// push v io
					if(k < q->len) {
						pbuf_header(q,-k);																// skrajsaj header
						break;
					}
					es->rx = es->rx->next;
					if (es->rx != NULL)
						pbuf_ref(es->rx);
					pbuf_free(q);
				}
				tcp_recved(es->pcb,n);																// free raw input
			}																														
																										
			n=_buffer_count(es->io[1]);																// koliko je v buferju za izpis ...
			if(n > tcp_sndbuf(es->pcb))															// ne sme biti vec kot je placa na raw output ....
				n = tcp_sndbuf(es->pcb);	
			if(n > 256)																							// ne sme bit vec kot 1024.... glej c[1024]
				n=256;
			if(n) {																									// ce je sploh kej ...
				struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, n , PBUF_POOL);
				if(p != NULL) {																				// ce je alokacija pbuf uspela
					n=_buffer_pull(es->io[1],c,n);											// kopiraj pull vsebino v vmesni buffer
					pbuf_take(p,c,n);																		// formiraj pbuf
					if(es->tx)																					// verizi, ce je se kaj od prej..
						pbuf_chain(es->tx,p);															// 
					else
						es->tx = p;																				// sicer nastavi nov pointer 
					tcp_sent(es->pcb, TCP_sent);												// set callback & send..
					TCP_send(es->pcb, es);					
					tcp_output(es->pcb);							
				}
			}
		}
	}


/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
extern struct netif *netif_default;
void 
	udp_echoserver_receive_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
		char c[64];
		union {int i;char c[4];} ipaddr;
		ipaddr.i=(int)netif_default->ip_addr.addr;
		pbuf_free(p); 
 		sprintf(c,"LwIP %d.%d.%d.%d",ipaddr.c[0],ipaddr.c[1],ipaddr.c[2],ipaddr.c[3]);
		p = pbuf_alloc(PBUF_TRANSPORT, strlen(c) , PBUF_POOL);
		pbuf_take(p,c,strlen(c));
		udp_connect(upcb, addr, port);
		udp_send(upcb, p);
		udp_disconnect(upcb);
		pbuf_free(p); 
 
}

void 
	UDP_init(int port) {
		struct udp_pcb *upcb = udp_new();
		err_t err;
		if (upcb) {
			err = udp_bind(upcb, IP_ADDR_ANY, port);
			if (err == ERR_OK) {
				udp_recv(upcb, udp_echoserver_receive_callback, NULL);
			} else {
				printf("can not bind pcb");
			}
		} else {
			printf("can not create pcb");
		} 
}
	
#endif /* LWIP_TCP */
