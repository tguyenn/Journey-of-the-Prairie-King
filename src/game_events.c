// game_event.c
// Last Modified: 4/20/2024
// Student names: Toby Nguyen
// handles game events

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
extern solid_t solids[MAX_SOLIDS];
extern bossData_t boss;
extern collData_t colls[MAX_COLLECTIBLES];
extern bossBulletData_t bossBullets[MAX_BOSS_BULLETS];
extern const unsigned short *currentMap;
extern Language_t myL;
extern uint8_t gameState;
extern uint8_t roundNum;
extern flag_t otherFlag;
extern flag_t flag;

// 0 -> start screen
// 1 -> map1
// 2 -> map2
// 3 -> map3 (mulitplayer)?
extern uint32_t score;
extern const unsigned short *currentMap;
extern uint8_t money;

// sounds
const uint16_t soundLength[4] = {5015,6624,7200,6048};
const uint8_t *currSound[4] = {Pew, collectible, orcHit, playerDeath};




// display score
void gameOver(uint8_t singleWin, uint8_t multiWin) {
//    makeSound(3);
    SysTick->CTRL = 0;  // disable systick
    ST7735_FillScreen(ST7735_BLACK);
    ST7735_SetTextColor(ST7735_WHITE);
    if(multiWin == 0) {
        if(singleWin == 0) {
            drawVictory(2);
        }
        if(singleWin) {
            ST7735_SetCursor(1, singleWin);
            ST7735_OutString((char *)Phrases[GAMEOVER][myL]);

            drawVictory(1);
            // TODO: put stuff here for victory animation
        }
        ST7735_SetCursor(1, singleWin+1);
        ST7735_OutString((char *)Phrases[SCORE][myL]);
        ST7735_OutUDec(score);
        ST7735_SetCursor(1, singleWin+2);
        ST7735_OutString((char *)Phrases[MONEY][myL]);
        ST7735_OutUDec(money);
    }
    if(multiWin == 1) {
        // output you win
        drawVictory(1);
    }
    if(multiWin == 2) {
        // output you lose
        drawVictory(2);
    }


    while(1){
//        if((GPIOB->DIN31_0 & (1 << 16)) > 1) {
//            money = 0;
//            score = 0;
//            for(int i = 0; i < MAX_GOONS; i++) {
//                goons[i].x = 0;
//                goons[i].y = 0;
//                goons[i].alive = 0;
//            }
//            for(int i = 0; i < MAX_COLLECTIBLES; i++) {
//                bullets[i].x = 0;
//                bullets[i].y = 0;
//                bullets[i].data = 0;
//            }
//            for(int i = 0; i < MAX_BULLETS; i++) {
//                colls[i].x = 0;
//                colls[i].y = 0;
//                colls[i].data = 0;
//            }
//            initSysTick();
//            initMap1();
//            break;
//        }
    }
}


extern uint8_t soundCount;
extern uint8_t soundIndex;
void makeSound(uint8_t state){
    soundIndex = state;
    soundCount = 0;
    TimerG12_IntArm(7256,2);
    __enable_irq();
}

