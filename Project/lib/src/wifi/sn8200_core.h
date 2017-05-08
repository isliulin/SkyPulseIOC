#ifndef __SN8200_CORE_H
#define __SN8200_CORE_H
#if defined  (STM32F2XX)
#include		"stm32f2xx.h"
#elif defined (STM32F10X_HD)
#include		"stm32f10x.h"
#elif	undefined (STM32F2XX || STM32F10X_HD)
*** undefined target !!!!
#endif

#include <stdbool.h>
#include <rtl.h>

// Maximum payload len
#define MAX_PAYLOAD_LEN 8000

// Special character defines
#define SOM_CHAR 0x02
#define EOM_CHAR 0x04
#define ESC_CHAR 0x10
#define ACK_CMD	0x7F
#define NAK_CMD 0x00

typedef enum {
    WIFI_ON_REQ = 0,
    WIFI_OFF_REQ,
    WIFI_JOIN_REQ,
    WIFI_DISCONNECT_REQ,
    WIFI_GET_STATUS_REQ,
    WIFI_SCAN_REQ,
    WIFI_GET_STA_RSSI_REQ,
    WIFI_AP_CTRL_REQ,

    WIFI_NETWORK_STATUS_IND = 0x10,
    WIFI_SCAN_RESULT_IND,
    WIFI_RSSI_IND,

    WIFI_ON_RSP = 0x80,
    WIFI_OFF_RSP,
    WIFI_JOIN_RSP,
    WIFI_DISCONNECT_RSP,
    WIFI_GET_STATUS_RSP,
    WIFI_SCAN_RSP,
    WIFI_GET_STA_RSSI_RSP,
    WIFI_AP_CTRL_RSP,
} WIFI_subcmd_id_e;

enum {
    WIFI_SUCCESS,
    WIFI_FAIL,
    WIFI_NETWORK_UP = 0x10,
    WIFI_NETWORK_DOWN,
};

typedef enum {
    CMD_ID_NACK = 0,  // reservered
    CMD_ID_WIFI = 0x50,
    CMD_ID_SNIC = 0x70,
    CMD_ID_ACK = 0x7F,
} cmd_id_e;


typedef struct {
    bool available;
    unsigned short payload_len;
    unsigned char ack_reqd;
    unsigned char cmd_id;
    unsigned char rx_payload[MAX_PAYLOAD_LEN];
    unsigned char chksum;
    unsigned char ackOk;
} rx_info_t;


#define NUM_RX_BUF 4

typedef enum {
    IDLE,
    SOM_RECD,
    LEN_RECD,
    ACK_SEQ_RECD,
    CMD_RECD,
    PAYLAD_RX,
    PAYLAD_RX_ESC,
    CHKSUM_RECD,
    EOM_RECD,
    WAIT_FOR_ACK_NAK,
} serial_rx_state_t;

typedef enum {
    MODE_WIFI_OFF,
    MODE_NO_NETWORK,
    MODE_STA_JOINED,
    MODE_AP_STARTED,
    MODE_SNIC_INIT_NOT_DONE,
    MODE_SNIC_INIT_DONE,
    /* Non-mode special values */
    MODE_LIST_END,
    MODE_ANY,
} serial_wifi_mode_t;

