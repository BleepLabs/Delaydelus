/*
Arduino S25FLx Serial Flash library
By John-Mike Reed (Dr. Bleep) of Bleep labs 


This free library is realeased under Creative comoms license CC BY-SA 3.0
http://creativecommons.org/licenses/by-sa/3.0/deed.en_US
*/


//Define S25FLx control bytes

#define WREN        0x06    /* Write Enable */
#define WRDI        0x04    /* Write Disable */ 
#define RDSR        0x05    /* Read Status Register */
#define WRSR        0x01    /* Write Status Register */
#define READ        0x03    /* Read Data Bytes  */
#define FAST_READ   0x0b    /* Read Data Bytes at Higher Speed //Not used as as the 328 isn't fast enough  */
#define PP          0x02    /* Page Program  */
#define SE          0x20    /* Sector Erase (4k)  */
#define BE          0xd8    /* Block Erase (64k)  */
#define CE          0xc7    /* Erase entire chip  */
#define DP          0xb9    /* Deep Power-down  */
#define RES         0xab    /* Release Power-down, return Device ID */
#define RDID        0x9F      /* Read Manufacture ID, memory type ID, capacity ID */

#define cs  10   //Chip select pin

#include "arduino.h"
#include <SPIFIFO.h>

#ifndef S25FLx_h
#define S25FLx_h


class flash
{
  public:

  flash();
    byte stat();
	void waitforit();
    void write_enable();
	void erase_4k(uint32_t loc);
	void erase_64k(uint32_t loc);
	void erase_all();
	void read(uint32_t loc, uint8_t* array, uint32_t length);
    void write(uint32_t loc, uint8_t* array, uint32_t length);
 	void read16(uint32_t loc,  int16_t* array, byte length);
    void write16(uint32_t loc, int16_t* array, byte length);
	void write_reg(byte w);
	void read_info();

  private:

};

#endif
