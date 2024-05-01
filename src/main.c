// main.c
// Last Modified: 4/20/2024
// Student names: Toby Nguyen
// game engine

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
#define MAX_BOSS_BULLETS 20
#define MAX_SOLIDS 2

/*
 * GLOBAL VARS =====================================================================
 */

// game data
playerData_t playerData;
otherPlayerData_t otherPlayerData;
bulletData_t bullets[MAX_BULLETS];
goonData_t goons[MAX_GOONS];
collData_t colls[MAX_COLLECTIBLES];
bossData_t boss;
bossBulletData_t bossBullets[MAX_BOSS_BULLETS];
solid_t solids[MAX_SOLIDS];
flag_t otherFlag;
flag_t flag;

uint8_t gameState = 0;
// game states
// 0 -> start screen
// 1 -> map1
// 2 -> map2
// 3 -> map3 (mulitplayer)?
uint32_t score = 0;
const unsigned short *currentMap;
uint16_t money = 0;
uint8_t soundIndex = 0;
uint16_t soundCount = 0;
extern Language_t myL;
extern bool bossDeathAnimationDoneLol;
uint8_t playerShot = false;
uint8_t multiGunReload; // mp
uint8_t multiState = 0;



/*
 * MAIN =========================================================================
 */

uint16_t gameTick = 0; // global incremented in SysTick used to drive roundHandler
uint16_t itemSpawn = 0; // mutliplayer item spawn timer

uint8_t numSpawns[5] = {5, 10, 10, 15, 5}; // number of singleplayer enemies
uint8_t roundGoons[6] = {0, 0, 0, 0, 0, 1}; // determines what kind of goon to spawn... each round increases chance of big goon
uint8_t roundNum = 0;
uint8_t goonKills = 0;
bool init = true;
uint16_t lastTime; // for timing things
uint8_t randIdx = 0;
uint8_t health = 1;
extern uint8_t goonSpeed;

void serviceUART2(void) {

    otherPlayerData.lastx = otherPlayerData.x;
    otherPlayerData.lasty = otherPlayerData.y;

    uint8_t xbuf, ybuf, databuf;
    GPIOA->DOUT31_0 ^= (1 << 16); // red
    otherPlayerData.data = UART2_InChar();
    if(otherPlayerData.data == 0) {
        return;
    }
    otherPlayerData.y = UART2_InChar();
    otherPlayerData.x = UART2_InChar();
}

void roundHandler(void) {
    if(init == true) { // round initialization
        clearGoons();
        goonKills = 0;
        roundGoons[roundNum] = 1; // increase likelihood of spawning big goon

        ST7735_FillScreen(ST7735_BLACK);
        ST7735_SetCursor(8, 6);
        ST7735_OutString((char *)Phrases[WAVE][myL]);
        ST7735_OutUDec(roundNum);
        if(roundNum == 5) { // boss text color change
            ST7735_SetCursor(8, 6);
            ST7735_SetTextColor(ST7735_Color565(0xFF, 0x00, 0x00));
            ST7735_OutString((char *)Phrases[WAVE][myL]);
            ST7735_OutUDec(roundNum);
        }
        Clock_Delay(50000000);
        ST7735_DrawBitmap(0, 160, currentMap, 128, 160);
        init = false;
        switch(roundNum) { // initializations specific for each round:
        case 0: // first round

            break;
        case 1: // more enemies

            break;
        case 2: // shop
            openShop();
            gameTick = 0;
            break;
        case 3: // faster

            break;
        case 4: // more hp
            health++;
            break;
        case 5: // boss
            spawnBoss();
            break;
        case 10: // multiplayer
            // TODO
            break;
        }
    }

    // active round processes !!!!!!!!!!!!!!!!!!!!!!!!!!!
    randIdx = rand() % 6; // 0 - 5 // always run for all cases

    switch(roundNum) { // initializations specific for each round:
    case 0: // first round
        if((gameTick > 100) && (goonKills <= numSpawns[roundNum])) { // spawn
            gameTick = 0;
            spawnGoon(health, roundGoons[randIdx]);
        }

        if((gameTick > 150) && (goonKills >= numSpawns[roundNum])) { // next round
            roundNum++;
            init = true;
        }
        break;
    case 1: // more enemies more frequently
        if((gameTick > 50) && (goonKills <= numSpawns[roundNum])) {// spawn
            gameTick = 0;
            spawnGoon(health, roundGoons[randIdx]);
        }
        if((gameTick > 150) && (goonKills >= numSpawns[roundNum])) { // next round
            roundNum++;
            init = true;
        }
        break;
    case 2: // shop
        if(gameTick > 300) { // next round
            roundNum++;
            init = true;
        }
        break;
    case 3: // faster enemies
        if((gameTick > 50) && (goonKills <= numSpawns[roundNum])) {// spawn
            gameTick = 0;
            spawnGoon(health, roundGoons[randIdx]);
            spawnGoon(health, roundGoons[randIdx]);
        }
        if((gameTick > 150) && (goonKills >= numSpawns[roundNum])) { // next round
            roundNum++;
            init = true;
        }
        break;
    case 4: // more hp
        if((gameTick > 50) && (goonKills <= numSpawns[roundNum])) {// spawn
            gameTick = 0;
            spawnGoon(health, roundGoons[randIdx]);
            spawnGoon(health, roundGoons[randIdx]);
        }
        if((gameTick > 150) && (goonKills >= numSpawns[roundNum])) { // next round
            roundNum++;
            init = true;
        }
        break;
    case 5: // boss
        if((gameTick > 100) && (goonKills <= numSpawns[roundNum])) {// spawn
            gameTick = 0;
            spawnGoon(health, roundGoons[randIdx]);
        }
        if(boss.needsBar && (boss.alive < 1)) { // make sure no goons after boss dead
            clearGoons();
        }
        if(boss.needsBar && (boss.alive < 1) && (boss.health == 0) && bossDeathAnimationDoneLol) { // wait to play win animation
            gameOver(1, 0);
        }
        break;
    case 10: // multiplayer
        break;
    }
}

