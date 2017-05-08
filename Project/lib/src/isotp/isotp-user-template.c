/** @file
 *
 *  @brief Template file for user defined ISO-TP function definitions
 *  @author T. Gašparič
 *  @date December 2015
 *  @version 1.0
 *  @copyright Fotona d.o.o.
 *
 *  How to use this file:
 *  -#  copy the file to the application project directory,
 *  -#  rename the file to \p isotp-user.c,
 *  -#  add the file to the application project,
 *  -#  optionally modify the ISO-TP functions in \p isotp-user.c according to
 *      the project requirements.
 *
 *  Some functions may be replaced with macros. \n
 *  Unused functions may be replaced with dummy macros.
 */

#include "isotp.h"


#if !ISOTP_RX_SA_OFFSET

// Sample ISO-TP receive endpoint address pair list
isotp_RxEP_t isotp_RxEPList[] = {
    {0x100, 0x110},             // 11-bit Id
    {0x200, 0x210},             // 11-bit Id
    {0x80000300, 0x80000310},   // 29-bit Id
};

// Sample ISO-TP receive endpoint address pair list size
unsigned isotp_RxEPCount = sizeof(isotp_RxEPList)/sizeof(isotp_RxEPList[0]);

#endif


#ifndef isotp_SendCallback
void isotp_SendCallback(uint32_t sa, uint32_t ta, isotp_N_Result_t result)
{
}
#endif


#ifndef isotp_ReceiveCallback
bool isotp_ReceiveCallback(int tpi, isotp_Msg_t *msg)
{
    /*
    ...
    Immediate message processing via the *msg pointer
    ...
     */
    return true;
}
#endif


#ifndef isotp_ReceiveFFCallback
void isotp_ReceiveFFCallback(uint32_t sa, uint32_t ta, uint32_t size)
{
}
#endif


#ifndef isotp_CanMsgGet
bool isotp_CanMsgGet(can_Msg_t *msg)
{
    return false;
}
#endif

#ifndef isotp_CanMsgPut
bool isotp_CanMsgPut(const can_Msg_t *msg)
{
    return false;
}
#endif
