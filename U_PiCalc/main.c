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
void vEuler_PI(void* pvParameters);
void Anzeige(void* pvParameters);




EventGroupHandle_t egPI_Calc;


#define	TASK_CHUDNOVSKY				1 << 0	
#define TASK_LEIBNITZ				1 << 1		
#define TASK_EULER					1 << 2		 //https://3.141592653589793238462643383279502884197169399375105820974944592.eu/pi-berechnen-formeln-und-algorithmen/
	
#define DATA_READ_REQUEST_LOCK		1 << 3
#define DATA_CALCULATION_READY		1 << 4
#define DATA_READ_LOCK_CLEARED		1 << 5

#define RESET_PI					1 << 6




//#define egPI_CALCULATE_PROGRESS		1	<< 0
//#define egPI_CALCULATE_DONE			1	<< 1
//#define egPI_WRITE_DISPLAY			1	<< 2

//#define LOCK_DATA	 1 << 0
//#define DATA_READY	 1 << 1
//#define LOCK_CLEARED 1 << 2
//#define LED1ENABLE	 1 << 3
//#define LED2ENABLE	 1 << 4
//#define RESET1		 1 << 5
//#define RESET2		 1 << 6
//#define ALGO1		 1 << 8
//#define ALGO2		 1 << 9





float32_t pi4 = 1;
float32_t Leib_PI = 0;  
float32_t g_Euler_PI = 0; 
float64_t f_Chudnov_PI;
float64_t g_f_Euler_PI;

