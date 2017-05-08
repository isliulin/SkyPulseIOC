/** @file
 *
 *  @brief ISO-TP protocol library declarations
 *  @author T. Gašparič
 *  @date December 2015
 *  @version 1.0
 *  @copyright Fotona d.o.o.
 *
 *  This file must be included in the application source files. \n
 *  The file must not be modified.
 */

/** @mainpage
 *
 *
 *  @section intro_section Introduction
 *
 *  The ISO-TP library implements the ISO 15765-2 standard, originally intended for
 *  vehicle diagnostics systems in the automotive industry.
 *
 *  The ISO 15765-2 covers the Network and Transport layers in the OSI stack. It uses
 *  the services of the Physical and Data Link layers implemented on CAN communication link
 *  and provides services to higher OSI stack layers (Session, Presentation and Application).
 *  The ISO 15765-2 extends the CAN communication link with the capability to handle larger
 *  data messages (up to 4095 bytes). It also adds some handshaking mechanisms.
 *
 *  This implementation is based on ISO 15765-2, Second edition (2011-11-15).
 *
 *
 *  @section conformance_section Conformance to the ISO 15765-2 Standard
 *
 *  The following requirements of the standard are not fully implemented:
 *  -   <em>CAN output message timeouts:</em> The standard requires, that the CAN
 *      message timeout counter starts counting, when the message is actually
 *      transmitted. In this implementation the counter starts counting when the
 *      message is written to the output buffer. Since the buffering usually
 *      includes some kind of FIFO mechanism, this may introduce additional delay
 *      between the buffering and actual transmission.
 *  -   <em>Addressing modes:</em> This implementation supports only normal
 *      addresing mode. Extended and mixed addressing modes are not supported. \n
 *      CAN message identifiers are not interpreted in any way. Any kind of
 *      message identifier data encoding (i.e. normal fixed addressing) must
 *      be done on application level.
 *
 *
 *  @section howto_section How to Use the ISO-TP Library
 *
 *  -#  Copy the following template files to the application project directory and rename them: \n
 *      <tt> @ref isotp-user-template.c</tt> (rename to \p isotp-user.c), \n
 *      <tt> @ref isotp-user-template.h</tt> (rename to \p isotp-user.h), \n
 *      <tt> @ref isotp-config-template.h</tt> (rename to \p isotp-config.h).
 *  -#  Add the following files to your application project build: \n
 *      <tt> @ref isotp.c</tt>, \n
 *      \p isotp-user.c.
 *  -#  \p \#include the <tt> @ref isotp.h</tt> file in your application source files.
 *  -#  In the \p isotp-config.h file set the ISO-TP protocol parameters acording to
 *      the application requirements.
 *  -#  In the \p isotp-user.c file define the functions \n
 *      <tt> @ref isotp_CanMsgGet()</tt>, \n
 *      <tt> @ref isotp_CanMsgPut()</tt>. \n
 *      The two functions represent the link to the CAN hardware. They must be implemented
 *      as non-blocking and must contain some kind of FIFO buffering.
 *  -#  If necessary, redefine the ISO-TP callback functions \n
 *      <tt> @ref isotp_ReceiveCallback()</tt>, \n
 *      <tt> @ref isotp_ReceiveFFCallback()</tt>, \n
 *      <tt> @ref isotp_SendCallback()</tt> \n
 *      in the \p isotp-user.c and \p isotp-user.h files. The functions may be defined
 *      as macros.
 *  -#  Set the method of finding the received ISO-TP message source address by defining
 *      the <tt> @ref isotp_RxEPList</tt> and <tt> @ref isotp_RxEPCount</tt> global varables
 *      and/or the <tt> @ref ISOTP_RX_SA_OFFSET</tt> parameter.
 *  -#  Make sure that the <tt> @ref isotp_Periodic()</tt> function is called
 *      from the application periodically with the period <tt> @ref ISOTP_TICK_PERIOD</tt>.
 *
 *
 *  @section isotp_adressing ISO-TP Addressing 
 *
 *  ISO-TP protocol can transfer different messages with different message ids.
 *  One pair of CAN identifiers must be assigned to each message. One identifier
 *  from the pair (target address) is used by the sender as a target address for
 *  the ISO-TP payload, while the other identifier (source address) is used by
 *  the message recipient to send flow control frames back to the sender. \n
 *  The generation of CAN identifier pairs for the input messages is described in the
 *  documentation of <tt> @ref ISOTP_RX_SA_OFFSET</tt>, <tt> @ref isotp_RxEPList</tt>
 *  and <tt> @ref isotp_RxEPCount</tt>. \n
 *  CAN identifier pairs for the output messages are encoded as members of \p msg
 *  parameter to the <tt> @ref isotp_Send()</tt> function.
 */

