/******************** (C) COPYRIGHT 2009 Embest Info&Tech Co.,LTD. ************
* File Name          : main.c
* Author             : Wuhan R&D Center, Embest
* Date First Issued  : 28/03/2013
* Description        : Main program body
*******************************************************************************
*******************************************************************************
* History:
* 28/03/2013		 : V1		   initial version
* 13/06/2013		 : V2
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "delay.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DBGU_RX_BUFFER_SIZE 256
#define TEST_BUFFERSIZE 128
#define UDP_NUM_PKT 10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t 	seqNo = 0;
int8_t		mysock = -1;
int8u			TxBuf[TEST_BUFFERSIZE];
int8_t		sockConnected = -1;
int8_t		sockClosed = -1;
int				timeout1 = 5;

extern int ipok, joinok;
extern int destIP, srcIP;
extern long int destPort, srcPort;
extern int32u pktcnt;

extern char domain[100];
extern char Portstr[8];
extern bool IsCreateSocketResponsed ;
extern int32u timeout;
extern bool IsWIFIJoinResponsed ;
char uri[100]={0};


#define GET_REQUEST \
    "GET / HTTP/1.1\r\n" \
    "Host: 192.168.2.125\r\n" \
    "Accept: text/html\r\n" \
    "\r\n"

/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small DbgPrint (option LD Linker->Libraries->Small DbgPrint
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

void DBGU_Init(void);
bool DBGU_RxBufferEmpty(void);
uint8_t DBGU_GetChar(void);
void ShowMenu(void);
bool ProcessUserInput(void);
int sendHttpReqTest(char *domain, char isHttps);
int sendHttpPostDemo(char *domain);
int sendHttpJsonPostDemo(char *domain);
int sendHttpChunkReqTest(char *domain);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
_buffer *Dbg;

void	WiFi(void) {

static 
void	(*f)(void)=NULL;	
static 
int		fo=true;

			if(f==NULL) {
				DbgPrint(NULL);
				Dbg=_buffer_init(128);
				SN8200_API_Init(921600);
				strcpy(domain, "www.murata-ws.com");
				strcpy(uri, "/index.html");	
				if(!__com0 || !Dbg)
					NVIC_SystemReset();
				else {
					f=App_Loop;
					App_Loop=WiFi;
					SnicCleanup(seqNo++);
					WifiOff(seqNo++);
					Wait(200,App_Loop);
					WifiOn(seqNo++);
					Wait(200,App_Loop);
	        SnicInit(seqNo++);
					Wait(200,App_Loop);
					GetStatus(seqNo++);
					Wait(200,App_Loop);
					_buffer_push(Dbg,"0\r",2);	
					SnicGetDhcp(seqNo++);
					_buffer_push(Dbg,"6000\r",5);
					while (setTCPinfo() == CMD_ERROR)
						Wait(200,App_Loop); 
					mysock = -1;
					tcpCreateSocket(1, srcIP, (unsigned short)srcPort, seqNo++, SNIC_TCP_CREATE_SOCKET_REQ);
					if (mysock != -1) {
						tcpCreateConnection(mysock, TEST_BUFFERSIZE, 0x5, seqNo++);
					}
				}
			} else {
				__sendFromSock();
				if(fo==true) {
					fo=false;
					fo=ProcessUserInput();
				}
				if(SN8200_API_HasInput())
					ProcessSN8200Input();	
				f();
			}
}

void ShowMenu(void)
{
    DbgPrint("---------------------\r\n");
    DbgPrint("0 Get WiFi status\r\n");
    DbgPrint("1 Wifi Scan\r\n");
    DbgPrint("2 Wifi Join\r\n");
    DbgPrint("3 Get IP\r\n");
    DbgPrint("4 TCP client\r\n");
    DbgPrint("5 TCP server\r\n");
    DbgPrint("6 Send from sock\r\n");
    DbgPrint("7 WiFi Leave\r\n");
    DbgPrint("8 AP On/Off\r\n");
    DbgPrint("9 UDP client\r\n");
    DbgPrint("a UDP server\r\n");
    DbgPrint("b Wifi Off\r\n");
    DbgPrint("c Wifi On\r\n");
    DbgPrint("d HTTP get req\r\n");
    DbgPrint("e HTTP post req\r\n");
    DbgPrint("f HTTP post Json req\r\n");
    DbgPrint("g HTTP chunked post req\r\n");
    DbgPrint("h HTTPS get req\r\n");
    DbgPrint("i TLS client\r\n");
    DbgPrint("j TLS server (HTTPS server)\r\n");
    DbgPrint("m: Show Menu\r\n");
    DbgPrint("q: press q to Quit \r\n");
    DbgPrint("---------------------\r\n");
}

bool ProcessUserInput(void)
{
    char	tmp[100];

		if(DbgScan("%s", tmp)==1)
		switch(tmp[0]) {
		case 'q':
        return false;
    
		case '0':
        GetStatus(seqNo++);
        break;

    case '1':
        WifiScan(seqNo++);
        break;

    case '2':
        WifiDisconn(seqNo++);
        WifiJoin(seqNo++);
        SnicInit(seqNo++);
        SnicIPConfig(seqNo++);
        break;

    case '3':
        SnicInit(seqNo++);
        SnicGetDhcp(seqNo++);
        break;

    case '4':
        mysock = -1;
        tcpCreateSocket(0, 0xFF, 0xFF, seqNo++, SNIC_TCP_CREATE_SOCKET_REQ);
        if (mysock != -1) {
            if (getTCPinfo() == CMD_ERROR) {
                DbgPrint("Invalid Server\r\n");
                break;
            }
            // This connection can receive data upto 0x0400=1K bytes at a time.
            tcpConnectToServer(mysock, destIP, (unsigned short)destPort, 0x0400, 0x5, seqNo++);
        }
        break;

    case '5':
        if (setTCPinfo() == CMD_ERROR) {
            DbgPrint("Invalid Server to create\r\n");
            break;
        }
        mysock = -1;
        tcpCreateSocket(1, srcIP, (unsigned short)srcPort, seqNo++, SNIC_TCP_CREATE_SOCKET_REQ);
        if (mysock != -1) {
            // This connection can receive data upto TEST_BUFFERSIZE at a time.
            tcpCreateConnection(mysock, TEST_BUFFERSIZE, 0x5, seqNo++);
        }
        break;

    case '6': {
        char tempstr[2] = {0};
        int8u datamode;
        char sockstr[8];
        int32u sock;
        char teststr[128];
        int len;

        DbgPrint("Enter socket number to send from: \r\n");
        DbgScan("%s", sockstr);
        sock = strtol(sockstr, NULL, 0);

        DbgPrint("Content Option? (0: Default  1: User specific) \r\n");
        DbgScan("%s", tempstr);
        datamode = atoi(tempstr);

        if (datamode) {
            DbgPrint("Enter payload to send (up to 128 bytes): \r\n");
            DbgScan("%s", teststr);
            len = (int)strlen(teststr);
            sendFromSock(sock, (int8u*)teststr, len, 2, seqNo++);
        } else {
            sendFromSock(sock, TxBuf, TEST_BUFFERSIZE, 2, seqNo++);
            pktcnt = 0;
        }
        break;
    }

    case '7':
        SnicCleanup(seqNo++);
        WifiDisconn(seqNo++);
        break;

    case '8':
        ApOnOff(seqNo++);
        break;

    case '9': {//udp send
        int i;
        udpCreateSocket(0, 0, 0, seqNo++);
        if (mysock != -1) {
            if (getUDPinfo() == CMD_ERROR) {
                DbgPrint("Invalid Server\r\n");
                break;
            }
            DbgPrint("Send %d\r\n", UDP_NUM_PKT);
            for (i=0; i<UDP_NUM_PKT; i++) {
                int si = i % TEST_BUFFERSIZE + 1;
                SendSNIC(TxBuf, si);
                DbgPrint("%d %d\r\n", i, si);
            }

            closeSocket(mysock,seqNo++);
        }
        break;
    }

    case 'a': {//udp recv
        int16u port = 43211;
        int32u ip = 0xAC1F0001; // 172.31.0.1
        udpCreateSocket(1, ip, port, seqNo++);
        udpStartRecv(mysock, 2048, seqNo++);
        break;
    }

    case 'b':
        SnicCleanup(seqNo++);
        WifiOff(seqNo++);
        break;
    case 'c':
        WifiOn(seqNo++);
        break;

    case 'd':
        DbgPrint("Enter server name:  %s\r\n", domain);
        DbgScan("%s", tmp);
        DbgPrint("\r\n");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpReqTest(domain, 0);
        break;

    case'e':
        DbgPrint("Enter server name: ([CR] to accept %s)\r\n", domain);
        DbgScan("%s", tmp);
        DbgPrint("\r\n");
        if (strlen(tmp)) 
        strcpy(domain, tmp);
        sendHttpPostDemo(domain);
        break;

    case 'f':
        DbgPrint("Make sure STA is connected to SN8200 soft AP.\r\n");
        strcpy(domain, "sn8200.com");
        DbgPrint("Enter server name: ([CR] to accept %s)\r\n", domain);
        DbgScan("%s", tmp);
        DbgPrint("\r\n");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpJsonPostDemo(domain);
        break;
#if 1

    case 'g':
        strcpy(domain, "192.168.10.100");
        DbgPrint("Enter server name (or the peer testclient IP, peer testclient should start TCP server on port 80): ([CR] to accept %s)\r\n", domain);
        DbgScan("%s", tmp);
        DbgPrint("\r\n");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpChunkReqTest(domain);
        break;
#endif

    case 'h':
        DbgPrint("Enter server name: ([CR] to accept %s)\r\n", domain);
        DbgScan("%s", tmp);
        DbgPrint("\r\n");
        if (strlen(tmp)) 
            strcpy(domain, tmp);
        sendHttpReqTest(domain, 1);
        break;

    case 'i':
        timeout1 = 5;
        mysock = -1;
        tcpCreateSocket(0, 0xFF, 0xFF, seqNo++, SNIC_TCP_CREATE_SIMPLE_TLS_SOCKET_REQ);  // use less memory in SN8200
        mdelay(500);
        if (mysock != -1) {
            strcpy(Portstr, "443");
 
					if (getTCPinfo() == CMD_ERROR) {
                DbgPrint("Invalid Server\r\n");
                break;
            }
					
            tcpConnectToServer(mysock, destIP, (unsigned short)destPort,0x0000,timeout1,seqNo++);
            while ((sockConnected == -1) && timeout1) {
                mdelay(500);
                timeout1--;
                if (sockClosed == mysock) {
                    sockClosed = -1;
                    break;
                }
            }

            if (sockConnected == mysock) {
                sendFromSock(mysock, (int8u*)GET_REQUEST, sizeof(GET_REQUEST)-1, 2, seqNo++);
                sockConnected = -1;
            }
            else DbgPrint("Connect failed.\r\n");
        }
        break;

    case 'j': //ssl server
        strcpy(Portstr, "443");
        if (setTCPinfo() == CMD_ERROR) {
            DbgPrint("Invalid Server to create\r\n");
            break;
        }
        mysock = -1;
        tcpCreateSocket(1, srcIP, (unsigned short)srcPort, seqNo++, SNIC_TCP_CREATE_ADV_TLS_SOCKET_REQ);
        if (mysock != -1) {
            // This connection can receive data upto TEST_BUFFERSIZE at a time. 
            tcpCreateConnection(mysock, TEST_BUFFERSIZE, 0x5, seqNo++);
        }
        break;

    case 'm':
        ShowMenu();
        break;

    default:
        break;
    }
		return true;

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
        ex: DbgPrint("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif

/**
  * @}
  */
