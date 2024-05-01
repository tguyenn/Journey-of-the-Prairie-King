// draw.c
// Last Modified: 4/20/2024
// Student names: Toby Nguyen
// outputs to the LCD
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
#define MAX_BOSS_BULLETS 10
#define MAX_SOLIDS 2

extern playerData_t playerData;
extern otherPlayerData_t otherPlayerData;
extern bulletData_t bullets[MAX_BULLETS];
extern goonData_t goons[MAX_GOONS];
extern collData_t colls[MAX_COLLECTIBLES];
extern bossBulletData_t bossBullets[MAX_BOSS_BULLETS];
extern bossData_t boss;
extern solid_t solids[MAX_SOLIDS];
extern const unsigned short *currentMap;
extern uint8_t gameState;
extern flag_t otherFlag;
extern flag_t flag;
extern uint8_t multiState;
// 0 -> start screen
// 1 -> map1
// 2 -> map2
// 3 -> map3 (mulitplayer)?
extern uint32_t score;
extern const unsigned short *currentMap;
extern uint8_t money;
extern uint8_t roundNum;

// sprite test
void testSprites(void) {
    ST7735_DrawBitmap(0, 20, cowboyDown, 17, 17);
    ST7735_DrawBitmap(0, 40, cowboyRight, 17, 16);
    ST7735_DrawBitmap(0, 60, cowboyLeft, 17, 16);
    ST7735_DrawBitmap(0, 80, cowboyBack, 17, 16);
//    ST7735_DrawBitmap(0, 150, p1LeftLeg, 12, 5);
//    ST7735_DrawBitmap(20, 20, p1RightLeg, 12, 5);
    ST7735_DrawBitmap(0, 100, goonRight, 17, 18);
    ST7735_DrawBitmap(0, 140, goonLeft, 17, 18);
    ST7735_DrawBitmap(0, 120, bullet0, 7, 7);
}

void drawVictory(uint8_t player) { // not actually player ... 1 means win and 2 means lose lol too lazy to change
    uint8_t r, g, b;


    if(player == 1) {
        // win transition animation
        r = 235 - 50;
        g = 171 - 50;
        b = 52 - 50;
        for(uint8_t i = 0; i < 80; i++) {
            ST7735_DrawFastHLine(0, i, 128, ST7735_Color565(r, g, b)); // top to bottom
            ST7735_DrawFastHLine(0, 160 - i, 128, ST7735_Color565(r, g, b)); // bottom to top
            Clock_Delay(100000);
            // shit did not work lol but oh well idc
//            if(g < 171) { // player 1
//                g++;
//            }
//            if(g > 171) {
//                g--;
//            }
//            if(r < 235) {
//                r++;
//            }
            if(r < 250) {
                r += 4;
            }
            if(g < 250) { // player 1
                g += 4;
            }
            if(b < 250) {
                b += 4;
            }

        }

        // middle banner
        if(player == 1) { // player 1
            r = 188;
            g = 71;
            b = 54;

            for(uint8_t i = 0; i < 128; i++) {
                ST7735_DrawFastVLine(i, 55, 34, ST7735_Color565(r, g, b));
                g++;
                b++;
                Clock_Delay(40000);
            }
            for(uint8_t i = 0; i < 128; i++) {
                ST7735_DrawFastVLine(i, 55, 34, ST7735_Color565(0xF8, 0xF8, 0xF4));
                Clock_Delay(50000);
            }
        }
    }


    if(player == 2) { // player 2
        r = 0;
        g = 0;
        b = 0;
        for(uint8_t i = 0; i < 80; i++) {
                   ST7735_DrawFastHLine(0, i, 160, ST7735_Color565(r, g, b)); // top to bottom
                   ST7735_DrawFastHLine(0, 160 - i, 160, ST7735_Color565(r, g, b)); // bottom to top
                   Clock_Delay(100000);
                   r += 5;
                   g += 5;
                   b += 5;
                   if(r > 255) {
                       r = 255;
                   }
                   if(g > 255) {
                       g = 255;
                   }
                   if(b > 255) {
                       b = 255;
                   }
        }

//        for(uint8_t i = 0; i < 128; i++) {
//            ST7735_DrawFastVLine(i, 55, 34, ST7735_Color565(r, g, b));
//            r++;
//            b--;
//            Clock_Delay(40000);
//        }
//        for(uint8_t i = 0; i < 128; i++) {
//            ST7735_DrawFastVLine(i, 55, 34, ST7735_Color565(0xFF, 0xFF, 0xFF));
//            Clock_Delay(50000);
//        }
    }


    // print white banner and player
//    ST7735_FillRect(0, 55, 160, 34, ST7735_Color565(0xFF, 0xFF, 0xFF));
    if(player == 1) {
        ST7735_SetCursor(7,5);
        ST7735_SetTextColor(ST7735_BLACK);
        lolstring((char *)Phrases[GAMEOVER][myL]);
        ST7735_DrawBitmap(56, 80, cowboyVictory, 16, 16);
    }
    if(player == 2) {
//        ST7735_DrawBitmap(56, 80, otherCowboyVictory, 16, 16);
        ST7735_SetCursor(4,3);
        ST7735_OutString("Loser!1!!haha");
        ST7735_DrawBitmap(34, 110, valvo, 60, 60);
    }



}

