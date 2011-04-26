/***************************************************************************/
/*  Filename:  sis3150usb_vme.c                                            */
/*                                                                         */
/*  Funktion:   Linux implementation file for low level USB/VME            */
/*              access software.     See sis3150usb_vme.h for all of the   */
/*              prototypes implemented by this file.                       */
/*                                                                         */
/*  Autor:      R. Fox                                                     */
/* date:        14.04.2006  (coding begins)                                */
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

#include "sis3150usb_vme.h"
#include "usrpadaptor.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifdef DEBUG_TRACE
#define RETURN(code) if (code) {fprintf(stderr, "FAIL %s returning %x\n", __PRETTY_FUNCTION__, code);}  \
                     return (code);
#else
#define RETURN(code) return (code)
#endif


#define SUCCESS 0

/*  Constant definitions */

#define USB_MAX_NOF_BYTES    0xf800
#define USB_MAX_NOF_LWORDS   USB_MAX_NOF_BYTES/4

#define sis3150usb_error_code_invalid_parameter  			0x110 
#define sis3150usb_error_code_usb_write_error    			0x111 
#define sis3150usb_error_code_usb_read_error     			0x112 
#define sis3150usb_error_code_usb_read_length_error     	0x113 


#define INSTDIR "/usr/opt/sisusb"

static const int    sisVendorId = 0x1657;          /* Vendor ID taken by SIS */
static const int    productId   = 0x3150;          /* Product ID we are looking for */

static const int    USB_WRITE_ENDPOINT = 2;
static const int    USB_READ_ENDPOINT  = 0x86;
static const int    USB_TIMEOUT        = 30000;     /* This is in ms. */

static const char*  rootEnv      = "SIS3150";
static const char*  firmwareName = "setup_8051.hex"; 
static const char*  usbDeviceDir = "/proc/bus/usb/";

/*  Fields/bits in the request Space/CTL word */

static const long USB_HEADER     = 0xaaaa0000; /*  USB Request header. */
static const long SPACE_REGISTER = 1 << 12;
static const long SPACE_TSBUS    = 2 << 12;
static const long SPACE_VME      = 4 << 12;

static const long  CTL_WRITE        = 0x0800; /* Operation is a write. */
static const long  CTL_DISABLE_INCR = 0x0400; /* Don't autoincrement addresses */
static const long  CTL_INCR_BYTE    = 0x0000; /* Increment for bytes    */
static const long  CTL_INCR_SHORT   = 0x0100; /* Increment for 16 bit words */
static const long  CTL_INCR_LONG    = 0x0200; /* Increment for 32 bit words */
static const long  CTL_INCR_DOUBLE  = 0x0300; /* Increment for 64 bit words */
static const long  CTL_XFER_WIDTH   = 0x0700; /* Mask for transfer width field */

static const long  CTL_LISTMODE     = 0x0020; /* Op is list mode?           */
static const long  CTL_TSBUS_SGLFLG = 0x0002; /* ???                        */
static const long  CTL_TSBUS_D64    = 0x0001; /* 64 bit wide Tigersharc bus access */

static const long  AMOD_SHIFT       = 16;     /* left shift count for addr. mod. */

static const int   XFER_WRITE_HEADERSIZE = 3*sizeof(long); /* Bytes in write xfer header */
static const int   XFER_READ_HEADERSIZE  = 2*sizeof(long);
/*   Local impure storage: */

static int usbInitialized = FALSE;



/*   Utility functions: */

/*   If necessary, initialize the USB. */
static void
initUSB()
{
  if(!usbInitialized) {
    usb_init();
    usbInitialized = TRUE;
  }
}


/*
   Function to perform a USB transaction.  In general, this is a
   usb_bulk_write followed by a usb_bulk_read   The endpoints are
   fixed by the constants USB_READ/WRITE_ENDPOINT.
   Parameters:
     device      - usb_dev_handle* open on the usb controller.
     outpacket   - Request packet in host byte order.
     outbytes    - bytes in output packet (should be divisible by sizeof(long)
     inpacket    - Space for reply packet.
     inbytes     - bytes for reply packet.(should be divisible by sizeof(long)

  Returns:
     -1   Write failed, errno has the reason.
     -2   Read  failed, errno has the reason.
      n   Number of bytes read into the inpacket.
*/
static int usbTransaction(usb_dev_handle* device,
			  void*           outpacket,
			  unsigned int    outbytes,
			  void*           inpacket,
			  unsigned int    inbytes)
{
  void* usboutpacket;
  int   status;
  int   i;
  
  /* Do the write and process the status:  */

  status = usb_bulk_write(device, USB_WRITE_ENDPOINT,
                          (const char*)outpacket, outbytes, USB_TIMEOUT);
  if(status < 0) {
    errno = -status;
    perror("write");
    return -1;
  }
  /* Do the write, process status, and if ok, transform the read data. */
  
  status = usb_bulk_read(device, USB_READ_ENDPOINT,
                         (char*)inpacket, inbytes, USB_TIMEOUT);
  if(status < 0) {
    errno = -status;
    perror("read");
    return -2;
  }
  return status;
}

/*
   Local function to return a device structure pointer given a usb
   device name.  NULL is returned if there is no matching device name.
*/
static struct usb_device*
getDeviceHandle(PCHAR usbDeviceName)
{
  struct SIS3150USB_Device_Struct* pDevices;
  unsigned int              numDevices;
  unsigned int              finalNumDevices;
  unsigned int              found = FALSE;
  int                       i;
  struct usb_device*        handle;

  /* First enumerate to get the device count: */

  FindAll_SIS3150USB_Devices(NULL,
			     &numDevices,
			     0);

  /* Now allocate enough descriptors to hold them all:                      */
  /* ASSUMPTION: There are no hot plug events between the previous find and */
  /*             completion of this find for SIS3150s                       */

  pDevices = (struct SIS3150USB_Device_Struct*)
    malloc(numDevices*sizeof(struct SIS3150USB_Device_Struct));
  if(!pDevices) {
    return NULL;
  }

  FindAll_SIS3150USB_Devices(pDevices, 
			     &finalNumDevices,
			     numDevices);

  for (i =0; i < numDevices; i++) {
    if (strcmp(usbDeviceName, (char*)pDevices[i].cDName) == 0) {
      found = TRUE;
      break;
    }
  }
  if (!found) {
    return NULL;
  }

  handle = pDevices[i].pDeviceStruct;
  free(pDevices);
  return handle;

}




