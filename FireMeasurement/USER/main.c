#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "adc.h"
#include "timer.h"
#include "fire.h"

/************************************************
 ALIENTEK��ӢSTM32������ʵ��17
 ADC ʵ��   
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

int main(void)
{
	u16 adcx;
	float temp;
	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);								//���ڳ�ʼ��Ϊ115200
	LED_Init();										//LED�˿ڳ�ʼ��
	LCD_Init();
	TIM3_Int_Init(99, 7199); // 100Hz
	Adc_Init();				 //ADC��ʼ��

	POINT_COLOR = BLUE; //��������Ϊ��ɫ
	LCD_ShowString(60, 130, 200, 16, 16, "ADC_CH0_VAL:");
	LCD_ShowString(60, 150, 200, 16, 16, "ADC_CH0_VOL:0.000V");
	int x=50,y=200;
	for(int i=0;i<500;i++){
		for(int j=0,c=outline[i][j];c!=0;j++,c=outline[i][j]){
			LCD_Fast_DrawPoint(x+c,y+i,RED);
		}
	}
	while (1)
	{
		adcx = Get_Adc_Average(ADC_Channel_1, 10);
		LCD_ShowxNum(156, 130, adcx, 4, 16, 0); //��ʾADC��ֵ
		temp = (float)adcx * (3.3 / 4096);
		adcx = temp;
		LCD_ShowxNum(156, 150, adcx, 1, 16, 0); //��ʾ��ѹֵ
		temp -= adcx;
		temp *= 1000;
		LCD_ShowxNum(172, 150, temp, 3, 16, 0X80);
		LED0 = !LED0;
		delay_ms(250);
	}
}


