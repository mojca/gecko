/***************************************************************************/
/*  Filename:  usrpadaptor.cpp                                             */
/*                                                                         */
/*  Funktion:   We rely on some functions from the linux packet            */
/*              radio project (usrp) to load the fx2 firmware.  These      */
/*              functions were written to be called in C++ and our lib     */
/*              is suposed to be C.  This file is a little adaptor between */
/*              the two                                                    */
/*                                                                         */
/*  Autor:      R. Fox                                                     */
/* date:        18.05.2006  (coding begins)                                */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  © 2006                                                                 */
/*                                                                         */
/***************************************************************************/
#include <usb.h>
#include <usrp_prims.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

extern "C" 
{

  struct usb_dev_handle*
  ccusrp_open_cmd_interface(struct usb_device* pHandle)
  {
    return usrp_open_cmd_interface(pHandle);
  }

  int 
  ccusrp_load_firmware(struct usb_dev_handle* pHandle, const char* filename)
  {
    bool status = usrp_load_firmware(pHandle, filename, true);
    return status ? TRUE : FALSE;
  }
  int 
  ccusrp_close_interface(struct usb_dev_handle* pHandle)
  {
    return usrp_close_interface(pHandle);
  }

}