/*!
    Locate all SIS3150 USB/VME devices on the bus and return 
    information described in the SIS3150USB_Device_Struct structure
    to the caller. 
    
    \param sis3150usb_device  : struct SIS3150USB_Device_struct* [out]
       Pointer to an array of structures into which the device description will
       be written.
    \param nof_usb_devices    : unsigned int* [out]
       Pointer to an unsigned int into which will be written the number of devices
       found...see below.
    \param max_usb_device_Number : unsigned int [in]
       Number of elements in the sis3150usb_device array.

    \return int
    \retval SUCCESS - Success, all of the usb SIS3150 devices were found and
                     information put in the sis3150usb_device array
    \retval 1      - Success >but< there were more interfaces than could
                     fit in the description array.  In this case, the
                     first max_usb_device_Number are written into the array,
                     but no_usbdevices is  filled in with the actual device count
                     so that this function can be called again with a 
                     correctly sized array.
    \retval other  - Errors from the USB subsystem...the negation of errno.
*/            
int 
FindAll_SIS3150USB_Devices(struct SIS3150USB_Device_Struct* sis3150usb_Device, 
			   unsigned int*                    nof_usbdevices, 
			   unsigned int                     max_usb_device_Number)
{
  int                  found = 0;
  struct  usb_bus*     busses;
  struct  usb_device*  aDevice;
  unsigned int         arraySize = max_usb_device_Number;

  /* Before we can scan we need to be sure that the USB lib is initialized: */ 

  initUSB();

  /* Scan the USB bus filling in as many devices as we can   */

  if (usb_find_busses() < 0) {
    return -errno;
  }
  if (usb_find_devices() < 0) {
    return -errno;
  }
  busses = usb_get_busses();

  /*  For each USB bus in the system: */

  while (busses) {
    aDevice = busses->devices;

    /* For each device on a bus */

    while (aDevice) {
      if( (aDevice->descriptor.idVendor  == sisVendorId) &&
	  (aDevice->descriptor.idProduct == productId)) {
	
	/* Got an SIS 3150 device: */
	
	found++;
	if (arraySize) {
	  char devname[PATH_MAX+1];
	  strcpy(devname, busses->dirname);
	  strncat(devname, "/", PATH_MAX);
	  strncat(devname, aDevice->filename, PATH_MAX);
	  strncpy((char*)sis3150usb_Device->cDName, devname, USB_DEVICE_NAME_SIZE-1);

	  sis3150usb_Device->idVendor  = aDevice->descriptor.idVendor;
	  sis3150usb_Device->idProduct = aDevice->descriptor.idProduct;
	  sis3150usb_Device->idSerNo   = aDevice->descriptor.bcdDevice; 
	  sis3150usb_Device->idFirmwareVersion = 0; /* TODO: Fill this in? */
	  sis3150usb_Device->pDeviceStruct     = aDevice;

	  sis3150usb_Device++;
	  arraySize--;
	}

      }

      aDevice = aDevice->next;
    }

    busses = busses->next;
  }
  /*  Return the number of devices found and normal  */

  *nof_usbdevices = found;
  return found <= max_usb_device_Number ? SUCCESS : 1;
}


/*!
    Open  a USB device.  This will really open any USB device.
    \param usbDeviceName    : char* [in]
       Name of device special usb file to open.  Note that
       this should be something returned in a
       SIS3150USB_Device_Struct from FindAll_SIS3150USB_Devices.
    \param usbDeviceHandle : HANDLE* [out]
       This will be filled in with the handle from the open.
       This handle should be used in future operations to manipulate
       the device.
   
     \return int
     \retval SUCCESS   - If this was successful.
     \retval  other    - Failed, usually this will be -errno.

     \note  Since the usb_open call on linux expects a usb_device struct,
            we need to enumerate the bus, hunt for a matching filename and
            open based on the pDeviceStruct in the enumeration.

     \note  The interface will have been claimed on behalf of the caller's
            process, close functions will release this claim.

*/
int
Sis3150usb_OpenDriver (PCHAR usbDeviceName, HANDLE *usbDeviceHandle )
{
  int                       status;
  struct usb_device*        deviceHardwareHandle;



  deviceHardwareHandle = getDeviceHandle(usbDeviceName);
  if (!deviceHardwareHandle) {
    return -ENODEV;
  }



  /*   Now attempt to open the device, i should index to the found device */

  *usbDeviceHandle = usb_open(deviceHardwareHandle);
  if (!usbDeviceHandle) {
    return -ENODEV;
  }

  status = usb_claim_interface(*usbDeviceHandle, 0);
  if (status < 0) {
    usb_close(*usbDeviceHandle);
    return status;
  }

  status =   usb_set_altinterface(*usbDeviceHandle, 1);
  if (status < 0) {
    usb_close(*usbDeviceHandle);
    return status;
  }
  usleep(5000);


  return 0;
}
/*!
    Open a device and download the firmware...well actually
    we do this in the reverse order...download the firmware
    and then open the device.  
    - We need to assume the firmware is in a particular location.
      For now assume that the env. variable SIS3150
      Points to the top of an installation directory and that the file
      $SIS3150/etc/setup_8051.hex contains the firmware file.
    - We need to make an assumption about which directory
      holds the usb special devices .. /proc/bus/usb 
    - We assume the program fxload is in the path.
      Any errors from that program will be passed back to the caller,
      however the output from that program and error from that program will
      get sent to /dev/null.
    - We will determine if the firmware file designated by the user actually
      exists.
   
    \param usbDeviceName : PCHAR
       Pointer to a null terminated character string that contains the
       USB device to open/load.  We assume that all usb devices are in
       /proc/bus/usb .. which may need to be modified for other
       operating systems.
    \param usbDeviceHande : HANDLE*
       Pointer to where the user wants the open handle to be stored.

    \return int
    \retval as for the plain old open file.

     \note - On linux we strongly discourage the use of this function.
             the hotplug system will load the device firmware when
             the device is discovered. If the device gets hung up,
             simply unplugging and replugging the USB cable will cause
	     rediscovery and re-download.
*/
int
Sis3150usb_OpenDriver_And_Download_FX2_Setup(PCHAR usbDeviceName, 
					     HANDLE *usbDeviceHandle)
{
  char*  sis3150RootDir;	/* Where the software lives. */
  char   firmwareFilename[PATH_MAX+1];
  char   usbDeviceFile[PATH_MAX+1];
  int    testFd;
  struct stat info;
  struct usb_device* hardwareHandle;
  struct usb_dev_handle* loadHandle;
  int             status;
  
  /* Figure out where the firmware file is supposed to live */

  strcpy(sis3150RootDir,INSTDIR);
  strncpy(firmwareFilename, sis3150RootDir, PATH_MAX);
  strncat(firmwareFilename, "/etc/", PATH_MAX);
  strncat(firmwareFilename, firmwareName, PATH_MAX);

  /* Figure out if the firmware file exists and is readable */

  testFd = open(firmwareFilename, O_RDONLY);
  if (testFd < 0) {
    return -errno;
  }
  close(testFd);

  /* Create the full name of the USB device..               */


  strcpy(usbDeviceFile, usbDeviceDir);
  strncat(usbDeviceFile, usbDeviceName, PATH_MAX);

  /* See if the USB special file exists.                   */

  
  hardwareHandle = getDeviceHandle(usbDeviceName);
  if (!hardwareHandle) {
    return -ENODEV;
  }

  /* Use the packet radio functions to load the firmware now: */

  loadHandle = ccusrp_open_cmd_interface(hardwareHandle);
  if (!loadHandle) {
    return -ENODEV;
  }
  status = ccusrp_load_firmware(loadHandle, firmwareFilename);
  ccusrp_close_interface(loadHandle);
  if (!status) {
    return -EIO;
  }

  /*  Return a new open device for the user to use.           */
  
  return Sis3150usb_OpenDriver(usbDeviceName, usbDeviceHandle);
}
/*!
   Close the channel open on an SIS 3150USB device.  Once the
   device is closed, no further transactions can be performed on it
   without re-opening it.

   \param usbDevice  : HANDLE
        The handle that was open on the USB device.
   \return int
   \retval 0 normal
   \retval other close failed for some reason (perhaps not open?).
*/
int
Sis3150usb_CloseDriver(HANDLE usbDevice)
{
  int status;
  status = usb_release_interface(usbDevice, 0);
  if (status) return status;

  status = usb_close(usbDevice);
  usleep(5000);
  return status;
}
       
