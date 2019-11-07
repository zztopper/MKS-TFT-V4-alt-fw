/*
  UTFT.cpp - Multi-Platform library support for Color TFT LCD Boards
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was origianlly based on the 
  demo-code provided by ITead studio (for the ITDB02 modules) and 
  NKC Electronics (for the RGB GLCD module/shield).

  This library supports a number of 8bit, 16bit and serial graphic 
  displays, and will work with both Arduino, chipKit boards and select 
  TI LaunchPads. For a full list of tested display modules and controllers,
  see the document UTFT_Supported_display_modules_&_controllers.pdf.

  When using 8bit and 16bit display modules there are some 
  requirements you must adhere to. These requirements can be found 
  in the document UTFT_Requirements.pdf.
  There are no special requirements when using serial displays.

  You can find the latest version of the library at 
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the 
  examples and tools supplied with the library.
*/
// Include hardware-specific functions for the correct MCU

#include "UTFT.h"
//#include "delay/user_delay.h"
#include <hardware/arm/HW_STM32F.h>
#include "memorysaver.h"



#ifdef  FSMC_TFT
UTFT::UTFT(byte model,uint16_t Xsize,uint16_t Ysize,byte trmodel)  //for ILI9486/9320 (16bit) STM32 fsmc
{
	display_model =			model;
	disp_x_size =			Xsize;
	disp_y_size =			Ysize;
	display_transfer_mode =	trmodel;
}



UTFT::UTFT()
{
}

#endif
#ifndef FSMC_TFT
UTFT::UTFT(byte model, int RS, int WR, int CS, int RST, int SER)
{ 
	uint16_t	dsx[] = {239, 239, 239, 239, 239, 239, 175, 175, 239, 127,		// 00-09
					 127, 239, 271, 479, 239, 239, 239, 239, 0, 239,			// 10-19
					 479, 319, 239, 175, 127, 239, 239, 319, 319, 799,		// 20-29
					 239, 127,239};												// 30-
	uint16_t	dsy[] = {319, 399, 319, 319, 319, 319, 219, 219, 399, 159,		// 00-09
					 127, 319, 479, 799, 319, 319, 319, 319, 0, 319,			// 10-19
					 799, 479, 319, 219, 159, 319, 319, 479, 479, 479,		// 20-29
					 319, 159,399};												// 30-
	byte	dtm[] = {16, 16, 16, 8, 8, 16, 8, SERIAL_4PIN, 16, SERIAL_5PIN,					// 00-09
					 SERIAL_5PIN, 16, 16, 16, 8, 16, LATCHED_16, 16, 0, 8,					// 10-19
					 16, 16, 16, 8, SERIAL_5PIN, SERIAL_5PIN, SERIAL_4PIN, 16, 16, 16,		// 20-29
					 8, SERIAL_5PIN,16};												// 30-

	display_model =			model;	
	disp_x_size =			dsx[model];
	disp_y_size =			dsy[model];
	display_transfer_mode =	dtm[model];
    
	__p1 = RS;
	__p2 = WR;
	__p3 = CS;
	__p4 = RST;
	__p5 = SER;

	if (display_transfer_mode == SERIAL_4PIN)
	{
		display_transfer_mode=1;
		display_serial_mode=SERIAL_4PIN;
	}
	if (display_transfer_mode == SERIAL_5PIN)
	{
		display_transfer_mode=1;
		display_serial_mode=SERIAL_5PIN;
	}

	if (display_transfer_mode!=1)
	{
		_set_direction_registers(display_transfer_mode);
		P_RS	= portOutputRegister(digitalPinToPort(RS));
		B_RS	= digitalPinToBitMask(RS);
		P_WR	= portOutputRegister(digitalPinToPort(WR));
		B_WR	= digitalPinToBitMask(WR);
		P_CS	= portOutputRegister(digitalPinToPort(CS));
		B_CS	= digitalPinToBitMask(CS);
		P_RST	= portOutputRegister(digitalPinToPort(RST));
		B_RST	= digitalPinToBitMask(RST);
		if (display_transfer_mode==LATCHED_16)
		{
			P_ALE	= portOutputRegister(digitalPinToPort(SER));
			B_ALE	= digitalPinToBitMask(SER);
			cbi(P_ALE, B_ALE);
			pinMode(8,OUTPUT);
			digitalWrite(8, LOW);
		}
	}
	else
	{
		P_SDA	= portOutputRegister(digitalPinToPort(RS));
		B_SDA	= digitalPinToBitMask(RS);
		P_SCL	= portOutputRegister(digitalPinToPort(WR));
		B_SCL	= digitalPinToBitMask(WR);
		P_CS	= portOutputRegister(digitalPinToPort(CS));
		B_CS	= digitalPinToBitMask(CS);
		if (RST != NOTINUSE)
		{
			P_RST	= portOutputRegister(digitalPinToPort(RST));
			B_RST	= digitalPinToBitMask(RST);
		}
		if (display_serial_mode!=SERIAL_4PIN)
		{
			P_RS	= portOutputRegister(digitalPinToPort(SER));
			B_RS	= digitalPinToBitMask(SER);
		}
	}
}
#endif
#ifndef FSMC_TFT
#ifdef STM32F107xC
#else
void UTFT::LCD_Write_COM(int VL)  
{   
	if (display_transfer_mode!=1)
	{
		cbi(P_RS, B_RS);
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_RESET);
		LCD_Writ_Bus(0x00,VL,display_transfer_mode);
	}
	else
		LCD_Writ_Bus(0x00,VL,display_transfer_mode);
}

