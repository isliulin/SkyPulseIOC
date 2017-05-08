#include "sn8200_core.h"
#include "sn8200_api.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define ACK_REQUIRED 1
#define ACK_NOT_REQUIRED 0

typedef struct {
    int ch;
    int rssi;
    int sectype;
    char SSIDname[33];
} scanlist_t ;

extern int buf_pos;
extern unsigned char RxBuffer[];
extern int ack_recv_status;
extern int8_t mysock;
extern int8s sockConnected;
extern int8s sockClosed;

int32u timeout = 10000;

int8u totalscan = 0;
scanlist_t sl[30];
char SSID[32];
int SSIDlen = 0;
int8u secMode = 0;
int8u APOnOff = 1;
char secKey[64];
int8u Keylen = 0;

int joinok = 0;
int ipok = 0;
int32u selfIP = 0;
int32u pktcnt = 0;

char domain[100];
char Portstr[8]={0};
char IPstr[32]={0};
char out[256];
#define HTTP_RSP_STR "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/html\r\n" \
    "Connection: close\r\n" \
    "Content-Length: 47\r\r\n\n" \
    "<html><body>Hello from SN8200 #%2d</body></html>\r\r\n\n"

bool IsWIFIGetStatusResponsed = false;
bool IsWIFIJoinResponsed = false;
bool IsWIFIApCtrlResponsed = false;

bool IsSNICIPConfigResponsed = false;
bool IsSNICGetDHCPInfoResponsed = false;
bool IsSNICTCPConnectToServerResponsed = false;
bool IsSNICSendFromSocketResponsed = false;
bool IsSNICUDPSendFromSocketResponsed = false;
bool IsCreateSocketResponsed = false;

int destIP = INADDR_NONE;
int srcIP = INADDR_NONE;
long int destPort = PORT_NONE;
long int srcPort = PORT_NONE;

int udpDestIP = INADDR_NONE;
int udpSrcIP = INADDR_NONE;
long int udpDestPort = PORT_NONE;
long int udpSrcPort = PORT_NONE;

volatile int8u sendUDPDone = 0;
extern int8u seqNo;


u32_t
inet_addr(const char *cp)
{
    struct in_addr val;

    if (inet_aton(cp, &val)) {
        return (val.s_addr);
    }
    return (INADDR_NONE);
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int
inet_aton(const char *cp, struct in_addr *addr)
{
    u32_t val;
    u8_t base;
    char c;
    u32_t parts[4];
    u32_t *pp = parts;

    c = *cp;
    for (;;) {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!isdigit(c))
            return (0);
        val = 0;
        base = 10;
        if (c == '0') {
            c = *++cp;
            if (c == 'x' || c == 'X') {
                base = 16;
                c = *++cp;
            } else
                base = 8;
        }
        for (;;) {
            if (isdigit(c)) {
                val = (val * base) + (int)(c - '0');
                c = *++cp;
            } else if (base == 16 && isxdigit(c)) {
                val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            } else
                break;
        }
        if (c == '.') {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3)
                return (0);
            *pp++ = val;
            c = *++cp;
        } else
            break;
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
        return (0);
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1) {

    case 0:
        return (0);       /* initial nondigit */

    case 1:             /* a -- 32 bits */
        break;

    case 2:             /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
            return (0);
        val |= parts[0] << 24;
        break;

    case 3:             /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4:             /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
            return (0);
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    }
    if (addr)
        addr->s_addr = htonl(val);
    return (1);
}


int getTCPinfo(void)
{
    char tempIPstr[32];
    char teststr[8];
    
    if (strlen(IPstr)==0)
        strcpy(IPstr, "192.168.10.101");

    if (strlen(Portstr)==0)
        strcpy(Portstr, "0x6990");

    DbgPrint("Enter server IP to connect: \r\n");
    DbgScan("%s", tempIPstr);
    DbgPrint("\r\n");
    if (strlen(tempIPstr))
        strcpy(IPstr, tempIPstr);
    destIP = inet_addr(IPstr);
    if (destIP == INADDR_NONE || destIP == INADDR_ANY) {
        return CMD_ERROR;
    }

    DbgPrint("Enter server port number: \r\n");
    DbgScan("%s", teststr);
    DbgPrint("\r\n");
    if (strlen(teststr))
        strcpy(Portstr, teststr);
    destPort = strtol(Portstr, NULL, 0);
    destPort = swap16(destPort);
    if (destPort > 0xFFFF) {
        DbgPrint("Invalid port, max limit 0xFFFF \r\n");
        return CMD_ERROR;
    }

    return 0;

}