/******************************************************************************/
/*                                                                            */
/*                             R E G I S T E R   S P A C E                    */
/*  Shamelessly stolen and ported from Tino's original Windows code           */
/*                                                                            */
/******************************************************************************/




/*********************************/
/*                               */
/*    Register Read Cycles       */
/*                               */
/*********************************/










static int sis3150_register_read(HANDLE hDevice, ULONG addr, ULONG* data, 
                      ULONG req_nof_lwords, ULONG* got_nof_lwords)
{
  int return_code ;
  unsigned int nBytes  = 0;
  unsigned int nLWords  = 0;
  unsigned int i ;

  BOOLEAN success;
  ULONG usb_wlength;
  ULONG usb_rlength;

  UINT uiData = 0;
  char* cUsbBuf_ptr;
  unsigned int* longUsbBuf_ptr;
  char cUsbBuf[0x100+USB_MAX_NOF_BYTES];
  char cInPacket[0x100 + USB_MAX_NOF_BYTES];

  cUsbBuf_ptr = (char*) data ;

  if(req_nof_lwords > USB_MAX_NOF_LWORDS) {
     return_code = sis3150usb_error_code_invalid_parameter ;
     return  return_code ;
  }

	cUsbBuf[0]  =   (char)  0x00 ;	   // header 7:0 	  ; :
	cUsbBuf[1]  =   (char)  0x10 ;	   // header 15:8 	   Bit0 = 11 : not Write
	cUsbBuf[2]  =   (char)  0xaa ;	   // header 23:16
	cUsbBuf[3]  =   (char)  0xaa ;	   // header 31:24
	
	cUsbBuf[4]  =   (char)  req_nof_lwords   ;       //length 7:0
	cUsbBuf[5]  =   (char) (req_nof_lwords >> 8);    //length 15:8
	cUsbBuf[6]  =   (char)  0x0 ;   
	cUsbBuf[7]  =   (char)  0x0 ;
         
	cUsbBuf[8]  =   (char)  addr   ;       //addr 7:0
	cUsbBuf[9]  =   (char) (addr >> 8);    //addr 15:8
	cUsbBuf[10] =   (char) (addr >> 16) ;  //addr 23:16 
	cUsbBuf[11] =   (char) (addr >> 24);   //addr 31:24

	usb_wlength = 12;

	usb_rlength = (req_nof_lwords * 4) ; // data: (req_nof_lwords * 4) Bytes; 
	usb_rlength = (usb_rlength + 0x1ff) & 0xfffffe00 ; // 512er Boundary 

	return_code = usbTransaction(hDevice,
				     cUsbBuf, usb_wlength,
				     cInPacket, sizeof(cInPacket));
	
	if (return_code == -1) {
	  return sis3150usb_error_code_usb_write_error;
	}
	if (return_code == -2) {
	  return sis3150usb_error_code_usb_read_error;
	}

	nBytes = return_code;	/* That's how usbTransaction works. */
	
	if(nBytes == 0) {
	  return_code = sis3150usb_error_code_usb_read_length_error;
	  return return_code;
	} 
	if (nBytes > 0) {
	  memcpy(data, cInPacket, nBytes);
	}
	nLWords =  (nBytes / 4) ;

		
	*got_nof_lwords = (ULONG) (nLWords )  ;


   return_code  = 0 ;
return return_code ;
}



int EXPORT sis3150Usb_Register_Single_Read(HANDLE usbDevice, ULONG addr, ULONG* data)
{
  int return_code ;
  ULONG req_nof_lwords, dma_got_nof_lwords;

  req_nof_lwords = 1 ;
  return_code = sis3150_register_read(usbDevice, addr, data, req_nof_lwords, &dma_got_nof_lwords) ;
  return return_code ;
}




int EXPORT sis3150Usb_Register_Dma_Read(HANDLE usbDevice, ULONG addr, ULONG* dmabufs, ULONG req_nof_data, ULONG* got_nof_data)
{
  int return_code ;
	int error ;
	unsigned int new_addr ; 
	unsigned int index_num_data ; 
	unsigned int rest_req_nof_lwords ; 
	unsigned int new_req_nof_lwords ; 
	ULONG new_got_nof_lwords ; 
	ULONG fifo_mode ; 


	fifo_mode = 0 ;
	error = 0x0 ;
	*got_nof_data = 0x0 ;
	if (req_nof_data == 0) return error ;

	new_addr = addr ;
	rest_req_nof_lwords = req_nof_data  ;
	index_num_data = 0x0 ;

	do {
		if (rest_req_nof_lwords >= USB_MAX_NOF_BYTES/4) {
			new_req_nof_lwords = USB_MAX_NOF_BYTES/4 ;
		}
		else {
			new_req_nof_lwords = rest_req_nof_lwords ;
		}
		
	  	error = sis3150_register_read(usbDevice, new_addr, &dmabufs[index_num_data], new_req_nof_lwords, &new_got_nof_lwords)  ;

		index_num_data = index_num_data + (new_got_nof_lwords) ;
		rest_req_nof_lwords = rest_req_nof_lwords - new_got_nof_lwords ; 

		if(!fifo_mode) new_addr = new_addr + (new_got_nof_lwords) ; 		
	} while ((error == 0) && (rest_req_nof_lwords>0)) ;
	*got_nof_data = index_num_data ;

  return_code = error ;
  return return_code ;
}


