/*
	���ߣ�Mr.luo
	����ʱ�䣺2018.11.28
	�޸�ʱ�䣺2018.11.28
	���ݣ�max31865 ����ͷ�ļ�
*/

#ifndef _MAX31865_H
#define _MAX31865_H

#include "sys.h" 
#include "spi.h"
#include "stdio.h"
#include "delay.h"
#include "stdbool.h"

//STM32��ض˿ںͿ⺯���궨��
#define	SPI_FLASH_CS             PAout(4)  	//PA4��������Ƭѡ�ź�
#define SPI_ReadWriteByte        SPI1_ReadWriteByte


#define MAX31856_CONFIG_REG            0x00
#define MAX31856_CONFIG_BIAS           0x80
#define MAX31856_CONFIG_MODEAUTO       0x40
#define MAX31856_CONFIG_MODEOFF        0x00
#define MAX31856_CONFIG_1SHOT          0x20
#define MAX31856_CONFIG_3WIRE          0x10
#define MAX31856_CONFIG_24WIRE         0x00
#define MAX31856_CONFIG_FAULTSTAT      0x02
#define MAX31856_CONFIG_FILT50HZ       0x01
#define MAX31856_CONFIG_FILT60HZ       0x00

#define MAX31856_RTDMSB_REG           0x01
#define MAX31856_RTDLSB_REG           0x02
#define MAX31856_HFAULTMSB_REG        0x03
#define MAX31856_HFAULTLSB_REG        0x04
#define MAX31856_LFAULTMSB_REG        0x05
#define MAX31856_LFAULTLSB_REG        0x06
#define MAX31856_FAULTSTAT_REG        0x07


#define MAX31865_FAULT_HIGHTHRESH     0x80
#define MAX31865_FAULT_LOWTHRESH      0x40
#define MAX31865_FAULT_REFINLOW       0x20
#define MAX31865_FAULT_REFINHIGH      0x10
#define MAX31865_FAULT_RTDINLOW       0x08
#define MAX31865_FAULT_OVUV           0x04


#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7


typedef enum max31865_numwires { 
  MAX31865_2WIRE = 0,
  MAX31865_3WIRE = 1,
  MAX31865_4WIRE = 0
} max31865_numwires_t;            //����ģʽö�ٶ���

void SPI_MAX31865_Init(void);
void setWires(max31865_numwires_t wires);  //RTD����ģʽ����
void autoConvert(bool b);  //�����Զ�ת��ģʽ
void enableBias(bool b);   //ʹ��ƫִ��ѹ
u8  writetest(u8 n);       //SPI���Ժ���

u8  readRegister8(u8 addr);    //���Ĵ�����8λ
u16 readRegister16(u8 addr);   //�������Ĵ�����16λ
void readRegisterN(u8 addr, u8 buffer[], u8 n);          //��N���Ĵ���

void     writeRegister8(u8 addr, u8 data);                //ָ���Ĵ���д8λ����

u16 readRTD(void);             //RTD���ݶ�ȡ
float temperature(float RTDnominal, float refResistor);  //�¶ȼ���

u8 readFault(void);        //���ϼ��
void clearFault(void);     //������ϱ�־λ
u8 MAX31865(u8 spi_cs, u8 spi_mosi, u8 spi_miso, u8 spi_clk);

#endif
