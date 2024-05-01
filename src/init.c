// init.c
// Last Modified: 4/20/2024
// Student names: Toby Nguyen
// initialization functions for game and title screen 

#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
#include "../inc/Clock.h"
#include "../inc/SPI.h"
#include "../inc/ST7735.h"
#include "../inc/timer.h"

#include <stdlib.h>
#include <stdio.h>

#include "structs.h"

#include "inc/sprites/cowboysprites.h"
#include "inc/sprites/maps.h"
#include "inc/sprites/dangerousthings.h"
#include "inc/sprites/projectiles.h"
#include "inc/sprites/mainscreen.h"
#include "inc/sprites/collectibles.h"

#include "draw.h"
#include "init.h"
#include "inputs.h"
#include "game_events.h"

#include"languages.h"

#include "../inc/dac.h"
#include "sound_data.h"

#include "../inc/uart1.h"
#include "../inc/uart2.h"
#include "../inc/fifo1.h"

#define MAX_GOONS 40
#define MAX_BULLETS 100
#define MAX_COLLECTIBLES 20
#define MAX_SOLIDS 2

extern playerData_t playerData;
extern bulletData_t bullets[MAX_BULLETS];
extern goonData_t goons[MAX_GOONS];
extern collData_t colls[MAX_COLLECTIBLES];
extern const unsigned short *currentMap;
extern Language_t myL;
extern uint8_t gameState;
extern solid_t solids[MAX_SOLIDS];
extern flag_t otherFlag;
extern flag_t flag;
// 0 -> start screen
// 1 -> map1
// 2 -> map2
// 3 -> map3 (mulitplayer)?
extern uint32_t score;
extern const unsigned short *currentMap;
extern uint8_t money;
extern uint8_t roundNum;

void initMap1(void) {
    // player init
    gameState = 1;
    playerData.x = 64;
    playerData.y = 80;
    playerData.speed = 1; // 1 pixel of motion per frame
    playerData.gun = 0; // default projectile scheme
    playerData.fireSpeed = 4; // slowest fire rate possible
    playerData.playerDir = 6; // down
    currentMap = map1;
//    ST7735_DrawBitmap(0, 160, currentMap, 128, 160);
}

void initDev() {
    gameState = 1;
    playerData.x = 64;
    playerData.y = 80;
    playerData.speed = 2;
    playerData.gun = 1;
    playerData.fireSpeed = 1;
    playerData.playerDir = 6;
    currentMap = map1;
    roundNum = 5;
    ST7735_DrawBitmap(0, 160, currentMap, 128, 160);
}

extern uint8_t multiState;
void initMultiplayer(uint8_t where) { // mode 0 = standrad, mode 1  = 1
    gameState = 1; // placeholder
    multiState = where;
    if((where == 3) || (where == 5)) { // p1
        playerData.x = playerData.lastx = 40;
        playerData.y = playerData.lasty = 34;
    }
    if((where == 4) || (where == 6)) { // p2
        playerData.x = 77;
        playerData.y = 150;
        if(where == 6) { // ctf
//            otherFlag.x = 20; // top left corner
//            otherFlag.y = 20;
//            flag.x = 90; // top left corner
//            flag.y = 90;
        }
    }
    playerData.speed = 1;
    playerData.gun = 0;
    playerData.fireSpeed = 4;
    playerData.playerDir = 6;
    roundNum = 10;
    if((where == 5) || (where == 6)) { // ctf flag
        roundNum = 11;
        flag.x = 200; // init outside of map so doesnt get janky results in multiplayer lol
        flag.y = 200;
        otherFlag.x = 200;
        otherFlag.y = 200;
        flag.isCaptured = false;
        otherFlag.isCaptured = false;
    }
    currentMap = map1;
    UART1_Init();
    UART2_Init();
    ST7735_DrawBitmap(0, 160, currentMap, 128, 160);
    solids[0].x = 36;
    solids[0].y = 53;
    solids[0].active = 1;
    solids[1].x = 73;
    solids[1].y = 126;
    solids[1].active = 1;
}

