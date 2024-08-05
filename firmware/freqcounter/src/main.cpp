//
//	freqcounter Ver 1.1
//
//  2024.07.31 New Create
//
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include "fix_fft.h"
#include "ch32v003fun.h"

#define micros() (SysTick->CNT / DELAY_US_TIME)
#define millis() (SysTick->CNT / DELAY_MS_TIME)

// what type of OLED - uncomment just one
//#define SSD1306_64X32
//#define SSD1306_128X32
#define SSD1306_128X64
#define SCALE 3

//#define GPIO_ADC_MUX_DELAY 100
//#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy
#include "ch32v003_GPIO_branchless.h"
#include "ssd1306_spi.h"
#include "ssd1306.h"

//#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define SW1_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 0)
#define SW2_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 4)
#define SW3_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)
#define ADC_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 3)
#define SPK_PIN GPIOv_from_PORT_PIN(GPIO_port_D, 2)

#define SAMPLES 256 // 2の累乗である必要があります
#define SAMPLING_FREQUENCY 6000

uint16_t sampling_period_us;
int8_t vReal[SAMPLES];
int8_t vImag[SAMPLES];

#include "qr.h"

// function prototype (declaration), definition in "ch32v003fun.c"
extern "C" int mini_snprintf(char* buffer, unsigned int buffer_len, const char *fmt, ...);
char title1[] = "FrequencyCounter";
char title2[] = "   Version 1.1  ";


void setup()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_D);
    GPIO_port_enable(GPIO_port_C);
    // 各ピンの設定
    GPIO_pinMode(SW1_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz); /// GPIO_Speed_In? @see https://github.com/cnlohr/ch32v003fun/blob/master/examples/GPIO/GPIO.c#L55
    GPIO_pinMode(SW2_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);
    GPIO_pinMode(SW3_PIN, GPIO_pinMode_I_pullDown, GPIO_Speed_10MHz);

	ssd1306_spi_init();		// i2c Setup
	ssd1306_init();			// SSD1306 Setup
	ssd1306_setbuf(0);		// Clear Screen
	ssd1306_drawstr_sz(0, 24, title1, 1, fontsize_8x8);
	ssd1306_drawstr_sz(0, 40, title2, 1, fontsize_8x8);
	ssd1306_refresh();
}

// void alertNotImpl() {
// 	ssd1306_setbuf(0);		// Clear Screen
// 	ssd1306_drawstr_sz(0, 24, "      Not       ", 1, fontsize_8x8);
// 	ssd1306_drawstr_sz(0, 40, "  Implemented!  ", 1, fontsize_8x8);
// 	ssd1306_refresh();
// 	Delay_Ms(1000);
// }

// void alertDefaultError() {
// 	ssd1306_setbuf(0);		// Clear Screen
// 	ssd1306_drawstr_sz(0, 24, "some error occurrs", 1, fontsize_8x8);
// 	ssd1306_drawstr_sz(0, 40, "    Reboot Me!    ", 1, fontsize_8x8);
// 	ssd1306_refresh();
// 	while(1);
// }

// void drawIcon(uint8_t *data, uint8_t size, uint8_t x_base, uint8_t y_base, uint8_t color) {
// 	for (uint8_t i = 0; i < size; i++) {
// 		for (uint8_t j = 0; j < size; j++) {
// 			uint8_t pixel = *(data + i * size + j);
// 			bool _color = (color && pixel) || (!color && !pixel);
// 			ssd1306_drawPixel(x_base + j, y_base + i, _color);
// 		}
// 	}
// }

// void drawNoteIcon(uint8_t x_base, uint8_t y_base, uint8_t color) {
// 	uint8_t data[8][8] = {
// 		{0, 0, 0, 0, 1, 0, 0, 0},
// 		{0, 0, 0, 0, 1, 1, 0, 0},
// 		{0, 0, 0, 0, 1, 1, 1, 0},
// 		{0, 0, 0, 0, 1, 0, 1, 1},
// 		{0, 0, 0, 0, 1, 0, 0, 0},
// 		{0, 0, 1, 1, 1, 0, 0, 0},
// 		{0, 1, 1, 1, 1, 0, 0, 0},
// 		{0, 0, 1, 1, 0, 0, 0, 0},
// 	};

