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
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );	  //使能PORTA通道时钟
   	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE );	  //使能DAC通道时钟 
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;			 // 端口配置
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		 //模拟输入
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);			           	 //  PA.4 输出高
					
	DAC_InitType.DAC_Trigger=DAC_Trigger_T4_TRGO;
	DAC_InitType.DAC_WaveGeneration=DAC_WaveGeneration_Triangle;
	DAC_InitType.DAC_LFSRUnmask_TriangleAmplitude=DAC_TriangleAmplitude_4095;
	DAC_InitType.DAC_OutputBuffer=DAC_OutputBuffer_Enable ;	//DAC1输出缓存关闭 BOFF1=1
    DAC_Init(DAC_Channel_1,&DAC_InitType);	 //初始化DAC通道1
 
	DAC_Cmd(DAC_Channel_1, ENABLE);  //使能DAC1
}

void Tim4_Init(u16 arr,u16 psc)
 {
	 TIM_TimeBaseInitTypeDef TIM_TimeBaseInitstructure;
	 
	 /*************************定时器时钟源使能*********************************/
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	 
	 /*************************定时器基本配置*********************************/
	 TIM_TimeBaseInitstructure.TIM_Prescaler = psc;//预分频系数
	 TIM_TimeBaseInitstructure.TIM_Period = arr;//自动装在值
	 TIM_TimeBaseInitstructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
	 TIM_TimeBaseInitstructure.TIM_ClockDivision = TIM_CKD_DIV1;    
	 TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitstructure);
	 
	 /*************************设置TIM4更新事件作为外部触发源*********************************/
	 TIM_SelectOutputTrigger(TIM4,TIM_TRGOSource_Update);//更新作为触发源
	 /*************************使能TIM4计数器*********************************/
	 TIM_Cmd(TIM4,ENABLE);
 }

float x[512];
int main(void)
{
	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);								//串口初始化为115200
	Adc_Init();
	LED_Init(); //LED端口初始化
	LCD_Init();
	KEY_Init();
	AD9834_Init();
	AD9834_Select_Wave(Sine_Wave);//Sine_Wave  Square_Wave  Triangle_Wave
	AD9834_Set_Freq(FREQ_0, 1000);
	// AD9833_Init();
	// AD9833_SetFrequencyQuick(0,AD9833_OUT_TRIANGLE  );
	
	Tim4_Init(1,0); // T = (878+1)*(71+1)/72M
	Dac1_Init();				//DAC初始化 
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
