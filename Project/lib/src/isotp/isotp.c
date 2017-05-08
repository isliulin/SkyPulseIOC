/** @file
 *
 *  @brief ISO-TP protocol library implementation
 *  @author T. Gašparič
 *  @date December 2015
 *  @version 1.0
 *  @copyright Fotona d.o.o.
 *
 *  This file must be added to the application project. \n
 *  The file must not be modified.
 */

#include <stdint.h>
#include <stdlib.h>

#include <isotp.h>


// User-defined ISO-TP parameter check

#if (ISOTP_TICK_PERIOD<100) || (ISOTP_TICK_PERIOD>100000)
#error Invalid ISOTP_TICK_PERIOD
#endif

#if ISOTP_RX_ST_MIN && (ISOTP_TICK_PERIOD > ISOTP_RX_ST_MIN)
#error ISOTP_TICK_PERIOD must be less or equal to ISOTP_RX_ST_MIN
#endif

#if ISOTP_RX_BS > 255
#error Invalid ISOTP_RX_BS
#endif

#if ISOTP_RX_BUF_NUM <= 0
#error Invalid ISOTP_RX_BUF_NUM
#endif

#if ISOTP_TX_BUF_NUM <= 0
#error Invalid ISOTP_TX_BUF_NUM
#endif

#if (ISOTP_BUF_SIZE<7) || (ISOTP_BUF_SIZE>4095)
#error Invalid ISOTP_BUF_SIZE
#endif


/** @brief Adjusted minimum RX CAN message separation time
 *
 *  STmin is derived from <tt> @ref ISOTP_RX_ST_MIN</tt>. The
 *  <tt> @ref ISOTP_RX_ST_MIN</tt> value is rounded and adjusted,
 *  so that it can be sent in a Flow Control frame.
 */
#if ISOTP_RX_ST_MIN < 0
#error Invalid ISOTP_RX_ST_MIN                  // Error: separation time < 0
#elif ISOTP_RX_ST_MIN == 0
#define STmin 0                                 // No message separation
#elif ISOTP_RX_ST_MIN <= 900
#define STmin (0xF0 + (ISOTP_RX_ST_MIN+99)/100) // Separation time <= 900us: round up to 100us
#elif ISOTP_RX_ST_MIN <= 127000
#define STmin ((ISOTP_RX_ST_MIN+999)/1000)      // Separation time <= 127ms: round up to 1ms
#else
#error Invalid ISOTP_RX_ST_MIN                  // Error: separation time > 127ms
#endif


/// Protocol Control Information codes
typedef enum {
    pci_SF, ///< Single Frame
    pci_FF, ///< First Frame
    pci_CF, ///< Consecutive Frame
    pci_FC, ///< Flow Control
} isotp_N_PCI_t;


/// Flow Status codes
typedef enum {
    fs_CTS,     ///< Continue To Send
    fs_WAIT,    ///< Wait
    fs_OVFLW,   ///< Overflow
} isotp_FS_t;


/// ISO-TP RX message buffer state list
typedef enum {
    rxFREE,     ///< The buffer is available for new message
    rxBUSY,     ///< Message reception in progress
    rxREADY,    ///< Reception complete, message ready
} isotp_RxState_t;

/// ISO-TP RX message buffer status data structure
typedef struct {
    isotp_RxState_t state;  ///< Buffer state
    isotp_Msg_t msg;        ///< ISO-TP message buffer
    uint16_t bytecount;     ///< Bytes transferred
    uint8_t sn;             ///< CAN message sequence number
    uint16_t timeout;       ///< Message timeout counter (countdown)
#if ISOTP_RX_BS
    uint16_t blockcount;    ///< CAN RX message block counter (countdown)
#endif
} isotp_RxBufStatus_t;

/// Array of RX message buffers and their status
static isotp_RxBufStatus_t isotp_RxStatus[ISOTP_RX_BUF_NUM];


/// ISO-TP RX message buffer state list
typedef enum {
    txFREE,     ///< The buffer is available for new message
    txBUSY,     ///< Message transmission in progress
    txLOCK,     ///< Message transmission suspended
} isotp_TxState_t;
    
