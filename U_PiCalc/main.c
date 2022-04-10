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
void vChudnovskyTask(void* pvParameters);
void testtask(void* pvParameters);
void Anzeige(void* pvParameters);


#define egPI_CALCULATE_PROGRESS		1	<< 0
#define egPI_CALCULATE_DONE			1	<< 1
#define egPI_WRITE_DISPLAY			1	<< 2

EventGroupHandle_t egPI_Calc;

float pi4 = 1;
float pi = 0;  
float64_t f_Chudnov_PI;

int main(void)
{
    vInitClock();
	vInitDisplay();
	
	egPI_Calc = xEventGroupCreate();
	
	xTaskCreate( vControllerTask,	(const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( vLeibnizTask,		(const char *) "leibniz_tsk", configMINIMAL_STACK_SIZE+159, NULL, 1, NULL);
	xTaskCreate( testtask,		(const char *) "testtask", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	xTaskCreate( vChudnovskyTask,	(const char *) "ky_tsk", configMINIMAL_STACK_SIZE+1500, NULL, 2, NULL);
	xTaskCreate( Anzeige, (const char *) "Anzeige", configMINIMAL_STACK_SIZE+500, NULL, 2, NULL);
	
	
	

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




void testtask(void* pvParameters)  {
	uint8_t x = 0;
	for(;;) {
		x++;
		vTaskDelay(10/portTICK_RATE_MS);
	}
}

void vChudnovskyTask(void* pvParameters)
{
	
	
	
	
	uint32_t count_pi = 1;  //summenzähler
/*	uint64_t Zaehler = 0; */
	
// 	uint64_t A = 13591409;
// 	uint64_t B = 545140134;
// 	float64_t f_Chudnov_Calculate; 
// 	float64_t f_chud_help1; 
// 	float64_t f_chud_help2; 

	float64_t f_chud_helpA; 
	float64_t f_chud_helpB; 
	float64_t f_Zahler = f_sd( 426880 * f_pow( f_sd(10005), f_sd(0.5))) ; 
	
//	float32_t f_sd_testvar; 


	
	while(1)
	{
		if(count_pi >= 2) 
		{
			while(1)
			{
				vTaskDelay(100/portTICK_RATE_MS);
			}
		}
		
		//Zaehler = (pow(-1, count_pi)) * (rCalcFakultaet(6 * count_pi)) * (B * count_pi + A);  //zahl wird schnell sehr gross
		//f_Chudnov_Calculate =  f_div( (f_sd(Zaehler)) , f_mult( f_pow( f_sd(640320) , (3 * count_pi + (3 / 2))) , f_sd( pow(rCalcFakultaet(count_pi), 3) * ( rCalcFakultaet(3*count_pi )))) );
		//
		//f_Chudnov_PI =  f_add( f_div( f_sd(1), f_Chudnov_Calculate), f_Chudnov_PI )  ;
		//f_Chudnov_PI =  f_add( f_div( f_sd(1), f_Chudnov_Calculate), f_Chudnov_PI )  ;
		//f_Chudnov_PI =  f_add(   f_div( f_sd(1) , f_mult( f_sd(12), f_Chudnov_Calculate)) , f_Chudnov_PI );    
		//f_Chudnov_PI =  f_add(   f_div( f_sd(1) , f_mult( f_sd(12), f_Chudnov_Calculate)) , f_Chudnov_PI );   
		
		 
		//f_chud_help1 =      f_sd( pow( -1, count_pi) *  rCalcFakultaet(6 * count_pi));   //nenner und zähler mit tr und k = 1 überprüft, stimmt   
		//f_chud_help2 = f_sd(  rCalcFakultaet(3 * count_pi) * rCalcFakultaet(count_pi) * pow(640320, (3 * count_pi)) );
		
 		f_chud_helpA = f_div( f_sd( pow( -1, count_pi) *  rCalcFakultaet(6 * count_pi)), f_sd(  rCalcFakultaet(3 * count_pi) * rCalcFakultaet(count_pi) * pow(640320, (3 * count_pi))) );
 		f_chud_helpB = f_mult( f_sd(count_pi) , f_chud_helpA);
 		
 		f_Chudnov_PI = f_div( f_Zahler  ,  (   f_add( f_mult(f_sd(13591409), f_chud_helpA )  ,   f_mult( f_sd(545140134) , f_chud_helpB)) )    );
		
		//count_pi++; 
		
		vTaskDelay(100/portTICK_RATE_MS);
	
		
	}

}

void vLeibnizTask(void* pvParameters) {

	uint32_t counter = 3; 
	
	while(1)
	{
		//xEventGroupSetBits(egPI_Calc, egPI_CALCULATE_PROGRESS);		//calculate Flag
		
		//xEventGroupWaitBits(egPI_Calc, egPI_WRITE_DISPLAY, pdTRUE, pdFALSE, portMAX_DELAY);  //wait for calculate enable?
		
		pi4 = pi4 - (1.0 / counter);  //1.0 damit als float erkannt int counter
		counter += 2; 
		pi4 = pi4 + (1. / counter);
		counter += 2; 
		pi = pi4 * 4; 
		
		if (pi >= 3.14)
		{
			//xEventGroupSetBits(egPI_Calc, egPI_CALCULATE_DONE);		//pi calculated
		}
		
		
	}
	
}

void Anzeige(void* pvParameters) 
{
	
	//f_Chudnov_PI = f_sd(0.99999);
	//char chundnov_result_sting[20];
	
	float64_t testvar1 = f_sd(2);									//Erstellen einer Double-Variable, Initialisiert mit dem Wert 2
	float64_t testvar2 = f_sd(3);
	char s_result_chudnov[20];
	char s_result_leibn[12];
	
	
	
	
	for(;;)
	{
		vDisplayClear();
		
		

		//sprintf(&pistring[0], "PI: %.8f", M_PI);
		sprintf(&s_result_leibn[0], "PI: %.8f", pi);
		vDisplayWriteStringAtPos(1,0, "%s", s_result_leibn);
		
		//float 64
		char* tempResultString = f_to_string(f_Chudnov_PI, 16, 16);		//Verwandeln einer Double-Variable in einen String
		sprintf(s_result_chudnov, "1: %s", tempResultString);			//Einsetzen des Strings in einen anderen String
		vDisplayWriteStringAtPos(2,0,"%s", s_result_chudnov);;		vDisplayWriteStringAtPos(3,0,"2 as float: %f", f_ds(f_Chudnov_PI));
		
		
		vTaskDelay(400 / portTICK_RATE_MS);
		
		
		
		
		
	}	
	
}


void vControllerTask(void* pvParameters) {
	initButtons();
	uint8_t counter_500ms = 0; 
	
	for(;;) {
		updateButtons();
		
		//if (counter_500ms == 0)        //ausgabe alle 500 ms, 50 * 10ms
		//{
			////xEventGroupWaitBits(egPI_Calc, egPI_CALCULATE_DONE, pdTRUE, pdFALSE, portMAX_DELAY);  //wait for finished pi calculation
			////xEventGroupClearBits(egPI_Calc, egPI_WRITE_DISPLAY);   //block calculating pi?
			//vDisplayClear();
			//counter_500ms = 50; 
			//char pistring[12];
			////sprintf(&pistring[0], "PI: %.8f", M_PI);
			//sprintf(&pistring[0], "PI: %.8f", pi);
			//vDisplayWriteStringAtPos(1,0, "%s", pistring);
			////xEventGroupSetBits(egPI_Calc, egPI_WRITE_DISPLAY);			//enable calculating pi?
		//}
		//else
		//{
			//counter_500ms--; 
		//}
		//
		
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