void clearGoons(void) {
    for(uint8_t i = 0; i < MAX_GOONS; i++) {
        if(goons[i].alive > 0) {
            goons[i].alive = 0;
            ST7735_MaskedBitmap(goons[i].x, goons[i].y, currentMap, goonMask, 17, 18); // lmao
        }
    }
}

// creates a goon at a randomly selected corner and creates a coin on death if rolled
// good luck reading this tbh lol
const unsigned short *currentFrame[5] = {goonDie0, goonDie1, goonDie2, goonDie3, goonMask};
void drawGoons(void) {
    uint8_t i = 0;
    uint8_t frameIndex = 0;
    for(uint8_t i = 0; i < MAX_GOONS; i++) {
        if((goons[i].alive & 1) > 0) { // step animation only affect alive goons
            if((goons[i].step > 0) && (goons[i].type == 1)) { // big goon
                ST7735_MaskedBitmap(goons[i].x, goons[i].y, currentMap, bigGoonLeft, 20, 19);
            }
            if((goons[i].step < 0) && (goons[i].type == 1)) {
                ST7735_MaskedBitmap(goons[i].x, goons[i].y, currentMap, bigGoonRight, 20, 20);
            }
            if(goons[i].type == 0) { // small goon
                if(goons[i].step > 0) {
                    ST7735_MaskedBitmap(goons[i].x, goons[i].y, currentMap, goonLeft, 17, 18);
                }
                if(goons[i].step < 0) {
                    ST7735_MaskedBitmap(goons[i].x, goons[i].y, currentMap, goonRight, 17, 18);
                }
            }
            goons[i].step += 4; // overflow toggles between - and +
        }
            // death animation
        if(((goons[i].alive & (1<<7)) > 0) && (((goons[i].alive & 0x0E) >> 1) == 3) && (goons[i].lastCounter > 0)){ // delay if on goonDie 3 before going to goonMask
          goons[i].lastCounter -= 5;
        }
        if(((goons[i].alive & (1<<7)) > 0) && (goons[i].lastCounter >= 200)) {
            goons[i].lastCounter = 0;
            frameIndex = ((goons[i].alive & 0x0E) >> 1); // xxxx111x
            ST7735_MaskedBitmap(goons[i].x, goons[i].y, currentMap, currentFrame[frameIndex], 17, 18); // lmao
            goons[i].alive = goons[i].alive + 2;
            if((((goons[i].alive & (0x0E))) >> 1) > 4) {
                goons[i].alive = 0; // DEAD dead fr (stop animations)
                if(goons[i].type == 1) {
                    ST7735_MaskedBitmap(goons[i].x + 20, goons[i].y, currentMap, cowboyFillV, 4, 17); // bandaid solution to big goon line leftovers
                }
                if(goons[i].type == 0) {
                    spawnColl(goons[i].x, goons[i].y, 0); // coin 1
                }
                if(goons[i].type == 1) {
                    spawnColl(goons[i].x, goons[i].y, 4); // spawn coin5
                }
                goons[i].x = 0;
                goons[i].y = 0;
            }
        }
        if((goons[i].alive) < 1) {
            continue;
        }
        else {
            goons[i].lastCounter += 10;
        }
        if(i > MAX_GOONS) {
            return;
        }
    }
}