// 	drawIcon((uint8_t*) data, 8, x_base, y_base, color);
// }

uint8_t showInitMenu() {
	uint8_t mode = 0;

	while(1) {
		ssd1306_setbuf(0);	// Clear Screen
		// drawNoteIcon(0, 0 + 4, !(mode == 0)); /// ♪
		ssd1306_drawstr_sz(0,  0, "Tone", !(mode == 0), fontsize_8x8);
		ssd1306_drawstr_sz(0, 13, "Real time", !(mode == 1), fontsize_8x8);
		ssd1306_drawstr_sz(0, 26, "30-3000Hz", !(mode == 2), fontsize_8x8);
		// ssd1306_drawstr_sz(0, 13, "900-1100Hz", !(mode == 3), fontsize_8x8);
		ssd1306_drawstr_sz(0, 39, "QR code", !(mode == 3), fontsize_8x8);
		ssd1306_refresh();

		if (GPIO_digitalRead(SW1_PIN)) {
			// printf("SW1: OK\n");
			switch (mode)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					Delay_Ms(300);
					return mode;
				default:
					// alertNotImpl();
				break;
			}
		}

		if (GPIO_digitalRead(SW2_PIN)) {
			// printf("SW2: Up\n");
			Delay_Ms(100);
			if (mode <= 0) {
				mode = 3;
			} else {
				mode--;
			}
		}

		if (GPIO_digitalRead(SW3_PIN)) {
			// printf("SW3: Down\n");
			Delay_Ms(100);
			if (mode >= 3) {
				mode = 0;
			} else {
				mode++;
			}
		}
		Delay_Ms(100);
	}
}

void setupModeFreqCounter0()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_D);
    // 各ピンの設定
    GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);
	GPIO_ADCinit();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
}

int loopModeFreqCounter0() {
	char buf[16];

	while(1) {
		uint16_t ave = 0;
		uint8_t  val = 0;
		ssd1306_setbuf(0);	// Clear Screen
		for (int i = 0; i < SAMPLES; i++) {
			unsigned long t = micros();
			val = (uint8_t)(GPIO_analogRead(GPIO_Ain4_D3) >> 2);
			ave += val;
			vImag[i] = val;
			while ((micros() - t) < sampling_period_us);
		}
		ave = ave / SAMPLES;
		//printf("ave = %d\n", ave);
		for (int i = 0; i < SAMPLES; i++) {
			vReal[i] = (int8_t)(vImag[i] - ave);
			vImag[i] = 0; // Imaginary partは0に初期化
		}

  		fix_fft((char *)vReal, (char *)vImag, 8, 0); // SAMPLES = 32なので、log2(SAMPLES) = 5

  		/* Magnitude Calculation */
		for (int i = 0; i < SAMPLES / 2; i++) {
			vReal[i] = abs(vReal[i]) + abs(vImag[i]); // Magnitude calculation without sqrt
			//vReal[i] = sqrt(vReal[i] * vReal[i] + vImag[i] * vImag[i]);
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
//			uint16_to_string(peakFrequency + 12, buf, sizeof(buf));
			mini_snprintf(buf, sizeof(buf), " %4dHz", peakFrequency + 12);
		    //printf("%d\n", peakFrequency); // Print out what frequency is the most dominant.
  		} else {
			strcpy(buf, "    0Hz");
		}
//		strcat(buf, "Hz");
		ssd1306_drawstr_sz((6 - strlen(buf)) * 16 + 16, 1, buf, 1, fontsize_16x16);
		ssd1306_drawFastHLine(  0,   0,  128, 1);
		ssd1306_drawFastHLine(  0,  16,  128, 1);
		ssd1306_drawFastHLine(  0,  63,  128, 1);
		ssd1306_drawFastVLine(  0,   0,   64, 1);
		ssd1306_drawFastVLine(127,   0,   64, 2);

		ssd1306_refresh();
	}
}

