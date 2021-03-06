#include "cog12864.h"
#include <msp430f5529.h>
#include "asciifonts.h"
#include "chinesefonts.h"
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
/* Global buffer for the display. This is especially useful on 1BPP, 2BPP, and 4BPP
displays. This allows you to update pixels while reading the neighboring pixels
from the buffer instead of a read from the LCD controller. A buffer is not required
as a read followed by a write can be used instead.*/

void delay_nms(uint ms)
{
  uint k,j;
  for(k=0;k<ms;k++)
  {for (j=250;j>0;j--);}  //延时1 ms	// 25000即当前时钟设在25MHz
}
/*****************************************************************************
功         能：数据移位 8bit 最高位在前）
适用范围：Template_Driver.c 内部函数
参         数： byte: 要写入的 1 Byte数据
返 回  值：   无
*****************************************************************************/
void SendByte(uchar byte)
{
  uchar i=0;
  for(i=0;i<8;i++)
  {
    LCD_SCK_L;
    if((byte<<i)&0x80)
      LCD_SDA_H;
    else
      LCD_SDA_L;
    LCD_SCK_H;
  }
}
/*****************************************************************************
功         能：向COG屏幕中连续写入cnt个数据
适用范围：Template_Driver.c 内部函数
参         数： ptr: 要写入的数据指针
cnt:指定数据个数
返 回  值：   无
*****************************************************************************/
void Write_Ndata( uchar *ptr ,uint cnt)
{
  uint i=0;
  uchar temp=0;
  uchar *tPtr;
  tPtr = ptr;
  LCD_CS_L;
  LCD_AO_H;
  for(i=0;i<cnt;i++)		// 连续写入
  {
    temp = *tPtr++;		// 后来写入字体覆盖之前的图形
    SendByte(temp);
  }
  LCD_CS_H;
}

void lcm()
{
  uchar i,j;
  i=0;j=0;
  for(i=0;i<16;i++)
  {
    LcdMemory[i+j*128]=chinese[i+j*16];
  }
  j++; 
  for(i=0;i<16;i++)
  {
    LcdMemory[i+j*128]=chinese[i+j*16];
  }
  for(i=0;i<6;i++)
  {
    SetAddress(0,i);
    Write_Ndata(LcdMemory+(uint)i*128,128);
  }
}

//*****************************************************************************
//
// Suggested functions to help facilitate writing the required functions below
//
//*****************************************************************************

// Writes data to the LCD controller
void WriteData(uint usData)
{
  /* Write data to the LCD controller. For instance this can be bit banged 
  with 6800 or 8080 protocol or this could be the SPI routine for a SPI LCD */
  LCD_CS_L;									//打开使能
  LCD_AO_H;									//写数据，AO（RS）为高电平
  SendByte(usData);
  LCD_CS_H;
}

// Writes a command to the LCD controller
void WriteCommand(uchar ucCommand)
{
  /* This function is typically very similar (sometimes the same) as WriteData()
  The difference is that this is for the LCD to interpret commands instead of pixel
  data. For instance in 8080 protocol, this means pulling the DC line low.*/
  LCD_CS_L;									//打开使能
  LCD_AO_L;									//写指令，AO（RS）为低电平
  SendByte(ucCommand);
  LCD_CS_H;
}

void COGClear()// (移植所需改动函数)
{
  uchar x,y;
  for(y=0;y<64/8;y++)
  {
    WriteCommand(0xb0+y);
    WriteCommand(0x10);
    WriteCommand(0x00);
    for(x=0;x<128;x++)
    {
      WriteData(0);
    }
  }
}
// Sets the pixel address of the LCD driver
void SetAddress(int lX, int lY)
{
  /* This function typically writes commands (using WriteCommand()) to the 
  LCD to tell it where to move the cursor for the next pixel to be drawn. */
  WriteCommand(0xb0+lY);
  WriteCommand(0x10+(lX/16));
  WriteCommand(0x00+(lX%16));
}



// Initialize DisplayBuffer.
// This function initializes the display buffer and discards any cached data.
void InitLCDDisplayBuffer(void *pvDisplayData, uint ulValue)
{
  
  uint i=0,j=0;
  for(j=0;j<64/8;j++)
    for(i=0;i<128;i++)
      LcdMemory[j*128+i]=ulValue;
  
}

