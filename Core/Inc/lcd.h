/*
 * lcd.h
 *
 *  Created on: Jun 10, 2022
 *      Author: pszymanski
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#define BLACK     0x0000
#define RED       0xf800
#define GREEN     0x07e0
#define BLUE      0x001f
#define YELLOW    0xffe0
#define MAGENTA   0xf81f
#define CYAN      0x07ff
#define WHITE     0xffff
#define LCD_WIDTH	160
#define LCD_HEIGHT	128

#pragma once

#include "stdint.h"

void lcd_init(void);
void lcd_fill_box(int x, int y, int width, int height, uint16_t color);
void lcd_sleepin(void);
void lcd_sleepout(void);
void lcd_invoff(void);
void lcd_invon(void);
void lcd_drawing_test(void);
void lcd_draw_point(int x, int y, uint16_t color);
void lcd_colorize_pixel(int x, int y, uint16_t color);
void lcd_draw_image(int x, int y, int width, int height, uint8_t *data);
void lcd_draw_image_fast(int x, int y, int width, int height, uint8_t *data);
void lcd_fill_box_fast(int x, int y, int width, int height, uint16_t color);

#endif /* INC_LCD_H_ */
