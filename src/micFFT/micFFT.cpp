#include "fix_fft.h"
// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64

#include "ch32v003fun.h"
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>

#define GPIO_ADC_MUX_DELAY 100
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"
#include "ssd1306_i2c.h"
#include "ssd1306.h"

#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 6)
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
    // 各ピンの設定
    GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);
	GPIO_ADCinit();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;

}
void reverse_str(char* str, size_t len) {
    size_t start = 0;
    size_t end = len - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void uint16_to_string(uint16_t value, char* str, size_t str_size) {
    size_t index = 0;
    if (value == 0) {
        if (str_size > 1) {
            str[index++] = '0';
        }
    } else {
        while (value != 0 && index < str_size - 1) {
            str[index++] = (value % 10) + '0';
            value /= 10;
        }
    }
    str[index] = '\0';
    reverse_str(str, index);
}

void Timer_Init(void) {
    // タイマークロックの有効化
    RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

    // タイマーのリセット
    TIM2->CTLR1 = 0;  // コントロールレジスタをリセット
    TIM2->PSC = 48 - 1;  // プリスケーラを48に設定（48 MHzクロックを1 MHzに分周）
    TIM2->ATRLR = 0xFFFFFFFF;  // 自動リロードレジスタを最大値に設定

    // タイマーのスタート
    TIM2->CTLR1 |= 0x01;  // タイマーを有効にする (TIM_CTLR1_CENの代わりにビット0を直接設定)
}

// マイクロ秒単位での経過時間を取得
uint32_t micros(void) {
    return TIM2->CNT;
}

int main()
{
	char buf[16];

	SystemInit();			// ch32v003 sETUP
	Timer_Init();			// TIM2 Setup
	
	setup();				// gpio Setup;
	ssd1306_i2c_init();		// i2c Setup
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen

	Delay_Ms( 1000 );
	printf("mic FFT Test Start\n\r");

	while(1) {
		ssd1306_setbuf(0);	// Clear Screen
		for (int i = 0; i < SAMPLES; i++) {
			unsigned long t = micros();
			vReal[i] = (GPIO_analogRead(GPIO_Ain7_D4) - 512) >> 2;
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
			ssd1306_drawFastVLine(i, 63 - vReal[i] * 2, vReal[i] * 2, 1);
			//ssd1306_drawLine(i, 63, i, 63 - vReal[i], 1);
			if (vReal[i] > maxValue) {
				maxValue = vReal[i];
				maxIndex = i;
			}
		}
		int peakFrequency = (SAMPLING_FREQUENCY / SAMPLES) * maxIndex;
 		 if (peakFrequency < SAMPLING_FREQUENCY / 2 && peakFrequency >= 300) {
			uint16_to_string(peakFrequency, buf, sizeof(buf));
			ssd1306_drawstr_sz(0,0, buf, 1, fontsize_16x16);
		    //printf("%d\n", peakFrequency); // Print out what frequency is the most dominant.
  		}
		ssd1306_refresh();
	}
}
