#include "main.h"
#include "LCD12881.h"
extern I2C_HandleTypeDef hi2c1;

/*
 程序代码基于STM32 HAL库
 其他单片机或者使用标准库的需修改以下4个函数
*/
void delay_us(uint16_t time)
{    
	uint32_t temp;
	SysTick->LOAD=time*72;//9为72/8 时间加载
	SysTick->VAL=0x00;//清空计数器
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;//启动计数
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));//等待计数到达
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//关闭计数
	SysTick->VAL=0x00;//清空计数器
}
void delay_ms(uint16_t time)
{    
   uint32_t temp;

   while(time--)
   {
      SysTick->LOAD=72*1000;//9为72/8 时间加载
			SysTick->VAL=0x00;//清空计数器
			SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;//启动计数		 
			do
			{
				temp=SysTick->CTRL;
			}while((temp&0x01)&&!(temp&(1<<16)));//等待计数到达			
			SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;//关闭计数			
			SysTick->VAL=0x00;//清空计数器 			
   }
}

void iic_send_cmd(uint8_t cmd)
{
	HAL_I2C_Master_Transmit(&hi2c1, UC1617_CMD_ADDR, &cmd, 1, 10);	
}
void iic_send_data(uint8_t* Data,uint16_t data_num)
{
	HAL_I2C_Master_Transmit(&hi2c1, UC1617_DATA_ADDR, Data, data_num, 100);	
}


/************************************************/
void LCD_12881_init(void)
{
	//拉低复位引脚10us 等待至少150ms
	HAL_GPIO_WritePin(GPIOB, LCD_RST_Pin,GPIO_PIN_SET);
	delay_us(100);
	HAL_GPIO_WritePin(GPIOB, LCD_RST_Pin,GPIO_PIN_RESET);
	delay_us(10);
	HAL_GPIO_WritePin(GPIOB, LCD_RST_Pin,GPIO_PIN_SET);
	delay_ms(170);

//	iic_send_cmd(0xe2);//软件复位
//	delay_ms(6);
	
	iic_send_cmd(0x31);//set APC command
	iic_send_cmd(0x00);
	
	iic_send_cmd(0x24);//温度补偿	
	iic_send_cmd(0xc4);//正向显示
		
	iic_send_cmd(0xD2);//灰度等级
	iic_send_cmd(0xEB);
	
	iic_send_cmd(0x81);
	iic_send_cmd(80);//调节对比度[0~193]
	
	iic_send_cmd(0xF4);
	iic_send_cmd(0);
	iic_send_cmd(0xF6);
	iic_send_cmd(20);
	iic_send_cmd(0xF5);
	iic_send_cmd(0);
	iic_send_cmd(0xF7);
	iic_send_cmd(127);
	
	iic_send_cmd(0xF9);
	
		
	LCD_Fill(1);	
	iic_send_cmd(0xAF);//打开显示
	delay_ms(12);
	
//	iic_send_cmd(0xa5);//全黑
//	iic_send_cmd(0xa7);//反色

}

//设置行地址和页地址
void setPageAndRow( uint8_t page, uint8_t row )
{
	iic_send_cmd(0x60 |   ( row        & 0x0F ));
	iic_send_cmd(0x70 | ( ( row >> 4 ) & 0x07 ));
	iic_send_cmd(0x00 |   ( page       & 0x1F ));
}

void set_XY( uint8_t x, uint8_t y )
{
  uint8_t page = x / 4;                           // page
  uint8_t q    = x % 4;                           // quad
  uint8_t row  = y;

  setPageAndRow( page, row );
}



void write_uint8_t( uint8_t page, uint8_t row, uint8_t b )
{
  setPageAndRow( page, row );

  iic_send_data(&b,1);
}
//清屏
void LCD_Fill( uint8_t color )
{
	uint8_t c = color & 0x03;
	uint8_t b = ( c << 6 ) | ( c << 4 ) | ( c << 2 ) | c;
	uint8_t txdata[32];
iic_send_cmd(0xAE);
	for(uint8_t i=0;i<32;i++)
		txdata[i]=b;

  for (uint8_t i = 0; i < 128; i++ )
  {       
		setPageAndRow( 0, i );
	  HAL_I2C_Master_Transmit(&hi2c1, 0x72, txdata, 21, 10);		
  }
iic_send_cmd(0xAF);
}

void LCD_draw_image(const uint8_t * image_data)
{
	uint8_t n=0;

iic_send_cmd(0xAE);
	for (uint8_t i = 0; i < 128; i++ )
  {       
		setPageAndRow( 0, i );
	  iic_send_data((uint8_t *)image_data+i*21,21);

  }
	iic_send_cmd(0xAF);
}
void LCD_8x16(uint8_t x0,uint8_t y0,uint8_t upColor,uint8_t downColor,int8_t *CHAR)//显示8*16的字符，参数分别为起点x0 yo,字符颜色，底纹颜色，字符串
{
	uint8_t x=x0,i=0,j,c,s;
	uint8_t tx_data[2];
	iic_send_cmd(0xAE);
	while(CHAR[i]!='\0')
	{
		for(j=0;j<16;j++)
		{
			setPageAndRow( x, y0+j );//定位显示区域
			
			c=F8X16[(CHAR[i]-32)][j];
			for(s=0;s<8;s++)
			{
				tx_data[s/4]>>=2;
				if((c&0x01)==0x01)
					tx_data[s/4]|=(upColor&0x03)<<6;
				else 
					tx_data[s/4]|=(downColor&0x03)<<6;
				
				c>>=1;
			}
			
			iic_send_data(tx_data,2);			
		}
		i++;
		x+=2;	
	}
	iic_send_cmd(0xAF);
}