void UTFT::LCD_Write_DATA(char VH,char VL)
{
	if (display_transfer_mode!=1)
	{
		sbi(P_RS, B_RS);
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		LCD_Writ_Bus(VH,VL,display_transfer_mode);
	}
	else
	{
		LCD_Writ_Bus(0x01,VH,display_transfer_mode);
		LCD_Writ_Bus(0x01,VL,display_transfer_mode);
	}
}

void UTFT::LCD_Write_DATA(char VL)
{
	if (display_transfer_mode!=1)
	{
		sbi(P_RS, B_RS);
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		LCD_Writ_Bus(0x00,VL,display_transfer_mode);
	}
	else
		LCD_Writ_Bus(0x01,VL,display_transfer_mode);
}

void UTFT::LCD_Write_COM_DATA(char com1,int dat1)
{
     LCD_Write_COM(com1);
     LCD_Write_DATA(dat1>>8,dat1);
}
#endif
#endif

#define delay_ms(x) delay(x)
#ifdef __USER_DELAY_H__
  #define LCD_delay_ms(x) USER_delay_ms(x)
#else
  #define LCD_delay_ms(x) delay_ms(x)	
#endif

int  UTFT::readID(void)
{
    LCD_Write_COM_DATA(0x00,0x0001);
    LCD_delay_ms(50); // delay 50 ms 
    return LCD_ReadData();
}			  