void setupModeTone()
{
    // 各GPIOの有効化
    GPIO_port_enable(GPIO_port_D);
    // 各ピンの設定
    // GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
    GPIO_pinMode(SPK_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
}

void displayModeTone(uint16_t f) {
	char buf[16];

	if (f) {
		mini_snprintf(buf, sizeof(buf), "%dHz",  f);
	} else {
		mini_snprintf(buf, sizeof(buf), "Mute");
	}
	ssd1306_setbuf(0);	// Clear Screen
	// ssd1306_drawstr_sz((6 - strlen(buf)) * 16 + 16, 1, buf, 1, fontsize_16x16);
	ssd1306_drawstr_sz(0, 28, buf, 1, fontsize_16x16);
	ssd1306_refresh();
}

int loopModeTone() {
	uint8_t midiNoteNum;
	uint16_t f;
	uint16_t delay;
	bool dirty;

	midiNoteNum = 69;
	f = 440;

	delay = (1000 * 1000) / (2 * f);
	dirty = false;

	displayModeTone(f);

	while(1) {
		if (dirty) {
			dirty = false;

			if (midiNoteNum > 69) {
//				f = 440 * (1 + (midiNoteNum - 69) / 12);
				f = 440 + (440 * midiNoteNum - 69 * 440) / 12;
			} else if (midiNoteNum < 69) {
//				f = 440 / (1 - (midiNoteNum - 69) / 12);
				f = 440 - (440 * midiNoteNum - 69 * 440) / 12;
			} else {
				f = 440;
			}

			// f = 440.0 * pow(2.0, (midiNoteNum - 69) / 12.0);

			displayModeTone(f);
			// printf("%d\n", midiNoteNum);

			delay = (1000 * 1000) / (2 * f);
		}

		// GPIO_digitalWrite(LED_PIN, high);
		GPIO_digitalWrite(SPK_PIN, high);
		Delay_Us(delay);
		// GPIO_digitalWrite(LED_PIN, low);
		GPIO_digitalWrite(SPK_PIN, low);
		Delay_Us(delay);

		if (GPIO_digitalRead(SW1_PIN)) {
			// printf("SW1: OK\n");
			displayModeTone(0);
			Delay_Ms(300);

			while(!GPIO_digitalRead(SW1_PIN));

			displayModeTone(f);
			Delay_Ms(300);
		}

		if (GPIO_digitalRead(SW2_PIN)) {
			// printf("SW2: Up\n");
			if (midiNoteNum >= UINT8_MAX) {
				;
			} else {
				midiNoteNum++;
			}
			dirty = true;
			Delay_Ms(100);
		}

		if (GPIO_digitalRead(SW3_PIN)) {
			// printf("SW3: Down\n");
			if (midiNoteNum <= 0) {
				;
			} else {
				midiNoteNum--;
			}
			dirty = true;
			Delay_Ms(100);
		}
	}
}

void setupModeRealtime()
{
	// 各GPIOの有効化
	GPIO_port_enable(GPIO_port_D);
	// 各ピンの設定
	GPIO_pinMode(ADC_PIN, GPIO_pinMode_I_analog, GPIO_Speed_10MHz);
	GPIO_ADCinit();
	sampling_period_us = 1000000 / SAMPLING_FREQUENCY;
}

int loopModeRealtime() {
	char buf[16];
//	uint16_t vals[128];
	uint16_t *vals;
	unsigned long t;

	uint8_t mode;
	uint32_t wait;
	uint32_t dt;

	vals = (uint16_t *)vReal;			// use vReal 
	mode = 0;
	wait = 100;
	dt = 0;

	t = millis();

	while(1) {
		if ((millis() - t) > wait) {
			t = millis();
			ssd1306_setbuf(0);	// Clear Screen

			if (dt) {
				for (int i = 0; i < 128; i++) {
					Delay_Us(dt);
					vals[i] = GPIO_analogRead(GPIO_Ain4_D3);
				}
			} else {
				for (int i = 0; i < 128; i++) {
					vals[i] = GPIO_analogRead(GPIO_Ain4_D3);
				}
			}

			for (int i = 0; i < 128; i++) {
				// val /= (int) ceil((max + 1) / 48.0);
				// val = floor(val / (max / 48.0));
				vals[i] = (int) (vals[i] / (1023 / 48)); /// MIC OUT is 0-1023
				vals[i] = (vals[i] < 48) ? vals[i] : 48;
				ssd1306_drawPixel(i, 63 - vals[i] + 1, 1);
			}
		}

		ssd1306_drawstr_sz(0, 0, "                ", 1, fontsize_8x8);

		switch (mode)
		{
			case 0: // wait
				mini_snprintf(buf, sizeof(buf), "wait: %u ms", wait);
			break;

			case 1: // dt
				mini_snprintf(buf, sizeof(buf), "dt: %u us", dt);
			break;
		}

		ssd1306_drawstr_sz(0, 0, buf, 1, fontsize_8x8);
		// ssd1306_drawFastHLine(0, 40, 127, 1); /// y = 40 = (64 - 16) / 2 + 16
		ssd1306_refresh();

		if (GPIO_digitalRead(SW1_PIN)) {
			// printf("SW1: OK\n");
			mode = (mode + 1) % 2;
			Delay_Ms(300);
		}

		if (GPIO_digitalRead(SW3_PIN)) {
			// printf("SW2: Up\n");
			switch (mode)
			{
				case 0: // wait
					wait += 10;
				break;

				case 1: // dt
					dt += 10;
				break;
			}
			Delay_Ms(200);
		}

		if (GPIO_digitalRead(SW2_PIN)) {
			// printf("SW3: Down\n");
			switch (mode)
			{
				case 0: // wait
					if (wait > (0 + 10)) {
						wait -= 10;
					} else {
						wait = 0;
					}
				break;

				case 1: // dt
					if (dt > (0 + 10))  {
						dt -= 10;
					} else {
						dt = 0;
					}
				break;
			}
			Delay_Ms(200);
		}
	}
}

int loopModeQrCode() {
	uint8_t color;

	color = 0;

	while(1) {
		ssd1306_setbuf(0);	// Clear Screen
		ssd1306_drawImage((128 - QR_WIDTH) / 2 - 8, (64 - QR_HEIGHT) / 2, qr_data, QR_WIDTH, QR_HEIGHT, color);
		ssd1306_refresh();

		if (GPIO_digitalRead(SW1_PIN)) {
			Delay_Ms(300);
			return 0;
		}

		if (GPIO_digitalRead(SW2_PIN) || GPIO_digitalRead(SW3_PIN)) {
			color = (color + 1) % 2;
			Delay_Ms(300);
		}
	}
}

int main()
{
	SystemInit();			// ch32v003 sETUP

	setup();				// gpio Setup;

	Delay_Ms( 2000 );
	//printf("Frequency Counter Start\n\r");

	uint8_t exitStatus;
	uint8_t mode;

 init_menu:
	exitStatus = NULL;
	mode = showInitMenu();
//	mode = 2;

	switch (mode)
	{
		case 2: // 30-3000Hz freqcounter
//			Timer_Init();			// TIM2 Setup
			setupModeFreqCounter0();
			// Delay_Ms( 2000 );
			exitStatus = loopModeFreqCounter0();
		break;

		case 0: // Tone
			setupModeTone();
			exitStatus = loopModeTone();
		break;

		case 1: // Real time
//			TimerInit_ModeRealtime();			// TIM2 Setup
			setupModeRealtime();
			exitStatus = loopModeRealtime();
		break;

		case 3: // QR code
			exitStatus = loopModeQrCode();
			if (exitStatus == 0) {
				goto init_menu;
			}
		break;

		default:
			// alertDefaultError();
		break;
	}

	if (exitStatus) {
		switch (mode)
		{
			default:
				// alertDefaultError();
			break;
		}
	}
}
