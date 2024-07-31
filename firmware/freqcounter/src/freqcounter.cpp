//
//	micFFT.cpp 
//
//  2024.07.31 New Create
//
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "fix_fft.h"
#include "ch32v003fun.h"

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64
#define SCALE 3

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"
#include "ssd1306_spi.h"
#include "ssd1306.h"
#include "micFFTlib.h"

//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 6)
#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 0)
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)
#define SW3_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 4)
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 4)

#define SAMPLES 256 // 2の累乗である必要があります
#define SAMPLING_FREQUENCY 6000

uint16_t sampling_period_us;
int8_t vReal[SAMPLES];
int8_t vImag[SAMPLES];

void setup()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);
    // 各ピンの設定
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
    GPIO_pinMode(SW3_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);
	GPIO_ADCinit();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;

	ssd1306_spi_init();		// i2c Setup
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen
	ssd1306_drawstr_sz(0, 24, "FrequencyCounter", 1, fontsize_8x8);
	ssd1306_drawstr_sz(0, 40, "   Version 1.0  ", 1, fontsize_8x8);
	ssd1306_refresh();

}

int main()
{
	char buf[16];

	SystemInit();			// ch32v003 sETUP
	Timer_Init();			// TIM2 Setup
	
	setup();				// gpio Setup;

	Delay_Ms( 2000 );
	printf("Frequency Counter Start\n\r");

	while(1) {
		ssd1306_setbuf(0);	// Clear Screen
		for (int i = 0; i < SAMPLES; i++) {
			unsigned long t = micros();
			vReal[i] = (GPIO_analogRead(GPIO_Ain7_D4) - 408) >> 2;
			vImag[i] = 0; // Imaginary partは0に初期化
			while ((micros() - t) < sampling_period_us);
		}

  		fix_fft((char *)vReal, (char *)vImag, 8, 0); // SAMPLES = 32なので、log2(SAMPLES) = 5

  		/* Magnitude Calculation */
		for (int i = 0; i < SAMPLES / 2; i++) {
			//vReal[i] = abs(vReal[i]) + abs(vImag[i]); // Magnitude calculation without sqrt
			vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
		}
		/* Find Peak */
		uint8_t maxIndex = 0;
		uint8_t maxValue = 0;
		for (int i = 8; i < SAMPLES / 2; i++) {
			int8_t val = (vReal[i] * SCALE < 48) ? vReal[i] * SCALE : 48;  
			ssd1306_drawFastVLine(i, 63 - val + 1, val + 1, 1);
			if (vReal[i] > maxValue) {
				maxValue = vReal[i];
				maxIndex = i;
			}
		}
		// disp freqeuncy
		int peakFrequency = (SAMPLING_FREQUENCY / SAMPLES) * maxIndex;
 		if (maxValue >= 4 ) {
			uint16_to_string(peakFrequency + 12, buf, sizeof(buf));
		    //printf("%d\n", peakFrequency); // Print out what frequency is the most dominant.
  		} else {
			strcpy(buf, "0");
		}
		strcat(buf, "Hz");
		ssd1306_drawFastHLine(  0,   0,  128, 1);
		ssd1306_drawFastHLine(  0,  16,  128, 1);
		ssd1306_drawFastHLine(  0,  63,  128, 1);
		ssd1306_drawFastVLine(  0,   0,   64, 1);
		ssd1306_drawFastVLine(127,   0,   64, 2);

		ssd1306_drawstr_sz((6 - strlen(buf)) * 16 + 16, 1, buf, 1, fontsize_16x16);
		ssd1306_refresh();
	}
}