void UTFT::Init(byte orientation)
{
	orient=orientation;
	_hw_special_init();
//	return;
#ifdef FSMC_TFT
	 _set_direction_registers(display_transfer_mode);
#else
	#if defined(STM32F107xC)
	//stm setup
	
	#else
	pinMode(__p1,OUTPUT);
	pinMode(__p2,OUTPUT);
	pinMode(__p3,OUTPUT);
	if (__p4 != NOTINUSE)
		pinMode(__p4,OUTPUT);
	if ((display_transfer_mode==LATCHED_16) or ((display_transfer_mode==1) and (display_serial_mode==SERIAL_5PIN)))
		pinMode(__p5,OUTPUT);
	if (display_transfer_mode!=1)

	  _set_direction_registers(display_transfer_mode);

	sbi(P_RST, B_RST);
	delay(5); 
	cbi(P_RST, B_RST);
	delay(15);
	sbi(P_RST, B_RST);
	delay(15);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	cbi(P_CS, B_CS);
	#endif
  #endif
#if defined(STM32F107xC)
	//stm setup
	
	#else
  LCD_delay_ms(50); // delay 50 ms 
  LCD_Write_COM_DATA(0x00,0x0001);
  LCD_delay_ms(50); // delay 50 ms 
  mode_id =  LCD_ReadData();  
  if((mode_id < 0XFF)||(mode_id == 0XFFFF)||(mode_id ==0X9300)){ //teset 9341 6804 5310 5510
    LCD_Write_COM(0xD3);      //test ili9341
	mode_id = LCD_ReadData(); //dummy read 
	mode_id = LCD_ReadData(); //read 0x00
	mode_id = LCD_ReadData();   //0x93
	mode_id <<=8;  
	mode_id |=LCD_ReadData();  //0x41
	if (mode_id !=0x9341){
      LCD_Write_COM(0xBF);      //test ili9341
	  mode_id = LCD_ReadData(); //dummy read 
	  mode_id = LCD_ReadData(); //read 0x01
	  mode_id = LCD_ReadData();   //0xD0
	  mode_id = LCD_ReadData();   //0x68
	  mode_id <<=8;  
	  mode_id |=LCD_ReadData();  //0x41
	  if (mode_id !=0X6804){
        LCD_Write_COM(0xD4);      //NT35310
	    mode_id = LCD_ReadData(); //dummy read 
	    mode_id = LCD_ReadData(); //read 0x01
		mode_id = LCD_ReadData();   //0x53
		mode_id <<=8;  
		mode_id |=LCD_ReadData();  //0x10
	    if (mode_id !=0X5310){    //NT35510
		  LCD_Write_COM(0xDA00); 
	      mode_id = LCD_ReadData(); //dummy read 
		  LCD_Write_COM(0xDB00); 
	      mode_id = LCD_ReadData(); //read 0x80
		  mode_id <<=8;  
		  LCD_Write_COM(0xDC00); 
		  mode_id |=LCD_ReadData();  //0x00
		  if(mode_id ==0x8000){
		    mode_id = 0x5510; // is NT35510
	      } else{
		    mode_id =0x00;
		  }
	    }
	  }
	}
  }
  #endif
 
  
 
	
	delay(150);
   
	  
  switch(display_model)
  {
#ifndef DISABLE_HX8347A
	#include "tft_drivers/hx8347a/initlcd.h"
#endif
#ifndef DISABLE_ILI9327
	#include "tft_drivers/ili9327/initlcd.h"
#endif
#ifndef DISABLE_SSD1289
	#include "tft_drivers/ssd1289/initlcd.h"
#endif
#ifndef DISABLE_ILI9325C
	#include "tft_drivers/ili9325c/initlcd.h"
#endif
#ifndef DISABLE_ILI9325D
	#include "tft_drivers/ili9325d/default/initlcd.h"
#endif
#ifndef DISABLE_ILI9325D_ALT
	#include "tft_drivers/ili9325d/alt/initlcd.h"
#endif
#ifndef DISABLE_HX8340B_8
	#include "tft_drivers/hx8340b/8/initlcd.h"
#endif
#ifndef DISABLE_HX8340B_S
	#include "tft_drivers/hx8340b/s/initlcd.h"
#endif
#ifndef DISABLE_ST7735
	#include "tft_drivers/st7735/std/initlcd.h"
#endif
#ifndef DISABLE_ST7735_ALT
	#include "tft_drivers/st7735/alt/initlcd.h"
#endif
#ifndef DISABLE_PCF8833
	#include "tft_drivers/pcf8833/initlcd.h"
#endif
#ifndef DISABLE_S1D19122
	#include "tft_drivers/s1d19122/initlcd.h"
#endif
#ifndef DISABLE_HX8352A
	#include "tft_drivers/hx8352a/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_480
	#include "tft_drivers/ssd1963/480/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_800
	#include "tft_drivers/ssd1963/800/initlcd.h"
#endif
#ifndef DISABLE_SSD1963_800_ALT
	#include "tft_drivers/ssd1963/800alt/initlcd.h"
#endif
#ifndef DISABLE_S6D1121
	#include "tft_drivers/s6d1121/initlcd.h"
#endif
#ifndef DISABLE_ILI9481
	#include "tft_drivers/ili9481/initlcd.h"
#endif
#ifndef DISABLE_S6D0164
	#include "tft_drivers/s6d0164/initlcd.h"
#endif
#ifndef DISABLE_ST7735S
	#include "tft_drivers/st7735s/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_S4P
	#include "tft_drivers/ili9341/s4p/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_S5P
	#include "tft_drivers/ili9341/s5p/initlcd.h"
#endif
#ifndef DISABLE_ILI9341_16
	#include "tft_drivers/ili9341/initlcd.h"
#endif
#ifndef DISABLE_R61581
	#include "tft_drivers/r61581/initlcd.h"
#endif
#ifndef DISABLE_ILI9486
	#include "tft_drivers/ili9486/initlcd.h"
#endif
#ifndef DISABLE_CPLD
	#include "tft_drivers/cpld/initlcd.h"
#endif
#ifndef DISABLE_HX8353C
	#include "tft_drivers/hx8353c/initlcd.h"
#endif
#ifndef DISABLE_ILI9320
	#include "tft_drivers/ili9320/initlcd.h"
#endif
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi (P_CS, B_CS); 

	setColor(255, 255, 255);
	setBackColor(0, 0, 0);
	cfont.font=0;
	_transparent = false;
}

void UTFT::setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	if (orient==LANDSCAPE)
	{
		swap(uint16_t, x1, y1);
		swap(uint16_t, x2, y2)
		y1=disp_y_size-y1;
		y2=disp_y_size-y2;
		swap(uint16_t, y1, y2)
	}

	switch(display_model)
	{
#ifndef DISABLE_HX8347A
	//#include "tft_drivers/hx8347a/setxy.h"
#endif
#ifndef DISABLE_HX8352A
	//#include "tft_drivers/hx8352a/setxy.h"
	
#endif
#ifndef DISABLE_ILI9327
	//#include "tft_drivers/ili9327/setxy.h"
#endif
#ifndef DISABLE_SSD1289
	//#include "tft_drivers/ssd1289/setxy.h"
#endif
#ifndef DISABLE_ILI9325C
	//#include "tft_drivers/ili9325c/setxy.h"
#endif
#ifndef DISABLE_ILI9325D
	//#include "tft_drivers/ili9325d/default/setxy.h"
#endif
#ifndef DISABLE_ILI9325D_ALT
	//#include "tft_drivers/ili9325d/alt/setxy.h"
#endif
#ifndef DISABLE_HX8340B_8
	//#include "tft_drivers/hx8340b/8/setxy.h"
#endif
#ifndef DISABLE_HX8340B_S
	//#include "tft_drivers/hx8340b/s/setxy.h"
#endif
#ifndef DISABLE_ST7735
	//#include "tft_drivers/st7735/std/setxy.h"
#endif
#ifndef DISABLE_ST7735_ALT
	//#include "tft_drivers/st7735/alt/setxy.h"
#endif
#ifndef DISABLE_S1D19122
	//#include "tft_drivers/s1d19122/setxy.h"
#endif
#ifndef DISABLE_PCF8833
	//#include "tft_drivers/pcf8833/setxy.h"
#endif
#ifndef DISABLE_SSD1963_480
	//#include "tft_drivers/ssd1963/480/setxy.h"
#endif
#ifndef DISABLE_SSD1963_800
	//#include "tft_drivers/ssd1963/800/setxy.h"
#endif
#ifndef DISABLE_SSD1963_800_ALT
//	#include "tft_drivers/ssd1963/800alt/setxy.h"
#endif
#ifndef DISABLE_S6D1121
	//#include "tft_drivers/s6d1121/setxy.h"
#endif
#ifndef DISABLE_ILI9481
	//#include "tft_drivers/ili9481/setxy.h"
#endif
#ifndef DISABLE_S6D0164
	//#include "tft_drivers/s6d0164/setxy.h"
#endif
#ifndef DISABLE_ST7735S
	//#include "tft_drivers/st7735s/setxy.h"
#endif
#ifndef DISABLE_ILI9341_S4P
	//#include "tft_drivers/ili9341/s4p/setxy.h"
#endif
#ifndef DISABLE_ILI9341_S5P
	//#include "tft_drivers/ili9341/s5p/setxy.h"
#endif
#ifndef DISABLE_ILI9341_16
	//#include "tft_drivers/ili9341/setxy.h"
#endif
#ifndef DISABLE_R61581
	//#include "tft_drivers/r61581/setxy.h"
#endif
#ifndef DISABLE_ILI9486
	//#include "tft_drivers/ili9486/setxy.h"
#endif
#ifndef DISABLE_CPLD
	//#include "tft_drivers/cpld/setxy.h"
#endif
#ifndef DISABLE_HX8353C
	#include "tft_drivers/hx8353c/setxy.h"
#endif
#ifndef DISABLE_ILI9320
	//#include "tft_drivers/ili9320/setxy.h"
#endif
#ifndef DISABLE_SPFD5420
	//#include "tft_drivers/SPFD5420/setxy.h"
#endif
	}
}