typedef enum {
    SNIC_INIT_REQ = 0,
    SNIC_CLEANUP_REQ,
    SNIC_SEND_FROM_SOCKET_REQ,
    SNIC_CLOSE_SOCKET_REQ,
    SNIC_SOCKET_PARTIAL_CLOSE_REQ,
    SNIC_GETSOCKOPT_REQ,
    SNIC_SETSOCKOPT_REQ,
    SNIC_SOCKET_GETNAME_REQ,
    SNIC_SEND_ARP_REQ,
    SNIC_GET_DHCP_INFO_REQ,
    SNIC_RESOLVE_NAME_REQ,
    SNIC_IP_CONFIG_REQ,

    SNIC_TCP_CREATE_SOCKET_REQ = 0x10,
    SNIC_TCP_CREATE_CONNECTION_REQ,
    SNIC_TCP_CONNECT_TO_SERVER_REQ,

    SNIC_UDP_CREATE_SOCKET_REQ,
    SNIC_UDP_START_RECV_REQ,
    SNIC_UDP_SIMPLE_SEND_REQ,
    SNIC_UDP_SEND_FROM_SOCKET_REQ,

    SNIC_HTTP_REQ,
    SNIC_HTTP_MORE_REQ,
    SNIC_HTTPS_REQ,

    SNIC_TCP_CREATE_ADV_TLS_SOCKET_REQ,
    SNIC_TCP_CREATE_SIMPLE_TLS_SOCKET_REQ,

    SNIC_TCP_CONNECTION_STATUS_IND = 0x20,
    SNIC_TCP_CLIENT_SOCKET_IND,
    SNIC_CONNECTION_RECV_IND,
    SNIC_UDP_RECV_IND,
    SNIC_ARP_REPLY_IND,
    SNIC_HTTP_RSP_IND,

    SNIC_SEND_RSP = 0x82,
    SNIC_CLOSE_SOCKET_RSP,
    SNIC_GET_DHCP_INFO_RSP = 0x89,
    SNIC_IP_CONFIG_RSP = 0x8B,
    SNIC_TCP_CREATE_SOCKET_RSP = 0x90,
    SNIC_TCP_CREATE_CONNECTION_RSP,
    SNIC_TCP_CONNECT_TO_SERVER_RSP,

    SNIC_UDP_CREATE_SOCKET_RSP = 0x93,
    SNIC_UDP_SEND_FROM_SOCKET_RSP = 0x96,
		
    SNIC_TCP_CREATE_ADV_TLS_SOCKET_RSP = 0x9A,
    SNIC_TCP_CREATE_SIMPLE_TLS_SOCKET_RSP,
} SNIC_subcmd_id_e;

typedef enum {
    SNIC_SUCCESS=0,
    SNIC_FAIL,
    SNIC_INIT_FAIL,
    SNIC_CLEANUP_FAIL,
    SNIC_GETADDRINFO_FAIL,
    SNIC_CREATE_SOCKET_FAIL,
    SNIC_BIND_SOCKET_FAIL,
    SNIC_LISTEN_SOCKET_FAIL,
    SNIC_ACCEPT_SOCKET_FAIL,
    SNIC_PARTIAL_CLOSE_FAIL,
    SNIC_CONNECTION_PARTIALLY_CLOSED = 0x0A,
    SNIC_CONNECTION_CLOSED,
    SNIC_CLOSE_SOCKET_FAIL,
    SNIC_PACKET_TOO_LARGE,
    SNIC_SEND_FAIL,
    SNIC_CONNECT_TO_SERVER_FAIL,
    SNIC_NOT_ENOUGH_MEMORY = 0x10,
    SNIC_TIMEOUT,
    SNIC_CONNECTION_UP,
    SNIC_GETSOCKOPT_FAIL,
    SNIC_SETSOCKOPT_FAIL,
    SNIC_INVALID_ARGUMENT,
    SNIC_SEND_ARP_FAIL,
    SNIC_INVALID_SOCKET,
    SNIC_CONNECT_TO_SERVER_PENDING,
    SNIC_SOCKET_NOT_BOUND,
    SNIC_SOCKET_NOT_CONNECTED,
} SNIC_return_code_e;

#define swap16(in) htons(in)
#define swap32(in) htonl(in)

unsigned char dummy_tx(unsigned char c);

void SN8200_Init(uint32_t baudrate);
bool SN8200_HasInput(void);
int serial_transmit(unsigned char cmd_id, unsigned char *payload, int payload_len, unsigned char ack_required);
void rx_thread_proc(void);
int sci_ser_cmd_proc(int commandId, int paramLength, unsigned char *params);
#endif
