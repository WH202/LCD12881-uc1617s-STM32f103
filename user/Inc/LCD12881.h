#include "main.h"
//写命令时地址为0x70，写数据时为0x72
#define UC1617_CMD_ADDR 0x70  
#define UC1617_DATA_ADDR 0x72  

extern const unsigned char gImage_666[2688];

void delay_us(uint16_t time);
void delay_ms(uint16_t time);
void iic_send_cmd(uint8_t cmd);
void iic_send_data(uint8_t* Data,uint16_t data_num);

void LCD_12881_init(void);
void LCD_Fill( uint8_t color );
void LCD_draw_image(const uint8_t * image_data);