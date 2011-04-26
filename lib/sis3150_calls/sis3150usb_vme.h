/***************************************************************************/
/*  Filename: sis3150usb_vme .h                                         */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.05.2004                                       */
/*  last modification:    20.12.2005                                       */
/*  20.12.2005 :  add Sis3150usb_CloseDriver                               */
/*  14.04.2006 :  Linux version (R.Fox)                                    */
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
/*  © 2005                                                                 */
/*                                                                         */
/***************************************************************************/
#ifndef __SIS3150USB_VME_H
#define __SIS3150USB_VME_H	/* multiple include gaurd. */

#ifdef unix


#ifndef __USB_H__
#include <usb.h>		/* LIBUSB header. */
#endif

/*  Windows MSVC does not bother to provide stdint as required by the C89
    standard hence the #ifdefery below with hopes that it's correct on 
    64 bit windows.
*/

#ifdef __WINDOWS__
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef long           LONG;
#else
#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif
typedef uint8_t  UCHAR;
typedef uint16_t USHORT;
typedef uint32_t ULONG;
typedef int32_t  LONG;
#endif


/*   Typedefs to allow common prototypes with WIN */
/*    NOTE: Should define the 'sized' based ones in terms of e.g. uint32_t 
            for ULONG that way this >might< port to 64 bits  
*/
typedef usb_dev_handle* HANDLE;
typedef char          *PCHAR;
typedef unsigned int   UINT;
typedef int            BOOLEAN;	/* c has no bool type. */

#endif

#ifdef __WINDOWS__
#define MAX_FILE_SIZE (1024*64)     //max size of download file
#define MAX_USB_DEV_NUMBER 4    
#define MAX_DRIVER_NAME 64
#endif

#define USB_DEVICE_NAME_SIZE 128

struct SIS3150USB_Device_Struct {
    UCHAR	cDName[USB_DEVICE_NAME_SIZE];
    USHORT	idVendor;
    USHORT	idProduct;
    USHORT	idSerNo;
    USHORT	idFirmwareVersion;
#ifdef unix
    struct usb_device*  pDeviceStruct;
#endif
} ;


// DLL support
#ifndef EXPORT
#ifdef __WINDOWS__
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif
#endif


#ifdef __cplusplus
extern "C" {
#endif




int EXPORT FindAll_SIS3150USB_Devices(struct SIS3150USB_Device_Struct* sis3150usb_Device, 
				      unsigned int*                    nof_usbdevices, 
				      unsigned int                     max_usb_device_Number) ;

int EXPORT Sis3150usb_OpenDriver (PCHAR usbDeviceName, HANDLE *usbDeviceHandle );
int EXPORT Sis3150usb_OpenDriver_And_Download_FX2_Setup (PCHAR usbDeviceName, HANDLE *usbDeviceHandle ) ;
int EXPORT Sis3150usb_CloseDriver(HANDLE usbDevice);




int EXPORT sis3150Usb_Register_Single_Read(HANDLE usbDevice, ULONG addr, ULONG* data) ;
int EXPORT sis3150Usb_Register_Dma_Read(HANDLE usbDevice, ULONG addr, ULONG* dmabufs,ULONG req_nof_data, ULONG* got_nof_data) ;

int EXPORT sis3150Usb_Register_Single_Write(HANDLE usbDevice, ULONG addr, ULONG data) ;
int EXPORT sis3150Usb_Register_Dma_Write(HANDLE usbDevice, ULONG addr,ULONG* dmabufs, ULONG req_nof_data, ULONG* put_nof_data) ;


int EXPORT sis3150Usb_TsBus_Single_Read(HANDLE usbDevice, ULONG addr, ULONG* data) ;
int EXPORT sis3150Usb_TsBus_Dma_Read(HANDLE usbDevice, ULONG addr, ULONG* dmabufs,ULONG req_nof_data, ULONG* got_nof_data) ;

int EXPORT sis3150Usb_TsBus_Single_Write(HANDLE usbDevice, ULONG addr, ULONG data) ;
int EXPORT sis3150Usb_TsBus_Dma_Write(HANDLE usbDevice, ULONG addr,ULONG* dmabufs, 
				      ULONG req_nof_data, ULONG* put_nof_data) ;


int EXPORT sis3150Usb_Vme_Single_Read(HANDLE usbDevice, ULONG addr, ULONG am, ULONG size, ULONG* data) ;
int EXPORT sis3150Usb_Vme_Dma_Read(HANDLE usbDevice, ULONG addr, ULONG am, 
				   ULONG size, ULONG fifo_mode,
				   ULONG* dmabufs, ULONG req_nof_data, 
				   ULONG* got_nof_data) ;


int EXPORT sis3150Usb_Vme_Single_Write(HANDLE usbDevice, ULONG addr, ULONG am, ULONG size, ULONG data);

int EXPORT sis3150Usb_Vme_Dma_Write(HANDLE usbDevice, ULONG addr, ULONG am, 
				    ULONG size, ULONG fifo_mode,
				    ULONG* dmabufs, ULONG req_nof_data, 
				    ULONG* put_nof_data);


int EXPORT sis3150Usb_VmeSysreset(HANDLE usbDevice) ;

#ifdef __cplusplus
}
#endif

#endif /* Multiple include gaurd. */