int main(void)
{
    vInitClock();
	vInitDisplay();
	
	egPI_Calc = xEventGroupCreate();
	xEventGroupSetBits(egPI_Calc, TASK_LEIBNITZ);			//temporär task enable
	
	xTaskCreate( vControllerTask,	(const char *) "control_tsk",	configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( vLeibnizTask,		(const char *) "leibniz_tsk",	configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	xTaskCreate( vEuler_PI,			(const char *) "Euler_PI_tsk",	configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	xTaskCreate( vChudnovskyTask,	(const char *) "Chud_PI_tsk",	configMINIMAL_STACK_SIZE+1000, NULL, 2, NULL);
	xTaskCreate( Anzeige,			(const char *) "Anzeige_tsk",	configMINIMAL_STACK_SIZE+1000, NULL, 2, NULL);
	
	
	

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
		
		//achtung wahrscheinlich f_squr nohc aktivieren oder mit f_pow lösen
		f_lokal_euler_pi = f_sqrt( f_mult( f_euler_pi_calc, f_sd(6)));   //testen ob f_sd 6 oder nicht unterschie macht
		g_f_Euler_PI = f_lokal_euler_pi; 
		
		counter++;
		
		
		
		
		
		vTaskDelay(1/portTICK_RATE_MS);
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
	float64_t f_Zahler = f_mult( f_sd(426880) , f_pow( f_sd(10005), f_sd(0.5)) );
	
	
	
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
		
		
		//zweite berechnung
		 
 		//f_chud_helpA = f_div( f_sd(  pow( -1, count_pi) *  rCalcFakultaet(6 * count_pi)), f_sd(  rCalcFakultaet(3 * count_pi) * rCalcFakultaet(count_pi) * pow(640320, (3 * count_pi))) );
 		//f_chud_helpB = f_mult( f_sd(count_pi) , f_chud_helpA);
		//f_Chudnov_PI = f_chud_helpB;
 		
 		//f_Chudnov_PI = f_div(   f_Zahler  ,  (   f_add( f_mult(f_sd(13591409), f_chud_helpA )  ,   f_mult( f_sd(545140134) , f_chud_helpB)) )    ); //dividend ist sehr hoch 
		
		count_pi++; 
		
		vTaskDelay(100/portTICK_RATE_MS);
	
		
	}

}

void vLeibnizTask(void* pvParameters) {

	uint32_t counter = 3; 
	float32_t  lokal_leib_pi;
	
	while(1)
	{
		//task auswahl
		if ( (xEventGroupGetBits(egPI_Calc) & TASK_LEIBNITZ) == TASK_LEIBNITZ)  //task enable
		{
			if ( (xEventGroupGetBits(egPI_Calc) & RESET_PI ) == RESET_PI)			//reset
			{
				xEventGroupClearBits(egPI_Calc, RESET_PI);
				lokal_leib_pi = 0; 
			}
			
			
			pi4 = pi4 - (1.0 / counter);  //1.0 damit als float erkannt int counter
			counter += 2;
			pi4 = pi4 + (1. / counter);
			counter += 2;
			Leib_PI = pi4 * 4;
			
			
			if ( (xEventGroupGetBits(egPI_Calc) & DATA_READ_REQUEST_LOCK) == DATA_READ_REQUEST_LOCK)
			{
				xEventGroupSetBits(egPI_Calc, DATA_CALCULATION_READY);
				xEventGroupWaitBits(egPI_Calc, DATA_READ_LOCK_CLEARED, pdTRUE, pdFALSE, portTICK_RATE_MS);
			}
			Leib_PI = lokal_leib_pi;	
			if (lokal_leib_pi == M_PI)   //foto für zeitmessung anschaue
			{							//string vergleichen
										//multiplzieren mit 10000 und danach int machen
			}
		}
	}
	
}

void Anzeige(void* pvParameters) 
{
	
	//f_Chudnov_PI = f_sd(0.99999);
	//char chundnov_result_sting[20];
	

	char s_result_chudnov[20];
	char s_result_leibn[12];
	char s_result_euler[12];
	float32_t lokal_leib_pi;
	float64_t lokal_chod_pi;
	
	for(;;)
	{
		
		
		//alle 500 ms Logik
		xEventGroupSetBits(egPI_Calc, DATA_READ_REQUEST_LOCK);
		xEventGroupWaitBits(egPI_Calc, DATA_CALCULATION_READY, pdTRUE, pdFALSE, portMAX_DELAY); 
		//lokale Variable??? mit Gobaler überschreiben??
		lokal_chod_pi =  f_Chudnov_PI; 
		lokal_leib_pi = Leib_PI;
		xEventGroupSetBits(egPI_Calc, DATA_READ_LOCK_CLEARED);  //berechnung wieder freigeben --> globale variable
		xEventGroupClearBits(egPI_Calc, DATA_READ_REQUEST_LOCK);
		
		
		
		vDisplayClear();
		

		sprintf(&s_result_leibn[0], "PI: %.8f", lokal_leib_pi);
		vDisplayWriteStringAtPos(1,0, "%s", s_result_leibn);
		
		//float 64 chudo
		//char* tempResultString = f_to_string(lokal_chod_pi, 16, 16);		//Verwandeln einer Double-Variable in einen String
		//sprintf(s_result_chudnov, "1: %s", tempResultString);			//Einsetzen des Strings in einen anderen String
		//vDisplayWriteStringAtPos(2,0,"%s", s_result_chudnov);	
		//vDisplayWriteStringAtPos(3,0,"2 as float: %f", f_ds(f_Chudnov_PI));
		
		
		//euler test
		
		sprintf(&s_result_euler[0], "PI: %.8f", g_Euler_PI);
		vDisplayWriteStringAtPos(0,0, "%s", s_result_euler);
		
		
		//float 64 euler test
		char* tempResultString = f_to_string(g_f_Euler_PI, 16, 16);		//Verwandeln einer Double-Variable in einen String
		sprintf(s_result_chudnov, "1: %s", tempResultString);			//Einsetzen des Strings in einen anderen String
		vDisplayWriteStringAtPos(2,0,"%s", s_result_chudnov);
		vDisplayWriteStringAtPos(3,0,"2 as float: %f", f_ds(f_Chudnov_PI));
		
		
		
		
		//delay until ausprobieren für 500ms takt
		
		vTaskDelay(500 / portTICK_RATE_MS); //SOLL ALLE 500MS 
		
		
		
		
		
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