/// ISO-TP TX message buffer status data structure
typedef struct {
    isotp_TxState_t state;  ///< Buffer state
    isotp_Msg_t msg;        ///< ISO-TP message buffer
    uint16_t bytecount;     ///< Bytes transferred
    uint8_t sn;             ///< CAN message sequence number
    uint16_t timeout;       ///< Message timeout counter (countdown)
    uint16_t blockcount;    ///< CAN TX message block counter (countdown)
    uint16_t txtm;          ///< TX message separation timeout (countdown)
    uint16_t txtmreload;    ///< TX message separation timeout reload value
} isotp_TxBufStatus_t;

/// Array of TX message buffers and their status
static isotp_TxBufStatus_t isotp_TxStatus[ISOTP_TX_BUF_NUM];


/// Calculate number of ISO-TP tick periods for 1.2 second timeout
#define TMO_COUNT (1200000U/ISOTP_TICK_PERIOD)


void isotp_Init(void)
{
    unsigned u;
    
    for (u=0; u<ISOTP_RX_BUF_NUM; u++)
    {
        isotp_RxStatus[u].state = rxFREE;
        isotp_RxStatus[u].bytecount = 0;
    }
    for (u=0; u<ISOTP_TX_BUF_NUM; u++)
    {
        isotp_TxStatus[u].state = txFREE;
        isotp_TxStatus[u].bytecount = 0;
    }
}


/// Find first busy ISO-TP message buffer with the specified target address in the RX buffer array
static int isotp_RxFindBusyTA(uint32_t ta)
{
    int i;
    isotp_RxBufStatus_t *bs;
    
    for (i=0, bs=isotp_RxStatus; i<ISOTP_RX_BUF_NUM; i++, bs++)
    {
        if ((bs->msg.ta==ta) && (bs->state==rxBUSY))
            break;
    }
    if (i >= ISOTP_RX_BUF_NUM)
        i = -1;
    return i;
}


/// Find first busy ISO-TP message buffer with the specified source address in the TX buffer array
static int isotp_TxFindBusySA(uint32_t sa)
{
    int i;
    isotp_TxBufStatus_t *bs;
    
    for (i=0, bs=isotp_TxStatus; i<ISOTP_TX_BUF_NUM; i++, bs++)
    {
        if ((bs->msg.sa==sa) && (bs->state==txBUSY))
            break;
    }
    if (i >= ISOTP_TX_BUF_NUM)
        i = -1;
    return i;
}


/// Find first busy ISO-TP message buffer with the specified target address in the TX buffer array
static int isotp_TxFindBusyTA(uint32_t ta)
{
    int i;
    isotp_TxBufStatus_t *bs;
    
    for (i=0, bs=isotp_TxStatus; i<ISOTP_TX_BUF_NUM; i++, bs++)
    {
        if ((bs->msg.ta==ta) && (bs->state==txBUSY))
            break;
    }
    if (i >= ISOTP_TX_BUF_NUM)
        i = -1;
    return i;
}


/// Find first free ISO-TP message buffer in the RX buffer array
static int isotp_RxFindFree(void)
{
    int i;
    isotp_RxBufStatus_t *bs;
    
    for (i=0, bs=isotp_RxStatus; i<ISOTP_RX_BUF_NUM; i++, bs++)
    {
        if (bs->state == rxFREE)
            break;
    }
    if (i >= ISOTP_RX_BUF_NUM)
        i = -1;
    return i;
}


/// Find first free ISO-TP message buffer in the RX buffer array
static int isotp_TxFindFree(void)
{
    int i;
    isotp_TxBufStatus_t *bs;
    
    for (i=0, bs=isotp_TxStatus; i<ISOTP_TX_BUF_NUM; i++, bs++)
    {
        if (bs->state == txFREE)
            break;
    }
    if (i >= ISOTP_TX_BUF_NUM)
        i = -1;
    return i;
}