int setTCPinfo(void)
{
    char teststr[8];
    if (selfIP == 0) {
        DbgPrint("IP address has not been obtained.\n\r");
        return CMD_ERROR;
    }

    srcIP = selfIP;

    DbgPrint("Enter server port number to set: \r\n");
    DbgScan("%s", teststr);
    DbgPrint("\r\n");
    srcPort = strtol(teststr, NULL, 0);
    srcPort = swap16(srcPort);

    if (srcPort > 0xFFFF) {
        DbgPrint("Invalid port, max limit 0xFFFF \r\n");
        return CMD_ERROR;
    }

    return 0;

}


int getUDPinfo(void)
{
    char tempIPstr[32];
    char teststr[8];
    DbgPrint("Enter server IP to connect: \r\n");
    DbgScan("%s", tempIPstr);
    DbgPrint("\n\r");
    udpDestIP = inet_addr(tempIPstr);
    if (udpDestIP == INADDR_NONE) {
        return CMD_ERROR;
    }

    DbgPrint("Enter server port number: \r\n");
    DbgScan("%s", teststr);
    DbgPrint("\r\n");
    udpDestPort = strtol(teststr, NULL, 0);
    udpDestPort = swap16(udpDestPort);
    if (udpDestPort > 0xFFFF) {
        DbgPrint("Invalid port, max limit 0xFFFF \r\n");
        return CMD_ERROR;
    }

    return 0;

}


int setUDPinfo(void)
{
    char teststr[8];
    if (selfIP == 0) {
        DbgPrint("IP address has not been obtained.\n\r");
        return CMD_ERROR;
    }

    udpSrcIP = selfIP;

    DbgPrint("Enter server port number to set: \r\n");
    DbgScan("%s", teststr);
    DbgPrint("\r\n");
    udpSrcPort = strtol(teststr, NULL, 0);
    udpSrcPort = swap16(udpSrcPort);

    if (udpSrcPort > 0xFFFF) {
        DbgPrint("Invalid port, max limit 0xFFFF \r\n");
        return CMD_ERROR;
    }

    return 0;

}

void SN8200_API_Init(uint32_t baudrate)
{
    SN8200_Init(baudrate);
}

bool SN8200_API_HasInput(void)
{
    return SN8200_HasInput();
}

void GetStatus(int8u seq)
{
    int8u buf[4];
    buf[0] = WIFI_GET_STATUS_REQ;
    buf[1] = seq;
    buf[2] = 0;
    serial_transmit(CMD_ID_WIFI, buf, 3, ACK_NOT_REQUIRED);

    DbgPrint("-GetStatus\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsWIFIGetStatusResponsed) {
            IsWIFIGetStatusResponsed = false;
            break;
        }
        mdelay(1);
    }
}

void WifiOn(int8u seq)
{
    int8u buf[4];
    buf[0] = WIFI_ON_REQ;
    buf[1] = seq;
    buf[2] = (char)'U';
    buf[3] = (char)'S';
    serial_transmit(CMD_ID_WIFI, buf, 4, ACK_NOT_REQUIRED);

    DbgPrint("-WifiOn\r\n");
}

void WifiOff(int8u seq)
{
    int8u buf[2];
    buf[0] = WIFI_OFF_REQ;
    buf[1] = seq;
    serial_transmit(CMD_ID_WIFI, buf, 2, ACK_NOT_REQUIRED);

    DbgPrint("-WifiOff\r\n");
}

void ApOnOff(int8u seq)
{
    int8u buf[4];

    APOnOff ^= 1;
    buf[0] = WIFI_AP_CTRL_REQ;
    buf[1] = seq;
    buf[2] = APOnOff;
    buf[3] = 0; //persistency hardcode set as 0 means NOT save to NVM
    serial_transmit(CMD_ID_WIFI, buf, 4, ACK_NOT_REQUIRED);

    DbgPrint("-AP status\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsWIFIApCtrlResponsed) {
            IsWIFIApCtrlResponsed = false;
            break;
        }
        mdelay(1);
    }
}

