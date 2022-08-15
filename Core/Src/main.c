/**
 * \file main.c
 *
 * \brief Glowna petla programu wraz ze sterowaniem
 *      Author: pszymanski
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
#include "24aa01.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAXTXTLEN			100


#define RX_BUF_SIZE		1
#define SF_BUF_SIZE		100


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

typedef enum prog_mode {live_mode, history_mode} prog_mode_t;

/**
 * \brief Zbior zmiennych na potrzeby podprogramu 'live mode'.
 */
typedef struct hmdata{
	bool history_loaded;
	uint8_t *msrm_history;

} hmdata_t;

/**
 * \brief Zbior zmiennych, z ktorych korzystaja wszystkie podprogramy i programy dzialajace w tle.
 */
typedef struct prgsdata{
	prog_mode_t which_program;
	hmdata_t *hmdt;
	char line[MAXTXTLEN];
} prgsdata_t;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void chck_wchdog_fl();
void live_mode_prog();
void history_mode_prog(hmdata_t *hmdt);
void print_history(uint8_t[]);
void write_all_history_to_eeprom(uint8_t[]);
void fflush_sc_buff(prgsdata_t *prgs);
void set_date(int dd, int mm, int yy);
void set_time(int hh, int mm);
void clrtxtbuff(char* str, int size);
void prscmd(prgsdata_t *prgs, char* cmd);
void print_date();
void print_time();
void print_man();
void get_entry(uint8_t *entry);
void save_entry();
void print_mem();
void erasehis();
void memr(uint8_t addr);
void set_alarm_m_s(uint8_t min, uint8_t sec);
void alarm_settings();
void do_alarm_action();
void refhis(uint8_t* msrm_history);
void chprog(prog_mode_t *which_prog);
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
  chck_wchdog_fl();
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

  paint_screen_black();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  uint8_t history[HISTORY_NUMS*HISTORY_ROW_SIZE];
  hmdata_t hmdt = {false, history};
  prgsdata_t prgs = {live_mode, &hmdt, {'\0'}};

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	fflush_sc_buff(&prgs);
	alarm_settings();


	switch(prgs.which_program){

		case live_mode:
			live_mode_prog();
			break;
		case history_mode:
			history_mode_prog(&hmdt);
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

}
void history_mode_prog(hmdata_t *hmdt){

	if(!hmdt->history_loaded){
		load_history_from_eeprom(hmdt->msrm_history);

		print_history(hmdt->msrm_history);
		hmdt->history_loaded = true;

	} else if(hmdt->history_loaded){

		for(int i=0; i<HISTORY_NUMS; i++){

			int c = HISTORY_ROW_SIZE * i;
			wchar_t text[MAXTXTLEN];
			swprintf(text, MAXTXTLEN, L"%02d/%02d/%02d %02d:%02d %02d.%02d C", hmdt->msrm_history[c],hmdt->msrm_history[c+1], hmdt->msrm_history[c+2], hmdt->msrm_history[c+3],
																				  hmdt->msrm_history[c+4], hmdt->msrm_history[c+5], hmdt->msrm_history[c+6]);
			hagl_put_text(text, 5, i*20, RED, font6x9);
		}



	}
    //printf("History mode prog\r\n");
}

void refhis(uint8_t* msrm_history){
	load_history_from_eeprom(msrm_history);
}



/**
 * \brief Wystawia w tablicy entry bieżący odczyt daty, czasu i temperatury
 */
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


void print_history(uint8_t msrm_history[HISTORY_NUMS*EEPROM_PAGE_SIZE]){
	for(int i=0; i<HISTORY_NUMS; i++){
		for(int j=0; j<HISTORY_ROW_SIZE; j++){
			printf("%d ", msrm_history[i*HISTORY_ROW_SIZE+j]);
		}
		printf("\r\n");
	}
}

void chck_wchdog_fl(){
	if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST)){
			printf("System zostal zresetowany przez Watchdoga\r\n");
			__HAL_RCC_CLEAR_RESET_FLAGS();
		} else{
			printf("System zostal uruchomiony poprawnie\r\n");
		}
}


void fflush_sc_buff(prgsdata_t *prgs){


	if(sf_buf_pos>0){
		sf_buf[sf_buf_pos] = '\0';
		int i=0;
		while(sf_buf[i]!='\0'){
			if(sf_buf[i]!='\r'){
				strncat(prgs->line, (char *)&sf_buf[i], 1);
			}

			if(sf_buf[i]=='\r') {
				printf("\n");
				prscmd(prgs, prgs->line);
				clrtxtbuff(prgs->line, MAXTXTLEN);
			}

			printf("%c", sf_buf[i]);
			i++;
			fflush(stdout);
		}
		sf_buf_pos = 0;
	}
}