int sendHttpReqTest(char *domain, char isHttps)
{
    char tmp[100];
    char method = 0; //GET
    char contentType[] = "text/html";
    char otherHeader[] = "";
    char content[] = "";
    unsigned char timeout = 10;
    DbgPrint("Enter URI after the server name: ([CR] to accept %s)\r\n", uri);
    DbgScan("%s",tmp);
    DbgPrint("\r\n");

    if (strlen(tmp))
        strcpy(uri, tmp);
    return fillNSendHttpReq(seqNo++, domain, uri, method, contentType, otherHeader, strlen(content), content, timeout, 0, isHttps);
}

/**
  * @
  */

int sendHttpPostDemo(char *domain)
{
    char content[256]={0};
    char tmp[100];
    char method = 1; //POST
    char contentType[] = "text/html";
    char otherHeader[] = "Accept-Language: en-US\r\n";
    unsigned char timeout = 10;

    DbgPrint("Enter URI after the server name: ([CR] to accept %s)\r\n", uri);
    DbgScan("%s",tmp);
    DbgPrint("\r\n");
    if (strlen(tmp))
        strcpy(uri, tmp);
    DbgPrint("Enter content to POST: \r\n");
    DbgScan("%s",content);
    DbgPrint("\r\n");
    if (strlen(uri)==0)
        strcpy(uri, "/add.php");
    return fillNSendHttpReq(seqNo++, domain, uri, method, contentType, otherHeader, strlen(content), content, timeout, 0,0);
}
/**
  * @
  */
