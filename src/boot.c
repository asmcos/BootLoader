/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Bootloader entry point
 * ----------------------------------------------------------------------------
 */

 #include <stdio.h>
 #include <string.h>
 #include <stm32f4xx_hal.h>


#define FLASH_START                      0x8000000
#define RAM_START                       0x20000000
#define BOOTLOADER_SIZE                 16384       // 0x4000 ,APP from 0x8004000
#define FLASH_Sector_0     ((uint16_t)0x0000) 
#define FLASH_Sector_1     ((uint16_t)0x0008)
#define VoltageRange_1        ((uint8_t)0x00)
#define VoltageRange_3        ((uint8_t)0x02)

#define BOOTLOADER_MAJOR_VERSION 3 // needed for Ext Erase in stm32loader.py
#define BOOTLOADER_MINOR_VERSION 0

#define CMD_GET        (0x00)
#define CMD_GET_ID     (0x02)
#define CMD_READ       (0x11)
#define CMD_GO         (0x21)
#define CMD_WRITE      (0x31)
#define CMD_EXTERASE   (0x44)

#define INIT_CHIP      (0x7F)

#define ACK            (0x79)
#define NACK           (0x1F)

int get_char(void)
{
	return fgetc(stdin);
}

void put_char(unsigned char c)
{
	fputc(c,stdout);
}

unsigned char get_char_blocking() {
  int c = get_char();
  while (c<0) c=get_char();
  return c;
}


int boot (void) 
{

  unsigned char currentCommand = 0;
  int count = -1;  
  int status = 0;

  while (count != 0) {
	   count --;
      // flash led
      int d = get_char();
		
      if (d != -1) { // if we have data
		  count  = -1 ; // count < 0
		  d = d & 0xff;	

		  if (status == 0)
		  {
			// first cmd
			currentCommand = d;
			status = 1;
			if (currentCommand == INIT_CHIP){
				put_char(ACK);
				status = 0;
			}
		  } else {
			status = 0;
		  }
		  

          if(currentCommand == (d ^ 0xff)){
            unsigned int addr,i;
            char chksum, chksumc, buffer[256];
            unsigned int nBytesMinusOne, nPages;
            // confirmed
			//second cmd = cmd ^ 0xff			

            switch (currentCommand) {
            case CMD_GET: // get bootloader info
			  
              put_char(ACK);
              put_char(6); // 
              // unow report what we support
              put_char(BOOTLOADER_MAJOR_VERSION<<4 | BOOTLOADER_MINOR_VERSION); // Bootloader version
              // list supported commands
              put_char(CMD_GET);         //1
              put_char(CMD_GET_ID);      //2
              put_char(CMD_READ);        //3
              put_char(CMD_GO);          //4
              put_char(CMD_WRITE);       //5
              put_char(CMD_EXTERASE); // erase  //6
              put_char(ACK); // last byte
              break;
            case CMD_GET_ID: // get chip ID
              put_char(ACK);
              put_char(1); // 2 bytes
              // 0x430 F1 XL density
              // 0x414 F1 high density
              // 0x6433 F401 CD

              put_char(0x04); put_char(0x13);  // f4

              put_char(ACK); // last byte
              break;
            case CMD_READ: // read memory
              put_char(ACK);
              addr = get_char_blocking() << 24;
              addr |= get_char_blocking()  << 16;
              addr |= get_char_blocking()  << 8;
              addr |= get_char_blocking();
              chksum = get_char_blocking();
              // TODO: check checksum
              put_char(ACK);
              nBytesMinusOne = get_char_blocking();
              chksum = get_char_blocking();
              // TODO: check checksum
              put_char(ACK);
              for (i=0;i<=nBytesMinusOne;i++)
                put_char(((unsigned char*)addr)[i]);
              break;
            case CMD_GO: // read memory
              put_char(ACK);
              addr = get_char_blocking() << 24;
              addr |= get_char_blocking()  << 16;
              addr |= get_char_blocking()  << 8;
              addr |= get_char_blocking();
              chksum = get_char_blocking();
              // TODO: check checksum
              put_char(ACK);
              unsigned int *ResetHandler = (unsigned int *)(addr + 4);
              void (*startPtr)() = *ResetHandler;
              startPtr();
              break;
            case CMD_WRITE: // write memory
              put_char(ACK);
              addr = get_char_blocking() << 24;
              addr |= get_char_blocking()  << 16;
              addr |= get_char_blocking()  << 8;
              addr |= get_char_blocking();
              chksumc = ((addr)&0xFF)^((addr>>8)&0xFF)^((addr>>16)&0xFF)^((addr>>24)&0xFF);
              chksum = get_char_blocking();
              if (chksumc != chksum) {
                put_char(NACK);
                break;
              }
              put_char(ACK);
              nBytesMinusOne = get_char_blocking();
              chksumc = nBytesMinusOne;
              for (i=0;i<=nBytesMinusOne;i++) {
                buffer[i] = get_char_blocking();
                chksumc = chksumc^buffer[i];
              }
              chksum = get_char_blocking(); // FIXME found to be stalled here
              if (chksumc != chksum || (nBytesMinusOne+1)&3!=0) {
                put_char(NACK);
                break;
              }
              if (addr>=FLASH_START && addr<RAM_START) {

                for (i=0;i<=nBytesMinusOne;i+=4) {
                  unsigned int realaddr = addr+i;
                  if (realaddr >= (FLASH_START+BOOTLOADER_SIZE)) // protect bootloader
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,realaddr, *(unsigned int*)&buffer[i]);
                }
              } else { 
                // normal write
                for (i=0;i<=nBytesMinusOne;i+=4) {
                  unsigned int realaddr = addr+i;
                  *((unsigned int*)realaddr) = *(unsigned int*)&buffer[i];
                }
              }
                
              put_char(ACK); //  TODO - could speed up writes by ACKing beforehand if we have space
              break;
            case CMD_EXTERASE: // erase memory
              put_char(ACK);
              nPages = get_char_blocking() << 8;
              nPages |= get_char_blocking();
              chksum = get_char_blocking();
              // TODO: check checksum
              if (nPages == 0xFFFF) {
                // all pages (except us!)
                  for (i=1;i<8;i++) { // might as well do all of them
                    FLASH_Erase_Sector(FLASH_Sector_0 + (FLASH_Sector_1-FLASH_Sector_0)*i, VoltageRange_3); // a FLASH_Sector_## constant
                  }
                put_char(ACK);
              } else {
                put_char(NACK); // not implemented
              }
              break;
            default: // unknown command
              put_char(NACK);
              break;
            }
		  } else {
			 // not correct
			//currentCommand = d;	
            //put_char(NACK);
          }
        }
		
		
  } //while
} //boot

