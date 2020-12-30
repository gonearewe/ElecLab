#include "spi.h"
#include "ad9833.h"
#include "delay.h"

#define FCLK 25000000	//设置晶振频率
//#define RealFreDat    268435456.0/FCLK//总的公式为 Fout=（Fclk/2的28次方）*28位寄存器的值
double RealFreDat = 268435456.0/FCLK;
/********往ad9833写******************************/
unsigned char AD9833_SPI_Write(unsigned char* data,
                        unsigned char bytesNumber)
{
	unsigned char i =0;
	unsigned char writeData[5]	= {0,0, 0, 0, 0};
	AD9833_FSYNC =0;
	for(i = 0;i < bytesNumber;i ++)
	{
		writeData[i] = data[i + 1];
	}
	for(i=0 ;i<bytesNumber ;i++) 
	{					 	
	//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET); //检查指定的SPI标志位设置与否:发送缓存空标志位
		SPI2_ReadWriteByte(writeData[i]);
	}
	AD9833_FSYNC = 1;
	delay_us(5);
	return i;	
}

void AD9833_SetRegisterValue(unsigned short regValue)
{
	unsigned char data[5] = {0x03, 0x00, 0x00};	
	
	data[1] = (unsigned char)((regValue & 0xFF00) >> 8);
	data[2] = (unsigned char)((regValue & 0x00FF) >> 0);    
	AD9833_SPI_Write(data,2);
}

void AD9833_SetFrequency(unsigned short reg, float fout)
{
	unsigned short freqHi = reg;
	unsigned short freqLo = reg;
	unsigned long val=RealFreDat*fout;//F寄存器的值
	freqHi |= (val & 0xFFFC000) >> 14 ;
	freqLo |= (val & 0x3FFF);
	AD9833_SetRegisterValue(AD9833_B28);
	AD9833_SetRegisterValue(freqLo);
	AD9833_SetRegisterValue(freqHi);
}

void AD9833_SetFrequencyQuick(float fout,unsigned short type)
{
	AD9833_SetFrequency(AD9833_REG_FREQ0, fout*1000);// 400 kHz
	AD9833_Setup(AD9833_FSEL0, AD9833_PSEL0, type);
}
unsigned char AD9833_Init(void)
{
    SPI2_Init();	
    AD9833_SetRegisterValue(AD9833_REG_CMD | AD9833_RESET);
    return (1);
}


void AD9833_Setup(unsigned short freq,
				  unsigned short phase,
			 	  unsigned short type)
{
	unsigned short val = 0;
	
	val = freq | phase | type;
	AD9833_SetRegisterValue(val);
}