// create instance of bullet in direction of player
void playerShoot(bool shotgun) {

    // create new instance of bullet by looking through bullet array and using the first one that is not in use
    uint8_t i = 0;
    uint8_t iterations = 1;
    uint8_t dirBuf = playerData.playerDir;
    makeSound(0);
    if(shotgun) {
        iterations = 3;
        dirBuf -= 1; // rotate clockwise 1
        if(dirBuf == 255) {
            dirBuf = 7;
        }
    }

    for(int j = 0; j < iterations; j++) {
        while(((bullets[i].data & (1<<3)) == (1<<3)) && (i < MAX_BULLETS))  { // if find active, loop until find inactive
            i++;
        }
        bullets[i].data |= (1<<3); // reactivate found bullet
        bullets[i].data |= dirBuf; // bullet and player same direction
        uint8_t offset = 6; // need so bullet isn't drawn directly on player

        switch(bullets[i].data & 0x7) { // check direction in packed data
               case 0:
                   bullets[i].x = playerData.x + 2*offset;
                   bullets[i].y = playerData.y;
                   break;
               case 1:
                   bullets[i].x = playerData.x + 2*offset;
                   bullets[i].y = playerData.y - 2*offset;
                   break;
               case 2:
                   bullets[i].x = playerData.x;
                   bullets[i].y = playerData.y - 2*offset;
                   break;
               case 3:
                   bullets[i].x = playerData.x - offset;
                   bullets[i].y = playerData.y - offset;
                   break;
               case 4:
                   bullets[i].x = playerData.x - 2*offset;
                   bullets[i].y = playerData.y;
                   break;
               case 5:
                   bullets[i].x = playerData.x - offset;
                   bullets[i].y = playerData.y + offset;
                   break;
               case 6:
                   bullets[i].x = playerData.x + 7;
                   bullets[i].y = playerData.y + 2*offset - 5;
                   break;
               case 7:
                   bullets[i].x = playerData.x + offset;
                   bullets[i].y = playerData.y + offset;
                   break;
       }
        dirBuf++;
    }
}

// updates position and if active or not
void updateBullets(void) {
    uint8_t i = 0;
    while(i < MAX_BULLETS) {
        if(bullets[i].data & (1<<3)) {
            if((bullets[i].x < 3) || (bullets[i].y < 3) || (bullets[i].x > 128) || (bullets[i].y > 160)) { // kill bullet if out of bounds
                GPIOA->DOUT31_0 ^= (1 << 16) | (1 << 17) | (1 << 24);
//                ST7735_MaskBitmap(bullets[i].x, bullets[i].y, currentMap, bullet0Mask, 11, 11);
//                ST7735_FillRect(bullets[i].x, bullets[i].y - 12, 11, 12, 0x55FE);
                bullets[i].data = (1<<5); // set for death mask
                continue;
            }
            switch(bullets[i].data & 0x7) { // check direction in packed data
            case 0:
                bullets[i].x = bullets[i].x  + 2;
                break;
            case 1:
                bullets[i].x = bullets[i].x + 2;
                bullets[i].y  = bullets[i].y - 2;
                break;
            case 2:
                bullets[i].y = bullets[i].y - 2;
                break;
            case 3:
                bullets[i].x = bullets[i].x - 2; 
                bullets[i].y = bullets[i].y - 2;
                break;
            case 4:
                bullets[i].x = bullets[i].x - 2;
                break;
            case 5:
                bullets[i].x = bullets[i].x - 2; 
                bullets[i].y = bullets[i].y + 2;
                break;
            case 6:
                bullets[i].y = bullets[i].y + 2;
                break;
            case 7:
                bullets[i].x = bullets[i].x + 2; 
                bullets[i].y = bullets[i].y + 2;
                break;
            }
        }
        i++;
    }
}