#ifndef __ISOTP_H__
#define __ISOTP_H__

#include <stdint.h>
#include <stdbool.h>
#include <isotp-config.h>

/// FlowControl wait (FC N_PDU WT) is not used
#define N_WFTmax 0


/// ISO-TP status codes
typedef enum {
    N_OK,           ///< No error
    N_TIMEOUT_A,    ///< CAN frame not transmitted within the time limit (can be issued to the sender and receiver)
    N_TIMEOUT_Bs,   ///< Max time until reception of the next Flow Control frame exceeded (can be issued to the sender)
    N_TIMEOUT_Cr,   ///< Max time until reception of the next Consecutive Frame exceeded (can be issued to the receiver)
    N_WRONG_SN,     ///< Wrong sequence number (can be issued to the receiver)
    N_INVALID_FS,   ///< Invalid flow status (can be issued to the sender)
    N_UNEXP_PDU,    ///< Unexpected protocol data unit (can be issued to the receiver)
    N_WFT_OVRN,     ///< Max number of WAIT flow status PDUs transmitted (not used in this implementation)
    N_BUFFER_OVFLW, ///< Insufficient RX buffer size (can be issued to the sender)
    N_ERROR,        ///< General or unknown error
} isotp_N_Result_t;

/// Invalid ISO-TP status code
#define N_invalid ((isotp_N_Result_t)(-1))

    
/// ISO-TP @ref isotp_Send() error codes
typedef enum {
    SEND_OK,        ///< No error
    SEND_SIZE,      ///< Invalid message size
    SEND_BUSY,      ///< TX buffer not available
    SEND_ADR,       ///< Source address equals target address
    SEND_ERROR,     ///< General or unknown error
} isotp_SendError_t;

    
/// ISO-TP data buffer with byte access and 32-bit word access
typedef union {
    uint8_t u8[ISOTP_BUF_SIZE];         ///< Byte access
    uint32_t u32[(ISOTP_BUF_SIZE+3)/4]; ///< 32-bit access
} isotp_Data_t;

/// ISO-TP message
typedef struct {
    uint32_t sa;        ///< Network source address (bit 31 indicates 29-bit addressing)
    uint32_t ta;        ///< Network target address (bit 31 indicates 29-bit addressing)
    uint16_t Size;      ///< Data size
    isotp_Data_t Data;  ///< Message payload
    
    /// ISO-TP status code: Message data is valid only if N_Result equals N_OK.
    isotp_N_Result_t N_Result;
    
} isotp_Msg_t;


/// CAN data buffer with byte access and 32-bit word access
typedef union {
    uint8_t u8[8];      ///< Byte access
    uint32_t u32[2];    ///< 32-bit access
} can_Data_t;

/// CAN message structure
typedef struct
{
    uint32_t Id;        ///< Message identifier (bit 31 indicates 29-bit addressing)
    uint8_t DLC;        ///< Frame length
    can_Data_t Data;    ///< Message data
} can_Msg_t;


/// ISO-TP receive endpoint address pair definition
typedef struct {
    uint32_t sa;        ///< Endpoint source address
    uint32_t ta;        ///< Endpoint target address
} isotp_RxEP_t;


#if !ISOTP_RX_SA_OFFSET

/** @brief List of ISO-TP receive endpoints
 *
 *  This is a list of received ISO-TP message source address and
 *  target address pairs. \n
 *  If <tt> @ref ISOTP_RX_SA_OFFSET</tt> is zero, the received message
 *  source address is obtained from its target address, with this list
 *  serving as a look-up table. Only ISO-TP messages with the target
 *  address from this list will be received. All other messages will
 *  be ignored. \n
 *  If <tt> @ref ISOTP_RX_SA_OFFSET</tt> is non-zero, this list
 *  is not used and does not have to be defined. \n
 */
extern isotp_RxEP_t isotp_RxEPList[];

/** @brief Number of ISO-TP endpoints
 *
 *  This is the size of the <tt> @ref isotp_RxEPList</tt> array.
 *  If <tt> @ref ISOTP_RX_SA_OFFSET</tt> is non-zero, this variable
 *  is not used and does not have to be defined. \n
 */
extern unsigned isotp_RxEPCount;

#endif