void UTFT::clrXY()
{
	if (orient==PORTRAIT)
		setXY(0,0,disp_x_size,disp_y_size);
	else
		setXY(0,0,disp_y_size,disp_x_size);
}

void UTFT::drawRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	drawHLine(x1, y1, x2-x1);
	drawHLine(x1, y2, x2-x1);
	drawVLine(x1, y1, y2-y1);
	drawVLine(x2, y1, y2-y1);
}

void UTFT::drawRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if ((x2-x1)>4 && (y2-y1)>4)
	{
		drawPixel(x1+1,y1+1);
		drawPixel(x2-1,y1+1);
		drawPixel(x1+1,y2-1);
		drawPixel(x2-1,y2-1);
		drawHLine(x1+2, y1, x2-x1-4);
		drawHLine(x1+2, y2, x2-x1-4);
		drawVLine(x1, y1+2, y2-y1-4);
		drawVLine(x2, y1+2, y2-y1-4);
	}
}

void UTFT::fillRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}
	if (display_transfer_mode==16)
	{
		
		cbi(P_CS, B_CS);
		setXY(x1, y1, x2, y2);
		sbi(P_RS, B_RS);
		_fast_fill_16(fch,fcl,((long(x2-x1)+1)*(long(y2-y1)+1)));
		sbi(P_CS, B_CS);
		
	}
	else if ((display_transfer_mode==8) and (fch==fcl))
	{
		
		cbi(P_CS, B_CS);
		setXY(x1, y1, x2, y2);
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		sbi(P_RS, B_RS);
		_fast_fill_8(fch,((long(x2-x1)+1)*(long(y2-y1)+1)));
		HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
		sbi(P_CS, B_CS);
		
	}
	else
	{
		if (orient==PORTRAIT)
		{
			for (int i=0; i<((y2-y1)/2)+1; i++)
			{
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
		else
		{
			for (int i=0; i<((x2-x1)/2)+1; i++)
			{
				drawVLine(x1+i, y1, y2-y1);
				drawVLine(x2-i, y1, y2-y1);
			}
		}
	}
}

void UTFT::fillRoundRect(int x1, int y1, int x2, int y2)
{
	if (x1>x2)
	{
		swap(int, x1, x2);
	}
	if (y1>y2)
	{
		swap(int, y1, y2);
	}

	if ((x2-x1)>4 && (y2-y1)>4)
	{
		for (int i=0; i<((y2-y1)/2)+1; i++)
		{
			switch(i)
			{
			case 0:
				drawHLine(x1+2, y1+i, x2-x1-4);
				drawHLine(x1+2, y2-i, x2-x1-4);
				break;
			case 1:
				drawHLine(x1+1, y1+i, x2-x1-2);
				drawHLine(x1+1, y2-i, x2-x1-2);
				break;
			default:
				drawHLine(x1, y1+i, x2-x1);
				drawHLine(x1, y2-i, x2-x1);
			}
		}
	}
}

void UTFT::drawCircle(int x, int y, int radius)
{
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;
 
	cbi(P_CS, B_CS);
	setXY(x, y + radius, x, y + radius);
	LCD_Write_DATA(fch,fcl);
	setXY(x, y - radius, x, y - radius);
	LCD_Write_DATA(fch,fcl);
	setXY(x + radius, y, x + radius, y);
	LCD_Write_DATA(fch,fcl);
	setXY(x - radius, y, x - radius, y);
	LCD_Write_DATA(fch,fcl);
 
	while(x1 < y1)
	{
		if(f >= 0) 
		{
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;    
		setXY(x + x1, y + y1, x + x1, y + y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - x1, y + y1, x - x1, y + y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x + x1, y - y1, x + x1, y - y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - x1, y - y1, x - x1, y - y1);
		LCD_Write_DATA(fch,fcl);
		setXY(x + y1, y + x1, x + y1, y + x1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - y1, y + x1, x - y1, y + x1);
		LCD_Write_DATA(fch,fcl);
		setXY(x + y1, y - x1, x + y1, y - x1);
		LCD_Write_DATA(fch,fcl);
		setXY(x - y1, y - x1, x - y1, y - x1);
		LCD_Write_DATA(fch,fcl);
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::fillCircle(int x, int y, int radius)
{
	for(int y1=-radius; y1<=0; y1++) 
		for(int x1=-radius; x1<=0; x1++)
			if(x1*x1+y1*y1 <= radius*radius) 
			{
				drawHLine(x+x1, y+y1, 2*(-x1));
				drawHLine(x+x1, y-y1, 2*(-x1));
				break;
			}
}

void UTFT::clrScr()
{
	long i;
	

    
	//cbi(P_CS, B_CS);
	clrXY();
	if (display_transfer_mode!=1)
		sbi(P_RS, B_RS);
	if (display_transfer_mode==16)
		_fast_fill_16(0,0,((disp_x_size+1)*(disp_y_size+1)));
	else if (display_transfer_mode==8)
		_fast_fill_8(0,((disp_x_size+1)*(disp_y_size+1)));
	else
	{
		for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
		{
			if (display_transfer_mode!=1)
				LCD_Writ_Bus(0,0,display_transfer_mode);
			else
			{
				LCD_Writ_Bus(1,0,display_transfer_mode);
				LCD_Writ_Bus(1,0,display_transfer_mode);
			}
		}
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	//sbi(P_CS, B_CS);
	
}

void UTFT::fillScr(byte r, byte g, byte b)
{
	uint16_t color = ((r&248)<<8 | (g&252)<<3 | (b&248)>>3);
	fillScr(color);
}

void UTFT::fillScr(uint16_t color)
{
	long i;
	char ch, cl;
	
	ch=byte(color>>8);
	cl=byte(color & 0xFF);
	#if defined(STM32F107xC)
		
		
		setXY(0,0,disp_x_size+1,disp_y_size+1);
		//clrXY();
		//ch=0xff;
		//cl=0xff;
	//	LCD_Write_COM(0x2c);
	if (disp_x_size>disp_y_size)
	{
		setXY(0,0,disp_x_size+1,disp_y_size+1);
		_fast_fill_8(color,((disp_x_size+1)*(disp_y_size+1)));
		
	}
		else
		{
			setXY(0,0,disp_y_size+1,disp_x_size+1);
		_fast_fill_8(color,((disp_y_size+1)*(disp_x_size+1)));
	}
		//_fast_fill_16(ch,cl,((disp_x_size+1)*(disp_y_size+1)));
		//_fast_fill_8(ch,((319+1)*(239+1)));
	//	Serial1.println("disp X size:");
	//	Serial1.println(disp_x_size);
	//	Serial1.println(" disp y size:");
	//	Serial1.println(disp_y_size);
		
	#else
	cbi(P_CS, B_CS);
	clrXY();
	if (display_transfer_mode!=1)
	
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);	
	//sbi(P_RS, B_RS);
	if (display_transfer_mode==16)
		_fast_fill_16(ch,cl,((disp_x_size+1)*(disp_y_size+1)));
	else if ((display_transfer_mode==8) and (ch==cl))
		_fast_fill_8(ch,((disp_x_size+1)*(disp_y_size+1)));
	else
	{
		for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
		{
			if (display_transfer_mode!=1)
				LCD_Writ_Bus(ch,cl,display_transfer_mode);
			else
			{
				LCD_Writ_Bus(1,ch,display_transfer_mode);
				LCD_Writ_Bus(1,cl,display_transfer_mode);
			}
		}
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);

	sbi(P_CS, B_CS);
	#endif
}

void UTFT::setColor(byte r, byte g, byte b)
{
	fch=((r&248)|g>>5);
	fcl=((g&28)<<3|b>>3);
}

void UTFT::setColor(uint16_t color)
{
	fch=byte(color>>8);
	fcl=byte(color & 0xFF);
}

uint16_t UTFT::getColor()
{
	return (fch<<8) | fcl;
}

void UTFT::setBackColor(byte r, byte g, byte b)
{
	bch=((r&248)|g>>5);
	bcl=((g&28)<<3|b>>3);
	_transparent=false;
}

void UTFT::setBackColor(uint32_t color)
{
	if (color==VGA_TRANSPARENT)
		_transparent=true;
	else
	{
		bch=byte(color>>8);
		bcl=byte(color & 0xFF);
		_transparent=false;
	}
}

uint16_t UTFT::getBackColor()
{
	return (bch<<8) | bcl;
}

void UTFT::setPixel(uint16_t color)
{
	//LCD_Write_DATA((color>>8),(color&0xFF));	// rrrrrggggggbbbbb
	LCD_Write_DATA(color);	
	
}

void UTFT::drawPixel(int x, int y)
{
	
	cbi(P_CS, B_CS);
	setXY(x, y, x, y);
	setPixel((fch<<8)|fcl);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::drawLine(int x1, int y1, int x2, int y2)
{
	if (y1==y2)
		drawHLine(x1, y1, x2-x1);
	else if (x1==x2)
		drawVLine(x1, y1, y2-y1);
	else
	{
		unsigned int	dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short			xstep =  x2 > x1 ? 1 : -1;
		unsigned int	dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short			ystep =  y2 > y1 ? 1 : -1;
		int				col = x1, row = y1;
		
		cbi(P_CS, B_CS);
		if (dx < dy)
		{
			int t = - (dy >> 1);
			while (true)
			{
				setXY (col, row, col, row);
				LCD_Write_DATA (fch, fcl);
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0)
				{
					col += xstep;
					t   -= dy;
				}
			} 
		}
		else
		{
			int t = - (dx >> 1);
			while (true)
			{
				setXY (col, row, col, row);
				LCD_Write_DATA (fch, fcl);
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0)
				{
					row += ystep;
					t   -= dx;
				}
			} 
		}
		sbi(P_CS, B_CS);
	}
	clrXY();
}

void UTFT::drawHLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		x -= l;
	}
	
	cbi(P_CS, B_CS);
	setXY(x, y, x+l, y);
	if (display_transfer_mode == 16)
	{
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		sbi(P_RS, B_RS);
		_fast_fill_16(fch,fcl,l);
	}
	else if ((display_transfer_mode==8) and (fch==fcl))
	{
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		sbi(P_RS, B_RS);
		_fast_fill_8(fch,l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
		{
			LCD_Write_DATA(fch, fcl);
		}
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::drawVLine(int x, int y, int l)
{
	if (l<0)
	{
		l = -l;
		y -= l;
	}
	
	cbi(P_CS, B_CS);
	setXY(x, y, x, y+l);
	if (display_transfer_mode == 16)
	{
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		sbi(P_RS, B_RS);
		_fast_fill_16(fch,fcl,l);
	}
	else if ((display_transfer_mode==8) and (fch==fcl))
	{
		HAL_GPIO_WritePin(LCD_RS_GPIO_Port,LCD_RS_Pin, GPIO_PIN_SET);
		sbi(P_RS, B_RS);
		_fast_fill_8(fch,l);
	}
	else
	{
		for (int i=0; i<l+1; i++)
		{
			LCD_Write_DATA(fch, fcl);
		}
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::printChar(byte c, int x, int y)
{
	byte i,ch;
	uint16_t j;
	uint16_t temp; 
    
	cbi(P_CS, B_CS);
  
	if (!_transparent)
	{
		if (orient==PORTRAIT)
		{
			setXY(x,y,x+cfont.x_size-1,y+cfont.y_size-1);
	  
			temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
			for(j=0;j<((cfont.x_size/8)*cfont.y_size);j++)
			{
				ch=pgm_read_byte(&cfont.font[temp]);
				for(i=0;i<8;i++)
				{   
					if((ch&(1<<(7-i)))!=0)   
					{
						setPixel((fch<<8)|fcl);
					} 
					else
					{
						setPixel((bch<<8)|bcl);
					}   
				}
				temp++;
			}
		}
		else
		{
			temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

			for(j=0;j<((cfont.x_size/8)*cfont.y_size);j+=(cfont.x_size/8))
			{
				setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));
				for (int zz=(cfont.x_size/8)-1; zz>=0; zz--)
				{
					ch=pgm_read_byte(&cfont.font[temp+zz]);
					for(i=0;i<8;i++)
					{   
						if((ch&(1<<i))!=0)   
						{
							setPixel((fch<<8)|fcl);
						} 
						else
						{
							setPixel((bch<<8)|bcl);
						}   
					}
				}
				temp+=(cfont.x_size/8);
			}
		}
	}
	else
	{
		temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
		for(j=0;j<cfont.y_size;j++) 
		{
			for (int zz=0; zz<(cfont.x_size/8); zz++)
			{
				ch=pgm_read_byte(&cfont.font[temp+zz]); 
				for(i=0;i<8;i++)
				{   
				
					if((ch&(1<<(7-i)))!=0)   
					{
						setXY(x+i+(zz*8),y+j,x+i+(zz*8)+1,y+j+1);
						setPixel((fch<<8)|fcl);
					} 
				}
			}
			temp+=(cfont.x_size/8);
		}
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::rotateChar(byte c, int x, int y, int pos, int deg)
{
	byte i,j,ch;
	uint16_t temp; 
	int newx,newy;
	double radian;
	radian=deg*0.0175;  
	
	cbi(P_CS, B_CS);

	temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
	for(j=0;j<cfont.y_size;j++) 
	{
		for (int zz=0; zz<(cfont.x_size/8); zz++)
		{
			ch=pgm_read_byte(&cfont.font[temp+zz]); 
			for(i=0;i<8;i++)
			{   
				newx=x+(((i+(zz*8)+(pos*cfont.x_size))*cos(radian))-((j)*sin(radian)));
				newy=y+(((j)*cos(radian))+((i+(zz*8)+(pos*cfont.x_size))*sin(radian)));

				setXY(newx,newy,newx+1,newy+1);
				
				if((ch&(1<<(7-i)))!=0)   
				{
					setPixel((fch<<8)|fcl);
				} 
				else  
				{
					if (!_transparent)
						setPixel((bch<<8)|bcl);
				}   
			}
		}
		temp+=(cfont.x_size/8);
	}
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::printStr(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (orient==PORTRAIT)
	{
	if (x==RIGHT)
		x=(disp_x_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_x_size+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x==RIGHT)
		x=(disp_y_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_y_size+1)-(stl*cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg==0)
			printChar(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void UTFT::printStr(const char *st, int x, int y, int deg){
	 this->printStr((char *)st, x, y, deg);
}

void UTFT::printStr(String st, int x, int y, int deg)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	printStr(buf, x, y, deg);
	
}

void UTFT::printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	printStr(st,x,y);
}

void UTFT::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char st[27];
	boolean neg=false;

	if (dec<1)
		dec=1;
	else if (dec>5)
		dec=5;

	if (num<0)
		neg = true;

	_convert_float(st, num, length, dec);

	if (divider != '.')
	{
		for (unsigned int i=0; i<sizeof(st); i++)
			if (st[i]=='.')
				st[i]=divider;
	}

	if (filler != ' ')
	{
		if (neg)
		{
			st[0]='-';
			for (unsigned int i=1; i<sizeof(st); i++)
				if ((st[i]==' ') || (st[i]=='-'))
					st[i]=filler;
		}
		else
		{
			for (unsigned int i=0; i<sizeof(st); i++)
				if (st[i]==' ')
					st[i]=filler;
		}
	}

	printStr(st,x,y);
}

void UTFT::setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=fontbyte(0);
	cfont.y_size=fontbyte(1);
	cfont.offset=fontbyte(2);
	cfont.numchars=fontbyte(3);
}

uint8_t* UTFT::getFont()
{
	return cfont.font;
}

uint8_t UTFT::getFontXsize()
{
	return cfont.x_size;
}

uint8_t UTFT::getFontYsize()
{
	return cfont.y_size;
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale)
{
	unsigned int col;
	int tx, ty, tc, tsx, tsy;

	if (scale==1)
	{
		if (orient==PORTRAIT)
		{
			cbi(P_CS, B_CS);
			setXY(x, y, x+sx-1, y+sy-1);
			for (tc=0; tc<(sx*sy); tc++)
			{
				col=pgm_read_word(&data[tc]);
				LCD_Write_DATA(col>>8,col & 0xff);
			}
			sbi(P_CS, B_CS);
		}
		else
		{
			cbi(P_CS, B_CS);
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+ty, x+sx-1, y+ty);
				for (tx=sx-1; tx>=0; tx--)
				{
					col=pgm_read_word(&data[(ty*sx)+tx]);
					LCD_Write_DATA(col>>8,col & 0xff);
				}
			}
			sbi(P_CS, B_CS);
		}
	}
	else
	{
		if (orient==PORTRAIT)
		{
			cbi(P_CS, B_CS);
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
				for (tsy=0; tsy<scale; tsy++)
					for (tx=0; tx<sx; tx++)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							LCD_Write_DATA(col>>8,col & 0xff);
					}
			}
			sbi(P_CS, B_CS);
		}
		else
		{
			cbi(P_CS, B_CS);
			for (ty=0; ty<sy; ty++)
			{
				for (tsy=0; tsy<scale; tsy++)
				{
					setXY(x, y+(ty*scale)+tsy, x+((sx*scale)-1), y+(ty*scale)+tsy);
					for (tx=sx-1; tx>=0; tx--)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++)
							LCD_Write_DATA(col>>8,col & 0xff);
					}
				}
			}
			sbi(P_CS, B_CS);
		}
	}
	clrXY();
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy)
{
	unsigned int col;
	int tx, ty, newx, newy;
	double radian;
	radian=deg*0.0175;  

	if (deg==0)
		drawBitmap(x, y, sx, sy, data);
	else
	{
		cbi(P_CS, B_CS);
		for (ty=0; ty<sy; ty++)
			for (tx=0; tx<sx; tx++)
			{
				col=pgm_read_word(&data[(ty*sx)+tx]);

				newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
				newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

				setXY(newx, newy, newx, newy);
				LCD_Write_DATA(col>>8,col & 0xff);
			}
		sbi(P_CS, B_CS);
	}
	clrXY();
}