// checks if bounding boxes of bullets overlap with goons(naive implementation)
extern uint8_t goonKills;
void bulletCollision(void) {
    uint8_t i = 0, j = 0;
    uint8_t range = 10;

    for(int i = 0; i < MAX_BULLETS; i++) { // boss and otherPlayer collision
        if(((bullets[i].data & (1<<3)) == (1<<3)) && (i < MAX_BULLETS)) {
            if(boss.alive == 1) { // find bullets intersecting with boss
                if((((boss.x - range) <= bullets[i].x) &&
                ((bullets[i].x) <= boss.x + range) && ((boss.y-range) <= bullets[i].y) &&
                ((boss.y + range) >= bullets[i].y))) {
                    boss.health--;
                    if(boss.health > 40) {
                        boss.health = 0; // overflow protection
                    }
                    bullets[i].data &= ~(1<<3);
                    bullets[i].data |= (1 << 5); // mark for death mask
                }
            }
            if(roundNum == 10 || roundNum == 11) { // multiplayer
                range = 7;
                if((((otherPlayerData.x - range) <= bullets[i].x) &&
                        ((bullets[i].x) <= otherPlayerData.x + range) && ((otherPlayerData.y-range) <= bullets[i].y) &&
                        ((otherPlayerData.y + range) >= bullets[i].y))) {
                    if(roundNum == 11) {
                        flag.isCaptured = false;
                        otherPlayerData.erase = true;
                        return;
                    }
                    gameOver(0, 1);
                }
            }
           for(int j = 0; j < MAX_GOONS; j++) { // goon collision
               if((((goons[j].alive & ~(1<<7)) > 0) && (goons[j].x - range) <= bullets[i].x) && // find bullets intersecting with live goons
              ((bullets[i].x) <= goons[j].x + range) && ((goons[j].y-range) <= bullets[i].y) &&
              ((goons[j].y + range) >= bullets[i].y)) {
                   goons[j].health -= 1;
                   bullets[i].data &= ~(1<<3); // kill bullet
                   bullets[i].data |= (1 << 5); // mark for death mask
                   if(goons[j].health < 1) {
                       goons[j].alive &= ~(1); // set alive status to 0
                       goons[j].alive = (1<<7); // mark for death animation
                       GPIOA->DOUT31_0 ^= (1 << 24);
                       bullets[i].data &= ~(1<<3);
                       bullets[i].data |= (1 << 5); // mark for death mask
                       goonKills++;
                       makeSound(2);
                       score += 100;
                   }
              }
           }
        }
    }
}



// checks if bounding boxes of goons overlap with player (naive implementation)
void goonCollision() {
    uint8_t bBox = 10;
    for (int i = 0; i < MAX_GOONS; i++) {
        if(((1 & (goons[i].alive)) == 1) && (i < MAX_GOONS)) {
            if(((playerData.x - bBox) <= goons[i].x) && ((goons[i].x) <= playerData.x + bBox)
                && ((playerData.y-bBox) <= goons[i].y) &&  ((playerData.y + bBox) >= goons[i].y)) {
                GPIOA->DOUT31_0 ^= (1 << 16);
                gameOver(0, 0);
            }
        }
    }
}


void collCollision(void) {
    uint8_t bBox = 5;
    for(int i = 0; i < MAX_COLLECTIBLES; i++) {
        if(((1 & (colls[i].data)) == 1) && (i < MAX_COLLECTIBLES)) {
            colls[i].active++; // despawn timer
            if(((otherPlayerData.x - bBox) <= colls[i].x) && ((colls[i].x) <= otherPlayerData.x + bBox)
                    && ((otherPlayerData.y-bBox) <= colls[i].y) &&  ((otherPlayerData.y + bBox) >= colls[i].y)) {
                colls[i].data = 0;
            }

            if(((playerData.x - bBox) <= colls[i].x) && ((colls[i].x) <= playerData.x + bBox)
                    && ((playerData.y-bBox) <= colls[i].y) &&  ((playerData.y + bBox) >= colls[i].y)) {
                        GPIOA->DOUT31_0 ^= (1 << 17);
                        makeSound(1);
                        playerData.item = 1;

                // do things based on coll type
                switch((colls[i].data & 0xE) >> 1) {
                case 0: // coin (coin)
                    money++;
                    score+= 100;
                    break;
                case 1: // shotgun (shotgun)
                    playerData.gun = 1;
//                    money -= 10;
                    break;
                case 2: // player movement (boot)
                    if(roundNum == 10) { // case for multiplayer in case i break something later?
                        playerData.speed = 2;
                    }
                    playerData.speed++;
//                    money -= 10;
                    break;
                case 3: // gun speed (badge)
                    playerData.fireSpeed -= 1;
                    money -= 10;
                    score += 5000;
                    break;
                case 4: // coin 5
                    money += 5;
                    score += 250;
                    break;
                }
                colls[i].data = 0;
                // no need to draw mask because player will be on top of it after it stops updating
            }
        }

    }
}