void WifiScan(int8u seq)
{
    int8u buf[12];
    buf[0] = WIFI_SCAN_REQ;
    buf[1] = seq;
    memset(&buf[2], 0, 10);
    buf[3] = 2; // bss type = any

    serial_transmit(CMD_ID_WIFI, buf, 12, ACK_NOT_REQUIRED);
    DbgPrint("-WifiScan\r\n");
}

void WifiJoin(int8u seq)
{
    int8u buf[128];
    char tempstr[2] = {0};
    int8u *p = buf;

    *p++ = WIFI_JOIN_REQ;
    *p++ = seq;

    DbgPrint("Enter SSID:\n\r");
    DbgScan("%s", SSID);
    DbgPrint("\r\n");
    while(!strlen(SSID)) {
        DbgPrint("SSID can't be empty. Enter SSID:\r\n");
        DbgScan("%s", SSID);
        DbgPrint("\r\n");
    }
    memcpy(p, SSID, strlen(SSID));

    p += strlen(SSID);
    *p++ = 0x00;

    DbgPrint("Enter Security Mode (e.g., 0 for open, 2 for WPA TKIP, 4 for WPA2 AES, 6 for WPA2 MIXED):\r\n");
    DbgScan("%s", tempstr);
    DbgPrint("\r\n");
    secMode = atoi(tempstr);

    if (secMode) {
        DbgPrint("Enter Security Key:\r\n");
        DbgScan("%s", secKey);
        DbgPrint("\r\n");
        Keylen = (unsigned char)strlen(secKey);

        if (Keylen <= 0) {
            DbgPrint("Invalid Key\r\n");
            return;
        }
    }

    *p++ = secMode;
    *p++ = Keylen;

    if (Keylen) {
        memcpy(p, secKey, Keylen);
        p += Keylen;
    }

    serial_transmit(CMD_ID_WIFI, buf, (int)(p - buf), ACK_NOT_REQUIRED);

    DbgPrint("-WifiJoin\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsWIFIJoinResponsed) {
            IsWIFIJoinResponsed = false;
            break;
        }
        mdelay(1);
    }
    joinok = 0;
}

void WifiDisconn(int8u seq)
{
    int8u buf[2];
    buf[0] = WIFI_DISCONNECT_REQ;
    buf[1] = seq;
    serial_transmit(CMD_ID_WIFI, buf, 2, ACK_NOT_REQUIRED);
    DbgPrint("-WifiDisconn\r\n");
}

void SnicInit(int8u seq)
{
    int8u buf[4];
    int tmp;
    tmp = 0x00;			//The Default receive buffer size
    buf[0] = SNIC_INIT_REQ;
    buf[1] = seq;
    memcpy(buf+2, (uint8_t*)&tmp, 2);
    serial_transmit(CMD_ID_SNIC, buf, 4, ACK_NOT_REQUIRED);
    DbgPrint("-SnicInit\r\n");
}

void SnicCleanup(int8u seq)
{
    int8u buf[2];
    buf[0] = SNIC_CLEANUP_REQ;
    buf[1] = seq;
    serial_transmit(CMD_ID_SNIC, buf, 2, ACK_NOT_REQUIRED);
    DbgPrint("-SnicCleanup\r\n");
}

void SnicIPConfig(int8u seq)
{
    int8u buf[16];

    buf[0] = SNIC_IP_CONFIG_REQ;
    buf[1] = seq;
    buf[2] = 0; //STA
    buf[3] = 1; //DHCP

    serial_transmit(CMD_ID_SNIC, buf, 4, ACK_NOT_REQUIRED);
    DbgPrint("-SnicIPConfig\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsSNICIPConfigResponsed) {
            IsSNICIPConfigResponsed = false;
            break;
        }
        mdelay(1);
    }
}

