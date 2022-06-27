/*
 * lps25hb.c
 *
 *  Created on: Jun 20, 2022
 *      Author: pszymanski
 */

#include <stdint.h>
#include "lps25hb.h"
#include "i2c.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define LPS25HB_ADDR				0xBA	// Adres urzadzenia
#define REF_P_XL 					0x08	// Reference pressure register
#define REF_P_L						0x09	// Reference pressure register
#define REF_P_H						0x0A	// Reference pressure register
#define	WHO_AM_I					0x0F	// Who am I register
#define RES_CONF					0x10	// Resolution register
#define CTRL_REG1					0x20	// Control register
#define	CTRL_REG2					0x21	// Control register
#define CTRL_REG3					0x22	// Control register
#define CTRL_REG4					0x23	// Control register
#define	INTERRUPT_CFG				0x24	// Interrupt register
#define INT_SOURCE					0x25	// Interrupt register
#define STATUS_REG					0x27	// Status register
#define PRESS_OUT_XL				0x28	// Pressure output register
#define PRESS_OUT_L					0x29	// Pressure output register
#define PRESS_OUT_H					0x2A	// Pressure output register
#define TEMP_OUT_L					0x2B	// Temperature output register
#define TEMP_OUT_H					0x2C	// Temperature output register
#define FIFO_CTRL					0x2E	// FIFO configure register
#define FIFO_STATUS					0x2F	// FIFO configure register
#define THS_P_L						0x30	// Pressure threshold register
#define THS_P_H						0x31	// Pressure threshold register
#define RPDS_L						0x39	// Pressure offset register
#define RPDS_H						0x3A	// Pressure offset register

// Stałe symboliczne ulatwiajace ustawienia konkretnych rejestrow

// CTRL_REG1
#define SET_CTRL_REG1_PD			0x80
#define SET_CTRL_REG1_ODR2			0x40
#define SET_CTRL_REG1_ODR1			0x20
#define SET_CTRL_REG1_ODR0			0x10

// CTRL_REG2
#define SET_CTRL_REG2_FIFO			0x40

// FIFO_CTRL
#define SET_FIFO_CTRL_MEAN_MODE		0xc0
#define	SET_FIFO_CTRL_WTM_32_SMPL	0x0f


/**
 * \brief Zapis do rejestru urzadzenia LPS25HB przez I2C (wskazanym jako &hi2c1)
 *
 * \param	reg adres rejestru
 * \param	data byte danych do zapisania
 * \return	status transmisji
 *
 */
static HAL_StatusTypeDef lps_write_to_reg(uint8_t reg, uint8_t data){
	HAL_StatusTypeDef status;
	uint8_t buffer[2] = {reg, data};

    status = HAL_I2C_Master_Transmit(&hi2c1, LPS25HB_ADDR, buffer, sizeof buffer, HAL_MAX_DELAY);
	return status;
}

/**
 * \brief Odczyt z rejestru urzadzenia LPS25HB przez I2C (wskazanym jako &hi2c1)
 *
 * \param	reg adres rejestru
 * \param	data odczyt zostanie zapisany pod wskazywanym adresem
 *
 * \return status transmisji (nie rozróżnia czy status dotyczy transmisji o dostep do rejestru, czy statusu odczytu z rejestru)
 *
 */
static HAL_StatusTypeDef lps_read_from_reg(uint8_t reg, uint8_t* data_ptr){
	HAL_StatusTypeDef status;
	if((status = HAL_I2C_Master_Transmit(&hi2c1, LPS25HB_ADDR, &reg, sizeof reg, HAL_MAX_DELAY))!=HAL_OK)
		return status;
	status = HAL_I2C_Master_Receive(&hi2c1, LPS25HB_ADDR, data_ptr, sizeof data_ptr, HAL_MAX_DELAY);
	return status;
}


void lps_init(){
	// wlacz urzadzenie i ustaw czestotliwosc pomiaru na 25Hz
	lps_write_to_reg(CTRL_REG1, SET_CTRL_REG1_PD|SET_CTRL_REG1_ODR2);

	// przy problemach z odczytem danych dodac oczekiwanie 100 ms

	// aktywacja i ustawienie fifo dla pomiarow
	lps_write_to_reg(CTRL_REG2,SET_CTRL_REG2_FIFO);
	lps_write_to_reg(FIFO_CTRL, SET_FIFO_CTRL_MEAN_MODE|SET_FIFO_CTRL_WTM_32_SMPL);

}

float lps_read_temperature(float temp_conv){
	uint8_t *ptr_lsb, *ptr_msb;
	ptr_lsb = malloc(sizeof(uint8_t));
	ptr_msb = malloc(sizeof(uint8_t));
	lps_read_from_reg(TEMP_OUT_L, ptr_lsb);
	lps_read_from_reg(TEMP_OUT_H, ptr_msb);
    int16_t val = *ptr_lsb + (*ptr_msb <<8);

    free(ptr_lsb);
    free(ptr_msb);

	return 42.5f + val / 480.0f + temp_conv;
}

void lps_pressure_correction(uint16_t offset){
	lps_write_to_reg(RPDS_L, offset);
	lps_write_to_reg(RPDS_H, offset >> 8);
}


float lps_read_absolute_pressure(){
	uint8_t *pressure = malloc(sizeof(uint8_t)*3);
	lps_read_from_reg(PRESS_OUT_XL, pressure);
	lps_read_from_reg(PRESS_OUT_L, pressure+1);
	lps_read_from_reg(PRESS_OUT_H, pressure+2);

	int32_t val = (*pressure + (*(pressure+1)<<8) + (*(pressure+2)<<16))/4096;
	free(pressure);
	return val;

}

float lps_read_relative_pressure(){
	const float h = 93; // jesli inna wysokosc to zmienic;
	float temp_K = lps_read_temperature(U_KELVIN);
	float abs_press = lps_read_absolute_pressure();

	return abs_press * exp(0.034162608734308*h / temp_K);
}

float lps_get_altitude_hyps_f(){

	float p0 = lps_read_relative_pressure(); // sea-level pressure
    float p = lps_read_absolute_pressure(); // pressure at location
    float temp_K = lps_read_temperature(U_KELVIN);

    float h = (( pow(p0/p, 1/5.257) - 1)*( temp_K ))/0.0065;

   	return h;

}