// advance Goons one step toward player
extern uint8_t roundNum;
void updateGoons(void) {
    static uint8_t count; // update timer
    count += 1;
    if(((count & 1) > 0) || (roundNum >= 3)) { // if count is odd
        int16_t dx, dy;
        uint8_t i = 0;

        // for loop test ( broken)... need to force iteration over EVERY living goon
        for(uint8_t i = 0; i < MAX_GOONS; i++) {
            if((1 & goons[i].alive) == 1) {
                // find direction
                dx = playerData.x - goons[i].x;
                dy = playerData.y - goons[i].y;
                // normalize direction
                if (dx > 0) dx = 1;
                else if (dx < 0) dx = -1;
                if (dy > 0) dy = 1;
                else if (dy < 0) dy = -1;
                // move
                goons[i].x += dx;
                goons[i].y += dy;
            }
        }
    }
}

void spawnGoon(uint8_t health, uint8_t type) {
    uint8_t i = 0;
    static const uint8_t map1spawnsx[4] = {0, 0, 112, 112}; // (0, 16), (0, 160), (112, 16), (112, 160)
    static const uint8_t map1spawnsy[4] = {10, 160, 10, 160};
    static const uint8_t numSpawns[2] = {4, 11}; // number of spawns on map - 1 for randomizer
    static uint8_t lastSpawn; // last spawn location to prevent too many enemies in one place
    uint8_t random;


    uint8_t spawnIndex = 0;
    while(((1 & goons[i].alive) == 1) && (i < MAX_GOONS)) {  // if find active, loop until find inactive
        i++;
    }
    if(i > MAX_GOONS-1) { // kick out of loop to prevent bugs
        return;
    }
    goons[i].alive = 1;
    goons[i].type = type;
    goons[i].health = health;
    if(goons[i].type > 0) { // big goon get more health
        goons[i].health++;
    }
    do { // reroll spawn location until different
        random = rand() % (numSpawns[gameState - 1] +1);
//        random %= numSpawns[gameState - 1];
    } while(lastSpawn == random);
    lastSpawn = random;
    goons[i].x = map1spawnsx[random];
    goons[i].y = map1spawnsy[random];
}

// assuming type is within range 0-3
void spawnColl(uint8_t inx, uint8_t iny, uint8_t type) {
    uint8_t i = 0;
    while((colls[i].data & 1) > 0) { // search for inactive
          i++;
          if(i > MAX_COLLECTIBLES) {
              return;
          }
    }
    colls[i].active = 0; // reset alive timer
    colls[i].data |= 1; // set active
    colls[i].x = inx;
    colls[i].y = iny;
    colls[i].data |= (type << 1);
}

//    spawnColl(50, 40, 0); // coin
//    spawnColl(50, 55, 1) ; // shotgun
//    spawnColl(50, 70, 2); // speed
//    spawnColl(50, 85, 3); // badge

void openShop(void) {
    spawnColl(35, 65, 1);
    spawnColl(60, 65, 2);
    spawnColl(85, 65, 3);
}

void spawnBoss(void) {
    boss.x = 60;
    boss.y = 65;
//    playerData.x = 60;
//    playerData.y = 100;
    boss.alive = 1;
    boss.health = 30;
    boss.needsBar = 1;
}

void bossShoot() {
    static uint8_t delay;
    uint8_t iterations = 8;
    uint8_t i = 0; // loop counter
    uint8_t offset = 6;

    delay++;
    if(delay > 50) {
        delay = 0;
        uint8_t dirBuf = boss.fireDir - 1;
    if(dirBuf > 250) {
        dirBuf = 7;
    }
    for(uint8_t j = 0; j < iterations; j++) {
        while((bossBullets[i].alive == true) && (i < MAX_BOSS_BULLETS))  { // if find active, loop until find inactive
            i++;
        }
            bossBullets[i].alive = true;
            bossBullets[i].dir = dirBuf;
            switch(bossBullets[i].dir) {
                   case 0:
                       bossBullets[i].x = boss.x + 2*offset;
                       bossBullets[i].y = boss.y;
                       break;
                   case 1:
                       bossBullets[i].x = boss.x + 2*offset;
                       bossBullets[i].y = boss.y - 2*offset;
                       break;
                   case 2:
                       bossBullets[i].x = boss.x;
                       bossBullets[i].y = boss.y - 2*offset;
                       break;
                   case 3:
                       bossBullets[i].x = boss.x - offset;
                       bossBullets[i].y = boss.y - offset;
                       break;
                   case 4:
                       bossBullets[i].x = boss.x - 2*offset;
                       bossBullets[i].y = boss.y;
                       break;
                   case 5:
                       bossBullets[i].x = boss.x - offset;
                       bossBullets[i].y = boss.y + offset;
                       break;
                   case 6:
                       bossBullets[i].x = boss.x + 7;
                       bossBullets[i].y = boss.y + 2*offset - 5;
                       break;
                   case 7:
                       bossBullets[i].x = boss.x + offset;
                       bossBullets[i].y = boss.y + offset;
                       break;
           }
            dirBuf++;
            if(dirBuf > 7) {
                dirBuf = 0;
            }
        }
    }
}