/*********************************/
/*                               */
/*    Register Write Cycles      */
/*                               */
/*********************************/














static int sis3150_register_write(HANDLE hDevice, ULONG addr, ULONG* data, 
				  ULONG req_nof_lwords, ULONG* got_nof_lwords)
{
  unsigned int i ;
  int return_code;
  int nBytes  = 0;
  int nLWords  = 0;
  
  ULONG usb_wlength;
  ULONG usb_rlength;
  UINT uiData = 0;
  char cUsbBuf[0x100 + USB_MAX_NOF_BYTES];
  char cInPacket[USB_MAX_NOF_BYTES];



  if(req_nof_lwords > USB_MAX_NOF_LWORDS) {
     return_code = sis3150usb_error_code_invalid_parameter ;
     return  return_code ;
  }


	cUsbBuf[0]  =   (char)  0x00 ;	   // header 7:0 	  ; :
	cUsbBuf[1]  =   (char)  0x18 ;	   // header 15:8 	   Bit0 = 11 :  Write
	cUsbBuf[2]  =   (char)  0xaa ;	   // header 23:16
	cUsbBuf[3]  =   (char)  0xaa ;	   // header 31:24
	
	cUsbBuf[4]  =   (char)  req_nof_lwords   ;       //length 7:0
	cUsbBuf[5]  =   (char) (req_nof_lwords >> 8);    //length 15:8
	cUsbBuf[6]  =   (char)  0x0 ;   
	cUsbBuf[7]  =   (char)  0x0 ;

         
	cUsbBuf[8]  =   (char)  addr   ;       //addr 7:0
	cUsbBuf[9]  =   (char) (addr >> 8);    //addr 15:8
	cUsbBuf[10] =   (char) (addr >> 16) ;  //addr 23:16 
	cUsbBuf[11] =   (char) (addr >> 24);   //addr 31:24

	for(i=0;i<req_nof_lwords;i++) {
		cUsbBuf[(i*4)+12] =   (char)  data[i]   ;       //data 7:0
		cUsbBuf[(i*4)+13] =   (char) (data[i] >> 8);    //data 15:8
		cUsbBuf[(i*4)+14] =   (char) (data[i] >> 16) ;  //data 23:16 
		cUsbBuf[(i*4)+15] =   (char) (data[i] >> 24);   //data 31:24
	}


	usb_wlength =  12 + (req_nof_lwords * 4) ; // Header: 4 Bytes; Length:4 Bytes; Addr: 4 Bytes; data: (req_nof_lwords * 4) Bytes; 
    //now send buffer to FX2

	return_code = usbTransaction(hDevice,
				     cUsbBuf, usb_wlength,
				     cInPacket, sizeof(cInPacket));

	if (return_code == -1) {
	  return sis3150usb_error_code_usb_write_error;
	}
	if (return_code == -2) {
	  return sis3150usb_error_code_usb_read_error;
	}

	nBytes = return_code;

	if(nBytes == 0) { // OK
		*got_nof_lwords = req_nof_lwords  ;
		return return_code;
		return_code = 0;
	}
	else {
		return_code = sis3150usb_error_code_usb_read_length_error;
	}
	


return return_code ;
}


int EXPORT sis3150Usb_Register_Single_Write(HANDLE usbDevice, ULONG addr, ULONG data)
{
  int return_code ;
  ULONG write_data ;
  ULONG req_nof_lwords, dma_put_nof_lwords;


  write_data = data   ;
  req_nof_lwords = 1 ;

  return_code = sis3150_register_write(usbDevice, addr, &write_data, req_nof_lwords, &dma_put_nof_lwords)  ;

  return return_code ;
}







int EXPORT sis3150Usb_Register_Dma_Write(HANDLE usbDevice, ULONG addr, 
                            ULONG* dmabufs, ULONG req_nof_data, ULONG* put_nof_data)
{
  unsigned int  fifo_mode ;

	int error ;
	unsigned int new_addr ; 
	unsigned int index_num_data ; 
	unsigned int rest_req_nof_lwords ; 
	unsigned int new_req_nof_lwords ; 
	unsigned int new_put_nof_lwords ; 

 	fifo_mode = 0 ;
	error = 0x0 ;
	*put_nof_data = 0x0 ;
	if (req_nof_data == 0) return error ;


	new_addr = addr ;
	rest_req_nof_lwords = req_nof_data  ;
	index_num_data = 0x0 ;

	do {
		if (rest_req_nof_lwords >= USB_MAX_NOF_BYTES/4) {
			new_req_nof_lwords = USB_MAX_NOF_BYTES/4 ;
		}
		else {
			new_req_nof_lwords = rest_req_nof_lwords ;
		}
	
  		error = sis3150_register_write(usbDevice, new_addr, 
					       &dmabufs[index_num_data], 
					       new_req_nof_lwords, 
					       (ULONG*)&new_put_nof_lwords)  ;

		index_num_data = index_num_data + (new_put_nof_lwords) ;
		rest_req_nof_lwords = rest_req_nof_lwords - new_put_nof_lwords ; 

		if(!fifo_mode) new_addr = new_addr + (new_put_nof_lwords) ; 		
	} while ((error == 0) && (rest_req_nof_lwords>0)) ;
	*put_nof_data = index_num_data ;

return error    ;
}

/******************************************************************************/
/*                                                                            */
/*                                 V M E                                      */
/* Shamelessly stolen/ported from Tino's original code.                       */
/*                                                                            */
/******************************************************************************/









/*********************************/
/*                               */
/*    VME Read Cycles            */
/*                               */
/*********************************/











