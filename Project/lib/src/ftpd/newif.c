/**
 * @file
 * Ethernet Interface for standalone applications (without RTOS) - works only for 
 * ethernet polling mode (polling for ethernet frame reception)
 *
 */

/*
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
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/mem.h"
#include "netif/etharp.h"
#include "ethernetif.h"
#include "netconf.h"
#include <string.h>
#include <stdlib.h>

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
#ifdef CHECKSUM_BY_HARDWARE
  int i; 
#endif
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] =  MAC_ADDR0;
  netif->hwaddr[1] =  MAC_ADDR1;
  netif->hwaddr[2] =  MAC_ADDR2;
  netif->hwaddr[3] =  MAC_ADDR3;
  netif->hwaddr[4] =  MAC_ADDR4;
  netif->hwaddr[5] =  MAC_ADDR5;
  
  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
#ifndef	WIN32
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  struct	pbuf *q;
  int			framelength = 0;
  char		*buffer ;
  
  putVCP(SLIP_END);
  /* copy frame from pbufs to driver buffers */
  for(q = p; q != NULL; q = q->next) 
  {
    buffer= (char *)(q->payload);
		for(framelength = 0; framelength < q->len; ++framelength)
			switch(buffer[framelength])
			{
				case SLIP_END:
					putVCP(SLIP_ESC);
					putVCP(SLIP_ESC_END);
					break;
				case SLIP_ESC:
					putVCP(SLIP_ESC);
					putVCP(SLIP_ESC_ESC);
					break;
				default:
					putVCP(buffer[framelength]);
			}			
  }
  putVCP(SLIP_END);
  return ERR_OK;
}
#else
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
void pcap_send(char[],int);

  struct	pbuf *q;
  int		i,j;
  char		*buf,pcap[2000];
  
  for(i=0, q=p; q!=NULL; q=q->next) 
  {
    buf= (char *)(q->payload);
	for(j = 0; j < q->len; ++j)
		pcap[i++]=buf[j];
	pcap_send(pcap,j);
  }
  return ERR_OK;
}
#endif

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
#ifndef WIN32
static struct	pbuf * low_level_input(struct netif *netif)
{
static struct	pbuf	*p=NULL,*q=NULL;
static int		plen=0;

	struct		pbuf	*pp;
	int 		c;
	
	do {
		c=getVCP();
		switch(c) {
			case -1:
				break;

			case SLIP_END:
				if(q) {
					for(pp=p; pp!=q; pp=pp->next)
						plen+=pp->len;
					pbuf_realloc(p,plen);
					q=NULL;
					return(p);
				}
				p=q=pbuf_alloc(PBUF_RAW, netif->mtu, PBUF_POOL);
				plen=0;
				break;

			case SLIP_ESC:
				if(q)
					((char *)(q->payload))[plen]=c;
		  break;

			case SLIP_ESC_END:
				if(q) {
					if (((char *)(q->payload))[plen] == SLIP_ESC)
						((char *)(q->payload))[plen++] = SLIP_END;
					else
						((char *)(q->payload))[plen++] = SLIP_ESC_END;
				}
		  break;
		
			case SLIP_ESC_ESC:  
				if(q) {
					if (((char *)(q->payload))[plen] == SLIP_ESC)
						((char *)(q->payload))[plen++] = SLIP_ESC;
					else
						((char *)(q->payload))[plen++] = SLIP_ESC_ESC;
				}
		  break;
			
			default:
				if(q)
					((char *)(q->payload))[plen++] = c;
				break;
		} 
		if(q && plen>=q->len) {
			q=q->next;
			plen=0;
		}
	} while(c !=-1);
  return NULL;
}
#else
struct	pbuf * low_level_input(struct netif *netif)
{
	extern int pkt_user_len;
	extern char pkt_user[];
	int i,j;
	struct	pbuf *p,*q= pbuf_alloc(PBUF_RAW, pkt_user_len, PBUF_POOL);
	p=q;
	for(i=j=0; i<pkt_user_len;++i) {
		if(!q)
			return(NULL);
		((char *)(q->payload))[j++] = pkt_user[i];
		if(j == q->len) {
			q=q->next;
			j=0;
		}
	}
	return p;
}
#endif

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t ethernetif_input(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  /* entry point to the LwIP stack */
  err = netif->input(p, netif);
  
  if (err != ERR_OK)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
  return err;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));
  
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}