/// Send flow control frame to message source address
static bool isotp_SendFC(uint32_t sa, isotp_FS_t fs)
{
    can_Msg_t canmsg;
    
    canmsg.Id = sa;
#ifdef ISOTP_TX_PADDING
    canmsg.Data.u32[0] = canmsg.Data.u32[1] =
        (ISOTP_TX_PADDING<<24)|(ISOTP_TX_PADDING<<16)|(ISOTP_TX_PADDING<<8)|ISOTP_TX_PADDING;
    canmsg.DLC = 8;
#endif
    canmsg.Data.u8[0] = (pci_FC << 4) | fs;
    if (fs == fs_CTS)
    {
#ifndef ISOTP_TX_PADDING
        canmsg.DLC = 3;
#endif
        canmsg.Data.u8[1] = ISOTP_RX_BS;
        canmsg.Data.u8[2] = STmin;
    }
#ifndef ISOTP_TX_PADDING
    else
        canmsg.DLC = 1;
#endif
    return isotp_CanMsgPut(&canmsg);
}


/// Process received Single Frame message
static void isotp_ProcessSF(int rxi, const can_Msg_t *canmsg)
{
    isotp_RxBufStatus_t *tprxstat;
    uint8_t *dst;
    const uint8_t *src;
    uint16_t dl;

    tprxstat = isotp_RxStatus + rxi;
    dl = canmsg->Data.u8[0] & 0xF;
    if (dl && (dl < canmsg->DLC))
    {
        tprxstat->msg.ta = canmsg->Id;
        tprxstat->msg.Size = tprxstat->bytecount = dl;
            
        // copy message buffer
        dst = tprxstat->msg.Data.u8;
        src = canmsg->Data.u8 + 1;
        while (dl--)
            *dst++ = *src++;
        
        tprxstat->msg.N_Result = N_OK;
        tprxstat->state = rxREADY;
        if (isotp_ReceiveCallback(rxi, &(tprxstat->msg)))
            tprxstat->state = rxFREE;
    }
}


/// Process received First Frame message
static void isotp_ProcessFF(int rxi, const can_Msg_t *canmsg)
{
    isotp_RxBufStatus_t *tprxstat;
    uint8_t *dst;
    const uint8_t *src;
    uint16_t dl;
    int j;

    tprxstat = isotp_RxStatus + rxi;
    if (canmsg->DLC >= 2)
    {
        // Ignore CAN messages with length less than 2
        dl = canmsg->Data.u8[0] & 0xF;
        dl = (dl << 8) | canmsg->Data.u8[1];
        if (dl > ISOTP_BUF_SIZE)
        {
            if (isotp_SendFC(canmsg->Id, fs_OVFLW))
                tprxstat->state = rxFREE;
            else
            {
                tprxstat->msg.N_Result = N_TIMEOUT_A;
                tprxstat->state = rxREADY;
                if (isotp_ReceiveCallback(rxi, &(tprxstat->msg)))
                    tprxstat->state = rxFREE;
            }
        }
        else if (dl > 7)
        {
            tprxstat->state = rxBUSY;
            tprxstat->msg.ta = canmsg->Id;
            tprxstat->msg.Size = dl;
            
            // copy message buffer
            j = canmsg->DLC - 2;
            dst = tprxstat->msg.Data.u8;
            src = canmsg->Data.u8 + 2;
            while (j--)
                *dst++ = *src++;
            
            tprxstat->bytecount = 6;
#if ISOTP_RX_BS
            tprxstat->blockcount = ISOTP_RX_BS;
#endif
            tprxstat->sn = 0;
            isotp_ReceiveFFCallback(tprxstat->msg.sa, tprxstat->msg.ta, tprxstat->msg.Size);
            if (isotp_SendFC(tprxstat->msg.sa, fs_CTS))
            {
                tprxstat->timeout = TMO_COUNT;
                tprxstat->msg.N_Result = N_TIMEOUT_Cr; // Prepare error code in case the reception times out
            }
            else
            {
                tprxstat->msg.N_Result = N_TIMEOUT_A;
                tprxstat->state = rxREADY;
                if (isotp_ReceiveCallback(rxi, &(tprxstat->msg)))
                    tprxstat->state = rxFREE;
            }
        }
        else
            tprxstat->state = rxFREE;
    }
}