void SnicGetDhcp(int8u seq)
{
    int8u buf[3];
    char tempstr[2] = {0};

    buf[0] = SNIC_GET_DHCP_INFO_REQ;
    buf[1] = seq;
    DbgPrint("\r\nInterface Type? (0: STA  1: AP) \r\n");
    DbgScan("%s", tempstr);
    DbgPrint("\r\n");
    buf[2] = atoi(tempstr);
    //buf[2] = 0; // STA  1; // AP

    serial_transmit(CMD_ID_SNIC, buf, 3, ACK_NOT_REQUIRED);
    DbgPrint("-SnicGetDhcp\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsSNICGetDHCPInfoResponsed) {
            IsSNICGetDHCPInfoResponsed = false;
            break;
        }
        mdelay(1);
    }
}


int tcpCreateSocket(int8u bindOption, int32u localIp, int16u port, int8u seq, int8u ssl)
{
    int8u buf[9];

    buf[0] = ssl;
    buf[1] = seq;
    buf[2] = bindOption;

    if (bindOption) {
        memcpy(buf+3, (uint8_t*)&localIp, 4);
        memcpy(buf+7, (uint8_t*)&port, 2);
        serial_transmit(CMD_ID_SNIC, buf, 9, ACK_NOT_REQUIRED);
    } else
        serial_transmit(CMD_ID_SNIC, buf, 3, ACK_NOT_REQUIRED);

    DbgPrint("-tcpCreateSocket\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsCreateSocketResponsed) {
            IsCreateSocketResponsed = false;
            break;
        }
        mdelay(1);
    }

    return 0;
}

int closeSocket(int8u shortSocket, int8u seq)
{
    int8u buf[3];
    buf[0] = SNIC_CLOSE_SOCKET_REQ;
    buf[1] = seq;
    buf[2] = shortSocket;
    serial_transmit(CMD_ID_SNIC, buf, 3, ACK_NOT_REQUIRED);

    DbgPrint("-closeSocket\r\n");
    DbgPrint("Socket %d closed\r\n", shortSocket);

    return 0;

}

int tcpConnectToServer(int8u shortSock, int32u ip, int16u port, int16u bufsize, int8u timeout, int8u seq)
{
    int8u buf[12];
    if (bufsize == 0 || bufsize > MAX_BUFFER_SIZE) {
        bufsize = MAX_BUFFER_SIZE;
    }

    buf[0] = SNIC_TCP_CONNECT_TO_SERVER_REQ;
    buf[1] = seq;
    buf[2] = shortSock;

    memcpy(buf+3, (int8u*)&ip, 4);
    memcpy(buf+7, (int8u*)&port, 2);
    bufsize = swap16(bufsize);
    memcpy(buf+9, (int8u*)&bufsize, 2);

    buf[11] = timeout;

    serial_transmit(CMD_ID_SNIC, buf, 12, ACK_NOT_REQUIRED);

    DbgPrint("-tcpConnectToServer\r\n");
    mdelay(1000);        //Wait module return value
    while (1) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsSNICTCPConnectToServerResponsed) {
            IsSNICTCPConnectToServerResponsed = false;
            break;
        }
        mdelay(1);
    }
    return 0;
}

int tcpCreateConnection(int8u shortSock, int16u size, int8u maxClient, int8u seq)
{
    int8u buf[6];
    if (size == 0 || size > MAX_BUFFER_SIZE) {
        size = MAX_BUFFER_SIZE;
    }

    if (maxClient == 0 || maxClient > MAX_CONNECTION_PER_SOCK)
        maxClient = MAX_CONNECTION_PER_SOCK;

    buf[0] = SNIC_TCP_CREATE_CONNECTION_REQ;
    buf[1] = seq;
    buf[2] = shortSock;
    size = swap16(size);
    memcpy(buf+3, (int8u*)&size, 2);

    buf[5] = maxClient;

    serial_transmit(CMD_ID_SNIC, buf, 6, ACK_NOT_REQUIRED);

    DbgPrint("-tcpCreateConnection\r\n");

    return 0;
}