int UTFT::getDisplayXSize()
{
	if (orient==PORTRAIT)
		return disp_x_size+1;
	else
		return disp_y_size+1;
}

int UTFT::getDisplayYSize()
{
	if (orient==PORTRAIT)
		return disp_y_size+1;
	else
		return disp_x_size+1;
}

#ifndef FSMC_TFT
#if defined(STM32F107xC)

void UTFT::lcdOff()
{
	
	cbi(P_CS, B_CS);
	LCD_Write_COM(0x28);
	HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_RESET);
		delay(10);
		HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_SET);
	return;
	switch (display_model)
	{
	case PCF8833:
		LCD_Write_COM(0x28);
		break;
	case CPLD:
		//LCD_Write_COM_DATA(0x01,0x0000);
		//LCD_Write_COM(0x0F);   
		break;
	case HX8353C:
		LCD_Write_COM(0x21);
		/*HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_RESET);
		delay(10);
		HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_RESET);
		delay(1);
		HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_SET);*/
		break;
	}
	sbi(P_CS, B_CS);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
}

void UTFT::lcdOn()
{
	
	cbi(P_CS, B_CS);
	LCD_Write_COM(0x29);
	
	return;
	switch (display_model)
	{
	case PCF8833:
		LCD_Write_COM(0x29);
		break;
	case CPLD:
		LCD_Write_COM_DATA(0x01,0x0010);
		LCD_Write_COM(0x0F);   
		break;
	case HX8353C:
		LCD_Write_COM(0x20);
	/*	HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(POWER_DI_GPIO_Port,POWER_DI_Pin, GPIO_PIN_RESET);
		delay(10);
		HAL_GPIO_WritePin(FILAMENT_DI_GPIO_Port,FILAMENT_DI_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(POWER_DI_GPIO_Port,POWER_DI_Pin, GPIO_PIN_SET);*/
		break;
	}
	sbi(P_CS, B_CS);
	HAL_GPIO_WritePin(LCD_nCS_GPIO_Port,LCD_nCS_Pin, GPIO_PIN_SET);
}
#endif
void UTFT::setContrast(char c)
{
	cbi(P_CS, B_CS);
	switch (display_model)
	{
	case PCF8833:
		if (c>64) c=64;
		LCD_Write_COM(0x25);
		LCD_Write_DATA(c);
		break;
	}
	sbi(P_CS, B_CS);
}


