/*
 * lcd.c
 *
 *  Created on: Jun 10, 2022
 *      Author: pszymanski
 */

#include "lcd.h"
#include "spi.h"

#define ST7735S_INVON			0x21
#define ST7735S_INVOFF			0x20
#define ST7735S_SLPIN			0x10
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

#define CMD(x)					((x)|0x100)

#define LCD_OFFSET_X			1
#define LCD_OFFSET_Y			2

static void lcd_cmd(uint8_t cmd){
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_data(uint8_t data){
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_send(uint16_t value){
	if(value & 0x100){
		lcd_cmd(value);
	} else{
		lcd_data(value);
	}
}


static void lcd_data16(uint16_t value){
	lcd_data(value >> 8);
	lcd_data(value);
}

static void lcd_set_window(int x, int y, int width, int height){
	lcd_cmd(ST7735S_CASET);
	lcd_data16(x);
	lcd_data16(x + width - 1);

	lcd_cmd(ST7735S_RASET);
	lcd_data16(y);
	lcd_data16(y + height - 1);

}

void lcd_fill_box(int x, int y, int width, int height, uint16_t color){
	lcd_set_window(x, y, width, height);

	lcd_cmd(ST7735S_RAMWR);
	for(int i=0; i< width * height; i++)
		lcd_data16(color);
}

void lcd_fill_box_fast(int x, int y, int width, int height, uint16_t color){
	lcd_set_window(x, y, width, height);

	lcd_cmd(ST7735S_RAMWR);

	uint8_t data_to_send[width*height*2];
	for(int i=0; i< width * height * 2; i++){
		data_to_send[i] = color >> 8;
		data_to_send[++i] = color;
	}

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi2, data_to_send, width*height*2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);



}

void lcd_sleepin(void){
	lcd_cmd(ST7735S_SLPIN);
	HAL_Delay(120);
}

void lcd_sleepout(void){
	lcd_cmd(ST7735S_SLPOUT);
	HAL_Delay(120);
}


static const uint16_t init_table[] = {
		CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
		CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
		CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
		CMD(ST7735S_INVCTR), 0x07,
		CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
		CMD(ST7735S_PWCTR2), 0xc5,
		CMD(ST7735S_PWCTR3), 0x0a, 0x00,
		CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
		CMD(ST7735S_PWCTR5), 0x8a, 0xee,
		CMD(ST7735S_VMCTR1), 0x0e,
		CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
							   0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
		CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         	   0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
		CMD(0xf0), 0x01,
		CMD(0xf6), 0x00,
		CMD(ST7735S_COLMOD), 0x05,
		CMD(ST7735S_MADCTL), 0xa0
};


void lcd_init(void){
	int i;
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	for(i=0; i< sizeof init_table / sizeof (uint16_t); i++){
		lcd_send(init_table[i]);
	}

	HAL_Delay(200);
	lcd_cmd(ST7735S_SLPOUT);
	HAL_Delay(120);
	lcd_cmd(ST7735S_DISPON);
}

void lcd_invon(){
	lcd_cmd(ST7735S_INVON);
}

void lcd_invoff(){
	lcd_cmd(ST7735S_INVOFF);
}

void lcd_drawing_test(void){
	lcd_cmd(ST7735S_CASET);
	lcd_data16(50);
	lcd_data16(100);

	lcd_cmd(ST7735S_RASET);
	lcd_data16(5);
	lcd_data16(20);

	lcd_cmd(ST7735S_RAMWR);
	for(int i=0; i++<=2000; i++)
		lcd_data16(CYAN);

}

void lcd_draw_point(int x, int y, uint16_t color){
	lcd_fill_box(x, y, 1, 1, color);
}

void lcd_colorize_pixel(int x, int y, uint16_t color)
{

	// zdefiniuj pixel
	lcd_cmd(ST7735S_CASET);
	lcd_data16(x);
	lcd_data16(x+1);

	lcd_cmd(ST7735S_RASET);
	lcd_data16(y);
	lcd_data16(y+1);

	// pokoloruj
	lcd_cmd(ST7735S_RAMWR);
	lcd_data16(color);
}

void lcd_draw_image(int x, int y, int width, int height, uint8_t *data){
	int array_counter = 0;

	for(int iy=y; iy<height+y; iy++)
		for(int ix=x ; ix<width+x; ix++){
			uint16_t mcolor = data[array_counter] << 8;
			array_counter++;
			mcolor |= (uint16_t) data[array_counter];
			array_counter++;
			lcd_colorize_pixel(ix, iy, mcolor);
		}
}

void lcd_draw_image_fast(int x, int y, int width, int height, uint8_t *data){

	lcd_cmd(ST7735S_CASET);
	lcd_data16(x);
	lcd_data16(x+width-1);

	lcd_cmd(ST7735S_RASET);
	lcd_data16(y);
	lcd_data16(y+height-1);

	lcd_cmd(ST7735S_RAMWR);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi2, data, width*height*2, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