int sendFromSock(int8u shortSocket, int8u * sendBuf, int16u len, int8u timeout, int8u seq)
{
    int8u buf[MAX_BUFFER_SIZE+6];
    int16u mybufsize;
    if (len == 0 || len > MAX_BUFFER_SIZE) {
        len = MAX_BUFFER_SIZE;
    }
    buf[0] = SNIC_SEND_FROM_SOCKET_REQ;
    buf[1] = seq;
    buf[2] = shortSocket;
    buf[3] = 0;

    mybufsize = swap16(len);
    memcpy(buf+4, (int8u*)&mybufsize, 2);
    memcpy(buf+6, sendBuf, len);

    serial_transmit(CMD_ID_SNIC, buf, 6+len, ACK_NOT_REQUIRED);

    DbgPrint("-sendFromSock\r\n");

    while (1) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsSNICSendFromSocketResponsed) {
            IsSNICSendFromSocketResponsed = false;
            break;
        }
        mdelay(1);
    }

    return 0;
}

int udpCreateSocket(int8u bindOption, int32u ip, int16u port, int8u seq)
{
    int8u buf[9];

    buf[0] = SNIC_UDP_CREATE_SOCKET_REQ;
    buf[1] = seq;
    buf[2] = bindOption;

    if (bindOption) {
        int32u myip = swap32(ip);
        int16u myport = swap16(port);
        memcpy(buf+3, (int8u*)&myip, 4);
        memcpy(buf+7, (int8u*)&myport, 2);
        serial_transmit(CMD_ID_SNIC, buf, 9, ACK_NOT_REQUIRED);
    } else
        serial_transmit(CMD_ID_SNIC, buf, 3, ACK_NOT_REQUIRED);

    DbgPrint("-udpCreateSocket\r\n");

    while (1) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(IsCreateSocketResponsed) {
            IsCreateSocketResponsed = false;
            break;
        }
        mdelay(1);
    }

    return 0;
}

int udpSendFromSock(int32u ip, int16u iPort, int8u shortsock, int8u conMode, int8u *sendbuf, int16u len, int8u seq)
{
    int8u buf[2048+12];
    int16u mybufsize;
    if (len == 0 || len > MAX_BUFFER_SIZE) {
        len = MAX_BUFFER_SIZE;
    }
    buf[0] = SNIC_UDP_SEND_FROM_SOCKET_REQ;
    buf[1] = seq;
    memcpy(buf+2, (int8u*)&ip, 4);
    memcpy(buf+6, (int8u*)&iPort, 2);
    buf[8] = shortsock;
    buf[9] = conMode;
    mybufsize = swap16(len);
    memcpy(buf+10, (int8u*)&mybufsize, 2);
    memcpy(buf+12, sendbuf, len);
    serial_transmit(CMD_ID_SNIC, buf, 12+len, ACK_NOT_REQUIRED);

    DbgPrint("-udpSendFromSock\r\n");

    timeout = 10000;
    while (timeout--) {
        if(SN8200_API_HasInput()) {
            ProcessSN8200Input();
        }
        if(sendUDPDone) {
            sendUDPDone = 0;
            break;
        }
        mdelay(1);
    }

    return 0;
}

int udpStartRecv(int32u sock, int16u bufsize, int8u seq)
{
    int8u buf[5];
    int tmp;
    if (bufsize ==0 || bufsize > MAX_BUFFER_SIZE) {
        bufsize = MAX_BUFFER_SIZE;
    }
    tmp = swap16(bufsize);
    buf[0] = SNIC_UDP_START_RECV_REQ;
    buf[1] = seq;
    buf[2] = sock;
    memcpy(buf+3, (int8u*)&tmp, 2);
    serial_transmit(CMD_ID_SNIC, buf, 5, ACK_NOT_REQUIRED);

    DbgPrint("-udpStartRecv\r\n");

    return 0;
}

void ProcessSN8200Input(void)
{
    rx_thread_proc();
}