/// Process received Consecutive Frame message
static void isotp_ProcessCF(int rxi, const can_Msg_t *canmsg)
{
    isotp_RxBufStatus_t *tprxstat;
    uint8_t *dst;
    const uint8_t *src;
    uint8_t sn;
    uint16_t rem;
    
    tprxstat = isotp_RxStatus + rxi;
    tprxstat->sn++;
    sn = canmsg->Data.u8[0] & 0xF;
    if (sn == tprxstat->sn)
    {
        rem = tprxstat->msg.Size - tprxstat->bytecount;
        if (rem > 7)
            rem = 7;
        if (rem > (canmsg->DLC-1))
            rem = canmsg->DLC-1;
        
        // copy message buffer
        dst = tprxstat->msg.Data.u8 + tprxstat->bytecount;
        src = canmsg->Data.u8 + 1;
        while (rem--)
        {
            *dst++ = *src++;
            tprxstat->bytecount++;
        }
        
        if (tprxstat->bytecount == tprxstat->msg.Size)
        {
            // ISO-TP message reception complete
            tprxstat->msg.N_Result = N_OK;
            tprxstat->state = rxREADY;
            if (isotp_ReceiveCallback(rxi, &(tprxstat->msg)))
                tprxstat->state = rxFREE;
        }
        else
        {
#if ISOTP_RX_BS
            // Handle block flow control
            tprxstat->blockcount--;
            if (tprxstat->blockcount == 0)
            {
                if (isotp_SendFC(tprxstat->msg.sa, fs_CTS))
                {
                    tprxstat->blockcount = ISOTP_RX_BS;
                    tprxstat->msg.N_Result = N_TIMEOUT_Cr; // Prepare error code in case the reception times out
                    tprxstat->timeout = TMO_COUNT;
                }
                else
                {
                    tprxstat->msg.N_Result = N_ERROR;
                    tprxstat->state = rxREADY;
                    if (isotp_ReceiveCallback(rxi, &(tprxstat->msg)))
                        tprxstat->state = rxFREE;
                }
            }
#else
            tprxstat->msg.N_Result = N_TIMEOUT_Cr; // Prepare error code in case the reception times out
            tprxstat->timeout = TMO_COUNT;
#endif
        }
    }
    else
    {
        // Invalid sequence number
        tprxstat->msg.N_Result = N_WRONG_SN;
        tprxstat->state = rxREADY;
        if (isotp_ReceiveCallback(rxi, &(tprxstat->msg)))
            tprxstat->state = rxFREE;
    }
}


/// Process received Flow Control message
static void isotp_ProcessFC(int rxi, const can_Msg_t *canmsg)
{
    isotp_TxBufStatus_t *tptxstat;
    uint8_t stmin;
    uint32_t sa, ta;
    
    tptxstat = isotp_TxStatus + rxi;
    switch (canmsg->Data.u8[0] & 0xF)
    {
        case fs_CTS:
            // Ignore CAN messages shorter than 3 bytes
            if (canmsg->DLC >= 3)
            {
                tptxstat->blockcount = (canmsg->Data.u8[1]>0) ? canmsg->Data.u8[1] : UINT16_MAX;
                stmin = canmsg->Data.u8[2];
                // Convert requested CAN frame separation time to the units of ISOTP_TICK_PERIOD (round up)
                if (stmin <= 0x7F)
                    // Separation time in miliseconds
                    tptxstat->txtmreload = ((uint32_t)stmin*1000+ISOTP_TICK_PERIOD-1)/ISOTP_TICK_PERIOD;
                else if (stmin <= 0xF0)
                    // Invalid separation time: set to maximum (127ms)
                    tptxstat->txtmreload = (127U*1000U+ISOTP_TICK_PERIOD-1)/ISOTP_TICK_PERIOD;
                else if (stmin <= 0xF9)
                    // Separation time in units of 100 microseconds
                    tptxstat->txtmreload = ((uint32_t)(stmin-0xF0)*100+ISOTP_TICK_PERIOD-1)/ISOTP_TICK_PERIOD;
                else
                    // Invalid separation time: set to maximum (127ms)
                    tptxstat->txtmreload = (127U*1000U+ISOTP_TICK_PERIOD-1)/ISOTP_TICK_PERIOD;
                tptxstat->txtm = 0;
            }
            break;
        case fs_WAIT:
            // Restart CAN frame separation timer
            tptxstat->txtm = tptxstat->txtmreload;
            break;
        case fs_OVFLW:
            sa = tptxstat->msg.sa;
            ta = tptxstat->msg.ta;
            tptxstat->state = txFREE;
            isotp_SendCallback(sa, ta, N_BUFFER_OVFLW);
            break;
        default:
            sa = tptxstat->msg.sa;
            ta = tptxstat->msg.ta;
            tptxstat->state = txFREE;
            isotp_SendCallback(sa, ta, N_INVALID_FS);
            break;
    }
}