int main(void) {
    Clock_Init40MHz();
    LaunchPad_Init();
    ST7735_InitR(INITR_BLACKTAB); //SPI HiLetgo ST7735R // #2 green tab + invert
//    ST7735_InitR(INITR_GREENTAB);
//    ST7735_InvertDisplay(1);
    ST7735_FillScreen(ST7735_BLACK);

    // button inits
    IOMUX->SECCFG.PINCM[29] = (uint32_t) 0x00040081; // PB13 right
    IOMUX->SECCFG.PINCM[44] = (uint32_t) 0x00040081; // PB19 up
    IOMUX->SECCFG.PINCM[42] = (uint32_t) 0x00040081; // PB17 left
    IOMUX->SECCFG.PINCM[32] = (uint32_t) 0x00040081; // PB16 down
    IOMUX->SECCFG.PINCM[54] = (uint32_t) 0x00040081; // PA25 joystick

    uint32_t frameTimeStart, frameTime, frameTimeStop = 0;

    // pcb leds
    const int outs = (1 << 16) | (1 << 17) | (1 << 24);

    // outs
    GPIOA->DOE31_0 |= outs;
    IOMUX->SECCFG.PINCM[37] = (uint32_t) 0x00000081; // PA16 (red)
    IOMUX->SECCFG.PINCM[38] = (uint32_t) 0x00000081; // PA17 (yellow)
    IOMUX->SECCFG.PINCM[53] = (uint32_t) 0x00000081; // PA24 (green)

    myL = English; // default language

    uint8_t res;
    do{
        res = initStart();
    } while (res == 2); // reinit with new language

    if(res == 1) { // start normal game
        initMap1();
    }
    else if((res == 3) || (res == 4) || (res == 5) || (res == 6)) { // start multiplayer game
        // 3 = P1 Standard, 4 = P2 Standard, 5 = P1 CTF, 6 = P2 CTF

        initMultiplayer(res);
        // TODO: init timer 7
        __enable_irq();
        initG7(65535, 21, 0);
    }
    else if(res == 0) { // start dev game
        initDev();
    }

    myADCInit();
    initSysTick();
    TimerG12_Init();
    DAC_Init();

    uint8_t powerTime = 0; // powerup timer for multiplayer
    while(1) {
        frameTimeStart = SysTick->VAL;


        if(roundNum == 10 || roundNum == 11) { // multiplayer
            uint16_t resx, resy;
            uint8_t data;
            ADC0->ULLMEM.CTL0 |= 0x00000003;             // 1) enable conversions
            ADC0->ULLMEM.CTL1 |= 0x00000100;             // 2) start ADC
            uint32_t volatile delay=ADC0->ULLMEM.STATUS; // 3) time to let ADC start
            while((ADC0->ULLMEM.STATUS&0x01)==0x01){}    // 4) wait for completion

            resy = ADC0->ULLMEM.MEMRES[1];
            resx = ADC0->ULLMEM.MEMRES[2];

            drawOtherPlayer();
            drawPlayer();
            drawBullets();
            drawBossBullets();
            drawWalls();
            drawColls();
            if(roundNum == 11) {
                drawFlags();
            }

            if(itemSpawn > 200) {
                spawnColl(55, 85, 2);
                itemSpawn = 0;
            }
            if(playerData.item > 0) {
                powerTime++;
            }
            if(powerTime > 100) {
                powerTime = 0;
                playerData.speed = 1;
                playerData.item = 0;
            }
        }

        else{ // normal gameplay
            roundHandler();
            drawPlayer();
            drawGoons();
            drawBullets();
            drawColls();
            drawBossBar();
            drawBoss();
            if(boss.alive == 1) {
                drawBossBullets();
            }
        }
        frameTimeStop = SysTick->VAL;
        frameTime = frameTimeStart - frameTimeStop;
    }
}

void SysTick_Handler(void) { // game engine
    itemSpawn++; // multiplayer
    gameTick++;
    if(gameTick > 500) {
        gameTick = 0;
    }
    if((roundNum == 10) || (roundNum == 11)) { // multiplayer
        serviceUART2();
        updateBossBullets();
        bossBulletCollision();
        getButtons();
        wallCollision();
        collCollision();
        updateBullets();
        bulletCollision();
        JoyADCin();
        if(roundNum == 11) { // capture the flag
            flagCollision();
            updateFlags();
            flagWin(); // check if either player has a flag when in specific area
        }
        multiGunReload++;
        if(otherPlayerData.data & 1) {
            otherPlayerShoot();
        }
    }
    else { // singleplayer
        goonCollision();
        bulletCollision();
        collCollision();
        updateBullets();
        getButtons();
        JoyADCin();
        updateGoons();
        if(boss.alive == 1) {
            updateBoss();
            updateBossBullets();
            bossShoot();
            bossBulletCollision();
        }
    }
}
void TIMG7_IRQHandler() { // multiplayer UART output
    playerData.data |= playerData.playerDir << 1;
    UART1_OutChar(playerData.data | (1<<6)); // bit 0 player shoot
    UART1_OutChar(playerData.y); // y
    UART1_OutChar( playerData.x); // x
    GPIOA->DOUT31_0 ^= (1 << 24); // green
}

void TIMG12_IRQHandler(){ // sound
    DAC_Out(currSound[soundIndex][soundCount]);
    soundCount++;
    if(soundCount > soundLength[soundIndex]){
        TIMG12->COUNTERREGS.LOAD = 0;
        soundCount = 0;
    }
}