void drawColls(void) {
    const unsigned short *items[5] = {coin, shotgun, movSpeedCollect, shinyBadge, coin5};
    for(int i = 0; i < MAX_COLLECTIBLES; i++) {
        if((colls[i].data & 1) > 0) { // if active
            uint8_t itemIndex = (colls[i].data & 0xE) >> 1;
            if(itemIndex == 11) {
                itemIndex = 4; // yeah ikik bad solution but whatever man
            }
            ST7735_MaskedBitmap(colls[i].x, colls[i].y, currentMap, items[itemIndex], 11, 11);
        }
        if(((colls[i].data & 1) > 0) && (colls[i].active > 250)) { // if active and alive for long time DIE
            ST7735_MaskedBitmap(colls[i].x, colls[i].y, currentMap, goonMask, 14, 13);
            colls[i].active = 0;
            colls[i].data = 0; // timeout KILL
        }
    }
}

void drawBullets(void) {
    uint8_t i = 0;
    while(i < MAX_BULLETS) {
      if((bullets[i].data & (1<<3)) > 1) {
          ST7735_MaskedBitmap(bullets[i].x, bullets[i].y, currentMap, bullet0, 11, 11);
      }
      if((bullets[i].data & (1<<5)) > 1) {
          ST7735_MaskedBitmap(bullets[i].x, bullets[i].y, currentMap, goonMask, 14, 13);
//          ST7735_FillRect(bullets[i].x, bullets[i].y - 12, 11, 12, 0x55FE);
          bullets[i].data = 0;
      }
      i++;
    }
}

bool bossDeathAnimationDoneLol;
void drawBoss(void) {
    static int8_t step = 0;
    static uint8_t frameIndex = 0;
    static uint8_t deadTimer = 59; // anim frame + dwell time
    if(boss.health > 0) {
        if(step > 0) {
            ST7735_MaskedBitmap(boss.x, boss.y, currentMap, bossLeft, 18, 18);
        }
        if(step < 0) {
            ST7735_MaskedBitmap(boss.x, boss.y, currentMap, bossRight, 18, 17);
        }
        step++;
    }
    if(boss.needsBar) {
        if((boss.health == 0) && (frameIndex < 5)) {
            if(deadTimer > 60) {
                ST7735_MaskedBitmap(boss.x + 5, boss.y + 1, currentMap, currentFrame[frameIndex], 17, 18);
                ST7735_MaskedBitmap(boss.x + 2, boss.y - 1, currentMap, currentFrame[frameIndex], 17, 18);
                ST7735_MaskedBitmap(boss.x - 3, boss.y + 4, currentMap, currentFrame[frameIndex], 17, 18);
                ST7735_MaskedBitmap(boss.x - 1, boss.y - 4, currentMap, currentFrame[frameIndex], 17, 18);
                ST7735_MaskedBitmap(boss.x + 1, boss.y + 2, currentMap, currentFrame[frameIndex], 17, 18);
                ST7735_MaskedBitmap(boss.x - 5, boss.y + 7, currentMap, currentFrame[frameIndex], 17, 18);
                ST7735_MaskedBitmap(boss.x + 2, boss.y + 3, currentMap, currentFrame[frameIndex], 17, 18);
                frameIndex++;
                deadTimer = 0;
                if(frameIndex > 4) {
                    bossDeathAnimationDoneLol = true;
                    score += 1000;
                }
            }
            deadTimer++;
        }
    }
}

void drawBossBar(void) {
    if(boss.needsBar) {
        if(boss.health > 0) {
            if(boss.health > 1) {
            ST7735_FillRect(0, 0, 8+boss.health*4, 10, ST7735_RED);
            ST7735_FillRect(8+boss.health*4, 0, 128, 10, ST7735_BLACK);
            }
        }
        if(boss.health == 0) {
            ST7735_FillRect(0, 0, 128, 10, ST7735_BLACK);
        }
    }
}


