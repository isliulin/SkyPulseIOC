/** @file
 *
 *  @brief Template file for user defined ISO-TP protocol parameters
 *  @author T. Gašparič
 *  @date December 2015
 *  @version 1.0
 *  @copyright Fotona d.o.o.
 *
 *  How to use this file:
 *  -#  copy the file to the application project directory,
 *  -#  reneme the file to \p isotp-config.h,
 *  -#  optionally modify the ISO-TP parameters in \p isotp-config.h
 *      according to the project requirements.
 */

#ifndef __ISOTP_CONFIG_H__
#define __ISOTP_CONFIG_H__


/** @addtogroup config ISO-TP Protocol Configuration Parameters
 */
//@{

/** @brief Timer tick period of the ISO-TP protocol state machine (in microseconds)
 *
 *  Some ISO-TP protocol timings are rounded up to the nearest multiple of
 *  @ref ISOTP_TICK_PERIOD.
 */
#define ISOTP_TICK_PERIOD   1000U


/** @brief Minimum RX CAN message separation time in microseconds
 *
 *  The parameter is relevant for message reception only. \n
 *  This parameter is sent by the receiver in the Flow Control frame.
 *  Due to the ISO-TP protocol requirements, this parameter is subject to
 *  rounding, therefore the actual separation time may be longer. \n
 *  If the parameter is zero, the transmitter may send the messages back to
 *  back, without separation time and with the maximum possible speed, but
 *  still respecting the <tt> @ref ISOTP_RX_BS</tt> parameter.
 */
#define ISOTP_RX_ST_MIN     2000U


/** @brief RX CAN message block size (number of CAN messages between
 *         two Flow Control frames)
 *
 *  The parameter is relevant for message reception only. \n
 *  This parameter is sent by the receiver in the Flow Control frame.
 *  It indicates, how many CAN payload frames the transmitter may send before
 *  receiver replies with the next Flow Control frame. \n
 *  If the parameter is zero, the transmitter may send the messages in
 *  continuous stream (respecting only the <tt> @ref ISOTP_RX_ST_MIN</tt> parameter),
 *  without waiting for Flow Control frames. In this case the receiver sends
 *  a Flow Control frame only after the first CAN payload frame.
 */
#define ISOTP_RX_BS         5U


/** @brief TX CAN message padding
 *
 *  If this parameter is defined, all CAN output messages are padded with the
 *  parameter value to the full message length of 8 bytes. If this parameter
 *  is not defined, there is no padding (i.e. output CAN messages may be shorter
 *  than 8 bytes).
 */
#define ISOTP_TX_PADDING    0xAAU


/** @brief RX message source address offset value
 *
 *  If this parameter is non-zero, the RX message source address is obtained
 *  by adding the offset value to the message target address. In this case
 *  the <tt> @ref isotp_RxEPList</tt> and  <tt> @ref isotp_RxEPCount</tt>
 *  global variables are not used and do not have to be defined. \n
 *  If this parameter is zero, the RX message source address is obtained from
 *  the <tt> @ref isotp_RxEPList</tt> array.
 */
#define ISOTP_RX_SA_OFFSET  1


/** @brief Size of one ISO-TP message buffer
 *
 *  Maximum buffer size is 4095, but the user may choose smaller size.
 */
#define ISOTP_BUF_SIZE      64


/** @brief Number of RX buffers
 *
 *  This parameter determines, how many ISO-TP messages can be received simultaneously.
 */
#define ISOTP_RX_BUF_NUM    5


/** @brief Number of TX buffers
 *
 *  This parameter determines, how many ISO-TP messages can be transmitted simultaneously.
 */
#define ISOTP_TX_BUF_NUM    2

//@}

#endif
