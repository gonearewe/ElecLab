#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"
#include "24cxx.h"
#include "w25qxx.h"
#include "touch.h"
#include "spi.h"

char cells[4][4] = {
	{'1', '2', '3', 'X'},
	{'4', '5', '6', ' '},
	{'7', '8', '9', 'Y'},
	{' ', '0', ' ', 'B'},
};

void user_set_temp(void);

void draw_keyboard(u16 sx, u16 sy, u8 cell_w, u8 cell_h, short clicked_id, char cells[4][4]);

short locate_click(u16 sx, u16 sy, u8 cell_w, u8 cell_h, u16 click_x, u16 click_y);

int main(void)
{
	delay_init();									//��ʱ������ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);								//���ڳ�ʼ��Ϊ115200

	LED_Init(); //LED�˿ڳ�ʼ��
	LCD_Init();
	KEY_Init();
	SPI2_Init();
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);
	tp_dev.init();

	POINT_COLOR = RED; //��������Ϊ��ɫ
	user_set_temp();
}

void user_set_temp()
{
	LCD_Clear(WHITE); //����
	while (1)
	{
		draw_keyboard(10, 200, 80, 50, -1, cells);
		while (!tp_dev.scan(0))
		{
			short id = locate_click(10, 200, 80, 50, tp_dev.x[0], tp_dev.y[0]);
			draw_keyboard(10, 200, 80, 50, id, cells);
			delay_ms(500);
		}
	}
}

void draw_keyboard(u16 sx, u16 sy, u8 cell_w, u8 cell_h, short clicked_id, char cells[4][4])
{
	LCD_Fill(sx, sy, sx + cell_w * 4, sy + cell_h * 4, WHITE);

	POINT_COLOR = YELLOW;
	for (int i = 0; i < 5; i++)
	{
		LCD_DrawLine(sx, sy + i * cell_h, sx + 4 * cell_w, sy + i * cell_h); // ��ָ���
	}
	for (int i = 0; i < 5; i++)
	{
		LCD_DrawLine(sx + i * cell_w, sy, sx + i * cell_w, sy + 4 * cell_h); // �ݷָ���
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

float read_res_factor()
{
	SPI2_ReadWriteByte(0x80); // write config register
	SPI2_ReadWriteByte(0xB1);
	delay_ms(100);					// wait for sampling and conversing
	SPI2_ReadWriteByte(0x01);		// read RTD register(MSB)
	u8 res = SPI2_ReadWriteByte(0); // read data
}