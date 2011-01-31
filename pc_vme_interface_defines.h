/***************************************************************************/
/*                                                                         */
/*  Project: SIS                                                           */
/*                                                                         */
/*  Filename: pc_vme_interface_defines.h                                   */
/*                                                                         */
/*  Funktion:                                                 		   */
/*                                                                         */
/*  Autor:                TH                                               */
/*  Modified		  BL                                               */
/*  date:                 21.02.2005                                       */
/*  last modification:    24.10.2008                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
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
/*   2007                                                                  */
/*                                                                         */
/***************************************************************************/


// PC interface
#define USB_VME_INTERFACE		// sis3150USB USB2.0 interface

#ifdef USB_VME_INTERFACE
	#define _DEBUG
	#define PLX_9054
	#define PCI_CODE
//	#define LITTLE_ENDIAN

	#include <sis3150usb_vme_calls.h>  
	#include <sis3150usb_utils.h>

extern HANDLE m_device;

#endif