static int sis3150_vmebus_read(HANDLE hDevice, ULONG addr, ULONG vme_am, ULONG vme_size, ULONG fifo_mode,
			       ULONG* data, ULONG req_nof_bytes, ULONG* got_nof_bytes)
{
  int return_code ;
  unsigned int nBytes  = 0;
  unsigned int nLWords  = 0;
  unsigned int i ;
  
  ULONG usb_wlength;
  ULONG usb_rlength;
  UINT uiData = 0;
  char cUsbBuf[0x100 + USB_MAX_NOF_BYTES];
  char cInPacket[0x100 + USB_MAX_NOF_BYTES];
  
  char cSize, cFifoMode;
  char* cUsbBuf_ptr;
  unsigned int* longUsbBuf_ptr;


  cSize = 0x2 ; // Default 4 Byte
  switch (vme_size) {
	case 1:	   // 1 Byte
   		cSize = 0x0 ; //  1 Byte
		break;
	case 2:	   // 2 Bytes
   		cSize = 0x1 ; //  2 Bytes
		break;
	case 4:	   // 4 Bytes
   		cSize = 0x2 ; //  4 Bytes
		break;
	case 8:	   // 8 Bytes
   		cSize = 0x3 ; //  8 Bytes
		break;
  }

  if (fifo_mode == 0) {
	 cFifoMode  = 0x0 ;
  }
   else {
	 cFifoMode  = 0x4 ;
  } 

  cUsbBuf_ptr = (char*) data ;


  if(req_nof_bytes > USB_MAX_NOF_BYTES) {
     return_code = sis3150usb_error_code_invalid_parameter ;
     RETURN(return_code );
  }


	cUsbBuf[0]  =   (char)  0x00 ;	                 	// header 7:0 	  ; :
	cUsbBuf[1]  =   (char)  (0x40 + cSize + cFifoMode);	// header 15:8 	   Bit0 = 11 : not Write	/ D32
	cUsbBuf[2]  =   (char)  0xaa ;	           			// header 23:16
	cUsbBuf[3]  =   (char)  0xaa ;	           			// header 31:24
	
	cUsbBuf[4]  =   (char)  req_nof_bytes   ;       //length 7:0
	cUsbBuf[5]  =   (char) (req_nof_bytes >> 8);    //length 15:8
	cUsbBuf[6]  =   (char)  vme_am ;   
	cUsbBuf[7]  =   (char) (vme_am >> 8);
        
	cUsbBuf[8]  =   (char)  addr   ;       //addr 7:0
	cUsbBuf[9]  =   (char) (addr >> 8);    //addr 15:8
	cUsbBuf[10] =   (char) (addr >> 16) ;  //addr 23:16 
	cUsbBuf[11] =   (char) (addr >> 24);   //addr 31:24


	usb_wlength = 12;
	usb_rlength = (req_nof_bytes) ; // data: (req_nof_lwords * 4) Bytes; 
	usb_rlength = (usb_rlength + 0x1ff) & 0xffffe00; // 512 byte boundary.


	return_code = usbTransaction(hDevice,
				     cUsbBuf, usb_wlength,
				     cInPacket, usb_rlength);
	if (return_code == -1) {
	  RETURN(sis3150usb_error_code_usb_write_error);
	}
	if (return_code == -2) {
	  RETURN(sis3150usb_error_code_usb_read_error);
	}

	nBytes = return_code;
	*got_nof_bytes = (ULONG) (nBytes )  ;

	if(nBytes != req_nof_bytes) {
	  return_code = sis3150usb_error_code_usb_read_length_error; 
	  RETURN(return_code);
	}

	if(nBytes > 0) {
	  memcpy(cUsbBuf_ptr, cInPacket, nBytes);	/* Clibs usually have good memcpy */
	}

	return_code  = 0 ;
	return return_code ;
}




int EXPORT sis3150Usb_Vme_Single_Read(HANDLE usbDevice, ULONG addr, ULONG am, ULONG size, ULONG* data)
{
  int return_code ;
  int ret_code ;
  ULONG req_nof_bytes, dma_got_no_of_bytes;
  ULONG reg_data, got_length ;

  if (size == 1) {
    req_nof_bytes = 2 ;
  }
  else {
  	req_nof_bytes = size ;
  }
  return_code = sis3150_vmebus_read(usbDevice, addr, am, size, 0, data, req_nof_bytes, &dma_got_no_of_bytes)  ;
  if (size == 1)  {
	if ((addr & 0x1) == 0x0) {
        data[0] =   data[0] >> 8	;
    }
  }
  
  if (return_code == 0x0) {
  	return return_code ;
  }
  if (return_code == sis3150usb_error_code_usb_read_length_error) {
	 ret_code = sis3150_register_read(usbDevice, 0x11, &reg_data, 1, &got_length)  ;
	 return_code = ((reg_data >> 16) & 0xffff) ;
  }
  return return_code ;
}




int EXPORT sis3150Usb_Vme_Dma_Read(HANDLE usbDevice, ULONG addr, ULONG am, 
				   ULONG size, ULONG fifo_mode,
                            ULONG* dmabufs, ULONG req_nof_data, ULONG* got_nof_data)
{
// size 2,4,8 sind nur erlaubt (BLT16,BLT32,MBLT64)
  int return_code ;
  int ret_code ;
  ULONG reg_data, got_length ;

	int error ;
	unsigned int new_addr ; 
	unsigned int index_num_data ; 
	unsigned int rest_req_nof_bytes ; 
	unsigned int new_req_nof_bytes ; 
	ULONG new_got_nof_bytes ; 


	error = 0x0 ;
	*got_nof_data = 0x0 ;
	if (req_nof_data == 0) return error ;

	new_addr = addr ;
	rest_req_nof_bytes = req_nof_data * 4 ;
	index_num_data = 0x0 ;

	do {
		if (rest_req_nof_bytes >= USB_MAX_NOF_BYTES) {
			new_req_nof_bytes = USB_MAX_NOF_BYTES ;
		}
		else {
			new_req_nof_bytes = rest_req_nof_bytes ;
		}
		
	  	error = sis3150_vmebus_read(usbDevice, new_addr, am, size, fifo_mode,
	  	                            &dmabufs[index_num_data], new_req_nof_bytes, &new_got_nof_bytes)  ;
		

		index_num_data = index_num_data + (new_got_nof_bytes >> 2) ;
		rest_req_nof_bytes = rest_req_nof_bytes - new_got_nof_bytes ; 

		if(!fifo_mode) new_addr = new_addr + (new_got_nof_bytes ) ; 		
	} while ((error == 0) && (rest_req_nof_bytes>0)) ;
	*got_nof_data = index_num_data ;

  return_code = error ;
  if (return_code == 0x0) {
  	return return_code ;
  }
  if (return_code == sis3150usb_error_code_usb_read_length_error) {
	 ret_code = sis3150_register_read(usbDevice, 0x11, &reg_data, 1, &got_length)  ;
	 return_code = ((reg_data >> 16) & 0xffff) ;
  }
  return return_code ;



	
return error    ;
}



/*********************************/
/*                               */
/*    VME Write Cycles           */
/*                               */
/*********************************/






