void drawBossBullets(void) {
    uint8_t i = 0;
    while(i < MAX_BOSS_BULLETS) {
      if(bossBullets[i].alive && (roundNum < 10)) {
          ST7735_MaskedBitmap(bossBullets[i].x, bossBullets[i].y, currentMap, bossBullet, 11, 11);
      }
      else if(bossBullets[i].alive && (roundNum == 10 || roundNum == 11)) {
          ST7735_MaskedBitmap(bossBullets[i].x, bossBullets[i].y, currentMap, bullet1, 11, 11);
      }
      // prob dont need to kill sprite on collision because game over if hit anyway
      if(bossBullets[i].kill > 0) {
          ST7735_MaskedBitmap(bossBullets[i].x, bossBullets[i].y, currentMap, goonMask, 14, 13);
////          ST7735_FillRect(bullets[i].x, bullets[i].y - 12, 11, 12, 0x55FE);
          bossBullets[i].kill = 0;
      }
      i++;
    }
}


// draw player based on which direction player travels in
void drawPlayer(void) {


    if(roundNum == 11 && playerData.erase) {
        if(multiState == 5) {
            if(playerData.erase) {
                ST7735_MaskedBitmap(playerData.x, playerData.y, currentMap, goonMask, 14, 13);
                ST7735_MaskedBitmap(playerData.x + 16, playerData.y, currentMap, goonMask, 14, 13);

                playerData.x = 36;
                playerData.y = 40;
                playerData.erase = false;
            }
        }
        if(multiState == 6) {
            if(playerData.erase) {
                ST7735_MaskedBitmap(playerData.x, playerData.y, currentMap, goonMask, 14, 13);
                ST7735_MaskedBitmap(playerData.x + 16, playerData.y, currentMap, goonMask, 14, 13);

                playerData.x = 77;
                playerData.y = 142;
                playerData.erase = false;
            }
        }
    }
    if((playerData.playerDir == 4) || playerData.playerDir == 5) {
        ST7735_MaskedBitmap(playerData.x, playerData.y, currentMap, cowboyLeft, 17, 16);

        ST7735_MaskedBitmap(playerData.x, playerData.y - 16, currentMap, cowboyFillH, 17, 4); // top
        ST7735_MaskedBitmap(playerData.x + 17, playerData.y, currentMap, cowboyFillV, 4, 17); // right


    }
    if((playerData.playerDir == 1) || (playerData.playerDir == 2) ||
        (playerData.playerDir == 3)) {
        ST7735_MaskedBitmap(playerData.x, playerData.y, currentMap, cowboyBack, 17, 16);

//            ST7735_FillRect(playerData.x + 17, playerData.y - 16, 4, 16, 0x55FE); // for when going really fast (right side)
//            ST7735_FillRect(playerData.x - 4, playerData.y - 16, 4, 16, 0x55FE); // for when going really fast (left side)
//            ST7735_FillRect(playerData.x, playerData.y + 1, 17, 4, 0x55FE); // for when going really fast (bottom side)
        ST7735_MaskedBitmap(playerData.x, playerData.y + 4, currentMap, cowboyFillH, 17, 4);
        ST7735_MaskedBitmap(playerData.x + 17, playerData.y + 4, currentMap, cowboyFillV, 4, 17);
        ST7735_MaskedBitmap(playerData.x - 4, playerData.y + 1, currentMap, cowboyFillV, 4, 17);



    }
    if((playerData.playerDir == 0) || playerData.playerDir == 7) {
        ST7735_MaskedBitmap(playerData.x, playerData.y, currentMap, cowboyRight, 17, 16);

//            ST7735_FillRect(playerData.x - 4, playerData.y - 16, 4, 16, 0x55FE); // for when going really fast (left side)
//            ST7735_FillRect(playerData.x, playerData.y - 20, 17, 4, 0x55FE); // for when going really fast (top side)
        ST7735_MaskedBitmap(playerData.x, playerData.y - 16, currentMap, cowboyFillH, 17, 4); // top
        ST7735_MaskedBitmap(playerData.x - 4, playerData.y + 1, currentMap, cowboyFillV, 4, 17);




    }
    if(playerData.playerDir == 6) {
        ST7735_MaskedBitmap(playerData.x, playerData.y, currentMap, cowboyDown, 17, 17);

//            ST7735_FillRect(playerData.x, playerData.y - 20, 17, 4, 0x55FE); // for when going really fast (top side)
        ST7735_MaskedBitmap(playerData.x, playerData.y - 16, currentMap, cowboyFillH, 17, 4); // top


    }
}


