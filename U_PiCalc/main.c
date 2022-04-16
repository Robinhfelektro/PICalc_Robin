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
#include "avr_f64.h"												//Library Include


void vControllerTask(void* pvParameters);
void vLeibnizTask(void* pvParameters);
void vEuler_PI(void* pvParameters);




EventGroupHandle_t egPI_Calc;


#define	TASK_CHUDNOVSKY				1 << 0	
#define TASK_LEIBNITZ				1 << 1		
#define TASK_EULER					1 << 2		 //https://3.141592653589793238462643383279502884197169399375105820974944592.eu/pi-berechnen-formeln-und-algorithmen/
	
#define DATA_READ_REQUEST_LOCK		1 << 3
#define DATA_CALCULATION_READY		1 << 4
#define DATA_READ_LOCK_CLEARED		1 << 5

#define RESET_PI					1 << 6

#define BUTTON_START				1 << 7
#define BUTTON_STOP					1 << 8
#define BUTTON_RESET				1 << 9
#define BUTTON_SWITCH				1 << 10

#define TASK_START_STOP				1 << 11

#define TASK_TIME_FINISHED			1 << 12

#define RESET_EG_BUTTONS			0x780 //Reset all Buttons


//Switch

#define STATE_STOP 0
#define STATE_START 1


float32_t Leib_PI = 0;  
float32_t g_Euler_PI = 0; 
float64_t g_f_Euler_PI;

int main(void)
{
    vInitClock();
	vInitDisplay();
	
	egPI_Calc = xEventGroupCreate();

	
	xTaskCreate( vControllerTask,	(const char *) "control_tsk",	configMINIMAL_STACK_SIZE+1000, NULL, 3, NULL);
	xTaskCreate( vLeibnizTask,		(const char *) "leibniz_tsk",	configMINIMAL_STACK_SIZE+500, NULL, 1, NULL);
	xTaskCreate( vEuler_PI,			(const char *) "Euler_PI_tsk",	configMINIMAL_STACK_SIZE+500, NULL, 1, NULL);
	
	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc FS2022");
	vTaskStartScheduler();
	return 0;
}


uint64_t rCalcFakultaet(uint64_t n)
{
	int c;
	uint64_t r = 1;

	for (c = 1; c <= n; c++)
	r = r * c;

	return r;
}



void vEuler_PI(void* pvParameters)  {
	uint32_t counter = 1; 
	float32_t  lokal_euler_pi;
	float32_t  euler_pi_calc;
	
	
	float64_t f_euler_pi_calc;
	float64_t f_lokal_euler_pi; 
	
	
	for(;;) 
	{
		euler_pi_calc = euler_pi_calc + ( 1.0 / pow(counter, 2) );
		lokal_euler_pi = sqrt( (euler_pi_calc * 6.0 ) );
		g_Euler_PI = lokal_euler_pi; 
		
		
		f_euler_pi_calc = f_add(f_euler_pi_calc, f_div( f_sd(1) , f_pow( f_sd(counter), f_sd(2) ) ));  //zu viel f_sd?
		f_lokal_euler_pi = f_pow( f_mult( f_euler_pi_calc, f_sd(6)) , f_sd(0.5));  //wurzel mit pow
		g_f_Euler_PI = f_lokal_euler_pi; 
		
		counter++;
		//vTaskDelay(1/portTICK_RATE_MS);
	}
}


void vLeibnizTask(void* pvParameters) {
	
	uint32_t counter = 3; 
	float32_t  lokal_leib_pi;
	
	float32_t pi4 = 1;
	
	
	
	char pi_compare[8];
	
	char ululu[] = "12345";
	char ululu2[] = "13345";
	char pi_comp_loesung[] = "3.14159";
	
	uint8_t lokal_task_flag = 0; 
	
	while(1)
	{
		//task auswahl
		if ( (xEventGroupGetBits(egPI_Calc) & (TASK_LEIBNITZ | TASK_START_STOP) ) == (TASK_LEIBNITZ | TASK_START_STOP))  //task enable
		{
			
			if ( (xEventGroupGetBits(egPI_Calc) & RESET_PI ) == RESET_PI)			//reset
			{
				xEventGroupClearBits(egPI_Calc, RESET_PI);
				lokal_leib_pi = 0; 
				pi4 = 0; 
				counter = 3; 
				lokal_task_flag = pdFALSE;
				
			}
			
			pi4 = pi4 - (1.0 / counter);  //1.0 damit als float erkannt int counter
			counter += 2;
			pi4 = pi4 + (1. / counter);
			counter += 2;
			lokal_leib_pi = pi4 * 4;
			
			if ( (xEventGroupGetBits(egPI_Calc) & DATA_READ_REQUEST_LOCK) == DATA_READ_REQUEST_LOCK)
			{
				xEventGroupSetBits(egPI_Calc, DATA_CALCULATION_READY);
				xEventGroupWaitBits(egPI_Calc, DATA_READ_LOCK_CLEARED, pdTRUE, pdFALSE, portTICK_RATE_MS);
			}
			
			Leib_PI = lokal_leib_pi;	
			
			sprintf(&pi_compare[0], "%f", lokal_leib_pi);
			pi_compare[7] = '\0';

			if ( strcmp(pi_comp_loesung, pi_compare ) == 0)
			{
				
				if (lokal_task_flag == pdFALSE )
				{
					xEventGroupSetBits(egPI_Calc, TASK_TIME_FINISHED);
					lokal_task_flag = pdTRUE; 
					
				}
			}

		}
	}
	
}