// Initializes the display driver.
// This function initializes the LCD controller
//
// TemplateDisplayFix
void  LCD_init(void)
{
  /*
  //简(0) 易(1) 数(2) 控(3) 电(4) 源(5) 测(6) 量(7) 电(8) 压(9) 流(10) 输(11) 出(12)
  //"0"位结束标志
  uchar disp0[]={0x80+0,0x80+1,0x80+2,0x80+3,0x80+4,0x80+5,0};//显示:简易数控电源
  uchar disp1[]={0x80+11,0x80+12,0x80+8,0x80+10,':',0};//显示：输出电流: 
  uchar disp2[]={0x80+11,0x80+12,0x80+8,0x80+9,':',0};//显示：输出电压:
  uchar disp3[]={0x80+6,0x80+7,0x80+8,0x80+10,':',0};//显示：目标电流:
  */
  
  
  /*Initialize the LCD controller.
  This typically looks like:
  
  InitGPIOLCDInterface();
  InitLCDDisplayBuffer();
  // Init LCD controller parameters
  // Enable LCD
  // Init Backlight
  // Clear Screen
  */
  //端口初始化
  P4DIR |= BIT0;
  P3DIR |= BIT6+BIT7+BIT4;
  P8DIR |= BIT2;
  P1DIR |= BIT2;
  P3OUT&=~BIT4;//开背光
  //InitLCDDisplayBuffer(0,0);
  
  LCD_CS_L;
  LCD_RST_L;
  delay_nms(20);
  LCD_RST_H;
  delay_nms(20);
  
  WriteCommand(0xe2);	//system reset
  delay_nms(200);
  WriteCommand(0x24);	//SET VLCD RESISTOR RATIO
  WriteCommand(0xa2);	//BR=1/9
  WriteCommand(0xa0);	//set seg direction（列地址从左到右递增）
  WriteCommand(0xc8);	//set com direction (设置屏幕显示自上而下（0xc8），还是至下而上（0xc0）)
  WriteCommand(0x2f);	//set power control
  WriteCommand(0x40);	//set scroll line
  WriteCommand(0x81);	//SET ELECTRONIC VOLUME
  WriteCommand(0x20);	//set pm: 通过改变这里的数值来改变电压
  //WriteCommand(0xa7);	//set inverse display	   a6 off, a7 on
  //WriteCommand(0xa4);	//set all pixel on
  WriteCommand(0xaf);	//set display enable
  COGClear();//清屏
  /*
  PrintString(2*8,0,disp0);//(x,y,显示数据)
  PrintString(0*8,2,disp1);
  PrintString(0*8,4,disp2);
  PrintString(0*8,6,disp3);
  */
}

/********
字母、ASCII码占8列
汉字2*8列，8*8=64个点
8*8点为1个page
********/
void PrintString(uchar col,uchar page,uchar *ch)
{
  uchar i,m,*p;
  uint n;
  while(*ch)
  {
    if(*ch&0x80)
    {
      m=16;
      n=*ch&0x7f;
      n*=32;
      p=(uchar *)chinese+n;
    }
    else
    {
      m=8;
      n=*ch-32;
      n*=16;
      p=(uchar *)asc+n;
    }
    SetAddress(col,page); 
    for(i=0;i<m;i++)  //显示一个字符的上半部分 col自动++
    {
      WriteData( *p ); 
      p++;
    }
    page++;
    SetAddress(col,page); 
    for(i=0;i<m;i++)  //显示一个字符的下半部分 col自动++
    {      
      WriteData( *p ); 
      p++;
    }
    page--;ch++;col+=m;
    if(col>121)
      break;
  }
}
// TemplateDisplayFix
void LcdDrawPoint(void *pvDisplayData, int lX, int lY,
                  uint ulValue)
{  
  
  uchar ulPageAddress, ulPixelHeight;
  uint ulBufferLocation;
  
  ulPageAddress=lY/8;
  ulPixelHeight = 0x01 << (lY%8);
  ulBufferLocation=ulPageAddress*128+lX;
  
  
  if(ulValue!=0)
  {
    LcdMemory[ulBufferLocation] |= ulPixelHeight;//  1 黑
  }
  // Black Pixel
  else
  {
    LcdMemory[ulBufferLocation] &= ~ulPixelHeight;// 0白
  }
  
  //if (ui8Flush_Flag==FLUSH_OFF)
  {
    SetAddress(lX,ulPageAddress);
    WriteData(LcdMemory[ulBufferLocation]);
  }
}

//*****************************************************************************
void LcdDrawLineH(void *pvDisplayData, int lX1, int lX2,
                  int lY, uint ulValue)
{
  /* Ideally this function shouldn't call pixel draw. It should have it's own
  definition using the built in auto-incrementing of the LCD controller and its 
  own calls to SetAddress() and WriteData(). Better yet, SetAddress() and WriteData()
  can be made into macros as well to eliminate function call overhead. */
  
  do
  {
    LcdDrawPoint(pvDisplayData, lX1, lY, ulValue);
  }
  while(lX1++ < lX2);
}

void
Template_DriverLineDrawV(void *pvDisplayData, int lX, int lY1,
                         int lY2, uint ulValue)
{
  do
  {
    LcdDrawPoint(pvDisplayData, lX, lY1, ulValue);
  }
  while(lY1++ < lY2);
}

/**************************/
//上面为最大值 31
/*********画线*************/
void draw_line(uchar y0,uchar y1,uchar col)
{
  uint i,j,k,m;
  if(y1<y0)//保证y1>=y0
  {
    i=y0;
    y0=y1;
    y1=i;
  }
  if(y0<16)
  {
    i=0xffff;
    j=0XFFFF>>(y0);
  }
  else
  {
    i=0XFFFF>>(y0-16);
    j=0;
  }
  if(y1<16)
  {
    k=0;
    m=0XFFFF<<(15-y1);
  }
  else
  {
    k=0XFFFF<<(31-y1);
    m=0xffff;
  }
  i&=k;
  j&=m;
  SetAddress(col,1);    WriteData(i>>8);
  SetAddress(col,0);    WriteData(i&0xff);
  SetAddress(col,3);    WriteData(j>>8);
  SetAddress(col,2);    WriteData(j&0xff);
}

//void print_number(uchar x1,uchar x2,uchar y,unsigned int data)
//{
//  uchar temp[1];
//  for(signed char i=x2-x1;i>=0;i--)
//  {
//    temp[0]=data%10+'0';
//    data/=10;
//    PrintString((x1+i)*8,y,temp);
//  }
//  
//}