void handleRxWiFi(int8u* buf, int len)
{
    int8u subCmdId = buf[0];

    switch (subCmdId) {
    case WIFI_GET_STATUS_RSP: {
        IsWIFIGetStatusResponsed = true;
        if (buf[2] == MODE_WIFI_OFF) {
            DbgPrint("WiFi Off.\r\n");
        } else {
            char val[20] = {0};
            int i=0;
            for(i=0; i<6; i++) {
                sprintf(val+3*i, "%02X:", buf[3+i]);
            }
            val[strlen(val)-1] = 0;
            DbgPrint("WiFi On.  Mac: %s.  ", val);

            if (buf[2] == MODE_NO_NETWORK) {
                DbgPrint("Not joined any network.\r\n");
            } else {
                DbgPrint("Joined SSID: %s\r\n", buf+9);
            }
        }
    }
    break;

    case WIFI_JOIN_RSP: {
        IsWIFIJoinResponsed = true;
        if (WIFI_SUCCESS == buf[2])
            DbgPrint("Join success\r\n");
        else
            DbgPrint("Join fail\r\n");
    }
    break;

    case WIFI_AP_CTRL_RSP: {
        IsWIFIApCtrlResponsed = true;
        if (WIFI_SUCCESS == buf[2]) {
            if (APOnOff)
                DbgPrint("AP is ON\r\n");
            else
                DbgPrint("AP is OFF\r\n");
        } else
            DbgPrint("AP control fail\r\n");
    }
    break;

    case WIFI_NETWORK_STATUS_IND: {
        if (WIFI_NETWORK_UP == buf[3]) {
            DbgPrint("Network UP\r\n");
            joinok = 1;
        } else {
            DbgPrint("Network Down\r\n");
        }
    }
    break;

    case WIFI_SCAN_RESULT_IND: {
        int cnt = buf[2];
        int i=3;
        int j;
        int8u ch, sec_tmp;
        int8s rssi;
        int8u len=32;

        if(cnt == 0) {
            for (j = 0; j < totalscan; j++) {
                DbgPrint("SSID: %20s CH: %2d RSSI: %3d Sec: %d\r\n",sl[j].SSIDname, sl[j].ch,sl[j].rssi,sl[j].sectype);
            }
            memset(sl, 0, totalscan * sizeof(scanlist_t));
            totalscan = 0; //reset current scan result
        } else {
            while (cnt--) {
                ch = buf[i++];
                rssi = (int8s)buf[i++];
                sec_tmp = buf[i++];
                i += 6;
                i++;
                i += 2;
                len = (int8u)strlen((char*)buf+i);
                if (len>32) {
                    break;
                }

                strcpy((char*)sl[totalscan].SSIDname,(char*)buf+i);

                sl[totalscan].ch = ch;
                sl[totalscan].rssi = rssi;
                sl[totalscan].sectype = sec_tmp;

                if (len == 0) {
                    while (buf[i] == 0) i++;
                } else
                    i += len+1;

                totalscan++;
            }
        }
    }
    break;

    default:
        break;
    }
    DbgPrint(".\r\n");
}


