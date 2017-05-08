/** @file
 *
 *  @brief Template file for user defined ISO-TP declarations
 *  @author T. Gašparič
 *  @date December 2015
 *  @version 1.0
 *  @copyright Fotona d.o.o.
 *
 *  How to use this file:
 *  -#  copy the file to the application project directory,
 *  -#  rename the file to \p isotp-user.h,
 *  -#  optionally modify the ISO-TP declarations in \p isotp-user.h according to
 *      the project requirements.
 *
 *  Some functions may be replaced with macros. \n
 *  Unused functions may be replaced with dummy macros.
 */

#ifndef __ISOTP_USER_H__
#define __ISOTP_USER_H__

#include <stdint.h>


/** @addtogroup callback_func ISO-TP Callback Functions
 *
 *  These functions are called by the ISO-TP state machine (i.e. from the
 *  <tt> @ref isotp_Periodic()</tt> function) on the following events:
 *  -   the state machine wants to send a CAN message,
 *  -   the state machine wants to receive a CAN message,
 *  -   an ISO-TP message has been sent,
 *  -   the first frame of a multi-frame ISO-TP message has been received,
 *  -   a complete ISO-TP message has been received.
 *
 *  The default implementations of these functions are provided in the
 *  <tt> @ref isotp-user-template.c</tt> file. \n
 *  They may be redefined by the user as functions or as macros.
 */
//@{

/** @brief Called, when one complete ISO-TP message is sent or transmission error occurrs
 *
 *  @param  sa      Message source address
 *  @param  ta      Message target address
 *  @param  result  Transmission status (error code)
 */
void isotp_SendCallback(uint32_t sa, uint32_t ta, isotp_N_Result_t result);
//#define isotp_SendCallback(sa,ta,result) ((void)0)


/** @brief Called, when one complete ISO-TP message is received or when message reception error occurs
 *
 *  In most cases this function will be redefined by the user.
 *
 *  Basically, there are two ways to handle the received messages:
 *  -#  <em>Immediate processing:</em> The message processing is done in the
 *      <tt> @ref isotp_ReceiveCallback()</tt> function itself, the function returns \p true.
 *      This approach is demonstrated in the default implementation of the
 *      <tt> @ref isotp_ReceiveCallback()</tt> function in the
 *      <tt> @ref isotp-user-template.c</tt> file. \n
 *      The immediate message processing should not be too complex and time consuming,
 *      because the ISO-TP protocol state machine handler (see <tt> @ref isotp_Periodic()</tt>)
 *      may miss the next timer tick.
 *  -#  <em>Delayed processing:</em> The user redefines the <tt> @ref isotp_ReceiveCallback()</tt>
 *      function, so that it simply stores the received message handle to a global
 *      variable and returns \p false. The application then uses this variable at some
 *      later time to process the message and to release the buffer. The message processing
 *      should not be delayed for too long, because, until the message buffer is
 *      released, it is not available for new incoming messages. \n
 *      See descriptions of <tt> @ref isotp_GetRxMsg()</tt> and <tt> @ref isotp_ReleaseRxMsg()</tt>
 *      functions for more details.
 *
 *  @param  tph     Handle of the received ISO-TP message buffer for delayed processing
 *  @param  msg     Pointer to the received message for immediate processing
 *  @retval true    The message has been processed in the <tt> @ref isotp_ReceiveCallback()</tt>
 *                  function, the buffer will be released automatically after the function returns
 *  @retval false   The message has not been processed in the <tt> @ref isotp_ReceiveCallback()</tt>
 *                  function, the message remains in the buffer for delayed processing
 */
bool isotp_ReceiveCallback(int tph, isotp_Msg_t *msg);
//#define isotp_ReceiveCallback(tph,msg) true


/** @brief Called, when the first frame of the ISO-TP multi-frame message is received
 *
 *  @param  sa      Message source address
 *  @param  ta      Message target address
 *  @param  size    Message size
 */
void isotp_ReceiveFFCallback(uint32_t sa, uint32_t ta, uint32_t size);
//#define isotp_ReceiveFFCallback(sa,ta,size) ((void)0)


/** @brief Get one CAN message (non-blocking)
 *
 *  The function must be defined by the user.
 *
 *  @retval true    Success
 *  @retval false   Message not available
 */
bool isotp_CanMsgGet(can_Msg_t *msg);
//#define isotp_CanMsgGet(msg) false


/** @brief Send one CAN message (non-blocking)
 *
 *  The function must be defined by the user. \n
 *  The ISO-TP library functions assume, that it is always possible to send a CAN message.
 *  This usually means, that some kind of output FIFO buffer must be used. \n
 *  If the message can not be sent (i.e. <tt> @ref isotp_CanMsgPut()</tt> returns \p false),
 *  the current ISO-TP message transfer is aborted with error indication (error code
 *  <tt> @ref N_TIMEOUT_A</tt>). There is no retry mechanism.
 *
 *  @retval true    Success
 *  @retval false   Message not sent
 */
bool isotp_CanMsgPut(const can_Msg_t *msg);
//#define isotp_CanMsgPut(msg) false

//@}

#endif