int sendHttpJsonPostDemo(char *domain)
{
    char content[]="Params=%7B%22if%22%3A%22sta%22%7D&callback=jsonp1363359950547";
    char tmp[100];

    char method = 1; //POST
    char contentType[] = "application/x-www-form-urlencoded";
    char otherHeader[] = "Accept-Language: en-US\r\n";
    unsigned char timeout = 10;

    strcpy(uri, "/sws/wifi/stat");
    DbgPrint("Enter URI after the server name: ([CR] to accept %s)\r\n", uri);
    DbgScan("%s",tmp);
    DbgPrint("\r\n");
    if (strlen(tmp))
        strcpy(uri, tmp);
    DbgPrint("Enter content to POST: ([CR] to accept %s)\r\n", content);
    DbgScan("%s",tmp);
    DbgPrint("\r\n");
    if (strlen(tmp))
        strcpy(content, tmp);
    return fillNSendHttpReq(seqNo++, domain, uri, method, contentType, otherHeader, strlen(content), content, timeout, 0,0);
}
/**
  * @
  */
int sendHttpChunkReqTest(char *domain)
{
    char uri[] = "/rest/thermostatGetTime";
    char method = 1; //POST
    char contentType[] = "application/x-www-form-urlencoded";
    char otherHeader[] = "Accept: text/html,application/xml\r\nAccept-Language: en-US\r\n";
    char content[] = "mcu_serial_number_hex=00112233445566778899AA";
    char content1[] = "&username=MyUsername&password=MyPassword"; 
    unsigned char timeout = 20;
    int16u len = strlen(content); // more data
    
    // first chunk
    fillNSendHttpReq(seqNo++, domain, uri, method, contentType, otherHeader, len, content, timeout, 1, 0);
    mdelay(1000);
    //second chunk
    fillNSendHttpMoreReq(seqNo++, strlen(content1), content1, 0);
    return 0;
}

/************* (C) COPYRIGHT 2013 Wuhan R&D Center, Embest *****END OF FILE****/
