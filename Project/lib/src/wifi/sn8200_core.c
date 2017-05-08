#include "sn8200_hal.h"
#include "sn8200_core.h"
#include "sn8200_api.h"
#include <string.h>
#include <stdio.h>
#define SERIAL_TX(x) dummy_tx(x)

#define SEND_BUF_SIZE 4096

static unsigned char tempReTXBuf[SEND_BUF_SIZE];
int tempReTXLen;

rx_info_t rx_frame[NUM_RX_BUF];
static serial_rx_state_t serial_rx_state;
static int curr_buf_idx = 0;
static int curr_read_idx = 0;
static int rx_frm_payload_index;
static int rx_frm_data_index;
static unsigned char rx_frm_chksum;

unsigned char dummy_tx(unsigned char c)
{
    tempReTXBuf[tempReTXLen++] = c;
    if (c == EOM_CHAR || tempReTXLen >= SEND_BUF_SIZE) {
        SN8200_HAL_SendData(tempReTXBuf, tempReTXLen);

        memset(tempReTXBuf, 0, tempReTXLen);
        tempReTXLen = 0;
    }
    return c;
}

static unsigned char escape_payload_and_transmit_no_ESC(int payload_len, unsigned char *payload)
{
    int i;
    unsigned char chksum=0;
    unsigned char tx;
    for (i=0; i<payload_len; ++i) {
        tx = *payload++;
        if (tx != SOM_CHAR && tx != EOM_CHAR && tx != ESC_CHAR) {
            SERIAL_TX(tx);
            chksum += tx;
        } else {
            SERIAL_TX(ESC_CHAR);
            SERIAL_TX(0x80|tx);
            chksum += ESC_CHAR;
            chksum += (0x80|tx);
        }
    }
    return chksum;
}

static int calc_escaped_payload_len_no_ESC(int payload_len, unsigned char *payload)
{
    int i;
    int len=0;
    unsigned char c;

    for (i=0; i<payload_len; ++i) {
        c = *payload++;
        if (c == SOM_CHAR || c == EOM_CHAR || c == ESC_CHAR) {
            len += 2;
        } else {
            len ++;
        }
    }
    return len;
}

static int no_ESC_transmit(unsigned char cmd_id, int payload_len, unsigned char *payload, unsigned char ack_required, int len)
{
    unsigned char cksum=0;

    unsigned char hdr = 0x80;

    SERIAL_TX(SOM_CHAR); // Send SOM character

    SERIAL_TX(0x80|len); // Send payload len
    cksum += (0x80|len);

    hdr |= (ack_required<<6);

    if (len > 0x7f) {
        hdr |= (len>>7);
    }

    cksum += (0x80 | hdr);

    SERIAL_TX(0x80|hdr); // ACK, SEQ and frag number
    SERIAL_TX(0x80|cmd_id); // ACK, SEQ and frag number
    cksum += (0x80 | cmd_id);
    cksum += escape_payload_and_transmit_no_ESC(payload_len, payload);
    cksum |= 0x80;
    SERIAL_TX(cksum); // checksum
    SERIAL_TX(EOM_CHAR); // Send EOM character

    return 0;
}

