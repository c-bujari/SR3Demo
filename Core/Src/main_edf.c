/*
 * implementation_edf.c
 *
 *  Created on: Oct 18, 2021
 *      Author: Clyde
 */
// Standard C includes
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// FreeRTOS kernel
#include "main.h"

UART_HandleTypeDef huart2;
void uart_write_char(uint8_t ch);
void HAL_UART_write_uint32(uint32_t n);
void uart_write_char(uint8_t ch);

#define EDF_TASK1_PERIOD (pdMS_TO_TICKS(4000))
#define EDF_TASK2_PERIOD (pdMS_TO_TICKS(6000))
#define EDF_TASK3_PERIOD (pdMS_TO_TICKS(8000))
#define EDF_TASK4_PERIOD (pdMS_TO_TICKS(12000))

#define EDF_TASK1_BUDGET (pdMS_TO_TICKS(1000))
#define EDF_TASK2_BUDGET (pdMS_TO_TICKS(1000))
#define EDF_TASK3_BUDGET (pdMS_TO_TICKS(2000))
#define EDF_TASK4_BUDGET (pdMS_TO_TICKS(3000))

void vEDFTask1(void *pvParameters);
void vEDFTask2(void *pvParameters);
void vEDFTask3(void *pvParameters);
void vEDFTask4(void *pvParameters);

void log_task_to_ready_state(char *taskName);
void log_task_switched_out(char *taskName);
void log_task_switched_in(char *taskName);

TaskHandle_t xTask1Handle = NULL;
TaskHandle_t xTask2Handle = NULL;
TaskHandle_t xTask3Handle = NULL;
TaskHandle_t xTask4Handle = NULL;

TickType_t xTotalTickCount = 0;
TickType_t xTaskTick1 = 0;
TickType_t xTaskTick2 = 0;
TickType_t xTaskTick3 = 0;
TickType_t xTaskTick4 = 0;

void main_edf(UART_HandleTypeDef pass_huart) {
	huart2 = pass_huart;

	// Initialize CMSIS/FreeRTOS Kernel
    osKernelInitialize();

    // xTaskVirtCreate --- Used to create VD-compatible tasks. New parameters:
    // Task criticality type: HI_CRIT or LO_CRIT
    // Task criticality scale factor: May be better suited as a global define, but code technically allows for independent definition per-task.
    xTaskVirtCreate( vEDFTask1, "task1", configMINIMAL_STACK_SIZE, NULL, 1, &xTask1Handle, EDF_TASK1_PERIOD, LO_CRIT, 0.80);
    xTaskVirtCreate( vEDFTask2, "task2", configMINIMAL_STACK_SIZE, NULL, 1, &xTask2Handle, EDF_TASK2_PERIOD, LO_CRIT, 0.70);
    xTaskVirtCreate( vEDFTask3, "task3", configMINIMAL_STACK_SIZE, NULL, 1, &xTask3Handle, EDF_TASK3_PERIOD, LO_CRIT, 0.00);
    xTaskVirtCreate( vEDFTask4, "task4", configMINIMAL_STACK_SIZE, NULL, 1, &xTask4Handle, EDF_TASK4_PERIOD, LO_CRIT, 0.00);

    // Periodic task creation functions used for initial testing
//    xTaskPeriodicCreate( vEDFTask1, "task1", configMINIMAL_STACK_SIZE, NULL, 1, &xTask1Handle, EDF_TASK1_PERIOD);
//    xTaskPeriodicCreate( vEDFTask2, "task2", configMINIMAL_STACK_SIZE, NULL, 1, &xTask2Handle, EDF_TASK2_PERIOD);
//    xTaskPeriodicCreate( vEDFTask3, "task3", configMINIMAL_STACK_SIZE, NULL, 1, &xTask3Handle, EDF_TASK3_PERIOD);
//    xTaskPeriodicCreate( vEDFTask4, "task4", configMINIMAL_STACK_SIZE, NULL, 1, &xTask4Handle, EDF_TASK4_PERIOD);

    uint8_t startmsg[] = "Initial tasks created, starting kernel...\r\n";
    HAL_UART_Transmit(&huart2, startmsg, strlen((char*)startmsg), 1000);

    vTaskSuspend(xTask3Handle);
    vTaskSuspend(xTask4Handle);

    // Start FreeRTOS Kernel
    osKernelStart();

    for ( ; ; );
}

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
//	uint8_t testbuf[] = "button interrupt detected\r\n";

	BaseType_t pxYieldRequired;
	if (GPIO_Pin == B1_Pin) {

		pxYieldRequired = xCritShiftFromISR(xTask1Handle, HI_CRIT);
		portYIELD_FROM_ISR(pxYieldRequired);

		// Test of basic resume functionality for low-crit tasks
//		pxYieldRequired = xCritShiftFromISR(xTask3Handle, LO_CRIT);
//		portYIELD_FROM_ISR(pxYieldRequired);

		// Test of suspend functionality for low-crit tasks
		// Not yet implemented - is a flag the best approach?
	}
	else {
		__NOP();
	}
}

void vEDFTask1(void *pvParameters) {
	const TickType_t xTaskFreq = EDF_TASK1_PERIOD;
	TickType_t xLastWakeTime;

	uint8_t task_start[] = "S1-";
	uint8_t task_end[] = "F1-";

	// uint16_t jobcount = 0;
	// uint8_t job_str[12];

	TickType_t capacity = EDF_TASK1_BUDGET;

	xLastWakeTime = xTaskGetTickCount();
	for(;;) {
		// jobcount++;
		// sprintf(job_str, "%d\t", jobcount);

		HAL_UART_Transmit(&huart2, task_start, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xLastWakeTime);

		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

		while(xTaskTick1 != capacity);
		xTaskTick1 = 0;

		TickType_t xFinishedTime = xTaskGetTickCount();
		HAL_UART_Transmit(&huart2, task_end, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xFinishedTime);
		uart_write_char('\n');

		vTaskDelayUntil(&xLastWakeTime, xTaskFreq);
	}
}

