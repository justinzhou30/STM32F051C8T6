#include "rc522_include.h"
#include "MFRC522.h"

unsigned char data1[16] = {0x12,0x34,0x56,0x78,0xED,0xCB,0xA9,0x87,0x12,0x34,0x56,0x78,0x01,0xFE,0x01,0xFE};
//M1����ĳһ��дΪ���¸�ʽ����ÿ�ΪǮ�����ɽ��տۿ�ͳ�ֵ����
//4�ֽڽ����ֽ���ǰ����4�ֽڽ��ȡ����4�ֽڽ�1�ֽڿ��ַ��1�ֽڿ��ַȡ����1�ֽڿ��ַ��1�ֽڿ��ַȡ�� 

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
	
	printf("��ʼ��������������\n");
	rc522_resetPin();
	PcdReset();
	PcdAntennaOff(); 
	PcdAntennaOn(); 
	while ( 1 )
	{
		status = PcdRequest(PICC_REQALL, g_ucTempbuf);//Ѱ��
 		if (status != MI_OK)
 		{
 			rc522_resetPin();
			PcdReset();
			PcdAntennaOff(); 
			PcdAntennaOn(); 
			continue;
		}
	
		printf("��������:");
		
		for(i=0;i<2;i++)
		{
			temp=g_ucTempbuf[i];
			printf("%X",temp);
		
		}

		status = PcdAnticoll(g_ucTempbuf);//����ײ
		if (status != MI_OK)
		{ continue; }

 
	////////����Ϊ�����ն˴�ӡ��������////////////////////////

		printf("�����кţ�");	//�����ն���ʾ,
		
		for(i=0;i<4;i++)
		{
			temp=g_ucTempbuf[i];
			printf("%X",temp);
		}

	///////////////////////////////////////////////////////////

		status = PcdSelect(g_ucTempbuf);//ѡ����Ƭ
		if (status != MI_OK)
		{ continue; }
 
		status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//��֤��Ƭ����
		if (status != MI_OK)
		{ continue; }
 
		status = PcdWrite(1, data1);//д��
		if (status != MI_OK)
		{ continue; }

		while(1)
		{
			status = PcdRequest(PICC_REQALL, g_ucTempbuf);//Ѱ��
			if (status != MI_OK)
			{
				//InitializeSystem( );
				rc522_resetPin();
				PcdReset();
				PcdAntennaOff(); 
				PcdAntennaOn(); 
				continue;
			}

			status = PcdAnticoll(g_ucTempbuf);//����ײ
			if (status != MI_OK)
			{ continue; }

			status = PcdSelect(g_ucTempbuf);//ѡ����Ƭ
			if (status != MI_OK)
			{ continue; }

			status = PcdAuthState(PICC_AUTHENT1A, 1, DefaultKey, g_ucTempbuf);//��֤��Ƭ����
			if (status != MI_OK)
			{ continue; }


			status = PcdValue(PICC_DECREMENT,1,data2);//�ۿ�
			if (status != MI_OK)
			{ continue; }

			status = PcdBakValue(1, 2);//�鱸��
			if (status != MI_OK)
			{ continue; }

			status = PcdRead(2, g_ucTempbuf);//����
			if (status != MI_OK)
			{ continue; }
			
			printf("�����飺"); //�����ն���ʾ,
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


