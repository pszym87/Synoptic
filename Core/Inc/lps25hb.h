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

#include "stm32l4xx.h"

#ifndef INC_LPS25HB_H_
#define INC_LPS25HB_H_

#define U_KELVIN		273.15
#define	U_CELSIUS		0


/**
 * \brief Wybudza urządzenie i inicjuje konfigurację pomiaru ciśnienia i temperatury
 *
 * 		  Wyburza urządzenie (bit ODR2), ustawia konfigurację pomiaru:
 * 		  Pressure 25Hz i Temperature 25Hz. Więcej w dokumentacji LPS25HB (bity ODR1 i ODR0)
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

/**
 * \brief Kalibracja czujnika
 *
 * Czujnik wymaga kalibracji dla danej lokalizacji. Aby dokonac kalibracji nalezy odczytac z czujnika
 * wskazanie cisnienia wzglednego (wyliczone cisnienie dla poziomu morza) oraz porownac je z cisnieniem
 * wzglednym wzorcowym np. z prognozy pogody http://meteo.imgw.pl, roznice cisnien pomnozyc przez 16 i podac jako
 * argument funkcji
 *
 * \param offset wartosc o jaka ma zostac skalibrownay czujnik cisnienia. Opis wyliczenia w opisie funkcji.
 */
void lps_pressure_correction(uint16_t offset);


/**
 * \brief Odczytuje ciśnienie absolutne (lokalizacja na poziomie 93m)
 *
 * Przy innej lokalizacji
 */

float lps_read_absolute_pressure();

/**
 * \brief Odczytuje cisnienie bezwzgledne (zredukowane do poziomu morza)
 *
 * \return Ciśnienie w hPa
 */
float lps_read_relative_pressure();


/**
 * \brief Wylicza wysokosc (za pomoca formuly hypsometrycznej)
 *
 * Szczegoly formuly pod linkiem: https://keisan.casio.com/has10/SpecExec.cgi?id=system/2006/1224585971
 * Wyliczenie na podstawie odczytanej z czytnika temperatury i cisnienia wzglednego i bezwzglednego
 *
 * \return Wysokosc w m.n.p.m.
 */
float lps_get_altitude_hyps_f();

#endif /* INC_LPS25HB_H_ */
