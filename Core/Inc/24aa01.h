/**
 * \file 24aa01.h
 *
 * \brief Plik naglowkowy biblioteki obslugujacej pamiec EEPROM 24AA01/24LC01
 */

#ifndef INC_24AA01_H_
#define INC_24AA01_H_

/// Maksymala ilosc rekordow w pamieci dla przechowywania historii
#define HISTORY_NUMS 		5
/// Dlugosc rekordu w bajtach
#define HISTORY_ROW_SIZE 	7
/// Adres pamieci EEPROM przechowujaca pierwszy byte historii
#define MEM_MSRM_START		0x0
#define EEPROM_PAGE_SIZE	8
#define	EEPROM_END_OF_PAGE			'\0'
/// Adres pamieci EEPROM przechowujacy wskaznik do zapisu kolejnego rekordu
#define MEM_POINTER		0x7F

/**
 * \brief Odczytuje zawartosc pamieci pod wskazanym adresem
 *
 * \param addr Adres do odczytania (zakres 0x00 - 0x7F)
 * \param *memcont Odczytana zawartosc pamieci jest umieszczana pod wskazanym adresem
 */
void read_eeprom_memcell(uint8_t addr, uint8_t *memcont);

/**
 * \brief Wczytuje do zmiennej lokalnej zawartosc historii zapisana na EEPROM
 *
 * \param msrm_history[] Tablica do ktorej zostanie zapisany odczyt pamieci EEPROM
 */
void load_history_from_eeprom(uint8_t msrm_history[HISTORY_NUMS*HISTORY_ROW_SIZE]);

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
void save_history_to_eeprom(uint8_t msrm[HISTORY_NUMS*HISTORY_ROW_SIZE]);

/** \brief Zapisuje podana ilosc byte'ow informacji do pamieci historii
 *
 *  Zapisuje w kolejnym dostepnym miejscu dane. Miejsce wskazywane jest w pamieci EEPROM w
 *  miejscu MEM_POINTER. Po zapisie do pamieci aktualizuje wskaznik pamieci MEM_POINTER
 *
 *  \param entry dane (domyslnie uzywany do zapisu linii informacji data+czas+odczyt temp)
 *  \param size	rozmiar danych w byte'ach
 */
void save_entry_to_eeprom(uint8_t* entry, int size);



#endif /* INC_24AA01_H_ */