void clrtxtbuff(char* str, int size){
	for(int i=0; i<size; i++){
		str[i] = '\0';
	}
}

void prscmd(prgsdata_t *prgs, char* cmd){
	char inst[10];
	int offset=0;

	sscanf(cmd, "%s %n", inst, &offset);

	if( strcmp(inst, "setdate" ) == 0){
		int d,m,y;
		sscanf(cmd+offset, "%d/%d/%d", &d, &m, &y);
		set_date(d,m,y);

	}
	else if( strcmp(inst, "settime" ) == 0){
		int h,m;
		sscanf(cmd+offset, "%d:%d", &h, &m);
		set_time(h, m);
	}

	else if( strcmp(inst, "print_time" ) == 0){
		print_time();
	}

	else if( strcmp(inst, "print_date" ) == 0){
		print_date();
	}

	else if( strcmp(inst, "man" ) == 0){
		print_man();
	}
	else if( strcmp(inst, "save" ) == 0){
		save_entry();
	}
	else if( strcmp(inst, "print_mem" ) == 0){
		print_mem();
	}
	else if( strcmp(inst, "erasemem" ) == 0){
		erasehis();
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
	else if( strcmp(inst, "refhis" ) == 0){
		refhis(prgs->hmdt->msrm_history);
	}
	else if( strcmp(inst, "chprog" ) == 0){
		chprog(&prgs->which_program);
	}
}

void memr(uint8_t addr){
	uint8_t cont;
	read_eeprom_memcell(addr, &cont);
	printf("%x %d\r\n", addr, cont);
}

void erasehis(){
	uint8_t msrm[HISTORY_NUMS*HISTORY_ROW_SIZE];

	for(int i=0; i<HISTORY_NUMS*HISTORY_ROW_SIZE; i++)
		msrm[i] = '\0';
	save_history_to_eeprom(msrm);
}

void print_mem(){
	uint8_t msrm_history[HISTORY_NUMS*HISTORY_ROW_SIZE];

	load_history_from_eeprom(msrm_history);
	print_history(msrm_history);
}

void save_entry(){
	uint8_t entry[HISTORY_ROW_SIZE];
	get_entry(entry); // czy ja tutaj nie potrzebuje zaalokowac sobie 7 komorek pamieci?
	save_entry_to_eeprom(entry,HISTORY_ROW_SIZE);
}

void chprog(prog_mode_t *which_prog){
	if(*which_prog == live_mode) *which_prog = history_mode;
	else if(*which_prog == history_mode) *which_prog = live_mode;
	paint_screen_black();
}

void print_man(){
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
	printf("refhis\r\n");
	printf("chprog\r\n");
	printf("\r\n ***************************\r\n\n");
}

void set_date(int dd, int mm, int yy){

	RTC_DateTypeDef tmp = {0};

	tmp.Date = (uint8_t) dd;
	tmp.Month = (uint8_t) mm;
	tmp.Year = (uint8_t)yy;

	HAL_RTC_SetDate(&hrtc, &tmp, RTC_FORMAT_BIN);
}

void set_time(int hh, int mm){
	RTC_TimeTypeDef tmp = {0};

	tmp.Hours = hh;
	tmp.Minutes = mm;

	HAL_RTC_SetTime(&hrtc, &tmp, RTC_FORMAT_BIN);

}



void print_date(){
	RTC_TimeTypeDef mTime;
	RTC_DateTypeDef mDate;

	HAL_RTC_GetTime(&hrtc, &mTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &mDate, RTC_FORMAT_BIN);

  	printf("Date: %d/%d/%d\r\n", mDate.Date, mDate.Month, mDate.Year);

}

void print_time(){
	RTC_TimeTypeDef mTime;
	RTC_DateTypeDef mDate;

	HAL_RTC_GetTime(&hrtc, &mTime, RTC_FORMAT_BIN);
  	HAL_RTC_GetDate(&hrtc, &mDate, RTC_FORMAT_BIN);

  	printf("Time: %d:%d\r\n", mTime.Hours, mTime.Minutes);
}

/** \brief Ustawia alarm w trybie przerwan dla podanej min i sec
 *
 */
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

/** \brief Wykonanie akcji zwiazanej z podniesiona flaga alarmu RTC
 *
 */
void alarm_settings(){

	if(alarm_fl == 1){
		alarm_fl++; // do usuniecia?
		do_alarm_action();
		alarm_fl = 0;
	}

}

/**
 * \brief Wykonuje akcje przypisana do alarmu
 */
void do_alarm_action(){
	save_entry();
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
