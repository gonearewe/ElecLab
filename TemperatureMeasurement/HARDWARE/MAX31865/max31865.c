#include "max31865.h"
#include <math.h>

//初始化SPI max31865 的IO口
void SPI_MAX31865_Init(void)
{
  SPI1_Init();                             //初始化SPI1
  SPI1_SetSpeed(SPI_BaudRatePrescaler_64); //设置为 72/64 = 1.125 M时钟
  writeRegister8(0x00, 0x00);              //清除配置寄存器
  enableBias(1);                           //使能偏置电压
  delay_ms(10);                            //等待10ms使得RTDIN的滤波电容充电
  setWires(MAX31865_3WIRE);                //使能PT1000 三线工作模式
  clearFault();                            //清除故障检测位
}

//RTD接线模式设置
void setWires(max31865_numwires_t wires)
{
  uint8_t t = readRegister8(MAX31856_CONFIG_REG);
  if (wires == MAX31865_3WIRE)
  {
    t |= MAX31856_CONFIG_3WIRE;
  }
  else
  {
    // 2 or 4 wire
    t &= ~MAX31856_CONFIG_3WIRE;
  }
  writeRegister8(MAX31856_CONFIG_REG, t);
}

//设置自动转换模式
void autoConvert(bool b)
{
  uint8_t t = readRegister8(MAX31856_CONFIG_REG);
  if (b)
  {
    t |= MAX31856_CONFIG_MODEAUTO; // enable autoconvert
  }
  else
  {
    t &= ~MAX31856_CONFIG_MODEAUTO; // disable autoconvert
  }
  writeRegister8(MAX31856_CONFIG_REG, t);
}

//使能偏执电压
void enableBias(bool b)
{
  uint8_t t = readRegister8(MAX31856_CONFIG_REG);
  if (b)
  {
    t |= MAX31856_CONFIG_BIAS; // enable bias
  }
  else
  {
    t &= ~MAX31856_CONFIG_BIAS; // disable bias
  }
  writeRegister8(MAX31856_CONFIG_REG, t);
}

//读寄存器，8位
u8 readRegister8(u8 addr)
{
  uint8_t ret = 0;
  addr &= 0x7F; // make sure top bit is set to 1
  SPI_FLASH_CS = 0;
  delay_us(100);
  SPI_ReadWriteByte(addr);       //发送写取状态寄存器命令
  ret = SPI_ReadWriteByte(0xff); //写入一个字节
  delay_us(100);
  SPI_FLASH_CS = 1; //取消片选
  return ret;
}

//读两个寄存器，16位
u16 readRegister16(u8 addr)
{
  uint8_t buffer[2] = {0, 0};
  uint16_t ret = 0;
  readRegisterN(addr, buffer, 2);

  ret = buffer[0];
  ret <<= 8;
  ret |= buffer[1];

  return ret;
}

//读N个寄存器
void readRegisterN(u8 addr, u8 buffer[], u8 n)
{
  addr &= 0x7F; // make sure top bit is 0
  SPI_FLASH_CS = 0;
  delay_us(100);
  SPI_ReadWriteByte(addr); //发送读取状态寄存器命令
  while (n--)
  {
    buffer[0] = SPI_ReadWriteByte(0Xff);
    buffer++;
  }
  delay_us(100);
  SPI_FLASH_CS = 1; //取消片选
}

//指定寄存器写8位数据
void writeRegister8(u8 addr, u8 data)
{
  addr |= 0x80; // make sure top bit is set to 1
  SPI_FLASH_CS = 0;
  delay_us(100);
  SPI_ReadWriteByte(addr); //发送读取状态寄存器命令
  SPI_ReadWriteByte(data); //写入一个字节
  delay_us(100);
  SPI_FLASH_CS = 1; //取消片选
}

//RTD数据读取
u16 readRTD()
{
  uint8_t t = 0;
  uint16_t rtd = 0;
  t = readRegister8(MAX31856_CONFIG_REG);
  t |= MAX31856_CONFIG_1SHOT;
  writeRegister8(MAX31856_CONFIG_REG, t);
  delay_ms(70); //单次转换读取时间在片选信号拉高后在50HZ工作模式下需要约62ms，60hz约52ms
  rtd = readRegister16(MAX31856_RTDMSB_REG);
  // remove fault
  rtd >>= 1;

  return rtd;
}

float lookup_table(float res)
{
  const int len = 16;
  const float table[len] = {-125.5,-100.4,-75.3,-50.6,-25.4,0.0,25.4,51.4,77.7,103.6,130.5,157.8,184.2,211.2,238.1,266.0};
  const float k = (295 + 125) / (210.24 - 50.06);
  const float b = 295 - 210.24 * k;
  float y = k * res + b; // 拟合
  int id = (y + 126) / 26 + 0.5;
  id = id < 0 ? 0 : id;
  id = id > len - 1 ? len - 1 : id;
  return table[id];
}

//温度计算
float temperature(float RTDnominal, float refResistor)
{
  float Z1, Z2, Z3, Z4, Rt, temp;
  float rpoly;
  Rt = readRTD();
  Rt /= 32768;
  Rt *= refResistor;
  return lookup_table(Rt);

  // Z1 = -RTD_A;
  // Z2 = RTD_A * RTD_A - (4 * RTD_B);
  // Z3 = (4 * RTD_B) / RTDnominal;
  // Z4 = 2 * RTD_B;

  // temp = Z2 + (Z3 * Rt);
  // temp = (sqrt(temp) + Z1) / Z4;

  // if (temp >= 0)
  //   return temp;

  // // ugh.
  // rpoly = Rt;

  // temp = -242.02;
  // temp += 2.2228 * rpoly;
  // rpoly *= Rt; // square
  // temp += 2.5859e-3 * rpoly;
  // rpoly *= Rt; // ^3
  // temp -= 4.8260e-6 * rpoly;
  // rpoly *= Rt; // ^4
  // temp -= 2.8183e-8 * rpoly;
  // rpoly *= Rt; // ^5
  // temp += 1.5243e-10 * rpoly;

  // return temp;
}

//故障检测
u8 readFault(void)
{
  return readRegister8(MAX31856_FAULTSTAT_REG);
}

//清除故障标志位
void clearFault(void)
{
  uint8_t t = readRegister8(MAX31856_CONFIG_REG);
  t &= ~0x2C;
  t |= MAX31856_CONFIG_FAULTSTAT;
  writeRegister8(MAX31856_CONFIG_REG, t);
}

//SPI测试函数
u8 writetest(u8 n)
{
  u8 ret;
  SPI_FLASH_CS = 0;
  ret = SPI_ReadWriteByte(n); //发送读取状态寄存器命令
  SPI_FLASH_CS = 1;           //取消片选
  return ret;
}
