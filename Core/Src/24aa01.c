/**
 * \file 24aa01.c
 *
 * \brief Biblioteka obslugujaca pamięc EEPROM 24AA01/24LC01
 *  Created on: Jun 10, 2022
 *      Author: pszymanski
 */
#include <stdint.h>
#include "i2c.h"
#include "24aa01.h"


void load_history_from_eeprom(uint8_t msrm_history[HISTORY_NUMS*HISTORY_ROW_SIZE]){
	if(HAL_I2C_Mem_Read(&hi2c1, 0xa0, MEM_MSRM_START, I2C_MEMADD_SIZE_8BIT, msrm_history, HISTORY_NUMS*HISTORY_ROW_SIZE, HAL_MAX_DELAY) != HAL_OK)
							 Error_Handler();
	while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);
}

void read_eeprom_memcell(uint8_t addr, uint8_t *memcont){
	if(HAL_I2C_Mem_Read(&hi2c1, 0xa0, addr, I2C_MEMADD_SIZE_8BIT, memcont, sizeof(*memcont), HAL_MAX_DELAY) != HAL_OK)
							 Error_Handler();
	while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);
}

/**
 * \brief Zapisuje podana historie pomiarow w EEPROM
 *
 * Wykorzystuje stronicowanie przesylu informacji. EEPROM 24AA01 na 6 stronie datasheetu
 * opisuje tryb stronicowania. Najwazniejsze ograniczenia to:
 * - strona liczy do 8 byte'ow (tyle ma wewn. buffor pamieci)
 * - jesli wysylane mniej niz 8 byte'ow to wskaznik kolejnej strony bedzie i tak wskazywal na 8 byte (wewn. inkremantacja o 8 byte'ow)
 * - transmisje mozna zaczac tylko od 0x00 i kolejnych stron wskazywanych przez multiplikacje 8 byte'ów przez liczbe calkowita (nie mozna zaczac np. od adresu 0x1)
 *
 * Funkcja wysyla cala historie dzielac ja na pelne strony (8 byte'owe). Strona mniejsza niz 8 byte'ow wysylana jest na koncu.
 *
 * \param msrm tablica z pomiarami
 */
void save_history_to_eeprom(uint8_t msrm[HISTORY_NUMS*HISTORY_ROW_SIZE]){

	// calculate number of full pages and size of the last page
	int full_pages = HISTORY_ROW_SIZE*HISTORY_NUMS/EEPROM_PAGE_SIZE;
	int last_page = HISTORY_ROW_SIZE*HISTORY_NUMS - full_pages*EEPROM_PAGE_SIZE;

	// send full pages
	for(int i=0; i<full_pages; i++){

		if(HAL_I2C_Mem_Write(&hi2c1, 0xa0, MEM_MSRM_START+i*EEPROM_PAGE_SIZE, I2C_MEMADD_SIZE_8BIT, msrm+i*EEPROM_PAGE_SIZE, EEPROM_PAGE_SIZE, HAL_MAX_DELAY) != HAL_OK)
			Error_Handler();
		while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);

	}

	// send remaining page (not full)
	if(last_page >0){
		if(HAL_I2C_Mem_Write(&hi2c1, 0xa0, MEM_MSRM_START+full_pages*EEPROM_PAGE_SIZE, I2C_MEMADD_SIZE_8BIT, msrm+full_pages*EEPROM_PAGE_SIZE, last_page, HAL_MAX_DELAY) != HAL_OK)
			Error_Handler();
		while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);
	}

	uint8_t mem_ptr = 0x0;

	// set mem pointer
	HAL_I2C_Mem_Write(&hi2c1, 0xa0, 0x7F, I2C_MEMADD_SIZE_8BIT, &mem_ptr, 1, HAL_MAX_DELAY);

}

/** \brief Zapisuje podana ilosc byte'ow informacji do pamieci historii
 *
 *  Zapisuje w kolejnym dostepnym miejscu dane. Miejsce wskazywane jest w pamieci EEPROM w
 *  miejscu MEM_POINTER. Po zapisie do pamieci aktualizuje wskaznik pamieci MEM_POINTER
 *
 *  \param entry dane (domyslnie uzywany do zapisu linii informacji data+czas+odczyt temp)
 *  \param size	rozmiar danych w byte'ach
 */
void save_entry_to_eeprom(uint8_t* entry, int size){

	uint8_t mem_ptr;

	// get mem pointer
	if(HAL_I2C_Mem_Read(&hi2c1, 0xa0, MEM_POINTER, I2C_MEMADD_SIZE_8BIT, &mem_ptr, 1, HAL_MAX_DELAY) != HAL_OK)
							 Error_Handler();
	while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);

	// write to mem
	for(int i=0; i<size; i++){
		uint8_t tmp[2] = {mem_ptr, *(entry+i)};
		HAL_I2C_Master_Transmit(&hi2c1, 0xa0, tmp, sizeof tmp, HAL_MAX_DELAY);
		while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);
		mem_ptr++;
	}

	// set mem pointer (zeruj jesli wystapilo przekroczenie zakresu historii: 5 pomiarow)
	if(mem_ptr > (MEM_MSRM_START+HISTORY_ROW_SIZE*HISTORY_NUMS)-1){
		mem_ptr = 0;
	}

	// write mem pointer to mem
	HAL_I2C_Mem_Write(&hi2c1, 0xa0, 0x7F, I2C_MEMADD_SIZE_8BIT, &mem_ptr, 1, HAL_MAX_DELAY);


}