static int sis3150_vmebus_write(HANDLE hDevice, ULONG addr, ULONG vme_am, ULONG vme_size, ULONG fifo_mode,
				ULONG* data, ULONG req_nof_bytes, ULONG* put_nof_bytes)
{
  int return_code ;
  int nBytes  = 0;
  int nLWords  = 0;
  
  BOOLEAN success;
  ULONG usb_wlength;
  ULONG usb_rlength;
  UINT uiData = 0;
  ULONG data_byte3, data_byte2, data_byte1, data_byte0 ;
  ULONG data_copy_nof_words ;
  char cUsbBuf[0x100 + USB_MAX_NOF_BYTES];
  char cInPacket[0x100 + USB_MAX_NOF_BYTES];


  unsigned int  i;

  char cSize, cFifoMode;
  long* long_cUsbBuf_ptr;

  cSize = 0x2 ; // Default 4 Byte
  data_copy_nof_words  =  (req_nof_bytes>>2 ) ; 
  switch (vme_size) {
	case 1:	   // 1 Byte
        data_copy_nof_words  =  (req_nof_bytes>>1 ) ; 
   		cSize = 0x0 ; //  1 Byte
		break;
	case 2:	   // 2 Bytes
        data_copy_nof_words  =  (req_nof_bytes>>1 ) ; 
   		cSize = 0x1 ; //  2 Bytes
		break;
	case 4:	   // 4 Bytes
   		cSize = 0x2 ; //  4 Bytes
		break;
	case 8:	   // 8 Bytes
   		cSize = 0x3 ; //  8 Bytes
		break;
  }

  if (fifo_mode == 0) {
	 cFifoMode  = 0x0 ;
  }
   else {
	 cFifoMode  = 0x4 ;
  } 


  if(req_nof_bytes > USB_MAX_NOF_BYTES) {
     return_code = sis3150usb_error_code_invalid_parameter ;
     RETURN(return_code) ;
  }


	cUsbBuf[0]  =   (char)  0x00 ;	                 	// header 7:0 	  ; :
	cUsbBuf[1]  =   (char)  (0x48 + cSize + cFifoMode);	// header 15:8 	   Bit11 = 1 : Write 
	cUsbBuf[2]  =   (char)  0xaa ;	           			// header 23:16
	cUsbBuf[3]  =   (char)  0xaa ;	           			// header 31:24
	
	cUsbBuf[4]  =   (char)  req_nof_bytes   ;       //length 7:0
	cUsbBuf[5]  =   (char) (req_nof_bytes >> 8);    //length 15:8
	cUsbBuf[6]  =   (char)  vme_am ;   
	cUsbBuf[7]  =   (char) (vme_am >> 8);
				
         
	cUsbBuf[8]  =   (char)  addr   ;       //addr 7:0
	cUsbBuf[9]  =   (char) (addr >> 8);    //addr 15:8
	cUsbBuf[10] =   (char) (addr >> 16) ;  //addr 23:16 
	cUsbBuf[11] =   (char) (addr >> 24);   //addr 31:24


    long_cUsbBuf_ptr =  (long*) (cUsbBuf + 12);
	for(i=0;i<data_copy_nof_words;i++) {
		*long_cUsbBuf_ptr++ =   data[i] ;
	//	long_cUsbBuf_ptr++;
	}

/*	for(i=0;i<(req_nof_bytes>>2);i++) {
		cUsbBuf[(i*4)+12] =   (char)  data[i]   ;       //data 7:0
		cUsbBuf[(i*4)+13] =   (char) (data[i] >> 8);    //data 15:8
		cUsbBuf[(i*4)+14] =   (char) (data[i] >> 16) ;  //data 23:16 
		cUsbBuf[(i*4)+15] =   (char) (data[i] >> 24);   //data 31:24
	}
*/

	usb_wlength =  12 + (req_nof_bytes) ; // Header: 4 Bytes; Length:4 Bytes; Addr: 4 Bytes; data: (req_nof_lwords * 4) Bytes; 
	usb_rlength = 4  ; // in case of error : Confirmation  
	usb_rlength = (usb_rlength + 0x1ff) & 0xfffffe00 ; // 512er Boundary 

	return_code = usbTransaction(hDevice, cUsbBuf, usb_wlength,
				     cInPacket, usb_rlength);

	if (return_code == -1) {
	  RETURN(sis3150usb_error_code_usb_write_error);
	}
	if (return_code == -2) {
	  RETURN(sis3150usb_error_code_usb_read_error);
	}

	nBytes      = return_code;
	return_code = 0;

	if(nBytes == 0) { // OK
		*put_nof_bytes = req_nof_bytes  ;
		return_code = 0;
		return return_code;
	}

	if(nBytes == 4) { // VME Error
		data_byte0 =  (ULONG) (cInPacket[0] & 0xff) ;
		data_byte1 =  (ULONG) (cInPacket[1] & 0xff) ;
		data_byte2 =  (ULONG) (cInPacket[2] & 0xff) ;
		data_byte3 =  (ULONG) (cInPacket[3] & 0xff) ;
		*put_nof_bytes =   (data_byte1 << 8)  + (data_byte0 )   ;
		return_code    =   (data_byte3 << 8)  + (data_byte2 ) ;
		RETURN(return_code);
	}

	return_code = sis3150usb_error_code_usb_read_length_error;
	

RETURN(return_code) ;
}






int EXPORT sis3150Usb_Vme_Single_Write(HANDLE usbDevice, ULONG addr, ULONG am, ULONG size, ULONG data)
{
  int return_code ;
  ULONG write_data ;
  ULONG req_nof_bytes, dma_put_nof_bytes;


  write_data = data   ;
  req_nof_bytes = 0x4 ;
  switch (size) {
	case 1:	   // 1 Byte
		write_data = data & 0xff ;
		write_data = write_data + (write_data << 8) + (write_data << 16) + (write_data << 24)  ;
  		req_nof_bytes = 0x2 ;
		break;
	case 2:	   // 2 Bytes
		write_data = data & 0xffff ;
		write_data = write_data +  (write_data << 16)   ;
  		req_nof_bytes = 0x2 ;
		break;
  }

  return_code = sis3150_vmebus_write(usbDevice, addr, am, size, 0, &write_data, req_nof_bytes, &dma_put_nof_bytes)  ;


  return return_code ;
}








