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
	
	xTaskCreate( vControllerTask,	(const char *) "control_tsk", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	xTaskCreate( vLeibnizTask,		(const char *) "leibniz_tsk", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	xTaskCreate( vChudnovskyTask,	(const char *) "chudnovsky_tsk", configMINIMAL_STACK_SIZE+150, NULL, 2, NULL);
	
	
	

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc FS2022");
	vTaskStartScheduler();
	return 0;
}


uint64_t rCalcFakultaet(uint64_t n)
{
	int c;
	float64_t r = 1;

	for (c = 1; c <= n; c++)
	r = r * c;

	return r;
}


void vChudnovskyTask(void* pvParameters)
{
	float64_t testvar1 = f_sd(2);									//Erstellen einer Double-Variable, Initialisiert mit dem Wert 2
	float64_t testvar2 = f_sd(12);
	uint32_t count_pi = 0; 
	
	
	uint64_t A = 13591409;
	uint64_t B = 545140134;
	float64_t C = f_sd(640320);
	
	uint64_t D = 426880;
	uint64_t E = 10005;
	
	
	
	uint64_t Zaehler = 0; 
	uint64_t Nenner = 0; 
	
	float64_t f_Zaehler = f_sd(0); 
	float64_t f_Nenner  = f_sd(0);
	float64_t f_help_var1 = f_sd(0);
	float64_t f_help_var2 = f_sd(10);
	float64_t f_help_var3 = f_sd(10);
	
	float64_t Chudnov_Sum = f_sd(0); 
	float64_t Chudnov_PI = f_sd(0); 
	
	uint64_t fak = 0;
	uint64_t fak2 = 0;
	float64_t fak3 = 0;
	fak = (rCalcFakultaet(3 * count_pi));
	fak2 = ( pow(rCalcFakultaet(count_pi), 3));
	//fak3 = f_pow(1.1, 2.2);
	fak3 = pow(1.1, 2.2);
	
	
	while(1)
	{
		Zaehler = (pow(-1, count_pi)) * (rCalcFakultaet(6 * count_pi)) * (B * count_pi + A);  //
		
		
		//f_help_var1 = f_pow(C, f_help_var2);
		//f_help_var1 = (f_exp(f_mult((y), f_log(x))))
		//f_help_var1 = (f_exp(f_mult((f_help_var2), f_log(f_help_var3))));
		
		Nenner	= (rCalcFakultaet(3 * count_pi)) * ( pow(rCalcFakultaet(count_pi), 3)) * ( pow (C, 3* count_pi + 3 / 2  )); //C bereits als ein float?
		f_Zaehler = f_sd(Zaehler);
		f_Nenner = f_sd(Nenner); 
		Chudnov_Sum = f_Nenner / f_Zaehler; 
		Chudnov_PI = 1 / (12 * Chudnov_Sum);
		
		count_pi++; 
		
		char pistring2[12];
		sprintf(&pistring2[0], "PI_C: %.8f", Chudnov_PI);
		vDisplayWriteStringAtPos(3,0, "%s", pistring2);
		
	}
	/*
	 pi_chud+=(((Decimal(-1))**k ) * (Decimal(mp.factorial(6*k)))*(13591409 + 545140134*k))/Decimal((mp.factorial(3*k)*((mp.factorial(k))**3)*(640320**((3*k)+(Decimal(1.5))))))
	 k+=1
	 pi_chud = (Decimal(pi_chud) * 12)
	 pi_chud = (Decimal(pi_chud**(-1)))
	 return int(pi_chud*10**n)
	 exact_pi_val = str(31415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989)
	 for n in range(1,1000):
	 print(int(exact_pi_val[:n+1]))
	 print(pi_chudn(n))
	 is_true = (pi_chudn(n) == int(exact_pi_val[:n+1]))
	 print("for n = ",n, " It is ",is_true)
	 if is_true == False:
	 break
	
	*/
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



void vControllerTask(void* pvParameters) {
	initButtons();
	uint8_t counter_500ms = 0; 
	
	for(;;) {
		updateButtons();
		
		if (counter_500ms == 0)        //ausgabe alle 500 ms, 50 * 10ms
		{
			//xEventGroupWaitBits(egPI_Calc, egPI_CALCULATE_DONE, pdTRUE, pdFALSE, portMAX_DELAY);  //wait for finished pi calculation
			//xEventGroupClearBits(egPI_Calc, egPI_WRITE_DISPLAY);   //block calculating pi?
			vDisplayClear();
			counter_500ms = 50; 
			char pistring[12];
			//sprintf(&pistring[0], "PI: %.8f", M_PI);
			sprintf(&pistring[0], "PI: %.8f", pi);
			vDisplayWriteStringAtPos(1,0, "%s", pistring);
			//xEventGroupSetBits(egPI_Calc, egPI_WRITE_DISPLAY);			//enable calculating pi?
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