/// Send Single Frame message
static void isotp_SendSF(isotp_TxBufStatus_t *tptxstat)
{
    int j;
    can_Msg_t canmsg;
    uint8_t *dst;
    const uint8_t *src;
    uint32_t sa, ta;

    canmsg.Id = tptxstat->msg.ta;
#ifdef ISOTP_TX_PADDING
    canmsg.Data.u32[0] = canmsg.Data.u32[1] =
        (ISOTP_TX_PADDING<<24)|(ISOTP_TX_PADDING<<16)|(ISOTP_TX_PADDING<<8)|ISOTP_TX_PADDING;
    canmsg.DLC = 8;
#else
    canmsg.DLC = tptxstat->msg.Size + 1;
#endif
    canmsg.Data.u8[0] = (pci_SF<<4) | tptxstat->msg.Size;

    // copy message buffer
    j = tptxstat->msg.Size;
    dst = canmsg.Data.u8 + 1;
    src = tptxstat->msg.Data.u8;
    while (j--)
        *dst++ = *src++;

    sa = tptxstat->msg.sa;
    ta = tptxstat->msg.ta;
    if (isotp_CanMsgPut(&canmsg))
    {
        tptxstat->state = txFREE;
        isotp_SendCallback(sa, ta, N_OK);
    }
    else
    {
        tptxstat->state = txFREE;
        isotp_SendCallback(sa, ta, N_TIMEOUT_A);
    }
}


/// Send First Frame of a multi-frame message
static void isotp_SendFF(isotp_TxBufStatus_t *tptxstat)
{
    int j;
    can_Msg_t canmsg;
    uint8_t *dst;
    const uint8_t *src;
    uint32_t sa, ta;

    canmsg.Id = tptxstat->msg.ta;
    canmsg.DLC = 8;
    canmsg.Data.u8[0] = (pci_FF<<4) | (0xF&(tptxstat->msg.Size>>8));
    canmsg.Data.u8[1] = 0xFF&(tptxstat->msg.Size);

    // copy message buffer
    j = 6;
    dst = canmsg.Data.u8 + 2;
    src = tptxstat->msg.Data.u8;
    while (j--)
        *dst++ = *src++;
    
    if (isotp_CanMsgPut(&canmsg))
    {
        // Initially set max separation time 127ms
        tptxstat->txtmreload = (127U*1000U+ISOTP_TICK_PERIOD-1)/ISOTP_TICK_PERIOD;
        tptxstat->bytecount = 6;
        tptxstat->sn = 0;
        tptxstat->timeout = TMO_COUNT;
        tptxstat->txtm = UINT16_MAX;
        tptxstat->msg.N_Result = N_TIMEOUT_Bs; // Prepare error code in case the transmission times out
    }
    else
    {
        sa = tptxstat->msg.sa;
        ta = tptxstat->msg.ta;
        tptxstat->state = txFREE;
        isotp_SendCallback(sa, ta, N_TIMEOUT_A);
    }
}