int rx_process_char_no_ESC(unsigned char rx_ch)
{
    int ch=-1;
    switch(serial_rx_state) {
    case IDLE:
        if (rx_ch == SOM_CHAR) {
						rx_process_char_no_ESC_RX_SOM:
            serial_rx_state = SOM_RECD;
            rx_frm_payload_index = 0;
            rx_frm_chksum = 0;
            rx_frm_data_index = 0;
        } else
            ch = rx_ch;
        break;
    case SOM_RECD:
        if( (rx_ch & 0x80) == 0 ) {
            goto rx_process_char_no_ESC_err;
        }
        serial_rx_state=LEN_RECD;
        rx_frame[curr_buf_idx].payload_len = rx_ch & 0x7F;
        rx_frm_chksum += rx_ch;
        break;
    case LEN_RECD:
        if( (rx_ch & 0x80) == 0 ) {
            goto rx_process_char_no_ESC_err;
        }
        serial_rx_state=ACK_SEQ_RECD;
        rx_frame[curr_buf_idx].payload_len |= (rx_ch & 0x3f)<<7;
        rx_frame[curr_buf_idx].ack_reqd = (rx_ch >> 6) & 0x01;
        rx_frm_chksum += rx_ch;
        break;
    case ACK_SEQ_RECD:
        if( (rx_ch & 0x80) == 0 ) {
            goto rx_process_char_no_ESC_err;
        }
        serial_rx_state=CMD_RECD;
        rx_frame[curr_buf_idx].cmd_id = rx_ch & 0x7F;
        rx_frm_chksum += rx_ch;
        break;
    case CMD_RECD:
        serial_rx_state=PAYLAD_RX;
    case PAYLAD_RX:
        rx_frm_payload_index++;
        if (rx_ch == SOM_CHAR) {
            goto rx_process_char_no_ESC_err;
        } else if (rx_ch == EOM_CHAR) {
            serial_rx_state = IDLE;
        } else if (rx_ch == ESC_CHAR) {
            serial_rx_state = PAYLAD_RX_ESC;
            rx_frm_chksum += rx_ch;
        } else if (rx_frm_payload_index > rx_frame[curr_buf_idx].payload_len) {
            serial_rx_state = CHKSUM_RECD;
            rx_frame[curr_buf_idx].chksum = rx_ch;
        } else {
            rx_frm_chksum += rx_ch;
            rx_frame[curr_buf_idx].rx_payload[rx_frm_data_index++] = rx_ch;
        }

        break;
    case PAYLAD_RX_ESC:
        rx_frm_payload_index++;
        if (rx_ch > 127) {
            rx_frm_chksum += rx_ch;
            rx_frame[curr_buf_idx].rx_payload[rx_frm_data_index++] = rx_ch&0x7F;
            serial_rx_state = PAYLAD_RX;
        } else {
            goto rx_process_char_no_ESC_err;
        }
        break;
    case CHKSUM_RECD:
        if (rx_ch != EOM_CHAR) {
        } else if( (rx_frame[curr_buf_idx].chksum & 0x7F) == (rx_frm_chksum & 0x07F)) { //checksum match
            rx_frame[curr_buf_idx].payload_len = rx_frm_data_index;
            rx_frame[curr_buf_idx].ackOk = 1;
            rx_frame[curr_buf_idx].available = true;
            curr_buf_idx = (curr_buf_idx +1) % NUM_RX_BUF;
            serial_rx_state = IDLE;
        } else {
            //rx_bad_frame();
        }
        serial_rx_state = IDLE;
        break;
    default:
rx_process_char_no_ESC_err:
        serial_rx_state = IDLE;
        if( rx_ch == SOM_CHAR ) {
            goto rx_process_char_no_ESC_RX_SOM;
        }
        break;
    }

    if (ch != -1) {
        return ch;
    } else
        return -1;
}

int sci_ser_cmd_proc(int commandId, int paramLength, unsigned char *params)
{
    switch (commandId) {
    case CMD_ID_WIFI:
        handleRxWiFi(params,paramLength);
        break;
    case CMD_ID_SNIC:
        handleRxSNIC(params,paramLength);
        break;
    default:
        break;
    }
    return 0;
}
bool SN8200_RxFrameEmpty(void)
{
    return (curr_read_idx == curr_buf_idx);
}

int process_rx_frame(int idx)
{
    if (rx_frame[idx].cmd_id == 0x7F) {
        //serial_ack_nak_recd(1);
    } else if (rx_frame[idx].cmd_id == 0x00) {
        //serial_ack_nak_recd(0);
    } else {
        if (rx_frame[idx].ack_reqd) {
            //send_ack_nak(rx_frame[idx].ackOk);
        }
        if (rx_frame[idx].ackOk) {
            sci_ser_cmd_proc(rx_frame[idx].cmd_id, rx_frame[idx].payload_len, rx_frame[idx].rx_payload);
        }
    }
    return 0;
}

void SN8200_Init(uint32_t baudrate)
{
    SN8200_HAL_Init(baudrate);
}

bool SN8200_HasInput(void)
{
    return !SN8200_HAL_RxBufferEmpty();
}

int serial_transmit(unsigned char cmd_id, unsigned char *payload, int payload_len, unsigned char ack_required)
{
    int len;

    if(payload_len > MAX_PAYLOAD_LEN)
        return 0;

    len = calc_escaped_payload_len_no_ESC(payload_len, payload);

    return no_ESC_transmit(cmd_id, payload_len, payload, ack_required, len);
}

void rx_thread_proc(void)
{
    uint8_t data;

    while(!SN8200_HAL_RxBufferEmpty()) {
        data = SN8200_HAL_ReadByte();
        rx_process_char_no_ESC(data);
    }

    while(!SN8200_RxFrameEmpty()) {
        if (rx_frame[curr_read_idx].available) {
            process_rx_frame(curr_read_idx);
            rx_frame[curr_read_idx].available = false;
        }
        curr_read_idx = (curr_read_idx+1) % NUM_RX_BUF;
    }
}
