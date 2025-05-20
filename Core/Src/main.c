/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */
  // 基底
  const int control_ms = 2;
  const int debug_ms = 100;
  uint32_t time = 0;
  uint32_t time_debug = 0;

  CAN_FilterTypeDef filter;
  uint32_t fId   =  0x000 << 21;        // フィルターID
  uint32_t fMask = (0x7F0 << 21) | 0x4; // フィルターマスク

  filter.FilterIdHigh         = fId >> 16;             // フィルターIDの上位16ビット
  filter.FilterIdLow          = fId;                   // フィルターIDの下位16ビット
  filter.FilterMaskIdHigh     = fMask >> 16;           // フィルターマスクの上位16ビット
  filter.FilterMaskIdLow      = fMask;                 // フィルターマスクの下位16ビット
  filter.FilterScale          = CAN_FILTERSCALE_32BIT; // 32モード
  filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;      // FIFO0へ格納
  filter.FilterBank           = 0;
  filter.FilterMode           = CAN_FILTERMODE_IDMASK; // IDマスクモード
  filter.SlaveStartFilterBank = 14;
  filter.FilterActivation     = ENABLE;

  HAL_CAN_ConfigFilter(&hcan, &filter);

  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);


  // 最初は制御非常停止をOFFにする
  HAL_GPIO_WritePin(EMS_GPIO_Port, EMS_Pin, GPIO_PIN_SET);

  // 最初に非常停止ONのCANを送る
  CAN_TxHeaderTypeDef header;
  uint32_t mailbox;
  uint8_t data[8];
  data[0] = 0x0;
  if(0 < HAL_CAN_GetTxMailboxesFreeLevel(&hcan)){
      header.StdId = 0x00F;
      header.RTR = CAN_RTR_DATA;
      header.IDE = CAN_ID_STD;
      header.DLC = 1;
      header.TransmitGlobalTime = DISABLE;
      HAL_CAN_AddTxMessage(&hcan, &header, data, &mailbox);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(HAL_GetTick() - time > control_ms){
        time = HAL_GetTick();

        if(HAL_GetTick() - time_debug > debug_ms){
            time_debug = HAL_GetTick();
            HAL_GPIO_TogglePin(CYCLE_LED_GPIO_Port, CYCLE_LED_Pin);
        }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// printf関数
int _write(int file, char *ptr, int len){
  int DataIdx;
  for(DataIdx=0; DataIdx<len; DataIdx++){
    ITM_SendChar(*ptr++);
  }
  return len;
}

// GPIO割り込みコールバック
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
  if(GPIO_Pin == EMS_OBSERVE_Pin){
    CAN_TxHeaderTypeDef header;
    uint32_t mailbox;
    uint8_t data[8];
    if (HAL_GPIO_ReadPin(EMS_OBSERVE_GPIO_Port, EMS_OBSERVE_Pin)) data[0] = 0x0;
    else data[0] = 0x1;

    if(0 < HAL_CAN_GetTxMailboxesFreeLevel(&hcan)){
        header.StdId = 0x00F;
        header.RTR = CAN_RTR_DATA;
        header.IDE = CAN_ID_STD;
        header.DLC = 1;
        header.TransmitGlobalTime = DISABLE;
        HAL_CAN_AddTxMessage(&hcan, &header, data, &mailbox);
    }
  }
}

// CANコールバック
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
    CAN_RxHeaderTypeDef RxHeader;
    uint32_t id;
    uint32_t dlc;
    uint8_t rxdata[8];
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rxdata) == HAL_OK){
        id = (RxHeader.IDE == CAN_ID_STD)? RxHeader.StdId : RxHeader.ExtId;
        dlc = RxHeader.DLC;

        if(id == 0x000){
            HAL_GPIO_WritePin(EMS_GPIO_Port, EMS_Pin, GPIO_PIN_RESET);
        }
        else if(id == 0x001){
            HAL_GPIO_WritePin(EMS_GPIO_Port, EMS_Pin, GPIO_PIN_SET);
        }
    }
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
