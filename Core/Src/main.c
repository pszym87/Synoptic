/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "lps25hb.h"
#include <stdio.h>
#include "hagl.h"
#include "font6x9.h"
#include "font5x8.h"
#include <wchar.h>
#include "icons.c"
#include <math.h>
#include <stdbool.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAXTXTLEN			100
#define HISTORY_NUMS 		5
#define HISTORY_ROW_SIZE 	7
#define MEM_MSRM_START		0x0
#define EEPROM_PAGE_SIZE	8
#define	EEPROM_END_OF_PAGE			'\0'
#define RX_BUF_SIZE		1
#define SF_BUF_SIZE		100
#define MEM_POINTER		0x7F

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t sf_buf[SF_BUF_SIZE];
uint8_t sf_buf_pos = 0;
uint8_t alarm_fl = 0;

static char line[MAXTXTLEN];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void checkFlags();
void live_mode_prog();
void history_mode_prog();
void load_history_from_eeprom(uint8_t[]);
void printHistory(uint8_t[]);
void write_all_history_to_eeprom(uint8_t[]);
void save_history_to_eeprom(uint8_t msrm[HISTORY_NUMS*HISTORY_ROW_SIZE]);
void fflush_sc_buff();
void setDate(int dd, int mm, int yy);
void setTime(int hh, int mm);
void clrTxtBuff(char* str, int size);
void prsCmd(char* cmd);
void printDate();
void printTime();
void printMan();
void save_entry_to_eeprom(uint8_t* entry, int size);
void get_entry(uint8_t *entry);
void saveEntry();
void printMem();
void eraseHis();
void memr(uint8_t addr);
void read_eeprom_memcell(uint8_t addr, uint8_t *memcont);
void set_alarm_m_s(uint8_t min, uint8_t sec);
void alarm_settings();
void do_alarm_action();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  typedef enum prog_mode {live_mode, history_mode} prog_mode_t;
  prog_mode_t which_program = 1;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_DMA_Init();
  MX_IWDG_Init();
  MX_USART1_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  checkFlags();
  lps_init();
  lps_pressure_correction(48);
  printf("\r\n\n *** URUCHOMIENIE URZADZENIA *** \r\n\n");
  printf("UART pracuje poprawnie\r\n");
  printf("Testowy odczyt temperatury= %.2f\r\n", lps_read_temperature(U_CELSIUS));
  printf("Testowy odczyt cisnienia= %.2f\r\n", lps_read_relative_pressure());
  printf("Testowy odczyt wysokosci= %.2f\r\n", lps_get_altitude_hyps_f());
  lcd_init();
  hagl_init();

  HAL_UART_Receive_DMA(&huart1, rx_buf, 1);

  lcd_fill_box(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	fflush_sc_buff();
	alarm_settings();


	switch(which_program){

		case live_mode:
			live_mode_prog();
			break;
		case history_mode:
			history_mode_prog();
			break;

	}


	HAL_Delay(500);

	HAL_IWDG_Refresh(&hiwdg);

  }
  hagl_close();
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void live_mode_prog(){

	// Konwersja tekstow do Wide Chara
	wchar_t text[MAXTXTLEN], text2[MAXTXTLEN], text3[MAXTXTLEN];
	swprintf(text, MAXTXTLEN, L"Temperatura: %.2f C", lps_read_temperature(U_CELSIUS));
	swprintf(text2, MAXTXTLEN, L"Cisnienie: %.2f Hpa", lps_read_relative_pressure());
	swprintf(text3, MAXTXTLEN, L"Wysokosc: %.0f m.n.p.m.", lps_get_altitude_hyps_f());

	hagl_put_text(text, 30, 17, RED, font6x9);
	hagl_put_text(text2, 30, 41, RED, font6x9);
	hagl_put_text(text3, 30, 70, RED, font6x9);

	// nie mam ikon?
	lcd_draw_image_fast(2,5,24,24,temp_icon);
	lcd_draw_image_fast(2,34,24,24,press_icon);
	lcd_draw_image_fast(2,63,24,24,alt_icon);



	printf("Live mode prog\r\n");
}
void history_mode_prog(){

	static bool historyLoaded;
	static uint8_t msrm_history[HISTORY_NUMS*HISTORY_ROW_SIZE];

	if(!historyLoaded){
		load_history_from_eeprom(msrm_history);

		printHistory(msrm_history);
		historyLoaded = true;

	} else if(historyLoaded){

		for(int i=0; i<HISTORY_NUMS; i++){

			int c = HISTORY_ROW_SIZE * i;
			wchar_t text[MAXTXTLEN];
			swprintf(text, MAXTXTLEN, L"%d/%d/%d: %d,%d C", msrm_history[c],msrm_history[c+1], msrm_history[c+2], msrm_history[c+3],
																			 msrm_history[c+4], msrm_history[c+5], msrm_history[c+5],
																			 msrm_history[c+6]);
			hagl_put_text(text, 5, i*20, RED, font6x9);
		}



	}
    //printf("History mode prog\r\n");
}

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