void drawOtherPlayer(void) {

    if(multiState == 5) {
        if(otherPlayerData.erase == true) {
            ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y, currentMap, goonMask, 14, 13);
            ST7735_MaskedBitmap(otherPlayerData.x+16, otherPlayerData.y, currentMap, goonMask, 14, 13);
            ST7735_MaskedBitmap(otherPlayerData.x+8, otherPlayerData.y - 8, currentMap, goonMask, 14, 13);
            otherPlayerData.erase = false;
            otherPlayerData.x = 77;
            otherPlayerData.y = 142;
        }
    }
    if(multiState == 6) {
        if(otherPlayerData.erase == true) {
            ST7735_MaskedBitmap(otherPlayerData.lastx, otherPlayerData.lasty, currentMap, goonMask, 14, 13);
            ST7735_MaskedBitmap(otherPlayerData.lastx+16, otherPlayerData.lasty, currentMap, goonMask, 14, 13);
            ST7735_MaskedBitmap(otherPlayerData.lastx+8, otherPlayerData.lasty - 8, currentMap, goonMask, 14, 13);
            otherPlayerData.erase = false;
            otherPlayerData.x = 36;
            otherPlayerData.y = 40;
        }
    }

    if((otherPlayerData.dir == 4) ||otherPlayerData.dir == 5) {
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y, currentMap, otherCowboyLeft, 19, 18);
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y - 17, currentMap, cowboyFillH, 17, 4); // top
        ST7735_MaskedBitmap(otherPlayerData.x + 18, otherPlayerData.y, currentMap, cowboyFillV, 4, 17); // right
    }
    if((otherPlayerData.dir == 1) || (otherPlayerData.dir == 2) ||
        (otherPlayerData.dir == 3)) {
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y + 1, currentMap, otherCowboyBack, 19, 18);
//            ST7735_FillRect(otherPlayerData.x + 17, otherPlayerData.y - 16, 4, 16, 0x55FE); // for when going really fast (right side)
//            ST7735_FillRect(otherPlayerData.x - 4, otherPlayerData.y - 16, 4, 16, 0x55FE); // for when going really fast (left side)
//            ST7735_FillRect(otherPlayerData.x, otherPlayerData.y + 1, 17, 4, 0x55FE); // for when going really fast (bottom side)
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y + 5, currentMap, cowboyFillH, 17, 4);
        ST7735_MaskedBitmap(otherPlayerData.x + 18, otherPlayerData.y + 5, currentMap, cowboyFillV, 4, 17);
        ST7735_MaskedBitmap(otherPlayerData.x - 5, otherPlayerData.y + 2, currentMap, cowboyFillV, 4, 17);
    }
    if((otherPlayerData.dir == 0) ||otherPlayerData.dir == 7) {
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y, currentMap, otherCowboyRight, 19, 18);
//            ST7735_FillRect(otherPlayerData.x - 4, otherPlayerData.y - 16, 4, 16, 0x55FE); // for when going really fast (left side)
//            ST7735_FillRect(otherPlayerData.x, otherPlayerData.y - 20, 17, 4, 0x55FE); // for when going really fast (top side)
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y - 17, currentMap, cowboyFillH, 17, 4); // top
        ST7735_MaskedBitmap(otherPlayerData.x - 5, otherPlayerData.y + 2, currentMap, cowboyFillV, 4, 17);
    }
    if(otherPlayerData.dir == 6) {
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y, currentMap, otherCowboyDown, 19, 19);
//            ST7735_FillRect(otherPlayerData.x, otherPlayerData.y - 20, 17, 4, 0x55FE); // for when going really fast (top side)
        ST7735_MaskedBitmap(otherPlayerData.x, otherPlayerData.y - 17, currentMap, cowboyFillH, 17, 4); // top


    }
}

void drawWalls(void) {
    uint8_t i = 0;
    while(i < MAX_SOLIDS) {
        if(solids[i].active > 0) {
            ST7735_MaskedBitmap(solids[i].x, solids[i].y, currentMap, wallH, 16, 16);
        }
        i++;
    }
}

void drawFlags(void) {
    ST7735_MaskedBitmap(flag.x, flag.y, currentMap, UT, 14, 16);
    ST7735_MaskedBitmap(otherFlag.x, otherFlag.y, currentMap, AM, 14, 16);
}




