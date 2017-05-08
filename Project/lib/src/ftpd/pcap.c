#define				WPCAP
#define				JUST_PCAP

#include			<pcap.h>
#include			<stdlib.h>
#include			"netconf.h"

int	putVCP (int);
int	getVCP (void);

pcap_t				*p_handle=NULL;
pcap_send_queue		*squeue=NULL;
char				errbuf[PCAP_ERRBUF_SIZE];
u_char				pkt_user[2000];
int					pkt_user_len;

#ifdef JUST_PCAP

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	unsigned int n;
	putVCP(SLIP_END);
	for(n = 0; n < header->caplen; ++n) {
		if(!memcmp((char *)pkt_data,"\x4\x1\x2\x1\x8\x5",6))
			printf("%02X",pkt_data[n]);
		switch(pkt_data[n])
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
				putVCP(pkt_data[n]);
		}
	}
	putVCP(SLIP_END);
		if(!memcmp((char *)pkt_data,"\x4\x1\x2\x1\x8\x5",6))
			printf("\r\n\r\n");


}

void pollVcp(void) {
void pcap_send(char [],int);

static char		pcap[2000];
static int		n=-1;

int 			c;
	do {
		c=getVCP();
		switch(c) {
			case -1:
				break;

			case SLIP_END:
				if(n>0)
				{
					pcap_send(pcap,n);
					n=-1;
				}
				else
					++n;
				break;

			case SLIP_ESC:
				if(n>=0)
					pcap[n]=c;
				break;

			case SLIP_ESC_END:
				if(n>=0) {
					if(pcap[n] == SLIP_ESC)
						pcap[n++] = SLIP_END;
					else
						pcap[n++] = SLIP_ESC_END;
				}
				break;

			case SLIP_ESC_ESC:
				if(n>=0) {
					if (pcap[n] == SLIP_ESC)
						pcap[n++] = SLIP_ESC;
					else
						pcap[n++] = SLIP_ESC_ESC;
				}
				break;

			default:
				if(n>=0)
					pcap[n++] = c;
				break;
		}
	} while(c !=-1);
}

#else
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	void LwIP_Pkt_Handle(void);
	pkt_user_len=header->caplen;
	memcpy(pkt_user,pkt_data,pkt_user_len);
	LwIP_Pkt_Handle();
}
#endif	
pcap_t *pcap_init()
{
	pcap_if_t *p;

	if(pcap_findalldevs( &p, errbuf) != -1)
		p_handle=pcap_open_live(p->name,65536,1,10,errbuf);
	return p_handle;
}


int pollPcap()
{
	if(squeue)
	{
		pcap_sendqueue_transmit(p_handle, squeue, TRUE);
		pcap_sendqueue_destroy(squeue);
	}
	squeue = pcap_sendqueue_alloc(20000);
	return pcap_dispatch(p_handle, 100, packet_handler, pkt_user);
}


void pcap_send(char pcap[],int j)
{
	struct pcap_pkthdr hdr;
	int n;
	hdr.caplen=hdr.len=j;
	hdr.ts.tv_usec=clock();
	hdr.ts.tv_sec=clock()*1000;

	for(n=0; n<j; ++n)
		printf("%02X",pcap[n]);
	printf("\r\n");
	if(!squeue)
		squeue = pcap_sendqueue_alloc(20000);	
	pcap_sendqueue_queue(squeue, &hdr, (const u_char *) pcap);
}