void get_entry(uint8_t *entry){

	RTC_TimeTypeDef mTime;
	RTC_DateTypeDef mDate;

	HAL_RTC_GetTime(&hrtc, &mTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &mDate, RTC_FORMAT_BIN);

  	float temp = lps_read_temperature(U_CELSIUS);

  	uint8_t int_part = (uint8_t) temp;
  	uint8_t dec_part = (uint8_t)((temp-int_part)*100);

  	*entry = mDate.Date;
  	*(entry+1) = mDate.Month;
  	*(entry+2) = mDate.Year;
	*(entry+3) = mTime.Hours;
	*(entry+4) = mTime.Minutes;
	*(entry+5) = int_part;
	*(entry+6) = dec_part;
}


void printHistory(uint8_t msrm_history[HISTORY_NUMS*EEPROM_PAGE_SIZE]){
	for(int i=0; i<HISTORY_NUMS; i++){
		for(int j=0; j<HISTORY_ROW_SIZE; j++){
			printf("%d ", msrm_history[i*HISTORY_ROW_SIZE+j]);
		}
		printf("\r\n");
	}
}

void checkFlags(){
	if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)){
			printf("System zostal zresetowany przez Watchdoga\r\n");
			__HAL_RCC_CLEAR_RESET_FLAGS();
		} else{
			printf("System zostal uruchomiony poprawnie\r\n");
		}
}

/*
 * ____ W funkcji trzeba poprawic korzystanie z globalnej zmiennej ____ line ____
 */
void fflush_sc_buff(){



	if(sf_buf_pos>0){
		sf_buf[sf_buf_pos] = '\0';
		int i=0;
		while(sf_buf[i]!='\0'){
			if(sf_buf[i]!='\r'){
				strncat(line, (char *)&sf_buf[i], 1);
			}

			if(sf_buf[i]=='\r') {
				printf("\n");
				prsCmd(line);
				clrTxtBuff(line, MAXTXTLEN);
			}

			printf("%c", sf_buf[i]);
			i++;
			fflush(stdout);
		}
		sf_buf_pos = 0;
	}
}

void clrTxtBuff(char* str, int size){
	for(int i=0; i<size; i++){
		str[i] = '\0';
	}
}

void prsCmd(char* cmd){
	char inst[10];
	int offset=0;

	sscanf(cmd, "%s %n", inst, &offset);

	if( strcmp(inst, "setdate" ) == 0){
		int d,m,y;
		sscanf(cmd+offset, "%d/%d/%d", &d, &m, &y);
		setDate(d,m,y);

	}
	else if( strcmp(inst, "settime" ) == 0){
		int h,m;
		sscanf(cmd+offset, "%d:%d", &h, &m);
		setTime(h, m);
	}

	else if( strcmp(inst, "printtime" ) == 0){
		printTime();
	}

	else if( strcmp(inst, "printdate" ) == 0){
		printDate();
	}

	else if( strcmp(inst, "man" ) == 0){
		printMan();
	}
	else if( strcmp(inst, "save" ) == 0){
		saveEntry();
	}
	else if( strcmp(inst, "printmem" ) == 0){
		printMem();
	}
	else if( strcmp(inst, "erasemem" ) == 0){
		eraseHis();
	}
	else if( strcmp(inst, "memr" ) == 0){
		int addr;
		sscanf(cmd+offset, "%d", &addr);
		memr((uint8_t)addr);
	}
	else if( strcmp(inst, "setal" ) == 0){
		int m,s;
		sscanf(cmd+offset, "%d:%d", &m, &s);
		set_alarm_m_s(m, s);
	}

}