void bossBulletCollision() {
    uint8_t bBox;
    if(roundNum == 10 || roundNum == 11) {
        bBox = 10;
    }
    else{
        bBox = 7;
    }
    for(int i = 0; i < MAX_BOSS_BULLETS; i++) {
        if(bossBullets[i].alive) {
            if(((playerData.x - bBox) <= bossBullets[i].x) && ((bossBullets[i].x) <= playerData.x + bBox)
            && ((playerData.y-bBox) <= bossBullets[i].y) &&  ((playerData.y + bBox) >= bossBullets[i].y)) {
                GPIOA->DOUT31_0 ^= (1 << 16);
                if(roundNum == 11) { // multiplayer CTF
                    otherFlag.isCaptured = false;
                    playerData.erase = true;
                    return;
                }
                if(roundNum == 10) { // multiplayer standard
                    gameOver(0, 2);
                }
                gameOver(0, 0);
            }
        }
    }
}



void updateBossBullets(void) {
    uint8_t i = 0;
     while(i < MAX_BOSS_BULLETS) {
         if(bossBullets[i].alive) {
             if((bossBullets[i].x < 3) || (bossBullets[i].y < 3) || (bossBullets[i].x > 128) || (bossBullets[i].y > 160)) { // kill bullet if out of bounds
                 bossBullets[i].alive = 0;
                 bossBullets[i].kill = 1;
                 continue;
             }
             switch(bossBullets[i].dir) { // check direction in packed data
             case 0:
                 bossBullets[i].x = bossBullets[i].x  + 2;
                 break;
             case 1:
                 bossBullets[i].x = bossBullets[i].x + 2;
                 bossBullets[i].y  = bossBullets[i].y - 2;
                 break;
             case 2:
                 bossBullets[i].y = bossBullets[i].y - 2;
                 break;
             case 3:
                 bossBullets[i].x = bossBullets[i].x - 2;
                 bossBullets[i].y = bossBullets[i].y - 2;
                 break;
             case 4:
                 bossBullets[i].x = bossBullets[i].x - 2;
                 break;
             case 5:
                 bossBullets[i].x = bossBullets[i].x - 2;
                 bossBullets[i].y = bossBullets[i].y + 2;
                 break;
             case 6:
                 bossBullets[i].y = bossBullets[i].y + 2;
                 break;
             case 7:
                 bossBullets[i].x = bossBullets[i].x + 2;
                 bossBullets[i].y = bossBullets[i].y + 2;
                 break;
             }
         }
         i++;
     }
}
void updateBoss(void) {
    static uint8_t bossTick; // movement update
    int8_t dx, dy;
    bossTick++;
    if(boss.health == 0) {
        boss.alive = 0;
    }
    if((bossTick % 3) >= 2) { //if odd
        // find direction
        dx = playerData.x - boss.x;
        dy = playerData.y - boss.y;
        // normalize direction
        if (dx > 0) { //
            dx = 1;
            boss.fireDir = 0;
        }
        else if (dx < 0) {
            dx = -1;
            boss.fireDir = 4;
        }
        if (dy > 0) {
            dy = 1;
            if(boss.fireDir == 0) {
                boss.fireDir = 1;
            }
            if(boss.fireDir == 4) {
                boss.fireDir = 7;
            }
        }
        else if (dy < 0) { // boss above player
            dy = -1;
            if(boss.fireDir == 0) {
                boss.fireDir = 7;

            }
            if(boss.fireDir == 4) {
                boss.fireDir = 3;
            }
        }
        // move
        boss.x += dx;
        boss.y += dy;
        if(bossTick > 20) {
            bossShoot();
            bossTick = 0;
        }

    }
}