/// Send Consecutive Frame of a multi-frame message
static void isotp_SendCF(isotp_TxBufStatus_t *tptxstat)
{
    int j;
    can_Msg_t canmsg;
    uint8_t *dst;
    const uint8_t *src;
    uint16_t dl;
    uint32_t sa, ta;

    tptxstat->blockcount--;
    canmsg.Id = tptxstat->msg.ta;
    dl = tptxstat->msg.Size - tptxstat->bytecount;
    dl = (dl > 7) ? 7 : dl;
#ifdef ISOTP_TX_PADDING
    canmsg.Data.u32[0] = canmsg.Data.u32[1] =
        (ISOTP_TX_PADDING<<24)|(ISOTP_TX_PADDING<<16)|(ISOTP_TX_PADDING<<8)|ISOTP_TX_PADDING;
    canmsg.DLC = 8;
#else
    canmsg.DLC = dl + 1;
#endif
    tptxstat->sn++;
    canmsg.Data.u8[0] = (pci_CF<<4) | (0xF&tptxstat->sn);

    // copy message buffer
    j = dl;
    dst = canmsg.Data.u8 + 1;
    src = tptxstat->msg.Data.u8 + tptxstat->bytecount;
    while (j--)
        *dst++ = *src++;
    
    if (isotp_CanMsgPut(&canmsg))
    {
        tptxstat->bytecount += dl;
        if (tptxstat->bytecount < tptxstat->msg.Size)
        {
            tptxstat->txtm = tptxstat->txtmreload;
            tptxstat->timeout = TMO_COUNT;
            tptxstat->msg.N_Result = N_TIMEOUT_Bs; // Prepare error code in case the transmission times out
        }
        else
        {
            sa = tptxstat->msg.sa;
            ta = tptxstat->msg.ta;
            tptxstat->state = txFREE;
            isotp_SendCallback(sa, ta, N_OK);
        }
    }
    else
    {
        sa = tptxstat->msg.sa;
        ta = tptxstat->msg.ta;
        tptxstat->state = txFREE;
        isotp_SendCallback(sa, ta, N_TIMEOUT_A);
    }
}


void isotp_Periodic(void)
{
    int i;
    isotp_N_PCI_t pci;
    can_Msg_t canmsg;
    isotp_RxBufStatus_t *tprxstat;
    isotp_TxBufStatus_t *tptxstat;
#if !ISOTP_RX_SA_OFFSET
    uint32_t sa;
    int j;
#endif
    
    // Process incoming CAN messages
    if (isotp_CanMsgGet(&canmsg))
    {
        // Ignore zero length and invalid length CAN messages
        if ((canmsg.DLC > 0) && (canmsg.DLC <=8))
        {
            pci = (isotp_N_PCI_t)((canmsg.Data.u8[0]>>4) & 0xF);
            switch (pci)
            {
                case pci_SF:
                case pci_FF:
                    // Single Frame CAN message (SF N_PDU)
                    // First Frame CAN message (FF N_PDU)
                    i = isotp_RxFindBusyTA(canmsg.Id);
                    if (i >= 0)
                    {
                        // Message reception with the same Id is already in progress:
                        // First, cancel old message reception and report error, then restart with the new message.
                        tprxstat = isotp_RxStatus + i;
                        tprxstat->msg.sa = canmsg.Id;
                        tprxstat->msg.N_Result = N_UNEXP_PDU;  // Set error code for the old message
                        tprxstat->state = rxREADY;
                        if (isotp_ReceiveCallback(i, &(tprxstat->msg)))
                            tprxstat->state = rxFREE;
                    }
#if ISOTP_RX_SA_OFFSET
                    // Message source address is obtained by offsetting the message target address
                    i = isotp_RxFindFree();
                    if (i >= 0)
                    {
                        // Start reception with new buffer
                        isotp_RxStatus[i].msg.sa = canmsg.Id + (uint32_t)ISOTP_RX_SA_OFFSET;
                        if (pci == pci_SF)
                            isotp_ProcessSF(i, &canmsg);
                        else
                            isotp_ProcessFF(i, &canmsg);
                    }
#else
                    // Message source address is obtained by searching the reception endpoint list
                    for (j=0; j<isotp_RxEPCount; j++)
                    {
                        if (isotp_RxEPList[j].ta == canmsg.Id)
                        {
                            sa = isotp_RxEPList[j].sa;
                            break;
                        }
                    }
                    if (j < isotp_RxEPCount)
                    {
                        i = isotp_RxFindFree();
                        if (i >= 0)
                        {
                            // Start reception with new buffer
                            isotp_RxStatus[i].msg.sa = sa;
                            if (pci == pci_SF)
                                isotp_ProcessSF(i, &canmsg);
                            else
                                isotp_ProcessFF(i, &canmsg);
                        }
                    }
#endif
                    break;
                
                case pci_CF:
                    // Consecutive Frame CAN message (CF N_PDU)
                    i = isotp_RxFindBusyTA(canmsg.Id);
                    if (i >= 0)
                        isotp_ProcessCF(i, &canmsg);
                    break;
                
                case pci_FC:
                    // Flow Control CAN message (FC N_PDU)
                    i = isotp_TxFindBusySA(canmsg.Id);
                    if (i >= 0)
                        isotp_ProcessFC(i, &canmsg);
                    break;
                
                default:
                    // Unknown N_PDU is ignored
                    break;
            }
        }
    }
    
    // Handle RX timeouts
    for (i=0, tprxstat=isotp_RxStatus; i<ISOTP_RX_BUF_NUM; i++, tprxstat++)
    {
        if (tprxstat->state == rxBUSY)
        {
            if (tprxstat->timeout > 0)
                tprxstat->timeout--;
            if (tprxstat->timeout == 0)
            {
                tprxstat->state = rxREADY;
                if (isotp_ReceiveCallback(i, &(tprxstat->msg)))
                    tprxstat->state = rxFREE;
            }
        }
    }
    
    // Handle TX buffers
    for (i=0, tptxstat=isotp_TxStatus; i<ISOTP_TX_BUF_NUM; i++, tptxstat++)
    {
        if (tptxstat->state == txBUSY)
        {
            if (tptxstat->msg.Size <= 7)
            {
                // Send single frame message
                isotp_SendSF(tptxstat);
            }
            else if (tptxstat->bytecount == 0)
            {
                // Send first frame in a multi-frame message
                isotp_SendFF(tptxstat);
            }
            else
            {
                if (tptxstat->blockcount && !tptxstat->txtm)
                {
                    // Send consecutive frame in a multi-frame message
                    isotp_SendCF(tptxstat);
                }
                if (tptxstat->txtm)
                    tptxstat->txtm--;
            }
        }
        if (tptxstat->state == txBUSY)
        {
            // Handle TX timeouts
            if (tptxstat->timeout > 0)
                tptxstat->timeout--;
            if (tptxstat->timeout == 0)
            {
                isotp_SendCallback(tptxstat->msg.sa, tptxstat->msg.ta, tptxstat->msg.N_Result);
                tptxstat->state = txFREE;
            }
        }
    }
}