void vEDFTask2(void *pvParameters) {
	const TickType_t xTaskFreq = EDF_TASK2_PERIOD;
	TickType_t xLastWakeTime;

	uint8_t task_start[] = "S2-";
	uint8_t task_end[] = "F2-";

	// uint16_t jobcount = 0;
	// uint8_t job_str[20];

	TickType_t capacity = EDF_TASK2_BUDGET;

	xLastWakeTime = xTaskGetTickCount();
	for(;;) {
		// jobcount++;
		// sprintf(job_str, "%d\t", jobcount);
		HAL_UART_Transmit(&huart2, task_start, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xLastWakeTime);

		while(xTaskTick2 != capacity);
		xTaskTick2 = 0;

		TickType_t xFinishedTime = xTaskGetTickCount();
		HAL_UART_Transmit(&huart2, task_end, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xFinishedTime);
		uart_write_char('\n');

		vTaskDelayUntil(&xLastWakeTime, xTaskFreq);
	}
}

void vEDFTask3(void *pvParameters) {
	const TickType_t xTaskFreq = EDF_TASK3_PERIOD;
	TickType_t xLastWakeTime;

	uint8_t task_start[] = "S3-";
	uint8_t task_end[] = "F3-";

	// uint16_t jobcount = 0;
	// uint8_t job_str[20];

	TickType_t capacity = EDF_TASK3_BUDGET;

	xLastWakeTime = xTaskGetTickCount();
	for(;;) {
		// jobcount++;
		// sprintf(job_str, "%d\t", jobcount);
		HAL_UART_Transmit(&huart2, task_start, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xLastWakeTime);

		while(xTaskTick3 != capacity);
		xTaskTick3 = 0;

		TickType_t xFinishedTime = xTaskGetTickCount();
		HAL_UART_Transmit(&huart2, task_end, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xFinishedTime);
		uart_write_char('\n');

		vTaskDelayUntil(&xLastWakeTime, xTaskFreq);
	}
}

void vEDFTask4(void *pvParameters) {
	const TickType_t xTaskFreq = EDF_TASK4_PERIOD;
	TickType_t xLastWakeTime;

	uint8_t task_start[] = "S4-";
	uint8_t task_end[] = "F4-";

	// uint16_t jobcount = 0;
	// uint8_t job_str[20];

	TickType_t capacity = EDF_TASK4_BUDGET;

	xLastWakeTime = xTaskGetTickCount();
	for(;;) {
		// jobcount++;
		// sprintf(job_str, "%d\t", jobcount);
		HAL_UART_Transmit(&huart2, task_start, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xLastWakeTime);

		while(xTaskTick4 != capacity);
		xTaskTick4 = 0;

		TickType_t xFinishedTime = xTaskGetTickCount();
		HAL_UART_Transmit(&huart2, task_end, 3, 50);
		// HAL_UART_Transmit(&huart2, job_str, strlen(job_str), 25);
		HAL_UART_write_uint32(xFinishedTime);
		uart_write_char('\n');

		vTaskDelayUntil(&xLastWakeTime, xTaskFreq);
	}
}

void uart_write_char(uint8_t ch) {
	HAL_UART_Transmit(&huart2, &ch, 1, 100);
}

void printState (eTaskState taskState) {
	uint8_t buffer[14];

	switch(taskState) {
	case eRunning:
		strcpy(buffer, "eRunning\r\n");
		break;
	case eReady:
		strcpy(buffer, "eReady\r\n");
		break;
	case eBlocked:
		strcpy(buffer, "eBlocked\r\n");
		break;
	case eSuspended:
		strcpy(buffer, "eSuspended\r\n");
		break;
	case eDeleted:
		strcpy(buffer, "eDeleted\r\n");
		break;
	default:
		strcpy(buffer, "ERROR\r\n");
		break;
	}

	HAL_UART_Transmit(&huart2, buffer, strlen((char*)buffer), 1000);

}
// Parses an unsigned long (uint32) into a buffer for the HAL function.
void HAL_UART_write_uint32(uint32_t n) {
	uint8_t longbuf[10];

	longbuf[0] = (uint8_t)(48 + n/1000000);	// MSB
	n = n % 1000000;
	longbuf[1] = (uint8_t)(48 + n/100000);
	n = n % 100000;
	longbuf[2] = (uint8_t)(48 + n/10000);
	n = n % 10000;
	longbuf[3] = (uint8_t)(48 + n/1000);
	n = n % 1000;
	longbuf[4] = (uint8_t)(48 + n/100);
	n = n % 100;
	longbuf[5] = (uint8_t)(48 + n/10);
	n = n % 10;
	longbuf[6] = (uint8_t)(48 + n);			// LSB

	// Terminate string properly
	longbuf[7] = '\r';
	longbuf[8] = '\n';
	longbuf[9] = '\0';

	// Send buffer to
	HAL_UART_Transmit(&huart2, longbuf, 9, 100);
}

void update_task_tick_counts(char * taskName, TickType_t currentTick)
{
    if (strcmp(taskName, "task1") == 0)
        xTaskTick1 += (currentTick - xTotalTickCount);
    else if (strcmp(taskName, "task2") == 0)
        xTaskTick2 += (currentTick - xTotalTickCount);
    else if (strcmp(taskName, "task3") == 0)
        xTaskTick3 += (currentTick - xTotalTickCount);
    else if (strcmp(taskName, "task4") == 0)
        xTaskTick4 += (currentTick - xTotalTickCount);

    xTotalTickCount = currentTick;
}
