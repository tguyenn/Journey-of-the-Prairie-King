// inputs.c
// Last Modified: 4/20/2024
// Student names: Toby Nguyen
// read buttons and joystick to update game

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

#define MAX_GOONS 20
#define MAX_BULLETS 50
#define MAX_COLLECTIBLES 20

extern playerData_t playerData;
extern bulletData_t bullets[MAX_BULLETS];
extern goonData_t goons[MAX_GOONS];
extern collData_t colls[MAX_COLLECTIBLES];
extern const unsigned short *currentMap;
extern uint8_t gameState;
// 0 -> start screen
// 1 -> map1
// 2 -> map2
// 3 -> map3 (mulitplayer)?
extern uint32_t score;
extern uint8_t roundNum;
extern const unsigned short *currentMap;
extern uint8_t money;
extern flag_t flag;
extern flag_t otherFlag;
// read joystick and update game vars
void JoyADCin(void){
   uint16_t resx, resy;
   ADC0->ULLMEM.CTL0 |= 0x00000003;             // 1) enable conversions
   ADC0->ULLMEM.CTL1 |= 0x00000100;             // 2) start ADC
   uint32_t volatile delay=ADC0->ULLMEM.STATUS; // 3) time to let ADC start
   while((ADC0->ULLMEM.STATUS&0x01)==0x01){}    // 4) wait for completion

   resy = ADC0->ULLMEM.MEMRES[1];
   resx = ADC0->ULLMEM.MEMRES[2];

   // diagonals
   if((resx > 3000) && (resy > 3000)) {
       playerData.x += playerData.speed;
       playerData.y -= playerData.speed;
       playerData.playerDir = 1;
//       if(otherFlag.isCaptured) {
//           otherFlag.x += playerData.speed;
//           otherFlag.y -= playerData.speed;
//       }
       return;
   }
   if((resx < 1000) && (resy > 3000)) {
       playerData.x -= playerData.speed;
       playerData.y -= playerData.speed;
       playerData.playerDir = 3;
//       if(otherFlag.isCaptured) {
//           otherFlag.x -= playerData.speed;
//           otherFlag.y -= playerData.speed;
//       }
       return;

   }
   if((resx < 1000) && (resy < 1000)) {
       playerData.x -= playerData.speed;
       playerData.y += playerData.speed;
       playerData.playerDir = 5;
//       if(otherFlag.isCaptured) {
//           otherFlag.x -= playerData.speed;
//           otherFlag.y += playerData.speed;
//       }
       return;

   }
   if((resx > 3000) && (resy < 1000)) {
       playerData.x += playerData.speed;
       playerData.y += playerData.speed;
       playerData.playerDir = 7;
//       if(otherFlag.isCaptured) {
//           otherFlag.x += playerData.speed;
//           otherFlag.y += playerData.speed;
//       }
       return;
   }
   // x axis
   if(resx > 3000) { // right
       playerData.x += playerData.speed;
       playerData.playerDir = 0;
//       if(otherFlag.isCaptured) {
//           otherFlag.x += playerData.speed;
//       }
   }
   if(resx < 1000) { // left
       playerData.x -= playerData.speed;
       playerData.playerDir = 4;
//       if(otherFlag.isCaptured) {
//           otherFlag.x -= playerData.speed;
//       }
   }
   // y axis
   if(resy > 3000) { // up
       playerData.y -= playerData.speed;
       playerData.playerDir = 2;
//       if(otherFlag.isCaptured) {
//           otherFlag.y -= playerData.speed;
//       }
   }
   if(resy < 1800) { // down
       playerData.y += playerData.speed;
       playerData.playerDir = 6;
//       if(otherFlag.isCaptured) {
//           otherFlag.y += playerData.speed;
//       }
   }
}

