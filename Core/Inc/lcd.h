/**
 * \file lcd.h
 *
 * \brief Plik naglowkowy biblioteki osblugujacej wyswietlacz TFT ST7735S
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

/**
 * \defgroup KOLORY
 *
 * \brief Kody HEX najczesciej uzywanych kolorów
 */

/** \addtogroup KOLORY
 * @{
 */
#define BLACK     0x0000

/// \ingroup KOLORY
#define RED       0xf800
#define GREEN     0x02C5
#define BLUE      0x001f
#define YELLOW    0xffe0
#define MAGENTA   0xf81f
#define CYAN      0x07ff
#define WHITE     0xffff
/** @}*/
#define LCD_WIDTH	160
#define LCD_HEIGHT	128

#pragma once

#include "stdint.h"
#include <stdbool.h>

/**
 * \brief Wybudzenie wyswietlacza i wstepna konfiguracja
 */
void lcd_init(void);

/**
 * \brief Rysuje wypelniony kolorem prostokat (tryb powolny)
 *
 * W trybie powolnym do urzadzenia wysylane sa pojedyncze pixele
 * \param x punkt startowy w osi x
 * \param y punkt startowy w osi y
 * \param width szerokosc
 * \param width wysokosc
 * \param color kolor (w wartosci 16 bitowej)
 */
void lcd_fill_box(int x, int y, int width, int height, uint16_t color);

/**
 * \brief Wprowadza wyswietlacz w uspienia (niski pobor pradu)
 */
void lcd_sleepin(void);

/**
 * \brief Wybudza wyswietlacz ze stanu uspienia
 */
void lcd_sleepout(void);

/**
 * \brief Wylacza inwersje kolorow wyswietlacza
 */
void lcd_invoff(void);

/**
 * \brief Wlacza inwersje kolorow wyswietlacza
 */
void lcd_invon(void);

/**
 * \brief Rysuje pojedynczy pixel zadanego koloru
 *
 * \param x
 * \param y
 * \param color kolor (w wartosci 16 bitowej)
 */
void lcd_draw_point(int x, int y, uint16_t color);

/**
 * \brief Rysuje obrazek zakodowany jako TFF 16-bit (tryb wolny)
 *
 * Obraz mozna zakodowac w TFF w aplikacji https://lvgl.io/tools/imageconverter
 * Metoda powolna - przesylany jest do wyswietlacza pixel po pixelu
 *
 * \param x koordynat poczatkowy os X
 * \param y koordynat poczatkowy os Y
 * \param width szerokosc obrazka
 * \param width wysokosc obrazka
 * \param data obrazek zakodowany jako tablica TFF
 */
void lcd_draw_image(int x, int y, int width, int height, uint8_t *data);

/**
 * \brief Draws a TFF 16 bit image on the screen via SPI in the DMA mode.
 *
 * Use the following web application to convert your image into the array - https://lvgl.io/tools/imageconverter
 *
 *
 * \param x x-coordinate of the top-left pixel of the image
 * \param y y-coordinate of the top left pixel of the image
 * \param width image width
 * \param width image height
 * \param data an image encoded as a 1-dimensional matrix
 */
void lcd_draw_image_fast(int x, int y, int width, int height, uint8_t *data);

/**
 * \brief Rysuje wypelniony kolorem prostokat (tryb szybki)
 *
 * W trybie szybkim szykowany jest bufor a pozniej wysylany do urzadzenia
 * \param x punkt startowy w osi x
 * \param y punkt startowy w osi y
 * \param width szerokosc
 * \param width wysokosc
 * \param color kolor (w wartosci 16 bitowej)
 */
void lcd_fill_box_fast(int x, int y, int width, int height, uint16_t color);

/**
 * \brief Maluje ekran na czarno
 *
 * Funkcja pomocnicza zwiekszajaca czytelnosc kodu. Przemalowuje ekran na czarno.
 * Przydatna przy odswiezaniu ekranu gdy malowana jest zupelnie nowa zawartosc
 */
void paint_screen_black();

/**
 * \brief Checks if SPI line is busy
 *
 * DMA auxiliary function
 * \return IF busy returns TRUE, otherwise returns FALSE
 */
bool lcd_is_spi_busy();

/** \brief Sets the CS line free (=sets to the idle state)
 *
 * DMA auxiliary function
 */
void lcd_transfer_completed();

#endif /* INC_LCD_H_ */
