/**
 * \file lps25hb.h
 *
 * \brief Bibioteka obslugujaca czujnik cisnienia LPS25HB
 *
 * 	Komunikacja z czujnikiem po I2C, wskaźnik do struktury &hi2c1 (na sztywno)
 * 	Czujnik umozliwia odczyt temperatury i cisnienia
 * 	Dokumentacja czujnika https://www.st.com/resource/en/datasheet/lps25hb.pdf
 *
 * 	\author Piotr Szymanski
 * 	\version 1.0
 * 	\date 20.06.2022
 *
 * 	*/

//#include "stm32f4xx.h"

#include <stm32l4xx.h>

#ifndef INC_LPS25HB_H_
#define INC_LPS25HB_H_

#define U_KELVIN		273.15
#define	U_CELSIUS		0


/**
 * \brief Wybudza urządzenie i inicjuje konfigurację pomiaru ciśnienia i temperatury
 *
 * 		  Wyburza urządzenie (bit ODR2), ustawia w konfigurację pomiaru:
 * 		  Pressure(25Hz) i Temperature 25Hz. Więcej w dokumentacji LPS25HB (bity ODR1 i ODR0)
 * 		  Więcej informacji w dokumentacji LPS25HB s.35
 */
void lps_init();

/**
 * \brief Odczyt temperatury
 *
 * Odczytuje temperature z rejestrów przechowujących temperaturę TEMP_OUT_L	i TEMP_OUT_H.
 * Wartosc w rejestrach przechowywana jest na dwoch byte'ach. UWAGA. Po zlozeniu sa liczba typu int_16
 * ktora funkcja konwertuje na odpowiedni zapis temperatury.
 *
 * \param temp_conv daj 0 jesli chcesz jednostki w Celsius, dodaj 273.15 jesli Kelvin (wartosci zdefiniowane
 * 				    jako define
 * \return zwraca temperature w rzadanej jednostce (Celsius/Kelvin)
 */

float lps_read_temperature(float temp_conv);

void lps_pressure_correction(uint16_t offset);

/**
 * \brief Odczytuje ciśnienie bezwzględne (w danym punkcie pomiarowym)
 */
float lps_read_absolute_pressure();

/**
 * \brief Odczytuje cisnienie wzgledne (zredukowane do poziomu morza)
 *
 * \return Ciśnienie w hPa
 */
float lps_read_relative_pressure();

//float lps_calculate_altitude();

#endif /* INC_LPS25HB_H_ */