int EXPORT sis3150Usb_Vme_Dma_Write(HANDLE usbDevice, ULONG addr, ULONG am, ULONG size, ULONG fifo_mode,
                            ULONG* dmabufs, ULONG req_nof_data, ULONG* put_nof_data)
{
// size 2,4,8 sind nur erlaubt (BLT16,BLT32,MBLT64)

	int error ;
	unsigned int new_addr ; 
	unsigned int index_num_data ; 
	unsigned int rest_req_nof_bytes ; 
	unsigned int new_req_nof_bytes ; 
	ULONG new_put_nof_bytes ; 


	error = 0x0 ;
	*put_nof_data = 0x0 ;
	if (req_nof_data == 0) return error ;

	new_addr = addr ;
	rest_req_nof_bytes = req_nof_data * 4 ;
	index_num_data = 0x0 ;

	do {
		if (rest_req_nof_bytes >= USB_MAX_NOF_BYTES) {
			new_req_nof_bytes = USB_MAX_NOF_BYTES ;
		}
		else {
			new_req_nof_bytes = rest_req_nof_bytes ;
		}
		
	  	error = sis3150_vmebus_write(usbDevice, new_addr, am, size, fifo_mode,
	  	                            &dmabufs[index_num_data], new_req_nof_bytes, &new_put_nof_bytes)  ;
		



		index_num_data = index_num_data + (new_put_nof_bytes >> 2) ;

		rest_req_nof_bytes = rest_req_nof_bytes - new_put_nof_bytes ; 

		if(!fifo_mode) new_addr = new_addr + (new_put_nof_bytes ) ; 		
	} while ((error == 0) && (rest_req_nof_bytes>0)) ;
	*put_nof_data = index_num_data ;

	
return error    ;
}






/******************************************************************************
 *
 * Function   :  sis1100w_VmeSysreset
 *
 *****************************************************************************/

int EXPORT sis3150Usb_VmeSysreset(HANDLE usbDevice)
{
  ULONG wdata ;
  ULONG nof_lwords ;


  wdata = 0x2 ; 
  sis3150_register_write(usbDevice, 0x10, &wdata, 1, &nof_lwords)   ;   // set VME Reset
  usleep(300000) ; /* min. 200ms */
  wdata = 0x20000 ; 
  sis3150_register_write(usbDevice, 0x10, &wdata, 1, &nof_lwords)   ;   // clear  VME Reset
  return 0 ;
}





/******************************************************************************/
/*                                                                            */
/*                             T S   B U S   S P A C E                        */
/*                                                                            */
/*   Shamelessly stolen and ported from Tino's code... also untested as my    */
/*   test board is not populated with any DSP chips (RF).                     */
/******************************************************************************/




/*********************************/
/*                               */
/*    TS BUS Read Cycles         */
/*                               */
/*********************************/










int sis3150_tsbus_read(HANDLE hDevice, ULONG addr, ULONG* data, 
                      ULONG req_nof_lwords, ULONG* got_nof_lwords)
{
  int return_code ;
  unsigned int nBytes  = 0;
  unsigned int nLWords  = 0;
  unsigned int i ;
  
  BOOLEAN success;

  ULONG usb_wlength;
  ULONG usb_rlength;

  UINT uiData = 0;
  char cUsbBuf[0x100+USB_MAX_NOF_BYTES];
  char cInPacket[0x100+USB_MAX_NOF_BYTES];
  
  char* cUsbBuf_ptr;
  unsigned int* longUsbBuf_ptr;

  cUsbBuf_ptr = (char*) data ;

  if(req_nof_lwords > USB_MAX_NOF_LWORDS) {
     return_code = sis3150usb_error_code_invalid_parameter ;
     return  return_code ;
  }


	cUsbBuf[0]  =   (char)  0x00 ;	   // header 7:0 	  ; :
	cUsbBuf[1]  =   (char)  0x20 ;	   // header 15:8 	   Bit0 = 11 : not Write
	cUsbBuf[2]  =   (char)  0xaa ;	   // header 23:16
	cUsbBuf[3]  =   (char)  0xaa ;	   // header 31:24
	
	cUsbBuf[4]  =   (char)  req_nof_lwords   ;       //length 7:0
	cUsbBuf[5]  =   (char) (req_nof_lwords >> 8);    //length 15:8
	cUsbBuf[6]  =   (char)  0x0 ;   
	cUsbBuf[7]  =   (char)  0x0 ;

         
	cUsbBuf[8]  =   (char)  addr   ;       //addr 7:0
	cUsbBuf[9]  =   (char) (addr >> 8);    //addr 15:8
	cUsbBuf[10] =   (char) (addr >> 16) ;  //addr 23:16 
	cUsbBuf[11] =   (char) (addr >> 24);   //addr 31:24

	usb_wlength = 12;
	usb_rlength = (req_nof_lwords * 4) ; // data: (req_nof_lwords * 4) Bytes; 
	usb_rlength = (usb_rlength + 0x1ff) & 0xfffffe00 ; // 512er Boundary 

	return_code = usbTransaction(hDevice, cUsbBuf, usb_wlength,
				 cInPacket, usb_rlength);
	if (return_code == -1) {
	  return_code = sis3150usb_error_code_usb_write_error;
	  return return_code;
	}
	if (return_code == -2) {
	  return_code = sis3150usb_error_code_usb_read_error;
	  return return_code;
	}
	nBytes = return_code;	/* Read this many bytes.  */
	return_code  = 0 ;	/* Can't fail anymore.    */




	if(nBytes == 0) {       /* Allows partial transfers to succeed */
	  return_code = sis3150usb_error_code_usb_read_length_error;
	  return return_code;
	} 
	nLWords =  (nBytes / 4) ; 
	longUsbBuf_ptr = (unsigned int*) cUsbBuf ;
	if (nBytes > 0) {
	  memcpy(data, cInPacket, nBytes); /* Most Clibs optimize this well. */
	}





	*got_nof_lwords = (ULONG) (nLWords )  ;


	return return_code ;
}



int EXPORT sis3150Usb_TsBus_Single_Read(HANDLE usbDevice, ULONG addr, ULONG* data)
{
  int return_code ;
  ULONG req_nof_lwords, dma_got_nof_lwords;

  req_nof_lwords = 1 ;
  return_code = sis3150_tsbus_read(usbDevice, addr, data, req_nof_lwords, &dma_got_nof_lwords) ;
  return return_code ;
}