// read buttons and update game vars
bool lastSprintState;
extern uint8_t multiGunReload;
extern bool barrierColl;
void getButtons(void) {
    static bool lastFireState;
    static uint8_t sprintReload;
    static uint8_t gunReload;


    if(roundNum == 10 || roundNum == 11) { // save a copy of x to roll back to if hit a barrier
        if(playerData.barrierColl == false) {
            if((playerData.lastx < playerData.x) || (playerData.lastx > playerData.x)) {
                playerData.lastx = playerData.x;
            }
            if((playerData.lasty < playerData.y) || (playerData.lasty > playerData.y)) {
                playerData.lasty = playerData.y;
            }
        }
    }

    if(((GPIOB->DIN31_0 & (1 << 17)) > 1)  && (lastSprintState == false)) { // left (sprint)
        if(playerData.speed == 3) { // no sprinting when 3 speed
            return;
        }
        uint8_t addSpeed = 2;
        lastSprintState = true;
        switch(playerData.playerDir) {
        case 0:
            playerData.x += addSpeed;
            break;
        case 1:
            playerData.x += addSpeed;
            playerData.y -= addSpeed;
            break;
        case 2:
            playerData.y -= addSpeed;
            break;
        case 3:
            playerData.x -= addSpeed;
            playerData.y -= addSpeed;
            break;
        case 4:
            playerData.x -= addSpeed;
            break;
        case 5:
            playerData.x -= addSpeed;
            playerData.y += addSpeed;
            break;
        case 6:
            playerData.y += addSpeed;
            break;
        case 7:
            playerData.x += addSpeed;
            playerData.y += addSpeed;
            break;
        }
//        if(otherFlag.isCaptured) {
//            switch(playerData.playerDir) {
//            case 0:
//                otherFlag.x += addSpeed;
//                break;
//            case 1:
//                otherFlag.x += addSpeed;
//                otherFlag.y -= addSpeed;
//                break;
//            case 2:
//                otherFlag.y -= addSpeed;
//                break;
//            case 3:
//                otherFlag.x -= addSpeed;
//                otherFlag.y -= addSpeed;
//                break;
//            case 4:
//                otherFlag.x -= addSpeed;
//                break;
//            case 5:
//                otherFlag.x -= addSpeed;
//                otherFlag.y += addSpeed;
//                break;
//            case 6:
//                otherFlag.y += addSpeed;
//                break;
//            case 7:
//                otherFlag.x += addSpeed;
//                otherFlag.y += addSpeed;
//                break;
//            }
//        }
    }
    if(((GPIOB->DIN31_0 & (1 << 17)) > 1) == 0 ) {
        lastSprintState = false;
    }
    if(((GPIOB->DIN31_0 & (1 << 16)) > 1)) { // down (nothing as of now)

    }

    gunReload++;
    if((GPIOB->DIN31_0 & (1 << 13)) > 1) { // right (shoot)
        if((lastFireState == false) && (gunReload > playerData.fireSpeed)) {
            lastFireState = true;
            gunReload = 0;
            if(playerData.gun == 0) {  //normal
                playerShoot(false);
                playerData.data = 1;
//                makeSound(0);

            }
            if(playerData.gun == 1) { // shotgun
                playerShoot(true);
                playerData.data = 3;
//                makeSound(0);

            }
        }
    }
    else {
        lastFireState = false;
        playerData.data = 0;
    }
    if((GPIOB->DIN31_0 & (1 << 19)) > 1) {// up (nothing as of now)

    }

    if((GPIOA->DIN31_0 & (1 << 25)) > 1)  { // joystick button
//        spawnGoon(2, 1); // 2 hp, big goon
        // spawnBoss();
        gameOver(1, 0); // single player win
//        gameOver(0, 2); // multiplayer lose
//        gameOver(0, 1); // multiplayer win

    }

    // snap to map bounds
//    if(playerData.x < 10) {
//        playerData.x = 16;
//    }
//    if(playerData.y < 30) {
//        playerData.y = 30;
//    }
//    if(playerData.x > 96) {
//        playerData.x = 96;
//    }
//    if(playerData.y > 144) {
//        playerData.y = 144;
//    }
    // outer bounds
    if(playerData.x < 4) {
        playerData.x = 4;
    }
    if(playerData.y < 16) {
        playerData.y = 16;
    }
    if(playerData.x > 112) {
        playerData.x = 112;
    }
    if(playerData.y > 159) {
        playerData.y = 159;
    }
}
