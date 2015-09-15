/*
Arduino S25FLx Serial Flash library
By John-Mike Reed (Dr. Bleep) of Bleep labs 


This free library is realeased under Creative comoms license CC BY-SA 3.0
http://creativecommons.org/licenses/by-sa/3.0/deed.en_US
*/

#include "arduino.h"
#include <SPIFIFO.h>
#include "S25FLx_FIFO.h"

//Define S25FLx control bytes

#define WREN        0x06    /* Write Enable */
#define WRDI        0x04    /* Write Disable */ 
#define RDSR        0x05    /* Read Status Register */
#define WRSR        0x01    /* Write Status Register */
#define READ        0x03    /* Read Data Bytes  */
#define FAST_READ   0x0b    /* Read Data Bytes at Higher Speed //Not used as as the 328 isn't fast enough  */
#define PP          0x02    /* Page Program  */
#define SE          0x20    /* Sector Erase (4k)  */
#define BE          0xD8    /* Block Erase (64k)  */
#define CE          0xc7    /* Erase entire chip  */
#define DP          0xb9    /* Deep Power-down  */
#define RES         0xab    /* Release Power-down, return Device ID */
#define RDID        0x9F      /* Read Manufacture ID, memory type ID, capacity ID */

#define cs  10   //Chip select pin
uint32_t prev_s25fl;
byte arrayB[255]={};
uint16_t array16B[32]={};
byte byte_array_a[255]={};
byte byte_array_b[255]={};
byte byte_array_r1[255]={};


flash::flash(){
}

void printBits(byte myByte){
  for(byte mask = 0x80; mask; mask >>= 1){
    if(mask  & myByte){
      Serial.print('1');
    }
    else{
      Serial.print('0');
    }
  }
      Serial.println("B ");

}

//read the status register. 
byte flash::stat(){                     

  SPIFIFO.write(RDSR, SPI_CONTINUE);
  SPIFIFO.read();
  SPIFIFO.write(0);
  byte s=SPIFIFO.read();

  return s;
}

// use between each communication to make sure S25FLxx is ready to go.

void flash::waitforit(){
  byte s=stat();
  int cnt=0;
  while ((stat() & B0000001)==B00000001){    //check if WIP bit is 1
    s=stat();
    Serial.println(cnt);
  }

}


// Must be done to allow erasing or writing
void flash::write_enable(){
  //waitforit();
  SPIFIFO.write(WREN);
  SPIFIFO.read();
  //waitforit();
  //Serial.print(" WE");
}


 // Erase an entire 4k sector the location is in.
 // For example "erase_4k(300);" will erase everything from 0-3999. 

void flash::erase_4k(uint32_t loc){

write_enable();

SPIFIFO.write(0x20, SPI_CONTINUE);
SPIFIFO.read();
SPIFIFO.write(loc>>16, SPI_CONTINUE);
SPIFIFO.read();
SPIFIFO.write(loc>>8, SPI_CONTINUE);
SPIFIFO.read();
SPIFIFO.write(loc & 0xFF);
SPIFIFO.read();


}

 // Errase an entire 64_k sector the location is in.
 // For example erase4k(530000) will erase everything from 524543 to 589823. 

void flash::erase_64k(uint32_t loc){

    write_enable();

    SPIFIFO.write(0xD8, SPI_CONTINUE);
    SPIFIFO.read();
    SPIFIFO.write(loc>>16, SPI_CONTINUE);
    SPIFIFO.read();
    SPIFIFO.write(loc>>8, SPI_CONTINUE);
    SPIFIFO.read();
    SPIFIFO.write(loc & 0xFF);
    SPIFIFO.read();


}

 //errases all the memory. Can take several seconds.
void flash::erase_all(){
  waitforit(); 
  write_enable();
  SPIFIFO.write(CE);
  SPIFIFO.read();

  waitforit();

}




// Read data from the flash chip. There is no limit "length". The entire memory can be read with one command.
//read_S25(starting location, array, number of bytes);

void flash::read(uint32_t loc, uint8_t* array, uint32_t length){

  SPIFIFO.write(0x03, SPI_CONTINUE);
  SPIFIFO.read();

  SPIFIFO.write((loc>>16) & 0xFF, SPI_CONTINUE);
  SPIFIFO.read();

  SPIFIFO.write((loc >> 8) & 0xFF, SPI_CONTINUE);
  SPIFIFO.read();
  
  SPIFIFO.write(loc & 0xFF, SPI_CONTINUE);
  SPIFIFO.read();

  for (int i = 0; i < length; i++)
  {
    if (i<length-1)
    {
      SPIFIFO.write(0, SPI_CONTINUE);
      array[i]=SPIFIFO.read();
    }

    if (i==length-1)
    {
      SPIFIFO.write(0);
      array[i]=SPIFIFO.read();
    }

  }

}