isotp_SendError_t isotp_Send(const isotp_Msg_t *msg)
{
    int i;
    isotp_TxBufStatus_t *tptxstat;
    uint32_t *dst;
    const uint32_t *src;
    isotp_SendError_t ret = SEND_ERROR;
    
    if ((msg->Size > 0) && (msg->Size <= ISOTP_BUF_SIZE))
    {
        if (msg->sa == msg->ta)
            ret = SEND_ADR;
        else
        {
            // Is a message with the same Id currently being transmitted?
            //  - Yes:  Cancel the currently transmitted message and start transmitting
            //          the new message in the same buffer.
            //  - No:   Find a new buffer and (if a new buffer is avaialable) start
            //          transmitting the new message in the new buffer.
            i = isotp_TxFindBusyTA(msg->ta);
            if (i < 0)
                i = isotp_TxFindFree();
            if (i < 0)
                ret = SEND_BUSY;
            else
            {
                // Initiate message transmission
                tptxstat->state = txLOCK;
                tptxstat = isotp_TxStatus + i;
                tptxstat->bytecount = 0;
                tptxstat->blockcount = 0;
                tptxstat->timeout = 0;
                tptxstat->msg.sa = msg->sa;
                tptxstat->msg.ta = msg->ta;
                tptxstat->msg.Size = msg->Size;

                // copy message buffer
                dst = tptxstat->msg.Data.u32;
                src = msg->Data.u32;
                i = (msg->Size+3)/4;
                while (i--)
                    *dst++ = *src++;
                
                tptxstat->state = txBUSY;
                ret = SEND_OK;
            }
        }
    }
    else
        ret = SEND_SIZE;
    return ret;
}


isotp_Msg_t *isotp_GetRxMsg(int tph)
{
    if (isotp_RxStatus[tph].state == rxREADY)
        return &(isotp_RxStatus[tph].msg);
    return NULL;
}


void isotp_ReleaseRxMsg(int tph)
{
    isotp_RxStatus[tph].state = rxFREE;
}
