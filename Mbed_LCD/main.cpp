/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
 
#include "mbed.h"
 
#include "DmTftHX8353C.h"
#include "DmTftS6D0164.h"
#include "DmTftIli9325.h"
#include "DmTftIli9341.h"
#include "DmTftSsd2119.h"
#include "DmTftRa8875.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/
 
/* Note that there are restrictions on which platforms that can use printf
   in combinations with the DmTftLibrary. Some platforms (e.g. LPC1549 LPCXpresso)
   use the same pins for USBRX/USBTX and display control. Printing will
   cause the display to not work. Read more about this on the display's notebook
   page. */
#define log(...) printf(__VA_ARGS__)
//#define log(...)
 
/******************************************************************************
 * Local variables
 *****************************************************************************/
 
//DmTftHX8353C tft(D2, D3, D4, D5, D6);  /* DmTftHX8353C(PinName mosi, PinName clk, PinName cs, PinName dc, PinName rst) DM_TFT18_101 */
//DmTftS6D0164 tft(A4, A3, A5, A2);  /* DmTftS6D0164(PinName wr, PinName cs, PinName dc, PinName rst) DM_TFT22_102 */
//DmTftIli9325 tft(A4, A3, A5, A2);  /* DmTftIli9325(PinName wr, PinName cs, PinName dc, PinName rst) DM_TFT28_103 and DM_TFT24_104 */
DmTftIli9341 tft(D11, D12, A6, A5, A4);  /* DmTftIli9341(PinName cs, PinName dc, PinName mosi, PinName miso, PinName clk)  DM_TFT28_105 */
//DmTftSsd2119 tft(D10, D9, D11, D12, D13);  /* DmTftSsd2119(PinName cs, PinName dc, PinName mosi, PinName miso, PinName clk) DM_TFT35_107 */
//DmTftRa8875  tft(D10, D9, D11, D12, D13);  /* DmTftRa8875(PinName cs, PinName sel, PinName mosi, PinName miso, PinName clk) DM_TFT43_108 and DM_TFT50_111   For DmTftRa8875 driver, The panel resolution should be config in DmTftRa8875::init() function on the DmTftRa8875.cpp file. */
 
int bmpWidth, bmpHeight;
uint8_t bmpImageoffset;

InterruptIn button1(A3, PullDown);
InterruptIn button2(A2, PullDown);
DigitalOut led(LED3);
 
/******************************************************************************
 * Global variables
 *****************************************************************************/
 
extern uint8_t dmlogo[];
 
/******************************************************************************
 * Local functions
 *****************************************************************************/
 
// LITTLE ENDIAN!
uint16_t read16(uint8_t *src)
{
  uint16_t d;
  uint8_t b;
  b = *src;
  d = *(src+1);
  d <<= 8;
  d |= b;
  return d;
}
 
// LITTLE ENDIAN!
uint32_t read32(uint8_t *src)
{
  uint32_t d;
  uint16_t b;
 
  b = read16(src);
  d = read16(src+2);
  d <<= 16;
  d |= b;
  return d;
}
 