//////////////////////////////////////////////////////////////






void flash::read16(uint32_t loc, int16_t* array, byte length){


  loc=loc<<1;

  SPIFIFO.write16 ((0x03 <<8) | (loc>>16),  SPI_CONTINUE);
  SPIFIFO.write16(loc , SPI_CONTINUE);
  SPIFIFO.read();
  SPIFIFO.read();

  for (int i = 0; i < length; i++)
  {
    if (i<length-1)
    {
      SPIFIFO.write16(0, SPI_CONTINUE);
      array[i]=SPIFIFO.read();
    }

    if (i==length-1)
    {
      SPIFIFO.write16(0);
      array[i]=SPIFIFO.read();
    }

  }

}

void flash::write16(uint32_t loc, int16_t* array, byte length){
  loc=loc<<1;
  //length=length<<1;

  write_enable(); 
  waitforit();
  waitforit();
  waitforit();


  SPIFIFO.write16 ((PP <<8) | (loc>>16),  SPI_CONTINUE);
  SPIFIFO.read();
  SPIFIFO.write16(loc , SPI_CONTINUE);
  SPIFIFO.read();
  
  for (int i = 0; i < length; i++)
  {
    if (i<length-1)
    {
      SPIFIFO.write16(array[i], SPI_CONTINUE);
      SPIFIFO.read();
    }

    if (i==length-1)
    {
      SPIFIFO.write16(array[i]);
      SPIFIFO.read();
    }

  }

}


///////////////////////////////////////////////





//Used in conjuture with the write protect pin to protect blocks. 
//For example on the S25FL216K sending "write_reg(B00001000);" will protect 2 blocks, 30 and 31.
//See the datasheet for more. http://www.mouser.com/ds/2/380/S25FL216K_00-6756.pdf

void flash::write_reg(byte w){

  SPIFIFO.write(WRSR, SPI_CONTINUE);
  SPIFIFO.read();
  SPIFIFO.write(w);
  SPIFIFO.read();

}


void flash::read_info(){

  digitalWriteFast(cs,LOW);
  SPIFIFO.write(0x9F, SPI_CONTINUE);
  SPIFIFO.read();

  SPIFIFO.write(0, SPI_CONTINUE);
  int s1=SPIFIFO.read();

  SPIFIFO.write(0, SPI_CONTINUE);
  int s2=SPIFIFO.read();
  
  SPIFIFO.write(0);
  int s3=SPIFIFO.read();


  Serial.print("Manufacturer ID: "); 
  Serial.print(s1);
  Serial.print("     Memory type: ");
  Serial.print(s2);
  Serial.print("     Capacity: "); 
  Serial.println(s3);
  Serial.println();

}

void flash::write(uint32_t loc, uint8_t* array, uint32_t length){
/*
 just use 16


  if (((loc & 0xff)+length)>255){

  //Serial.print("short w cross");

    byte remainer = loc & 0xff;
    byte length1 =256-remainer;
    byte length2 = length-length1;

    for (int i = 0; i < length2; i++)
    {
      arrayB[i]=array[i+length1];
    }

    uint32_t page1_loc = loc;
    uint32_t page2_loc = loc+length1;

//Serial.print("page1_loc ");
//Serial.println(page1_loc);
//Serial.print("page2_loc ");
//Serial.println(page2_loc);

    write_enable(); 
    digitalWriteFast(cs,LOW);
    spi4teensy3::send(PP);
    spi4teensy3::send(page1_loc>>16);
    spi4teensy3::send(page1_loc>>8);
    spi4teensy3::send(page1_loc & 0xff);

  spi4teensy3::send(array,length1); 

  digitalWriteFast(cs,HIGH);

//delay(10);
  write_enable(); 

  waitforit();

  digitalWriteFast(cs,LOW);
  spi4teensy3::send(PP);
  spi4teensy3::send(page2_loc>>16);
  spi4teensy3::send(page2_loc>>8);
  spi4teensy3::send(page2_loc & 0xff);

  spi4teensy3::send(arrayB,length2); 

  digitalWriteFast(cs,HIGH);



}



else{
//////Serial.print("loc & 0xff = ");////Serial.println(loc & 0xff);
////Serial.print("short");
  write_enable(); // Must be done before writing can commence. Erase clears it. 
 // waitforit();
  digitalWriteFast(cs,LOW);
  spi4teensy3::send(PP);
  spi4teensy3::send(loc>>16);
  spi4teensy3::send(loc>>8);
  spi4teensy3::send(loc & 0xff);

  spi4teensy3::send(array,length); 

  digitalWriteFast(cs,HIGH);
 // waitforit();
}
*/
}
