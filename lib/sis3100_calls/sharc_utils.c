/*===========================================================================*/
/*                                                                           */
/* File:             sharc_utils.c                                           */
/*                                                                           */
/* OS:               LINUX (Kernel >= 2.4.4                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Version:          1.0                                                     */
/*                                                                           */
/*                                                                           */
/* Generated:        18.12.01                                                */
/*                                                                           */
/* Author:           TH                                                      */
/*                                                                           */
/* Last Change:                       Installation                           */
/*---------------------------------------------------------------------------*/
/* SIS GmbH                                                                  */
/* Harksheider Str. 102A                                                     */
/* 22399 Hamburg                                                             */
/*                                                                           */
/* http://www.struck.de                                                      */
/*                                                                           */
/*===========================================================================*/

#define _GNU_SOURCE

/*===========================================================================*/
/* Headers								     */
/*===========================================================================*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

/* sis1100/3100 PCI to VME specific */   
#include "dev/pci/sis1100_var.h"  /* pfad im Makefile angeben */

#include "sis3100_vme_calls.h"

/* SIS9200 DSP */
#include "header/sis9200.h"
    


/*===========================================================================*/
/* Prototypes					  			     */
/*===========================================================================*/


#include "sharc_utils.h"


/*===========================================================================*/
/* Load DSP     					  		     */
/*===========================================================================*/

int load_dsp(int p, int p_sharc, char* dsppath){
    int retcode=1;
    int count=0,loadcount=0;
    int offset;
    int currentaddress ;
    char line_in[128];
    FILE *loaderfile;
    unsigned int tempword[0x10000];
    unsigned int read_tempword[0x10000];
    u_int32_t data ;
    u_int32_t addr ;

  /* Enable Optical Control of SHARC */
    offset =  0x00000300 ;
    if ((retcode = s3100_control_write(p, offset, 0x00000800)) != 0) {  
	printf("s3100_control_write:   retcode = 0x%08x\n", retcode ); 
        return -1;
    } 

  /* set SHARC in Reset state */
    offset =  0x00000300 ;
    if ((retcode = s3100_control_write(p, offset, 0x01000000)) != 0) {  
	printf("s3100_control_write:   retcode = 0x%08x\n", retcode ); 
        return -1;
    } 




    loaderfile=fopen(dsppath,"r");
    retcode = 1 ;
    if (loaderfile>0) {
       printf("loader file %s opened\n",dsppath);
       while (retcode>0) {
          tempword[count]= strtoul(line_in,NULL,16); 
	  retcode=fscanf(loaderfile,"0x%4x\n",&tempword[count]); 
          if (count<0x10000) {
             count++;
	  }
          else {
	     printf("load file size too big\n");
             return -1;
	  }
      }
      printf("load file length: %d\n",count);
    
    }
    else {
      printf("loader file %s not found\n",dsppath);
      return -1;
    }
    fclose(loaderfile);

    printf("loading SHARC DSP\n");

    currentaddress=SHARCRAM;
	   printf("currentaddress = 0x%08x\n", currentaddress ); 



    loadcount=0 ;
    while (loadcount<count) {  
       addr = D48REG;
       data = tempword[loadcount];

       if ((retcode = s3100_sharc_write(p_sharc, addr, &data, 0x1))!= 4) { 
	   printf("s3100_sharc_write:   retcode = 0x%08x\n", retcode ); 
           return -1;
       }
       loadcount++;

       addr = currentaddress;
       data = ((tempword[loadcount+1] << 16 ) & 0xFFFF0000) + (tempword[loadcount] & 0x0000FFFF);

       if ((retcode = s3100_sharc_write(p_sharc, addr, &data, 0x1)) != 4) {
          printf("s3100_sharc_write:   retcode = 0x%08x\n", retcode );
          return -1;
       }         
       currentaddress+=4;
       loadcount+=2;
    }

    /* read */
    printf("readback from DSP\n");
    currentaddress=SHARCRAM;
    loadcount=0 ;
    while (loadcount<count) {  

       addr = currentaddress;
       if ((retcode = s3100_sharc_read(p_sharc, addr, &data, 0x1))!= 4) { 
	   printf("s3100_sharc_read:   retcode = 0x%08x\n", retcode ); 
           return -1;
       }

       read_tempword[loadcount+1] = (data) & 0x0000FFFF ;
       read_tempword[loadcount+2] = (data >> 16) & 0x0000FFFF ;

       addr = D48REG;
       if ((retcode = s3100_sharc_read(p_sharc, addr, &data, 0x1))!= 4) { 
	   printf("D48REG s3100_sharc_read:   retcode = 0x%08x\n", retcode ); 
           return -1;
       }
       read_tempword[loadcount] = (data) & 0x0000FFFF ;
       currentaddress+=4;
       loadcount+=3;
    }

    /* verifier */
    loadcount=0 ;
    while (loadcount<count) {  

     if (read_tempword[loadcount] != tempword[loadcount]) {
	   printf("Verifier ERROR     i = 0x%08x    written = 0x%08x     read = 0x%08x    \n",
                                           loadcount, tempword[loadcount], read_tempword[loadcount] ); 
         return -1;
       }
       loadcount++;
    }


    /* start SHARC */
    printf("starting SHARC DSP\n");
    offset =  0x00000300 ;
     if ((retcode = s3100_control_write(p, offset, 0x0100)) != 0) {     
       printf("s3100_control_write:   retcode = 0x%08x\n", retcode );
       return -1;
    }         
 return 0 ;
}






/*===========================================================================*/
/* reset DSP   			     		                             */
/*===========================================================================*/
int reset_dsp(int p) {
    int retcode;
    int offset;
    /* Reset SHARC */
    printf("resetting SHARC DSP\n");
    offset =  0x00000300 ;
  /* Enable Optical Control of SHARC */
    if ((retcode = s3100_control_write(p, offset, 0x00000800)) != 0) {  
	printf("s3100_control_write:   retcode = 0x%08x\n", retcode ); 
        return -1;
    } 

  /* set SHARC in Reset state */
    if ((retcode = s3100_control_write(p, offset, 0x01000000)) != 0) {
	printf("s3100_control_write:   retcode = 0x%08x\n", retcode ); 
        return -1;
    } 
   return 0;
}




