int EXPORT sis3150Usb_TsBus_Dma_Read(HANDLE usbDevice, ULONG addr, ULONG* dmabufs, ULONG req_nof_data, ULONG* got_nof_data)
{
  int return_code ;
	int error ;
	unsigned int new_addr ; 
	unsigned int index_num_data ; 
	unsigned int rest_req_nof_lwords ; 
	unsigned int new_req_nof_lwords ; 
	ULONG new_got_nof_lwords ; 
	ULONG fifo_mode ; 


	fifo_mode = 0 ;
	error = 0x0 ;
	*got_nof_data = 0x0 ;
	if (req_nof_data == 0) return error ;

	new_addr = addr ;
	rest_req_nof_lwords = req_nof_data  ;
	index_num_data = 0x0 ;

	do {
		if (rest_req_nof_lwords >= USB_MAX_NOF_BYTES/4) {
			new_req_nof_lwords = USB_MAX_NOF_BYTES/4 ;
		}
		else {
			new_req_nof_lwords = rest_req_nof_lwords ;
		}
		
	  	error = sis3150_tsbus_read(usbDevice, new_addr, &dmabufs[index_num_data], new_req_nof_lwords, &new_got_nof_lwords)  ;

		index_num_data = index_num_data + (new_got_nof_lwords) ;
		rest_req_nof_lwords = rest_req_nof_lwords - new_got_nof_lwords ; 

		if(!fifo_mode) new_addr = new_addr + (new_got_nof_lwords) ; 		
	} while ((error == 0) && (rest_req_nof_lwords>0)) ;
	*got_nof_data = index_num_data ;

  return_code = error ;
  return return_code ;
}


















/*********************************/
/*                               */
/*    TS BUS  Write Cycles       */
/*                               */
/*********************************/














int sis3150_tsbus_write(HANDLE hDevice, ULONG addr, ULONG* data, 
                      ULONG req_nof_lwords, ULONG* got_nof_lwords)
{
  unsigned int i ;
  int return_code;
  int nBytes  = 0;
  int nLWords  = 0;
  
  BOOLEAN success;
 
  ULONG usb_wlength;
  ULONG usb_rlength;
 
  UINT uiData = 0;
  
  char cUsbBuf[0x100+USB_MAX_NOF_BYTES];
  char cInPacket[0x100+USB_MAX_NOF_BYTES];


  if(req_nof_lwords > USB_MAX_NOF_LWORDS) {
     return_code = sis3150usb_error_code_invalid_parameter ;
     return  return_code ;
  }


  cUsbBuf[0]  =   (char)  0x00 ;	   // header 7:0 	  ; :
  cUsbBuf[1]  =   (char)  0x28 ;	   // header 15:8 	   Bit0 = 11 :  Write
  cUsbBuf[2]  =   (char)  0xaa ;	   // header 23:16
  cUsbBuf[3]  =   (char)  0xaa ;	   // header 31:24
  
  cUsbBuf[4]  =   (char)  req_nof_lwords   ;       //length 7:0
  cUsbBuf[5]  =   (char) (req_nof_lwords >> 8);    //length 15:8
  cUsbBuf[6]  =   (char)  0x0 ;   
  cUsbBuf[7]  =   (char)  0x0 ;
  
  
  cUsbBuf[8]  =   (char)  addr   ;       //addr 7:0
  cUsbBuf[9]  =   (char) (addr >> 8);    //addr 15:8
  cUsbBuf[10] =   (char) (addr >> 16) ;  //addr 23:16 
  cUsbBuf[11] =   (char) (addr >> 24);   //addr 31:24
  
  for(i=0;i<req_nof_lwords;i++) {
    cUsbBuf[(i*4)+12] =   (char)  data[i]   ;       //data 7:0
    cUsbBuf[(i*4)+13] =   (char) (data[i] >> 8);    //data 15:8
    cUsbBuf[(i*4)+14] =   (char) (data[i] >> 16) ;  //data 23:16 
    cUsbBuf[(i*4)+15] =   (char) (data[i] >> 24);   //data 31:24
  }
  
  
  usb_wlength =  12 + (req_nof_lwords * 4) ; // Header: 4 Bytes; Length:4 Bytes; Addr: 4 Bytes; data: (req_nof_lwords * 4) Bytes; 
  usb_rlength = 4  ; // in case of error : Confirmation  
  usb_rlength = (usb_rlength + 0x1ff) & 0xfffffe00 ; // 512er Boundary 

  return_code = usbTransaction(hDevice,
			       cUsbBuf,      usb_wlength,
			       cInPacket, usb_rlength);


  if (return_code == -1) {
    return_code = sis3150usb_error_code_usb_write_error;
    return return_code;
  }
  if (return_code == -2) {
    return_code = sis3150usb_error_code_usb_read_error;
    return return_code;
  }
  nBytes      = return_code;	/* adapt to rest of Ti */
  return_code = 0;

  
  if(nBytes == 0) { // OK
    *got_nof_lwords = req_nof_lwords  ;
    return return_code;
    return_code = 0;
  }
  else {
    return_code = sis3150usb_error_code_usb_read_length_error;
  }
  
  
  
  return return_code ;
}


int EXPORT sis3150Usb_TsBus_Single_Write(HANDLE usbDevice, ULONG addr, ULONG data)
{
  int return_code ;
  ULONG write_data ;
  ULONG req_nof_lwords, dma_put_nof_lwords;


  write_data = data   ;
  req_nof_lwords = 1 ;

  return_code = sis3150_tsbus_write(usbDevice, addr, &write_data, req_nof_lwords, &dma_put_nof_lwords)  ;

  return return_code ;
}







int EXPORT sis3150Usb_TsBus_Dma_Write(HANDLE usbDevice, ULONG addr, 
                            ULONG* dmabufs, ULONG req_nof_data, ULONG* put_nof_data)
{
  unsigned int  fifo_mode ;

	int error ;
	unsigned int new_addr ; 
	unsigned int index_num_data ; 
	unsigned int rest_req_nof_lwords ; 
	unsigned int new_req_nof_lwords ; 
	ULONG         new_put_nof_lwords ; 

 	fifo_mode = 0 ;
	error = 0x0 ;
	*put_nof_data = 0x0 ;
	if (req_nof_data == 0) return error ;


	new_addr = addr ;
	rest_req_nof_lwords = req_nof_data  ;
	index_num_data = 0x0 ;

	do {
		if (rest_req_nof_lwords >= USB_MAX_NOF_BYTES/4) {
			new_req_nof_lwords = USB_MAX_NOF_BYTES/4 ;
		}
		else {
			new_req_nof_lwords = rest_req_nof_lwords ;
		}
	
  		error = sis3150_tsbus_write(usbDevice, new_addr, &dmabufs[index_num_data], new_req_nof_lwords, &new_put_nof_lwords)  ;

		index_num_data = index_num_data + (new_put_nof_lwords) ;
		rest_req_nof_lwords = rest_req_nof_lwords - new_put_nof_lwords ; 

		if(!fifo_mode) new_addr = new_addr + (new_put_nof_lwords) ; 		
	} while ((error == 0) && (rest_req_nof_lwords>0)) ;
	*put_nof_data = index_num_data ;

return error    ;
}




