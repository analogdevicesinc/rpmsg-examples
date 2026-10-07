/*****************************************************************************
 * adsp-sc5xx-bm-led-toggle_Core1.c
 *****************************************************************************/

#include "adi_initialize.h"
#include "adsp-sc846-ezkit-bm-led-toggle_Core1.h"

#include <sys/platform.h>
#include <sys/platform.h>
#include <services/gpio/adi_gpio.h>
#include <cycle_count.h>
#include <time.h>

/** 
 * If you want to use command program arguments, then place them in the following string. 
 */
char __argv_string[] = "";


#if defined (__ADSPSC846__)
#define ADI_LED1_PORT ADI_GPIO_PORT_B
#define ADI_LED2_PORT ADI_GPIO_PORT_B
#define ADI_LED3_PORT ADI_GPIO_PORT_B
#define ADI_LED1_PIN ADI_GPIO_PIN_4
#define ADI_LED2_PIN ADI_GPIO_PIN_5
#define ADI_LED3_PIN ADI_GPIO_PIN_6
#elif defined (__ADSPSC598W__) || defined (__ADSPSC598__)
#define ADI_LED1_PORT ADI_GPIO_PORT_C
#define ADI_LED2_PORT ADI_GPIO_PORT_C
#define ADI_LED3_PORT ADI_GPIO_PORT_C
#define ADI_LED1_PIN ADI_GPIO_PIN_1
#define ADI_LED2_PIN ADI_GPIO_PIN_2
#define ADI_LED3_PIN ADI_GPIO_PIN_3
#elif defined (__ADSPSC594W__) || defined (__ADSPSC594__)
#define ADI_LED1_PORT ADI_GPIO_PORT_C
#define ADI_LED2_PORT ADI_GPIO_PORT_C
#define ADI_LED3_PORT ADI_GPIO_PORT_C
#define ADI_LED1_PIN ADI_GPIO_PIN_1
#define ADI_LED2_PIN ADI_GPIO_PIN_2
#define ADI_LED3_PIN ADI_GPIO_PIN_3
#endif

#define LED_BLINK_DELAY_TIME 1 // 1 SECOND

int init_leds(void){

	ADI_GPIO_RESULT ret;

	ret = adi_gpio_PortInit(ADI_LED1_PORT, ADI_LED1_PIN,
		ADI_GPIO_DIRECTION_OUTPUT, false);
	if (ret != ADI_GPIO_SUCCESS)
		return ret;

	ret = adi_gpio_PortInit(ADI_LED2_PORT, ADI_LED2_PIN,
		ADI_GPIO_DIRECTION_OUTPUT, false);
	if (ret != ADI_GPIO_SUCCESS)
		return ret;

	ret = adi_gpio_PortInit(ADI_LED3_PORT, ADI_LED3_PIN,
		ADI_GPIO_DIRECTION_OUTPUT, false);
	if (ret != ADI_GPIO_SUCCESS)
		return ret;

	ret = adi_gpio_SetDirection(ADI_LED1_PORT, ADI_LED1_PIN, ADI_GPIO_DIRECTION_OUTPUT);
	if (ret != ADI_GPIO_SUCCESS)
		return ret;

	ret = adi_gpio_SetDirection(ADI_LED2_PORT, ADI_LED2_PIN, ADI_GPIO_DIRECTION_OUTPUT);
	if (ret != ADI_GPIO_SUCCESS){
		return ret;
	}

	ret = adi_gpio_SetDirection(ADI_LED3_PORT, ADI_LED3_PIN, ADI_GPIO_DIRECTION_OUTPUT);
	if (ret != ADI_GPIO_SUCCESS)
		return ret;

	turn_off_led(0);
	turn_off_led(1);
	turn_off_led(2);
	return 0;
}

void toggle_led(int led){
	switch(led){
	case 0:
		adi_gpio_Toggle(ADI_LED1_PORT, ADI_LED1_PIN);
		break;
	case 1:
		adi_gpio_Toggle(ADI_LED2_PORT, ADI_LED2_PIN);
		break;
	case 2:
		adi_gpio_Toggle(ADI_LED3_PORT, ADI_LED3_PIN);
		break;
	default:
		break;
	}
}

void turn_on_led(int led){
	switch(led){
	case 0:
		adi_gpio_Set(ADI_LED1_PORT, ADI_LED1_PIN);
		break;
	case 1:
		adi_gpio_Set(ADI_LED2_PORT, ADI_LED2_PIN);
		break;
	case 2:
		adi_gpio_Set(ADI_LED3_PORT, ADI_LED3_PIN);
		break;
	default:
		break;
	}
}

void turn_off_led(int led){
	switch(led){
	case 0:
		adi_gpio_Clear(ADI_LED1_PORT, ADI_LED1_PIN);
		break;
	case 1:
		adi_gpio_Clear(ADI_LED2_PORT, ADI_LED2_PIN);
		break;
	case 2:
		adi_gpio_Clear(ADI_LED3_PORT, ADI_LED3_PIN);
		break;
	default:
		break;
	}
}

void my_delay() {
#if defined (__ADSPSC846__)
	//not supported
	//sleep(LED_BLINK_DELAY_TIME);
	volatile int i=100000000;
	while (i>0) {
		i--;
	}

#else
	/* The SHARC+ run-time library has no sleep(): spin on the EMUCLK cycle counter */
	unsigned int sec;

	for (sec = 0; sec < LED_BLINK_DELAY_TIME; sec++) {
		unsigned long start = (unsigned long)clock();

	while ((unsigned long)clock() - start < (unsigned long)CLOCKS_PER_SEC)
		;
	}
#endif
}

void test_leds(void){
	volatile int i;
	turn_on_led(0);
	turn_on_led(1);
	turn_on_led(2);
	my_delay();
	//sleep(1);
	turn_off_led(0);
	turn_off_led(1);
	turn_off_led(2);
	my_delay();
	//sleep(1);
	turn_on_led(0);
	turn_on_led(1);
	turn_on_led(2);
	my_delay();
	//sleep(1);
	turn_off_led(0);
	turn_off_led(1);
	turn_off_led(2);
	my_delay();
	//sleep(1);
}

int main()
{
	/**
	 * Initialize managed drivers and/or services that have been added to 
	 * the project.
	 * @return zero on success 
	 */
	adi_initComponents();
	
	/* Begin adding your custom code here */
	init_leds();
	/* Begin adding your custom code here */
	while (1) {
		test_leds();
	}

	return 0;
}