void UTFT::setBrightness(byte br)
{
	cbi(P_CS, B_CS);
	switch (display_model)
	{
	case CPLD:
		if (br>16) br=16;
		LCD_Write_COM_DATA(0x01,br);
		LCD_Write_COM(0x0F);   
		break;
	}
	sbi(P_CS, B_CS);
}

void UTFT::setDisplayPage(byte page)
{
	cbi(P_CS, B_CS);
	switch (display_model)
	{
	case CPLD:
		if (page>7) page=7;
		LCD_Write_COM_DATA(0x04,page);
		LCD_Write_COM(0x0F);   
		break;
	}
	sbi(P_CS, B_CS);
}

void UTFT::setWritePage(byte page)
{
	cbi(P_CS, B_CS);
	switch (display_model)
	{
	case CPLD:
		if (page>7) page=7;
		LCD_Write_COM_DATA(0x05,page);
		LCD_Write_COM(0x0F);   
		break;
	}
	sbi(P_CS, B_CS);
}

#endif

//#include "tft_drivers/ili9320/cpp.h"
//#include "tft_drivers/ili9325d/alt/cpp.h"
//#include "tft_drivers/ili9341/cpp.h"
//#include "tft_drivers/ili9481/cpp.h"
//#include "tft_drivers/ili9486/cpp.h"
//#include "tft_drivers/SPFD5420/cpp.h"
