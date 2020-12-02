#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "24cxx.h"
#include "timer.h"
#include "rtc.h"
#include "touch.h"
#include "spi.h"
#include "max31865.h"

volatile Record records[5];
volatile int records_len = 0;

volatile float TEMPERATURE_UPPER_LIMIT = 50;

char cells[4][4] = {
	{'1', '2', '3', 'X'},
	{'4', '5', '6', ' '},
	{'7', '8', '9', 'Y'},
	{'-', '0', '.', 'B'},
};

void user_set_temperature(void);

void draw_keyboard(u16 sx, u16 sy, u8 cell_w, u8 cell_h, short clicked_id, char cells[4][4]);

short locate_click(u16 sx, u16 sy, u8 cell_w, u8 cell_h, u16 click_x, u16 click_y);

int main(void)
{
	delay_init();									//延时函数初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);								//串口初始化为115200
	LED_Init();										//LED端口初始化
	LCD_Init();
	KEY_Init();
	RTC_Init(); //RTC初始化
	SPI_MAX31865_Init();
	TIM3_Int_Init(4999, 7199); //10Khz的计数频率，计数到5000为500ms
	//RTC_Set(2020,11,28,21,45,0);
	tp_dev.init();

	u8 t = 0;
	u8 key;
	u8 arrow_pos = 0; // [0,4]
	while (1)
	{
		t = (t + 1) % 10;
		key = KEY_Scan(0);
		u16 x = 60, y = 200;
		if (key == WKUP_PRES)
		{
			LCD_Fill(x, y, x + 300, y + 175, WHITE);
			user_set_temperature();
			LCD_Fill(10, 200, 350, 500, WHITE);
		}
		else if (key == KEY0_PRES || key == KEY1_PRES)
		{
			POINT_COLOR = BLUE;
			if (key == KEY0_PRES)
			{
				arrow_pos = (arrow_pos + 1) % 5;
			}
			else if (key == KEY1_PRES)
			{
				arrow_pos = (arrow_pos + 4) % 5;
			}

			for (int i = 0; i < 5; i++)
			{
				if (i >= records_len)
				{
					LCD_ShowString(x, y + 16 * i, 150, 16, 16, "  <empty>");
				}
				else
				{
					char buf[20] = {0};
					sprintf(buf, "  %0d:%0d:%0d", records[i].time.hour, records[i].time.min, records[i].time.sec);
					LCD_ShowString(x, y + 16 * i, 150, 16, 16, (u8 *)buf);
				}
				LCD_ShowChar(x, y + arrow_pos * 16, '>', 16, 0);
			}
			if (arrow_pos < records_len)
			{
				LCD_ShowString(x, y + 6 * 16, 150, 24, 24, "Records:");

				char buf[35] = {0};
				sprintf(buf, "limit(Celsius): %3.0f", records[arrow_pos].temp_limit);
				LCD_ShowString(x, y + 6 * 16 + 24, 300, 24, 24, (u8 *)buf);
				for (int i = 0; i < 35; i++)
				{
					buf[i] = '\0';
				}

				sprintf(buf, "actual(Celsius): %3.0f", records[arrow_pos].actual_temp);
				LCD_ShowString(x, y + 6 * 16 + 24 * 2, 300, 24, 24, (u8 *)buf);
			}
		}
		delay_ms(10);
	}
}

void user_set_temperature(void)
{
	draw_keyboard(10, 250, 75, 50, -1, cells);
	char num[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
	u8 num_id = 0;
	while (1)
	{
		if (!tp_dev.scan(0))
		{
			continue;
		}

		short id = locate_click(10, 250, 75, 50, tp_dev.x[0], tp_dev.y[0]);
		if (id == -1)
		{
			continue;
		}

		char button = cells[id / 4][id % 4];
		if (button == 'B') // cancel and go back
		{
			break;
		}
		else if (button == 'X') // clear
		{
			for (int i = 0; i < 7; i++)
			{
				num[i] = ' ';
			}
			num_id = 0;
		}
		else if (button == 'Y') // confirm
		{
			num[7] = '\0';
			float res = atof(num);
			if (res >= -100 && res <= 200)
			{
				TEMPERATURE_UPPER_LIMIT = res; // reset limit
			}
			break;
		}
		else
		{
			num[num_id] = button;
			num_id++;
			if (num_id == 8) // overflow
			{
				for (int i = 0; i < 7; i++)
				{
					num[i] = ' ';
				}
				num_id = 0;
			}
		}

		LCD_ShowString(60, 200, 100, 24, 24, (u8 *)num);
		draw_keyboard(10, 250, 75, 50, id, cells);
		delay_ms(500);
		draw_keyboard(10, 250, 75, 50, -1, cells);
	}
}

void draw_keyboard(u16 sx, u16 sy, u8 cell_w, u8 cell_h, short clicked_id, char cells[4][4])
{
	LCD_Fill(sx, sy, sx + cell_w * 4, sy + cell_h * 4, WHITE);

	POINT_COLOR = BLUE;
	for (int i = 0; i < 5; i++)
	{
		LCD_DrawLine(sx, sy + i * cell_h, sx + 4 * cell_w, sy + i * cell_h); // 横分割线
	}
	for (int i = 0; i < 5; i++)
	{
		LCD_DrawLine(sx + i * cell_w, sy, sx + i * cell_w, sy + 4 * cell_h); // 纵分割线
	}

	if (clicked_id >= 0 && clicked_id < 4 * 4)
	{ // cell click feedback
		u16 x = sx + clicked_id % 4 * cell_w, y = sy + clicked_id / 4 * cell_h;
		LCD_Fill(x, y, x + cell_w, y + cell_h, GREEN);
	}

	POINT_COLOR = BLACK;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			u16 x = sx + j * cell_w + cell_w / 2 - 16 / 2;
			u16 y = sy + i * cell_h + cell_h / 2 - 24 / 2;
			LCD_ShowChar(x, y, (u8)cells[i][j], 24, 1);
		}
	}
}

short locate_click(u16 sx, u16 sy, u8 cell_w, u8 cell_h, u16 click_x, u16 click_y)
{
	if (click_x < sx || click_x > sx + 4 * cell_w || click_y < sy || click_y > sy + 4 * cell_h)
	{
		return -1;
	}

	return (click_y - sy) / cell_h * 4 + (click_x - sx) / cell_w;
}
