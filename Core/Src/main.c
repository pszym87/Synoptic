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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t rx_buf[RX_BUF_SIZE];
uint8_t sf_buf[SF_BUF_SIZE];
uint8_t sf_buf_pos = 0;
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

  uint8_t tmp_mes[HISTORY_NUMS*HISTORY_ROW_SIZE] =
  	  	  	  	  	  { 101,102,103, 104, 105, 106, 107,
  	  	  	  	  	    108,109,110, 111, 112, 113, 114,
						115,116,117, 118, 119, 120, 121,
						122,123,124, 125, 126, 127, 128,
						129,130,131, 132, 133, 134, 135
  	  	  	  	  	   };

  uint8_t tmp_mes2[HISTORY_NUMS*HISTORY_ROW_SIZE];



  printf("STOP\r\n");
  save_history_to_eeprom(tmp_mes);
  load_history_from_eeprom(tmp_mes2);
  printHistory(tmp_mes2);

  // *** TESTOWY ZAPISU I ODCZYT TEMPERATURY Z PAMIECI EEPROM ***
  //float tempr = lps_read_temperature(U_CELSIUS);
  //uint8_t data_rec[2];

  // zmien date float na dwa inty z czescia calkowita i dziesietna

  //uint8_t data[] = {(uint8_t)tempr, (uint8_t)(100*(tempr-(uint8_t)tempr))};

  //if(HAL_I2C_Mem_Write(&hi2c1, 0xa0, 0x20, I2C_MEMADD_SIZE_8BIT, data, sizeof data, HAL_MAX_DELAY) != HAL_OK)
//Error_Handler();

  //HAL_Delay(200);

  //if(HAL_I2C_Mem_Read(&hi2c1, 0xa0, 0x20, I2C_MEMADD_SIZE_8BIT, data_rec, sizeof data_rec, HAL_MAX_DELAY) != HAL_OK)
//	 Error_Handler();

  //printf("EEPROM %d i %d i %d\n\r", data_rec[0], data_rec[1], data_rec[2]);

  // *** KONIEC TESTU Z EEPROM ***
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	fflush_sc_buff();

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

void load_history_from_eeprom(uint8_t msrm_history[HISTORY_NUMS*EEPROM_PAGE_SIZE]){
	if(HAL_I2C_Mem_Read(&hi2c1, 0xa0, MEM_MSRM_START, I2C_MEMADD_SIZE_8BIT, msrm_history, HISTORY_NUMS*HISTORY_ROW_SIZE, HAL_MAX_DELAY) != HAL_OK)
							 Error_Handler();
	while(HAL_I2C_IsDeviceReady(&hi2c1, 0xa0, 1, HAL_MAX_DELAY) != HAL_OK);
}


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

}


void printHistory(uint8_t msrm_history[HISTORY_NUMS*EEPROM_PAGE_SIZE]){
	for(int i=0; i<HISTORY_NUMS; i++){
		for(int j=0; j<HISTORY_ROW_SIZE; j++){
			printf("%d ", msrm_history[i+j]);
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

void fflush_sc_buff(){

	if(sf_buf_pos>0){
		sf_buf[sf_buf_pos] = '\0';
		int i=0;
		while(sf_buf[i]!='\0'){
			if(sf_buf[i]=='\r') printf("\n");
			printf("%c", sf_buf[i]);
			i++;
			fflush(stdout);
		}
		sf_buf_pos = 0;
	}
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
		sf_buf[sf_buf_pos] = rx_buf[0];
		++sf_buf_pos;
		HAL_UART_Receive_DMA(&huart1, rx_buf, 1);
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
