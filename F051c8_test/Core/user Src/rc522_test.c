#include "rc522_include.h"
#include "MFRC522.h"

unsigned char data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
//M1卡的某一块写为如下格式，则该块为钱包，可接收扣款和充值命令
//4字节金额（低字节在前）＋4字节金额取反＋4字节金额＋1字节块地址＋1字节块地址取反＋1字节块地址＋1字节块地址取反 

unsigned char data2[4]  = {0,0,0,0x01};
unsigned char DefaultKey[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

unsigned char g_ucTempbuf[20]; 

extern UART_HandleTypeDef huart1;



void set_RST(GPIO_PinState state)
{
	HAL_GPIO_WritePin(RC522_GPIO, RC522_RST, state);
}

void set_CS(GPIO_PinState state)
{
	HAL_GPIO_WritePin(RC522_GPIO, RC522_SPI_CS, state);
}

void set_SCK(GPIO_PinState state)
{
	HAL_GPIO_WritePin(RC522_GPIO, RC522_SPI_SCK, state);
}

void set_MOSI(GPIO_PinState state)
{
	HAL_GPIO_WritePin(RC522_GPIO, RC522_SPI_MOSI, state);
}

GPIO_PinState get_MISO(void)
{
	return HAL_GPIO_ReadPin(RC522_GPIO , RC522_SPI_MISO);
}

void rc522_resetPin(void)
{
	set_RST(GPIO_PIN_SET);
	set_CS(GPIO_PIN_SET);
	set_SCK(GPIO_PIN_SET);
	set_MOSI(GPIO_PIN_SET);

}

void rc522_init(void)
{
	GPIO_InitTypeDef gpio;

	//gpio.Alternate = GPIO_AF0_USART1
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pin = RC522_RST | RC522_SPI_CS | RC522_SPI_SCK | RC522_SPI_MOSI;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(RC522_GPIO, &gpio);

	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pin = RC522_SPI_MISO;
	gpio.Pull = GPIO_PULLUP;
	gpio.Speed = GPIO_SPEED_HIGH;

	HAL_GPIO_Init(RC522_GPIO, &gpio);

}


void rc522_run(void)
{
	//rc522_init();

	unsigned char status,i;
	unsigned int temp;

	rc522_init();
	
	printf("开始工作。。。。。\n");
	rc522_resetPin();
	PcdReset();
	PcdAntennaOff(); 
	PcdAntennaOn(); 
	while ( 1 )
	{
		status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
 		if (status != MI_OK)
 		{
 			rc522_resetPin();
			PcdReset();
			PcdAntennaOff(); 
			PcdAntennaOn(); 
			continue;
		}
	
		printf("卡的类型:");
		
		for(i=0;i<2;i++)
		{
			temp=g_ucTempbuf[i];
			printf("%X",temp);
		
		}

		status = PcdAnticoll(g_ucTempbuf);//防冲撞
		if (status != MI_OK)
		{ continue; }

 
	////////以下为超级终端打印出的内容////////////////////////

		printf("卡序列号：");	//超级终端显示,
		
		for(i=0;i<4;i++)
		{
			temp=g_ucTempbuf[i];
			printf("%X",temp);
		}

	///////////////////////////////////////////////////////////

		status = PcdSelect(g_ucTempbuf);//选定卡片
		if (status != MI_OK)
		{ continue; }
 
		status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//验证卡片密码
		if (status != MI_OK)
		{ continue; }
 
		status = PcdWrite(1, data1);//写块
		if (status != MI_OK)
		{ continue; }

		while(1)
		{
			status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
			if (status != MI_OK)
			{
				//InitializeSystem( );
				rc522_resetPin();
				PcdReset();
				PcdAntennaOff(); 
				PcdAntennaOn(); 
				continue;
			}

			status = PcdAnticoll(g_ucTempbuf);//防冲撞
			if (status != MI_OK)
			{ continue; }

			status = PcdSelect(g_ucTempbuf);//选定卡片
			if (status != MI_OK)
			{ continue; }

			status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//验证卡片密码
			if (status != MI_OK)
			{ continue; }


			status = PcdValue(PICC_DECREMENT,1,data2);//扣款
			if (status != MI_OK)
			{ continue; }

			status = PcdBakValue(1, 2);//块备份
			if (status != MI_OK)
			{ continue; }

			status = PcdRead(2, g_ucTempbuf);//读块
			if (status != MI_OK)
			{ continue; }
			
			printf("卡读块："); //超级终端显示,
			for(i=0;i<16;i++)
			{
				temp=g_ucTempbuf[i];
				printf("%X",temp);
				
			}

			printf("\n");
			/*
			LED_GREEN = 0;
			delay1(100);
			LED_GREEN = 1;
			delay1(100);
			LED_GREEN = 0;
			delay1(200);
			LED_GREEN = 1;
			*/
				PcdHalt();
			}
	}

}



int fputc(int ch, FILE *f)
{
	unsigned char temp;
	//USART_SendData(DEBUG_USART, (unsigned char) ch);
	temp = (unsigned char)ch;
	HAL_UART_Transmit(&huart1 , &temp , 1 , 10);
	while(!(huart1.Instance->ISR & 0x00000040));
	//while (!(DEBUG_USART->SR & USART_FLAG_TXE));
	return (ch);
}