/** @addtogroup lib_func ISO-TP Public Library Functions
 *
 *  These are public library functions, available to the application. \n
 *  They must not be modified by the user.
 */
//@{

/** @brief Initialize ISO-TP library
 *
 *  This function must be called before any ISO-TP operation. \n
 *  The ISO-TP library assumes, that the CAN hardware interface
 *  has been initialized before the <tt> @ref isotp_Init()</tt> is called.
 */
void isotp_Init(void);


/** @brief ISO-TP protocol state machine handler
 *
 *  This function must be called by the application periodically with
 *  the period of <tt> @ref ISOTP_TICK_PERIOD</tt>. \n
 *  In a singlethreaded environment the function is usually called from the
 *  timer interrupt service routine. \n
 *  In a multithreaded environment the function is usually called either from a
 *  special thread, that is woken up periodically, or from the timer interrupt
 *  service routine.
 */
void isotp_Periodic(void);


/** @brief Initiate ISO-TP message transmission
 *
 *  This function starts the message transmission and returns immediately.
 *  It does not wait for the transmission to be finished. If the application
 *  needs to know, when the transmission is finished, it should redefine the
 *  <tt> @ref isotp_SendCallback()</tt> function. \n
 *  Several messages may be transmitted simultaneously, if they have different
 *  Ids. If the function tries to send a message with the same Id as a message
 *  that is currently being transmitted, the current message transmission is
 *  aborted and new message transmission is initiated. \n
 *  The maximum number of simultaneously transmitted messages equals the number
 *  of TX buffers (see <tt> @ref ISOTP_TX_BUF_NUM</tt>).
 *
 *  @note <tt> @ref isotp_Send()</tt> is not thread safe. If the function is
 *  used in a multithreaded environment and if it is called from more than one
 *  thread, each call must be protected with a mutex or similar locking
 *  mechanism:
 *  -#  lock the mutex,
 *  -#  call <tt> @ref isotp_Send()</tt>,
 *  -#  unlock the mutex.
 *
 *  @param msg      Pointer to message to be sent
 *  @returns        Error code
 */
isotp_SendError_t isotp_Send(const isotp_Msg_t *msg);


/** @name ISO-TP Received Message Buffer Handling Functions
 *
 *  The two functions provide the access to the ISO-TP RX message buffer via the
 *  buffer handle for delayed message processing. The handle can be obtained as
 *  parameter to the <tt> @ref isotp_ReceiveCallback()</tt> function. The two
 *  functions should always be used in pair:
 *  -#  Get the pointer to the received message by calling the
 *      <tt> @ref isotp_GetRxMsg()</tt>,
 *  -#  Process the received message,
 *  -#  Release the message buffer by calling the <tt> @ref isotp_ReleaseRxMsg()</tt>
 *      function.
 *
 *  @note The <tt> @ref isotp_GetRxMsg()</tt> and <tt> @ref isotp_ReleaseRxMsg()</tt>
 *  functions are not thread safe.  If the functions are used in a multithreaded
 *  environment and if they are called from more than one thread, each call must
 *  be protected with a mutex or similar locking mechanism:
 *  -#  lock the mutex,
 *  -#  get a pointer to the message by calling <tt> @ref isotp_GetRxMsg()</tt>,
 *  -#  process the message,
 *  -#  release the message buffer by calling <tt> @ref isotp_ReleaseRxMsg()</tt>,
 *  -#  unlock the mutex.
 */
//@{

/** @brief Get pointer to received ISO-TP message
 *
 *  @param  tph     Handle of the received ISO-TP message buffer
 *                  ( \p tph is a parameter in the <tt> @ref isotp_ReceiveCallback()</tt>
 *                  function)
 *  @retval !=NULL  Pointer to the received ISO-TP message
 *  @retval NULL    The message buffer is not valid, because it has already been released
 */
isotp_Msg_t *isotp_GetRxMsg(int tph);


/** @brief Release received ISO-TP message buffer
 *
 *  This function invalidates the received ISO-TP message and makes the
 *  associated RX message buffer available for reception of the next message. \n
 *  The application should process the message and release the buffer as soon
 *  as possible to make space for new incoming messages. This is especially
 *  important, if the number of RX buffers (see <tt> @ref ISOTP_RX_BUF_NUM</tt>) is low.
 *
 *  @param  tph     Handle of the received ISO-TP message buffer
 *                  ( \p tph is a parameter in the <tt> @ref isotp_ReceiveCallback()</tt>
 *                  function)
 */
void isotp_ReleaseRxMsg(int tph);

//@}

//@}

#include <isotp-user.h>

#endif