void drawBmpFromFlash(int x, int y)
{
  uint16_t pos = bmpImageoffset;
 
  uint16_t p;  // pixel
  uint8_t g, b;
  int i, j; // line, column
 
  for(i=bmpHeight; i>0; i--) {
    for(j=0; j<bmpWidth; j++) {
      b = *(dmlogo+pos++);
      g = *(dmlogo+pos++);
      p = *(dmlogo+pos++);
 
      p >>= 3;
      p <<= 6;
 
      g >>= 2;
      p |= g;
      p <<= 5;
 
      b >>= 3;
      p |= b;
 
      // write out the 16 bits of color
      tft.setPixel(j, i+y, p);
    }
  }
}
 
 
int bmpReadHeader() {
  uint32_t fileSize;
  uint32_t headerSize;
  uint16_t bmpDepth;
  uint16_t pos = 0;
  log("reading bmp header\r\n");
  log("Magic byte is: %d \r\n", read16(dmlogo));
 
  if (read16(dmlogo) !=0x4D42){ // read magic byte
    log("Magic byte not found\r\n");
    return false;
  }
  pos += 2;
 
  // read file size
  fileSize = read32(dmlogo+pos);
  log("filesize is: %d \r\n", fileSize);
  log("");
  pos += 4;
 
  pos += 4; // Skip creator bytes
 
  bmpImageoffset = read32(dmlogo+pos);
  pos += 4;
 
  // read DIB header
  headerSize = read32(dmlogo+pos);
  pos +=4;
  bmpWidth = read32(dmlogo+pos);
  pos += 4;
  bmpHeight = read32(dmlogo+pos);
  pos += 4;
 
  log("Image size:        %d\r\n", fileSize);
  log("Image offset:      %d\r\n", bmpImageoffset);
  log("Header size:       %d\r\n", headerSize);
  log("Image width:       %d\r\n", bmpWidth );
  log("Image height:      %d\r\n", bmpHeight );
 
  if (read16(dmlogo+pos) != 1){
    // number of color planes must be 1
    return false;
  }
  pos += 2;
 
  bmpDepth = read16(dmlogo+pos);
  pos +=2;
  log("Bitdepth:          %d\r\n", bmpDepth);
 
  if (read16(dmlogo+pos) != 0) {
    // compression not supported!
    return false;
  }
  pos += 2; // Should really be 2??
 
  return true;
}

// Interrupt service routine(ISR)
void button_isr() {
    led = !led; //Toggle the LED state
}

 
/******************************************************************************
 * Main
 *****************************************************************************/
 
int main() {
  log("init tft \r\n");
  tft.init();
  tft.clearScreen(BLACK);

  tft.drawString(0,32,"University of Glasgow.");
  tft.drawString(12,48,"Welcome");
  tft.drawString(88,64,"Mingxuan Hao, Yi Liu, Yueming Wen");
 
  tft.drawRectangle(20,85,40,105,GREEN);
  tft.drawCircle(60,95,10,BLUE);
  tft.drawTriangle(90,85, 80,105, 100,105, RED);
  
  wait_us(1000000);
  tft.clearScreen(BLACK);
 
  tft.drawXAxis(0,20,WHITE);
  tft.drawYAxis(0,0,WHITE);
  tft.drawWave(0, 140, YELLOW, 1);
  wait_us(1000000);
  
  tft.drawString(30,200,"finished");
  //button.fall(&button_isr);
  button1.mode(PullUp);
  button2.mode(PullUp);
 /*
  if (! bmpReadHeader()) {
    log("bad bmp\r\n");
    return -1;
  }
 
  drawBmpFromFlash(0, 0);
  drawBmpFromFlash(0, 130);
 */
  int i=1;
  double adj = 1.0;
  double itv;
  while(1) {
      if (!button1)
      {
          adj = adj * 2;
          itv = adj * 1000 / 8 ;
          tft.clearScreen(BLACK);
          tft.drawXAxis(0,20,WHITE);
          tft.drawYAxis(0,0,WHITE);
          tft.drawWave(0, 140, YELLOW, adj);
          tft.drawString(10,200,"us/d");
          tft.drawNumber(10, 215, itv, 4);
      }

      if (!button2)
      {
          adj = adj / 2;
          itv = adj * 1000 / 8; 
          tft.clearScreen(BLACK);
          tft.drawXAxis(0,20,WHITE);
          tft.drawYAxis(0,0,WHITE);
          tft.drawWave(0, 140, YELLOW, adj);
          tft.drawString(10,200,"us/d");
          tft.drawNumber(10, 215, itv, 4);
      }
      //tft.drawString(150,150,"Hello2");
      //wait_us(1000000);
      //i++;
  }
}