void handleRxSNIC(uint8_t* buf, int len)
{
    uint8_t subCmdId = buf[0];
    static int times = 0;
    static int isPrintable = 0;

    switch (subCmdId) {

    case SNIC_CLOSE_SOCKET_RSP: {
        if (SNIC_SUCCESS != buf[2]) 
            DbgPrint("Close socket failed\r\n");
        else {
            DbgPrint("Socket closed\r\n");
        }
    }
            break;

    case SNIC_IP_CONFIG_RSP: {
        IsSNICIPConfigResponsed = true;
        ipok = 0;
        if (SNIC_SUCCESS == buf[2]) {
            DbgPrint("IPConfig OK\r\n");
            ipok = 1;
        } else
            DbgPrint("IPConfig fail\r\n");
    }
    break;

    case SNIC_GET_DHCP_INFO_RSP: {
        IsSNICGetDHCPInfoResponsed = true;
        if (SNIC_SUCCESS == buf[2]) {
            DbgPrint("IP assigned as %i.%i.%i.%i \r\n", buf[9],buf[10],buf[11],buf[12]);
            //save IP
            memcpy(&selfIP, buf+9, 4);
        } else
            DbgPrint("IP not assigned\r\n");
    }
    break;

    case SNIC_TCP_CREATE_SOCKET_RSP:
    case SNIC_TCP_CREATE_ADV_TLS_SOCKET_RSP:
    case SNIC_TCP_CREATE_SIMPLE_TLS_SOCKET_RSP:
    case SNIC_UDP_CREATE_SOCKET_RSP: {
        IsCreateSocketResponsed = true;
        if (SNIC_SUCCESS == buf[2]) {
            mysock = buf[3];
            DbgPrint("Socket %d opened\r\n", mysock);
        } else
            DbgPrint("Socket creation failed\r\n");
    }
    break;

    case SNIC_TCP_CONNECT_TO_SERVER_RSP: {
        IsSNICTCPConnectToServerResponsed = true;
        if (SNIC_CONNECT_TO_SERVER_PENDING == buf[2] || SNIC_SUCCESS == buf[2])
					
					;
        else
            DbgPrint("Unable to connect server\r\n");
    }
    break;
    
    case SNIC_TCP_CREATE_CONNECTION_RSP:
            {
                if (SNIC_SUCCESS != buf[2])
                    DbgPrint("Unable to create TCP server\r\n");
            }
    break;

    case SNIC_TCP_CONNECTION_STATUS_IND: {
        if (SNIC_CONNECTION_UP == buf[2]) {
            DbgPrint("Socket connection UP\r\n");
            sockConnected = buf[3];
        }
        else if (SNIC_CONNECTION_CLOSED == buf[2]) {
            DbgPrint("Socket %i closed\r\n", buf[3]);
            sockClosed = buf[3];
        }
    }
    break;

    case SNIC_SEND_RSP: {
        int32u sentsize;
        IsSNICSendFromSocketResponsed = true;
        if (SNIC_SUCCESS == buf[2]) {
            pktcnt ++;
            sentsize = ((int32u)(buf[3] << 8) | (int32u)buf[4]);
            DbgPrint("pkt %d, %d bytes sent \r\n", pktcnt, sentsize);
        }
    }
    break;

    case SNIC_CONNECTION_RECV_IND: {
        int i=0;
        int32u sentsize = ((int32u)(buf[3] << 8) | (int32u)buf[4]);
        int32u sock = (int32u)buf[2];
        DbgPrint("%d bytes received from socket %d \r\n", sentsize, sock);
        mdelay(10);
        if (strncmp((char*)buf+5, "GET /", 5) == 0 || strncmp((char*)buf+5, "POST /", 6) == 0) { // Receives a HTTP(S) get/post request.
            static int i=0;
            for (i=0; i<sentsize; i++) {
                DbgPrint("%c", buf[5+i]);
            }
            i = 0;
            sprintf(out, HTTP_RSP_STR, i++);
            // Send back something
            sendFromSock(sock, (int8u*)out, strlen(out), 2, seqNo++);
        
            // If it is a TLS server, it only accepts one connection, so
            // it is better for the host app to close it after some idle time for new connections.
            //	Sleep(100);
            //	closeSocket((int8u)sock,seqNo++);
        }
        else if (strncmp((char*)buf+5, "HTTP/1.", 7) == 0) { // Receives HTTP response, close socket. If the socket is not closed,
             for (i=0; i<sentsize; i++) {
                DbgPrint("%c", buf[5+i]);
            }                                              // it can be used for more data communication (using send from socket).
            closeSocket((int8u)sock,seqNo++); }
    }
    break;

    case SNIC_TCP_CLIENT_SOCKET_IND: {
//        int8u listen_sock = buf[2];
        DbgPrint("Accepted connection from %i.%i.%i.%i\r\n", buf[4], buf[5], buf[6], buf[7]);
        DbgPrint("Connection socket: %d\r\n", buf[3]);

    }
    break;
    case SNIC_UDP_RECV_IND: {
        DbgPrint("%d %d\r\n", times++, htons(*((int16u*)&buf[9])));
    }
    break;
    case SNIC_UDP_SEND_FROM_SOCKET_RSP: {
        IsSNICUDPSendFromSocketResponsed = true;
        if (SNIC_SUCCESS != buf[2]) {
            DbgPrint("UDP Send bad\r\n");
        }
        sendUDPDone = 1;
        break;
    }

    case SNIC_HTTP_REQ|0x80:
    case SNIC_HTTPS_REQ|0x80:
    case SNIC_HTTP_MORE_REQ|0x80: {	
        char *contentT = "";
        unsigned short len = *((short*)&buf[4]);
        short contTLen = 0;
        unsigned short moreData = len & 0x8000;	
        char more[10] = {0};
        int8u seq = buf[1];
        short status = *((short*)&buf[2]);
                
        status = swap16(status);
        len = swap16(len);
        
        if (subCmdId == (SNIC_HTTP_MORE_REQ|0x80))
            strcpy(more, "more ");
        len &= 0x7fff;
        
        if (status >= 100) {
            contentT = (char*)&buf[6];;
            contTLen = strlen(contentT)+1;
        }
        
        if (status < 100) {
            DbgPrint("\nHTTP %sRSP code: %d, seq#: %d\r\n", more, status, seq);
            break;
        }

        DbgPrint("\nHTTP %sRSP code: %d, seq#: %d, Content Length: %d, Type: %s, More data: %s\r\n", more, status, seq, len, contentT, moreData?"yes":"no");
        isPrintable = 0;
        if (contTLen && (strstr(contentT, "text") || strstr(contentT, "xml") ||
            strstr(contentT, "javascript") || strstr(contentT, "html") ||
            strstr(contentT, "json"))) {
            isPrintable = 1;
            buf[6+contTLen+len] = 0;
            DbgPrint("Content: \n%s\r\n", buf+6+contTLen);
        }
    }
    break;
    case SNIC_HTTP_RSP_IND: {
                int8u seq = buf[1];
                unsigned short moreData;
                unsigned short len = *((short*)&buf[2]);
                len = swap16(len);
                moreData = len & 0x8000;
                len &= 0x7fff; 
                DbgPrint("\nHTTP RSP indication, seq#: %d, content length: %d, More data: %s\r\n", seq, len, moreData?"yes":"no");
                if (isPrintable) { 
                    buf[4+len] = 0;
                    DbgPrint("Content: \n%s\r\n", buf+4);
                }
		}
		break;

    default:
			break;

    }
    DbgPrint(".\r\n");
}