void vControllerTask(void* pvParameters) {
	initButtons();
	uint8_t counter_500ms = 0; 
	uint8_t task_modus = 0; 
	
	TickType_t xTaskTimeStart; 
	TickType_t xTaskTimeStop; 
	uint16_t Calculation_Time_ms = 0; 
	uint16_t Calculation_Time_s = 0; 
	uint16_t lokal_time_meas = 0; 
	
	char s_result_chudnov[20];
	char s_result_leibn[12];
	char s_result_euler[12];
	float32_t lokal_leib_pi;
	uint8_t mode = STATE_STOP; 
	bool b_Time_meas_finished;
	xEventGroupSetBits(egPI_Calc, TASK_LEIBNITZ);		//start with leib
	
	for(;;) {
		updateButtons();
		
		if (counter_500ms == 0)        //ausgabe alle 500 ms, 50 * 10ms
		{
			if ((xEventGroupGetBits(egPI_Calc) & BUTTON_START) == BUTTON_START)
			{
				
				mode = STATE_START;
				xTaskTimeStart = xTaskGetTickCount(); 
				xEventGroupSetBits(egPI_Calc, TASK_START_STOP);
				

				
			}
			if ((xEventGroupGetBits(egPI_Calc) & BUTTON_STOP) == BUTTON_STOP)
			{
				mode = STATE_STOP;
				xEventGroupClearBits(egPI_Calc, TASK_START_STOP);
			}
			if ((xEventGroupGetBits(egPI_Calc) & BUTTON_RESET) == BUTTON_RESET)
			{
				xEventGroupSetBits(egPI_Calc, RESET_PI);
				xEventGroupClearBits(egPI_Calc, TASK_TIME_FINISHED);
				b_Time_meas_finished = pdFALSE; 
				
			}
			if ((xEventGroupGetBits(egPI_Calc) & BUTTON_SWITCH) == BUTTON_SWITCH)
			{
				task_modus++; 
				if (task_modus == 2)
				{
					task_modus = 0; 
				}
			}
			switch (mode)
			{
				case STATE_STOP: 
				
					vDisplayClear();
					vDisplayWriteStringAtPos(0,0,"Mode:Stop, PI: ---");
					vDisplayWriteStringAtPos(1,0,"Press Start to");
					vDisplayWriteStringAtPos(2,0,"begin calculation");
					vDisplayWriteStringAtPos(3,0,"Start-Stop-Res-Swit");
					
					
				break;	
				case STATE_START:
				
					vDisplayClear();
					vDisplayWriteStringAtPos(0,0,"Mode:Start, PI: Leib");
					vDisplayWriteStringAtPos(3,0,"Start-Stop-Res-Swit");
					
					
					
					xEventGroupSetBits(egPI_Calc, DATA_READ_REQUEST_LOCK);
					xEventGroupWaitBits(egPI_Calc, DATA_CALCULATION_READY, pdTRUE, pdFALSE, portMAX_DELAY);
					//lokale Variable??? mit Gobaler überschreiben??
					lokal_leib_pi = Leib_PI;
					xEventGroupSetBits(egPI_Calc, DATA_READ_LOCK_CLEARED);  //berechnung wieder freigeben --> globale variable
					xEventGroupClearBits(egPI_Calc, DATA_READ_REQUEST_LOCK);
					sprintf(&s_result_leibn[0], "PI: %.8f", lokal_leib_pi);
					vDisplayWriteStringAtPos(1,0, "%s", s_result_leibn);
					
					
					if ((xEventGroupGetBits(egPI_Calc) & TASK_TIME_FINISHED) == TASK_TIME_FINISHED)
					{
						xTaskTimeStop = xTaskGetTickCount(); 
						Calculation_Time_ms = (xTaskTimeStop - xTaskTimeStart);
						Calculation_Time_s = Calculation_Time_ms / 1000; 
						vDisplayWriteStringAtPos(2,0,"time s: %d", Calculation_Time_s);
						xEventGroupClearBits(egPI_Calc, TASK_TIME_FINISHED);
						b_Time_meas_finished = pdTRUE;
						
						
					}
					else
					{
						if (b_Time_meas_finished == pdFALSE)
						{
							lokal_time_meas += 500;
							vDisplayWriteStringAtPos(2,0,"time s: %d", lokal_time_meas / 1000);
						}
						else
						{
							vDisplayWriteStringAtPos(2,0,"time s: %d", Calculation_Time_s);
						}
						
					}
					
					
					

				break; 
				
			}
			
			//euler test
			//sprintf(&s_result_euler[0], "PI: %.8f", g_Euler_PI);
			//vDisplayWriteStringAtPos(2,0, "%s", s_result_euler);
			//
			////float 64 euler test
			//char* tempResultString = f_to_string(g_f_Euler_PI, 16, 16);		//Verwandeln einer Double-Variable in einen String
			//sprintf(s_result_chudnov, "1: %s", tempResultString);			//Einsetzen des Strings in einen anderen String
			//vDisplayWriteStringAtPos(2,0,"%s", s_result_chudnov);

			xEventGroupClearBits(egPI_Calc, RESET_EG_BUTTONS); //rest buttons
			counter_500ms = 50; 
		}
		else
		{
			counter_500ms--; 
		}
		
		
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			xEventGroupSetBits(egPI_Calc, BUTTON_START);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			xEventGroupSetBits(egPI_Calc, BUTTON_STOP);
			
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			xEventGroupSetBits(egPI_Calc, BUTTON_RESET);
			
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			xEventGroupSetBits(egPI_Calc, BUTTON_SWITCH);
			
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