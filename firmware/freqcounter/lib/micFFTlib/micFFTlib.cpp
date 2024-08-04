#include <stdint.h> 
#include <stdio.h>
#include "micFFTlib.h"
#include "ch32v003fun.h"
/*
static void reverse_str(char* str, size_t len) {
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
*/
void Timer_Init(void) {
    // タイマークロックの有効化
    RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

    // タイマーのリセット
    TIM2->CTLR1 = 0;  // コントロールレジスタをリセット
    TIM2->PSC = 48 - 1;  // プリスケーラを48に設定（48 MHzクロックを1 MHzに分周）
    //TIM2->ATRLR = 0xFFFFFFFF;  // 自動リロードレジスタを最大値に設定
    TIM2->ATRLR = 0xFFFF;  // 自動リロードレジスタを最大値に設定

    // タイマーのスタート
    TIM2->CTLR1 |= 0x01;  // タイマーを有効にする (TIM_CTLR1_CENの代わりにビット0を直接設定)
}

// マイクロ秒単位での経過時間を取得
uint32_t micros(void) {
    return TIM2->CNT;
}

