void gameOver(uint8_t win, uint8_t player);

void initPlayerShoot(void);

void updateBullets(void);

void updateColls(void);

void updateGoons(void);

void spawnColl(uint8_t x, uint8_t y, uint8_t type);

void spawnGoon(uint8_t health, uint8_t type);

void goonCollision(void);

void bulletCollision(void);

void collCollision(void);

void playerShoot(bool shotgun);

void openShop(void);

void spawnBoss(void);

void updateBossBullets(void);

void bossShoot(void);

void drawBossBar(void);

void updateBoss(void);

void bossBulletCollision(void);

void victory(uint8_t player);

void otherPlayerShoot(void);

void wallCollision(void);

void flagCollision(void);

void updateFlags(void);

void flagWin(void);

extern const uint8_t *currSound[];
extern uint8_t soundIndex;
extern const uint16_t soundLength[];
void makeSound(uint8_t state);