void memr(uint8_t addr){
	uint8_t cont;
	read_eeprom_memcell(addr, &cont);
	printf("%x %d\r\n", addr, cont);
}

void eraseHis(){
	uint8_t msrm[HISTORY_NUMS*HISTORY_ROW_SIZE];

	for(int i=0; i<HISTORY_NUMS*HISTORY_ROW_SIZE; i++)
		msrm[i] = '\0';
	save_history_to_eeprom(msrm);
}

void printMem(){
	uint8_t msrm_history[HISTORY_NUMS*HISTORY_ROW_SIZE];

	load_history_from_eeprom(msrm_history); // tu jest blad
	printHistory(msrm_history);				// tu jest blad
}

void saveEntry(){
	uint8_t entry[HISTORY_ROW_SIZE];
	get_entry(entry); // czy ja tutaj nie potrzebuje zaalokowac sobie 7 komorek pamieci?
	save_entry_to_eeprom(entry,HISTORY_ROW_SIZE);
}



void printMan(){
	printf("\r\n ***** CONSOLE MANUAL ****** \r\n\n");
	printf("setdate dd/mm/yy\r\n");
	printf("settime hh:mm\r\n");
	printf("printtime\r\n");
	printf("printdate\r\n");
	printf("save\r\n");
	printf("printmem\r\n");
	printf("erasemem\r\n");
	printf("memr\r\n");
	printf("setal\r\n");
	printf("\r\n ***************************\r\n\n");
}

void setDate(int dd, int mm, int yy){

	RTC_DateTypeDef tmp = {0};

	tmp.Date = (uint8_t) dd;
	tmp.Month = (uint8_t) mm;
	tmp.Year = (uint8_t)yy;

	HAL_RTC_SetDate(&hrtc, &tmp, RTC_FORMAT_BIN);
}

void setTime(int hh, int mm){
	RTC_TimeTypeDef tmp = {0};

	tmp.Hours = hh;
	tmp.Minutes = mm;

	HAL_RTC_SetTime(&hrtc, &tmp, RTC_FORMAT_BIN);

}



void printDate(){
	RTC_TimeTypeDef mTime;
	RTC_DateTypeDef mDate;

	HAL_RTC_GetTime(&hrtc, &mTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &mDate, RTC_FORMAT_BIN);

  	printf("Date: %d/%d/%d\r\n", mDate.Date, mDate.Month, mDate.Year);

}

void printTime(){
	RTC_TimeTypeDef mTime;
	RTC_DateTypeDef mDate;

	HAL_RTC_GetTime(&hrtc, &mTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &mDate, RTC_FORMAT_BIN);

  	printf("Time: %d:%d\r\n", mTime.Hours, mTime.Minutes);
}

void set_alarm_m_s(uint8_t min, uint8_t sec){

	  RTC_AlarmTypeDef alarm;

	  alarm.AlarmTime.Hours = 0x0;
	  alarm.AlarmTime.Minutes = min;
	  alarm.AlarmTime.Seconds = sec;
	  alarm.AlarmTime.SubSeconds = 0x0;
	  alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  alarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  alarm.AlarmMask = RTC_ALARMMASK_NONE;
	  alarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  alarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	  alarm.AlarmDateWeekDay = 0x1;
	  alarm.Alarm = RTC_ALARM_A;

	  HAL_RTC_SetAlarm_IT(&hrtc, &alarm, RTC_FORMAT_BIN);
}

void alarm_settings(){

	if(alarm_fl == 1){
		alarm_fl++;
		do_alarm_action();
		alarm_fl = 0;
	}

}

void do_alarm_action(){
	saveEntry();
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
		sf_buf[sf_buf_pos] = rx_buf[0];
		++sf_buf_pos;
		HAL_UART_Receive_DMA(&huart1, rx_buf, 1);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){
	printf("Alarm!\r\n");
	alarm_fl++;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
