/*
 * U_PiCalc_FS2022
 *
 * Created: 20.03.2018 18:32:07
 * Author : Robin bühler
 */ 

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "ButtonHandler.h"

void controllerTask(void* pvParameters);
void leibnizTask(void* pvParameters);






#define egPI_CALCULATE_PROGRESS		1	<< 0
#define egPI_CALCULATE_DONE			1	<< 1
#define egPI_WRITE_DISPLAY			1	<< 2

EventGroupHandle_t egPI_Calc;

float pi4 = 1;
float pi = 0;  

int main(void)
{
    vInitClock();
	vInitDisplay();
	
	egPI_Calc = xEventGroupCreate();
	
	xTaskCreate( controllerTask, (const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( leibnizTask, (const char *) "leibniz_tsk", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc FS2022");
	vTaskStartScheduler();
	return 0;
}

void leibnizTask(void* pvParameters) {

	uint32_t counter = 3; 
	
	while(1)
	{
		xEventGroupSetBits(egPI_Calc, egPI_CALCULATE_PROGRESS);		//calculate Flag
		
		xEventGroupWaitBits(egPI_Calc, egPI_WRITE_DISPLAY, pdTRUE, pdFALSE, portMAX_DELAY);  //wait for calculate enable?
		
		pi4 = pi4 - (1.0 / counter);  //1.0 damit als float erkannt int counter
		counter += 2; 
		pi4 = pi4 + (1. / counter);
		counter += 2; 
		pi = pi4 * 4; 
		
		if (pi >= 3.14)
		{
			xEventGroupSetBits(egPI_Calc, egPI_CALCULATE_DONE);		//pi calculated
		}
	}
	
}



void controllerTask(void* pvParameters) {
	initButtons();
	uint8_t counter_500ms = 0; 
	
	for(;;) {
		updateButtons();
		
		if (counter_500ms == 0)        //ausgabe alle 500 ms, 50 * 10ms
		{
			xEventGroupWaitBits(egPI_Calc, egPI_CALCULATE_DONE, pdTRUE, pdFALSE, portMAX_DELAY);  //wait for finished pi calculation
			xEventGroupClearBits(egPI_Calc, egPI_WRITE_DISPLAY);   //block calculating pi?
			vDisplayClear();
			counter_500ms = 50; 
			char pistring[12];
			//sprintf(&pistring[0], "PI: %.8f", M_PI);
			sprintf(&pistring[0], "PI: %.8f", pi);
			vDisplayWriteStringAtPos(1,0, "%s", pistring);
			xEventGroupSetBits(egPI_Calc, egPI_WRITE_DISPLAY);			//enable calculating pi?
		}
		else
		{
			counter_500ms--; 
		}
		
		
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			char pistring[12];
			sprintf(&pistring[0], "PI: %.8f", M_PI);
			vDisplayWriteStringAtPos(1,0, "%s", pistring);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			
		}
		if(getButtonPress(BUTTON1) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON2) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON3) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON4) == LONG_PRESSED) {
			
		}
		vTaskDelay(10/portTICK_RATE_MS);  //vtast delay until für genauere Zeit?
	}
}