uint8_t initStart(void) {
    ST7735_FillScreen(ST7735_BLACK);
    gameState = 0;
    uint8_t ptrYCoord[4] = {58+18*1, 61+18*2, 63+18*3, 65+18*4}; // magic numbers oo oh no (manual offset for menu cursor)
    static bool isInverted = false;



    // print banner image
    ST7735_DrawBitmap(16, 58, title, 94, 55);
    // print options
    int8_t i = 0;
    for(i = 0; i < 4; i++) {
        ST7735_SetCursor(6,7+2*i);
        ST7735_OutString((char *)Phrases[i][myL]);
    }
    i = 0;
    // use buttons to cycle through
    uint8_t arrowOffset[10] = {0, -1, -2, -1, -2, -1, 0, 1, 2, 1};
    uint8_t arrowOffsetIndex = 0;
    uint8_t lastUp = 0, lastDown = 0;
    int8_t j = 0; // multiplayer menu index
    while(1) {
        ST7735_DrawBitmap(20 + arrowOffset[arrowOffsetIndex], ptrYCoord[i], arrow, 11, 9);
        arrowOffsetIndex = (arrowOffsetIndex + 1) % 10;
        Clock_Delay(1000000);

        if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp < 1)) {// up
            lastUp = 1;
            ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
            i--;
            if(i < 0) {
                i = 3;
            }
        }
        else if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp > 0)){ // buffering system up
            lastUp = 1;
        }
        else {
            lastUp = 0;
        }
        if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown < 1)) { // down
            ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
            lastDown = 1;
            i++;
            if(i > 3) {
                i = 0;
            }
        }
        else if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown > 0)) { // buffering system down
            lastDown = 1;
        }
        else {
            lastDown = 0;
        }


        if((GPIOB->DIN31_0 & (1 << 13)) > 1) { // right (select)
            switch(i) {
            case 0: // INIT map 1 and START
                return 1;
            case 1: // switch language menu and reprint menu
                if(myL == English) {
                    myL = Spanish;
                }
                else {
                    myL = English;
                }
                return 2;
            case 2: // INIT multiplayer
                // copy pasted everything because im lazy (good luck reading this im trying to go for speed here)
                ST7735_FillRect(20,40+18, 180, 90, ST7735_BLACK);
                ST7735_SetCursor(6,7);
                ST7735_OutString((char *)Phrases[GAMEMODE1][myL]);
                ST7735_SetCursor(6,9);
                ST7735_OutString((char *)Phrases[GAMEMODE2][myL]);
                Clock_Delay(10000000);
                while(1) {
                    ST7735_DrawBitmap(20 + arrowOffset[arrowOffsetIndex], ptrYCoord[j], arrow, 11, 9);
                    arrowOffsetIndex = (arrowOffsetIndex + 1) % 10;
                    Clock_Delay(1000000);

                    if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp < 1)) {// up
                        lastUp = 1;
                        ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
                        j--;
                        if(j < 0) {
                            j = 1;
                        }
                    }
                    else if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp > 0)){ // buffering system up
                        lastUp = 1;
                    }
                    else {
                        lastUp = 0;
                    }
                    if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown < 1)) { // down
                        ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
                        lastDown = 1;
                        j++;
                        if(j > 1) {
                            j = 0;
                        }
                    }
                    else if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown > 0)) { // buffering system down
                        lastDown = 1;
                    }
                    else {
                        lastDown = 0;
                    }

                    if((GPIOB->DIN31_0 & (1 << 13)) > 1) { // i am so sorry to whoever is reading this
                        switch(j) {
                        case 0:
                            j = 0;
                            ST7735_FillRect(20,40+18, 180, 90, ST7735_BLACK);
                            ST7735_SetCursor(6,7);
                            ST7735_OutString((char *)Phrases[P1][myL]);
                            ST7735_SetCursor(6,9);
                            ST7735_OutString((char *)Phrases[P2][myL]);
                            Clock_Delay(10000000);
                            while(1) {
                                ST7735_DrawBitmap(20 + arrowOffset[arrowOffsetIndex], ptrYCoord[j], arrow, 11, 9);
                                arrowOffsetIndex = (arrowOffsetIndex + 1) % 10;
                                Clock_Delay(1000000);

                                if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp < 1)) {// up
                                    lastUp = 1;
                                    ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
                                    j--;
                                    if(j < 0) {
                                        j = 1;
                                    }
                                }
                                else if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp > 0)){ // buffering system up
                                    lastUp = 1;
                                }
                                else {
                                    lastUp = 0;
                                }
                                if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown < 1)) { // down
                                    ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
                                    lastDown = 1;
                                    j++;
                                    if(j > 1) {
                                        j = 0;
                                    }
                                }
                                else if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown > 0)) { // buffering system down
                                    lastDown = 1;
                                }
                                else {
                                    lastDown = 0;
                                }

                                if((GPIOB->DIN31_0 & (1 << 13)) > 1) {
                                    switch(j) {
                                    case 0:
                                        return 3; // p1 standard
                                    case 1:
                                        return 4; // p2 standard
                                    }

                                }
                                if((GPIOB->DIN31_0 & (1 << 17)) > 1) { // go back and reinit main menu
                                    return 2;
                                }

                            }