void SendSNIC(unsigned char *buf, int size)
{
    const unsigned int IP = udpDestIP;
    int16u port = udpDestPort;
    sendUDPDone = 0;
    udpSendFromSock(IP, port, mysock, 0, buf, size, seqNo++);
}


int fillNSendHttpReq(int8u seq, char* domain, char* uri, char method, char* contentType, char* otherHeader, int contentLen, char* content, unsigned char timeout, char moreData, char isHttps)
{
    char *ptr = NULL;
    int8u buf[1024];
    int16u encodedLen = moreData?contentLen|0x8000:contentLen;
    memset(buf, 0, sizeof(buf));
    buf[0] = SNIC_HTTP_REQ;
    buf[1] = seq;
    *((int16u*)&buf[2]) = 0x5000; //swapped
    buf[4] = method;
    buf[5] = timeout;
    
    if (isHttps) {
        buf[0] = SNIC_HTTPS_REQ;
        *((int16u*)&buf[2]) = 0xbb01; // 443 swapped 
    }

    ptr = (char*)buf+6;
    ptr += sprintf(ptr, "%s", domain)+1;
    ptr += sprintf(ptr, "%s", uri)+1;
    ptr += sprintf(ptr, "%s", contentType)+1;
    ptr += sprintf(ptr, "%s", otherHeader)+1;
    *((int16u*)ptr) = swap16(encodedLen);
    ptr += 2;
    if (contentLen) 
        memcpy(ptr, content, contentLen);

    serial_transmit(CMD_ID_SNIC, buf, ptr-(char*)buf+contentLen, ACK_NOT_REQUIRED);
    return 0;
}
/*add HttpMoreReq cmd*/
int fillNSendHttpMoreReq(int8u seq, int contentLen, char* content, char moreData)
{
    int8u buf[1024];
    int16u len = moreData?contentLen|0x8000:contentLen;
    buf[0] = SNIC_HTTP_MORE_REQ;
    buf[1] = seq;
    *((int16u*)&buf[2]) = swap16(len);

    if (contentLen+4 > sizeof(buf))
        return -1;

    memcpy(&buf[4], content, contentLen);
    serial_transmit(CMD_ID_SNIC, buf, contentLen+4, ACK_NOT_REQUIRED);
    return 0;
}