// note that last fire state is broken beacuse my UART SUCKS (pulls a false 0 every so often, which breaks normal player shoot system)
extern uint8_t multiGunReload;
void otherPlayerShoot(void) {
//    if((otherPlayerData.lastFireState < 1) && (multiGunReload > playerData.firespeed)) { // TODO might need to debug this thing lol
//        otherPlayerData.lastFireState = 1;
//        multiGunReload = 0;
//    }

    if((otherPlayerData.lastFireState == 0) && (multiGunReload > playerData.fireSpeed)) {
        multiGunReload = 0;

        // create new instance of bullet by looking through bullet array and using the first one that is not in use
        uint8_t i = 0;
        makeSound(0);

        while(bossBullets[i].alive && (i < MAX_BOSS_BULLETS))  { // if find active, loop until find inactive
            i++;
        }

        bossBullets[i].alive = 1; // reactivate found bullet
        bossBullets[i].dir = otherPlayerData.dir; // bullet and player same direction
        uint8_t offset = 6; // need so bullet isn't drawn directly on player

        switch(bossBullets[i].dir) { // check direction in packed data
            case 0:
                bossBullets[i].x = otherPlayerData.x + 2*offset;
                bossBullets[i].y = otherPlayerData.y;
                break;
            case 1:
                bossBullets[i].x = otherPlayerData.x + 2*offset;
                bossBullets[i].y = otherPlayerData.y - 2*offset;
                break;
            case 2:
                bossBullets[i].x = otherPlayerData.x;
                bossBullets[i].y = otherPlayerData.y - 2*offset;
                break;
            case 3:
                bossBullets[i].x = otherPlayerData.x - offset;
                bossBullets[i].y = otherPlayerData.y - offset;
                break;
            case 4:
                bossBullets[i].x = otherPlayerData.x - 2*offset;
                bossBullets[i].y = otherPlayerData.y;
                break;
            case 5:
                bossBullets[i].x = otherPlayerData.x - offset;
                bossBullets[i].y = otherPlayerData.y + offset;
                break;
            case 6:
                bossBullets[i].x = otherPlayerData.x + 7;
                bossBullets[i].y = otherPlayerData.y + 2*offset - 5;
                break;
            case 7:
                bossBullets[i].x = otherPlayerData.x + offset;
                bossBullets[i].y = otherPlayerData.y + offset;
                break;
        }
    }
    else { // reduindant but whatever ig
//        multiGunReload++;// move to main so it always increments
        otherPlayerData.lastFireState = 0;
    }
}

