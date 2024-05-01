#ifndef STRUCTURES_H
#define STRUCTURES_H

// player data
struct playerData { // player
    uint8_t x;
    uint8_t y;
    uint8_t playerDir;
    uint8_t gun; // gun type
    uint8_t fireSpeed; // gun reload time (lower = faster)
    uint8_t speed; // movement speed
    uint8_t data;
    uint8_t lastx;
    uint8_t lasty;
    uint8_t item;
    bool barrierColl;
    bool erase;
};
typedef struct playerData playerData_t;

struct otherPlayerData { // player
    uint8_t x;
    uint8_t y;
    uint8_t data;

    // 00000000              00000000                 00000000          00000000
    // ^^                        ^^^                         ^            ^xxxxx
    //valid flags?                 direction               shot?            game over (this flag will be raised when other player wins)

    uint8_t lastx;
    uint8_t lasty;
    uint8_t dir;
    uint8_t lastFireState;
    bool erase;
};
typedef struct otherPlayerData otherPlayerData_t;

struct bulletData { // bullets
    uint8_t x;
    uint8_t y;
    uint8_t data;
    // data packed as following:
    //
    // 00000000   00000000               00000000
    // ^^              ^^^                   ^
    // type        direction (0-7)         status
    // type and direction change how bullet behaves
};
typedef struct bulletData bulletData_t;

struct goonData { // enemies
    uint8_t x;
    uint8_t y;
    uint8_t lastCounter; // used to delay death animation
    int8_t step; // determine which step animation to play (negative = left, positive = right)
    uint8_t type;
    uint8_t health;
    uint8_t alive; // determines if need to be playing alive or death animation
    // alive data packed as following:
    //
    // 00000000   00000000
    //     ^^^           ^
    //   animation      alive (0 dead)
};
typedef struct goonData goonData_t;

struct bossData {
    uint8_t x;
    uint8_t y;
    uint8_t health;
    uint8_t alive;
    uint8_t fireDir;
    uint8_t needsBar;
};
typedef struct bossData bossData_t;

struct collectibleData { // money!, powerup
    uint8_t x;
    uint8_t y;
    uint8_t active; // despawn timer
    uint8_t data;
    // data packed as following:
    //
    // 00000000                 00000000             00000000
    // ^^                              ^                 ^^^
    // animation frame      // is active?       // item type
    // 0 - coin,
};
typedef struct collectibleData collData_t;

struct bossBulletData {
    uint8_t x;
    uint8_t y;
    uint8_t dir;
    uint8_t alive; // 0 or 1
    uint8_t kill; // mark for kill
};
typedef struct bossBulletData bossBulletData_t;


struct solid { // for impassable objects on the map
    uint8_t x;
    uint8_t y;
    uint8_t active;
};
typedef struct solid solid_t;

struct flag {
    uint8_t x;
    uint8_t y;
    uint8_t isCaptured;
};
typedef struct flag flag_t;


#endif // STRUCTURES_H
