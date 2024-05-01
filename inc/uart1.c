/* UART1.c
 * Toby Nguyen
 * Date: 4/20/24
 * PA8 UART1 Tx to other microcontroller PA22 UART2 Rx
 */


#include <ti/devices/msp/msp.h>
#include "../inc/uart1.h"
#include "../inc/Clock.h"
#define PA8INDEX  18 // UART1_TX  SPI0_CS0  UART0_RTS TIMA0_C0  TIMA1_C0N


// power Domain PD0
// for 80MHz bus clock, UART clock is ULPCLK 40MHz
// initialize UART1 for 2000 baud rate
// blind, no synchronization on transmit
void UART1_Init(void){
  UART1->GPRCM.RSTCTL = 0xB1000003;
  UART1->GPRCM.PWREN = 0x26000001;
  Clock_Delay(24); // time for uart to power up
  IOMUX->SECCFG.PINCM[PA8INDEX]  = 0x00000082;
  UART1->CLKSEL = 0x08; // bus clock
  UART1->CLKDIV = 0x00; // no divide
  UART1->CTL0 &= ~0x01; // disable UART0
  UART1->CTL0 = 0x00020018;
 // assumes an 80 MHz bus clock
  UART1->IBRD = 1250;
  UART1->FBRD = 0;
  UART1->LCRH = 0x00000030;
  UART1->CTL0 |= 0x01; // enable UART0
}



//------------UART1_OutChar------------
// Output 8-bit to serial port
// blind synchronization
// 10 bit frame, 2000 baud, 5ms per frame
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART1_OutChar(uint8_t data){
  UART1->TXDATA = data;
}