// handle bullet and wall collision
void wallCollision(void) {
    uint8_t bBox = 10;
    for(int i = 0; i < MAX_BOSS_BULLETS; i++) { // external player bullets
        if(bossBullets[i].alive) {
            for(int j = 0; j < MAX_SOLIDS; j++) {
                if(((solids[j].x - bBox) <= bossBullets[i].x) && ((bossBullets[i].x) <= solids[j].x + bBox)
                && ((solids[j].y-bBox) <= bossBullets[i].y) &&  ((solids[j].y + bBox) >= bossBullets[i].y)) {
                    bossBullets[i].alive = 0;
                    bossBullets[i].kill = 1;
                }
            }
        }
    }

    for(int i = 0; i < MAX_BULLETS; i++) { // local player bullets
        if(bullets[i].data & (1<<3)) {
            for(int j = 0; j < MAX_SOLIDS; j++) {
                if(((solids[j].x - bBox) <= bullets[i].x) && (bullets[i].x <= solids[j].x + bBox)
                && ((solids[j].y-bBox) <= bullets[i].y) &&  ((solids[j].y + bBox) >= bullets[i].y)) {
                    bullets[i].data &= ~(1<<3);
                    bullets[i].data |= (1 << 5); // mark for death mask
                }
            }
        }
    }

    bBox = 10; // player collisions
    for(int j = 0; j < MAX_SOLIDS; j++) {
        if(((solids[j].x - bBox) <= playerData.x) && (playerData.x <= solids[j].x + bBox)
        && ((solids[j].y-bBox) <= playerData.y) &&  ((solids[j].y + bBox) >= playerData.y)) {
            playerData.x = playerData.lastx;
            playerData.y = playerData.lasty;
            playerData.barrierColl = true;
            return; // stop checking if find wall collision
        }
        else{
            playerData.barrierColl = false;
        }
    }

//    for(int j = 0; j < MAX_SOLIDS; j++) { // dont need this because otherplayer is handled on other MCU locally
//        if(((solids[j].x - bBox) <= otherPlayerData.x) && (otherPlayerData.x <= solids[j].x + bBox)
//        && ((solids[j].y-bBox) <= otherPlayerData.y) &&  ((solids[j].y + bBox) >= otherPlayerData.y)) {
//            otherPlayerData.x = otherPlayerData.lastx;
//            otherPlayerData.y = otherPlayerData.lasty;
//        }
//    }
}

void flagCollision(void) { // smth is wrong with this
    uint8_t bBox = 10;
    if(((otherFlag.x - bBox) <= playerData.x) && (playerData.x <= otherFlag.x + bBox)
    && ((otherFlag.y-bBox) <= playerData.y) &&  ((otherFlag.y + bBox) >= playerData.y)) {
        otherFlag.isCaptured = true;
    }
    if(((flag.x - bBox) <= otherPlayerData.x) && (otherPlayerData.x <= flag.x + bBox)
    && ((flag.y-bBox) <= otherPlayerData.y) &&  ((flag.y + bBox) >= otherPlayerData.y)) {
        flag.isCaptured = true;
    }



}

extern uint8_t multiState;
void updateFlags(void){ // FLAG MOVEMENT (isCaptured == true) UPDATED IN GETBUTTONS() TO SYNC WITH PLAYER MOTION
    if(multiState == 5) { // local is p1 (home is top left)
        if(flag.isCaptured == false) {
            flag.x = 30; // top left corner
            flag.y = 25;
        }
        else{
            flag.x = otherPlayerData.x + 20;
            flag.y = otherPlayerData.y;
        }

        if(otherFlag.isCaptured == false) {
            otherFlag.x = 95; //bottom right
            otherFlag.y = 140;
        }
        else{
            otherFlag.x = playerData.x + 20;
            otherFlag.y = playerData.y;
            playerData.data |= (1<<7); // if local player captured enemy flag, then broadcast in data packet
        }
    }
    if(multiState == 6) { // local is p2 (home is botto right)
        if(flag.isCaptured == false) {
            flag.x = 95; // bottom right
            flag.y = 140;
        }
        if(otherFlag.isCaptured == false) {
            otherFlag.x = 30; // top left corner
            otherFlag.y = 25;
        }
        else{
            otherFlag.x = playerData.x + 20;
            otherFlag.y = playerData.y;
            playerData.data |= (1<<7); // if local player captured enemy flag, then broadcast in data packet
        }
    }

}

void flagWin(void) {
    if(multiState == 5) { // local top left
        if((playerData.x < 35) && (playerData.y < 39) && otherFlag.isCaptured) {  // local wins
            gameOver(0, 1);
        }
        if((otherPlayerData.x > 82) && (otherPlayerData.y > 132) && flag.isCaptured) {// local lose
            gameOver(0, 2);
        }
    }
    if(multiState == 6) { // local bottom right
        if((playerData.x > 82) && (playerData.y > 132) && otherFlag.isCaptured) {// local win
            gameOver(0, 1);
        }
        if((otherPlayerData.x < 35) && (otherPlayerData.y < 39) && flag.isCaptured) {  // local lose
            gameOver(0, 2);
        }


    }
}




