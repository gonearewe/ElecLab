#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "24cxx.h"
#include "w25qxx.h"
#include "touch.h"
#include "ad9834.h"
#include "adc.h"
#include "timer.h"
#include "fft.h"

void Dac1_Init(void)
{
  
	GPIO_InitTypeDef GPIO_InitStructure;
	DAC_InitTypeDef DAC_InitType;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	  //ʹ��PORTAͨ��ʱ��
   	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE );	  //ʹ��DACͨ��ʱ�� 
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;			 // �˿�����
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		 //ģ������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);			           	 //  PA.4 �����
					
	DAC_InitType.DAC_Trigger=DAC_Trigger_T4_TRGO;
	DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_Triangle;
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_TriangleAmplitude_4095;
	DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Enable ;	//DAC1�������ر� BOFF1=1
    DAC_Init(DAC_Channel_1,&DAC_InitType);	 //��ʼ��DACͨ��1
 
	DAC_Cmd(DAC_Channel_1, ENABLE);  //ʹ��DAC1
}

void Tim4_Init(u16 arr,u16 psc)
 {
	 TIM_TimeBaseInitTypeDef TIM_TimeBaseInitstructure;
	 
	 /*************************��ʱ��ʱ��Դʹ��*********************************/
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	 
	 /*************************��ʱ����������*********************************/
	 TIM_TimeBaseInitstructure.TIM_Prescaler = psc;//Ԥ��Ƶϵ��
	 TIM_TimeBaseInitstructure.TIM_Period = arr;//�Զ�װ��ֵ
	 TIM_TimeBaseInitstructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
	 TIM_TimeBaseInitstructure.TIM_ClockDivision = TIM_CKD_DIV1;    
	 TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitstructure);
	 
	 /*************************����TIM4�����¼���Ϊ�ⲿ����Դ*********************************/
	 TIM_SelectOutputTrigger(TIM4,TIM_TRGOSource_Update);//������Ϊ����Դ
	 /*************************ʹ��TIM4������*********************************/
	 TIM_Cmd(TIM4,ENABLE);
 }

float x[512];
int main(void)
{
	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);								//���ڳ�ʼ��Ϊ115200
	Adc_Init();
	LED_Init(); //LED�˿ڳ�ʼ��
	LCD_Init();
	KEY_Init();
	AD9834_Init();
	AD9834_Select_Wave(Sine_Wave);//Sine_Wave  Square_Wave  Triangle_Wave
	AD9834_Set_Freq(FREQ_0, 1000);
	// AD9833_Init();
	// AD9833_SetFrequencyQuick(0,AD9833_OUT_TRIANGLE  );
	
	Tim4_Init(1,0); // T = (878+1)*(71+1)/72M
	Dac1_Init();				//DAC��ʼ�� 
	// TIM3_PWM_Init(8999,0);
	// TIM_SetCompare2(TIM3,250);
	// LED0 = 1;
	while (1)
	{
		for(int i=0;i<512;i++){
			x[i]=Get_Adc(ADC_Channel_1)*(3.3/4096);
			// delay_ms(1);
		}
		LCD_Clear(WHITE);
		for(int i=0;i<480;i++){
			LCD_DrawLine(10,i+20,10+x[i]*(280/3.3),i+20);
		}
		fft(x,512);
		float max=0;
		for(int i=1;i<512;i++){
			if(x[i]>max){
				max=x[i];
			}
		}
		LCD_ShowNum(0,0,(u32)(x[0]+0.5),4,16);
		LCD_ShowNum(50,0,(u32)(max+0.5),4,16);
		// LCD_ShowNum(50,0,Get_Adc_Average(ADC_Channel_1,200)/10,4,16);
		
	}
	
}