//                            return 3; // p1 standard
                        case 1:
                            j = 0;
                            ST7735_FillRect(20,40+18, 180, 90, ST7735_BLACK);
                            ST7735_SetCursor(6,7);
                            ST7735_OutString((char *)Phrases[P1][myL]);
                            ST7735_SetCursor(6,9);
                            ST7735_OutString((char *)Phrases[P2][myL]);
                            Clock_Delay(10000000);
                            while(1) {
                                ST7735_DrawBitmap(20 + arrowOffset[arrowOffsetIndex], ptrYCoord[j], arrow, 11, 9);
                                arrowOffsetIndex = (arrowOffsetIndex + 1) % 10;
                                Clock_Delay(1000000);

                                if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp < 1)) {// up
                                    lastUp = 1;
                                    ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
                                    j--;
                                    if(j < 0) {
                                        j = 1;
                                    }
                                }
                                else if(((GPIOB->DIN31_0 & (1 << 19)) > 0) && (lastUp > 0)){ // buffering system up
                                    lastUp = 1;
                                }
                                else {
                                    lastUp = 0;
                                }
                                if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown < 1)) { // down
                                    ST7735_FillRect(20,40+18*1, 14, 90, ST7735_BLACK);
                                    lastDown = 1;
                                    j++;
                                    if(j > 1) {
                                        j = 0;
                                    }
                                }
                                else if(((GPIOB->DIN31_0 & (1 << 16)) > 0) && (lastDown > 0)) { // buffering system down
                                    lastDown = 1;
                                }
                                else {
                                    lastDown = 0;
                                }

                                if((GPIOB->DIN31_0 & (1 << 13)) > 1) {
                                    switch(j) {
                                    case 0:
                                        return 5; // p1 ctf
                                    case 1:
                                        return 6; // p2 ctf
                                    }

                                }
                                if((GPIOB->DIN31_0 & (1 << 17)) > 1) { // go back and reinit main menu
                                    return 2;
                                }

                            }
//                            return 4; // p2 standard
                        }

                    }
                    if((GPIOB->DIN31_0 & (1 << 17)) > 1) { // go back and reinit main menu
                        return 2;
                    }

                }
            case 3: // INIT dev mode
                return 0;
            }
        }
        uint32_t i = 0;
        if(((GPIOA->DIN31_0 & (1 << 25)) > 1) && isInverted == false) {
            ST7735_InitR(INITR_GREENTAB);
            ST7735_InvertDisplay(1);
            isInverted = true;
            return 2;
        }
        if(((GPIOA->DIN31_0 & (1 << 25)) > 1) && isInverted == true) {
            ST7735_InitR(INITR_BLACKTAB);
            ST7735_InvertDisplay(0);
            isInverted = false;
            return 2;
        }
    }
}

