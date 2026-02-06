#include <msp430.h>
#include <stdint.h>

//GPIO Part---------------------------------------------------------------------
/* --- 只做一般 GPIO ---
 * - 解除 FRAM 裝置的高阻態鎖 (PM5CTL0)
 * - P1.0、P2.0 設為輸出，初始為 Low
 * - 清除可能殘留的 Port 中斷旗標
 */


static void P1_0_Switch(uint8_t value){
    if(value==1)
        P1OUT |= BIT0;// 把 P1.0 設成 1 → 腳位輸出 High
    else
        P1OUT &=~BIT0;// 把 P1.0 設成 0 → 腳位輸出 Low
}
static void P2_0_Switch(uint8_t value){
    if(value==1)
        P2OUT |= BIT0;// 把 P2.0 設成 1 → 腳位輸出 High
    else
        P2OUT &=~BIT0;// 把 P2.0 設成 0 → 腳位輸出 Low
}


static void gpio_output_init(void)
{
    
    P1DIR |= BIT0;          // P1.0 output 
    P1OUT &= ~BIT0;         // 初始 Low

    P2DIR |= BIT0;          // P2.0 output 
    P2OUT &= ~BIT0;         // 初始 Low
    
    P1IFG = 0;              // 清 Port IFG（保險）
    P2IFG = 0;
}


static void gpio_input_init(void)
{
    // P1.1 當按鈕：內部上拉、下降緣中斷
    P1DIR  &= ~BIT1;               // Input
    P1REN  |=  BIT1;               // 啟用上/下拉
    P1OUT  |=  BIT1;               // 上拉
    P1IES  |=  BIT1;               // 高->低（按下）觸發
    P1IFG  &= ~BIT1;               // 清旗標
    P1IE   |=  BIT1;               // 開中斷
}


//=================== 中斷 ===================
// P1.1 按鈕中斷：先關自己的 IE，啟動 10ms 定時做防彈跳
//Demo 用+++++
static volatile uint8_t btn_toggle = 0;
//Demo 用-----
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    if (P1IFG & BIT1) {
        P1IFG &= ~BIT1;
        P1IE  &= ~BIT1;         // 先關中斷避免彈跳重入
        
        P1_0_Switch(1);
        P2_0_Switch(1);

        // 重新設定下降緣（避免鬆開的上升緣誤觸）
        P1IES  |=  BIT1;        // 高->低
        P1IFG  &= ~BIT1;
        P1IE   |=  BIT1;        // 再次開啟按鈕中斷
    }
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;                   // 解除高阻態，讓 GPIO 可用

    //GPIO Init
    gpio_output_init();         // P1.0 P2.0 Outout
    gpio_input_init();          // P1.1 按鈕 input

    __enable_interrupt();      // 全域中斷

    while(1){
        __no_operation();
    }
}