void initSysTick(void) {
    SysTick->CTRL = 0; // reset everything
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x00000007; // enable
    SCB->SHP[1] = (SCB->SHP[1]&(~0xC0000000))| 1 << 30; // set priority to 1

//    SysTick->LOAD = 2666666; // interrupt at 15 Hz
    SysTick->LOAD = 1333333; // interrupt at 30 Hz
//    SysTick->LOAD = 2666666*4; // debug
}

// initialize G7 for periodic interrupt
// frequency = TimerClock/prescale/period
void initG7(uint16_t period, uint32_t prescale, uint32_t priority){
  TIMG7->GPRCM.RSTCTL = (uint32_t)0xB1000003;
  TIMG7->GPRCM.PWREN = (uint32_t)0x26000001;
  Clock_Delay(24); // time for TimerG7 to power up
  TIMG7->CLKSEL = 0x08; // bus clock
  TIMG7->CLKDIV = 0x00; // divide by 1
  TIMG7->COMMONREGS.CPS = prescale-1;     // divide by prescale
  TIMG7->COUNTERREGS.LOAD  = period-1;    // set reload register
  TIMG7->COUNTERREGS.CTRCTL = 0x02;
    // bits 5-4 CM =0, down
    // bits 3-1 REPEAT =001, continue
    // bit 0 EN enable (0 for disable, 1 for enable)
  TIMG7->CPU_INT.IMASK |= 1; // zero event mask
  TIMG7->COMMONREGS.CCLKCTL = 1;
  NVIC->ISER[0] = 1 << 20; // TIMG7 interrupt
  NVIC->IP[5] = (NVIC->IP[5]&(~0x000000FF))|(priority<<6);    // set priority (bits 7,6) IRQ 20
  TIMG7->COUNTERREGS.CTRCTL |= 0x01;
}

// init ADC
void myADCInit(void){
// ADC1 Channel 5 - PB18 (slide pot)
      ADC1->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) reset
      ADC1->ULLMEM.GPRCM.PWREN = 0x26000001;  // 2) activate
      Clock_Delay(24);                        // 3) wait
      ADC1->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // 4) ULPCLK
      ADC1->ULLMEM.CLKFREQ = 7;               // 5) 40-48 MHz
      ADC1->ULLMEM.CTL0 = 0x03010000;         // 6) divide by 8
      ADC1->ULLMEM.CTL1 = 0x00000000;         // 7) mode
      ADC1->ULLMEM.CTL2 = 0x00000000;         // 8) MEMRES
      ADC1->ULLMEM.MEMCTL[0] = 5;             //
      ADC1->ULLMEM.SCOMP0 = 0;                // 10) 8 sample clocks
      ADC1->ULLMEM.CPU_INT.IMASK = 0;         // 11) no interrupt
      // ADC0 Channel 1 - PA27 (joystick x axis) and PA26 (joystick y axis)
        ADC0->ULLMEM.GPRCM.RSTCTL = 0xB1000003; // 1) reset
        ADC0->ULLMEM.GPRCM.PWREN = 0x26000001;  // 2) activate
        Clock_Delay(24);                        // 3) wait
        ADC0->ULLMEM.GPRCM.CLKCFG = 0xA9000000; // 4) ULPCLK
        ADC0->ULLMEM.CLKFREQ = 7;               // 5) 40-48 MHz
        ADC0->ULLMEM.CTL0 = 0x03010000;         // 6) divide by 8
        ADC0->ULLMEM.CTL1 = 0x00010000;         // 7) mode
        ADC0->ULLMEM.CTL2 = 0x02010000;         // 8) MEMRES
        ADC0->ULLMEM.MEMCTL[1] =0;             // vertical (PA27 - channel 0) ?
        ADC0->ULLMEM.MEMCTL[2] =1;             // horizontal (PA26 - channel 1) ?
        ADC0->ULLMEM.SCOMP0 = 0;                // 10) 8 sample clocks
        ADC0->ULLMEM.CPU_INT.IMASK = 0;         // 11) no interrupt
}
