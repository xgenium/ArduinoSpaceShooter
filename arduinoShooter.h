#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ezButton.h>
#include <Arduino.h>

#ifndef arduinoShooter_h
#define arduinoShooter_h

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define UINT32_MAX 4294967295

#define HUD_WIDTH 80
#define HUD_HEIGHT 7

#define WAVE_RECT_WIDTH 35
#define WAVE_RECT_HEIGHT 10
#define WAVE_TEXT_DURATION 2000

#define JOYSTICK_REAL_Y_PIN A0
#define JOYSTICK_REAL_X_PIN A1
#define JOYSTICK_BUTTON_PIN 3

#define BULLET_WIDTH 3
#define BULLET_HEIGHT 2 // if ySpeed == 2, then change HEIGHT to 3
#define BULLET_SPREAD 4

#define BULLET_POOL_SIZE 30
#define BULLET_BURST 5
#define DIAGONAL_BURST 3
#define BURST_COOLDOWN 500
#define MAX_STRAIGHT_BULLETS 3

#define MAX_LEVEL 5
#define SCORE_FOR_NEXT_LEVEL 7

#define FIRST_LEVEL_HEALTH 4
#define ENEMY_FIRST_LEVEL_HEALTH 3
#define LEVEL_HEALTH_SPREAD 2
#define ENEMY_LEVEL_HEALTH_SPREAD 2

#define ENEMY_RESPAWN_COOLDOWN 1200
#define ENEMY_SHOOTING_SPREAD 800


#define ENEMY_LVL1_WIDTH 10
#define ENEMY_LVL1_HEIGHT 10
// change height with width if something doesnt look right
#define ENEMY_LVL2_WIDTH 12
#define ENEMY_LVL2_HEIGHT 8

#define ENEMY_LVL3_WIDTH 12
#define ENEMY_LVL3_HEIGHT 8

#define NORMAL_LVL1_WIDTH 10
#define NORMAL_LVL1_HEIGHT 10

#define GAME_OVER_PRINT_DELAY 400
#define CHAR_WIDTH 12
#define CHAR_HEIGHT 18
#define INITIAL_CHAR_X (SCREEN_WIDTH/2 - CHAR_WIDTH*2)
#define INITIAL_CHAR_Y 0
#define NEXT_LINE_Y (INITIAL_CHAR_Y + CHAR_HEIGHT)

#define DEFAULT_DEATH_ANIMATION_DURATION 800
#define DEFAULT_FLICKER_DELAY 100

#define INITIAL_X ((SCREEN_WIDTH / 4) * 3 - NORMAL_LVL1_WIDTH)
#define INITIAL_Y ((SCREEN_HEIGHT / 4) * 3 - NORMAL_LVL1_HEIGHT)

#define DEADZONE_X 20
#define DEADZONE_Y 20

#define SHOOTING_COOLDOWN 100
#define RANDOM_MOVE_COOLDOWN 2000

#define UNPRESSED_BUTTON HIGH
#define PRESSED_BUTTON LOW

#define BONUS_ICON_WIDTH 9
#define BONUS_ICON_HEIGHT 7
#define BONUS_XSPEED 1

#define B_ADDHEALTH_AMOUNT 3
#define B_DIAGONALBULLETS_DURATION 4000
#define B_MULTIPLEBULLETS_DURATION 4000

// BITMASKS
// use it instead of multiple booleans
// class Game (G_)
#define G_GAMEOVER (1<<0)
#define G_FIRSTLOOP (1<<1)
#define G_GAMESTARTED (1<<2)
#define G_SHOULDADDHEALTH (1<<3)
#define G_SHOULDDRAWWAVE (1<<4)
#define G_CANINCREASELVL (1<<5)
//class SpaceShip (S_)
#define S_BURSTENDED (1<<0)
#define S_ISACTIVE (1<<1)
#define S_ISMOVING (1<<2)
#define S_ISBONUSACTIVE (1<<3)

static const unsigned char normal_lvl1_bmp[] PROGMEM = {
0x00, 0x00, 0x07, 0x80, 0x08, 0x40, 0x16, 0x40, 0xE9, 0xC0, 0xE9, 0xC0, 0x16, 0x40, 0x08, 0x40, 0x07, 0x80, 0x00, 0x00
};

// enemy ships need to be mirrored
// image2cpp settings: read as horizontal, flip horizontally, output horizontal
static const unsigned char enemy_lvl1_bmp[] PROGMEM = {
0x70, 0x00, 0x8c, 0x00, 0xf3, 0x00, 0x8c, 0x80, 0xf3, 0x40, 0xf3, 0x40, 0x8c, 0x80, 0xf3, 0x00, 
0x8c, 0x00, 0x70, 0x00
};

static const unsigned char enemy_lvl2_bmp[] PROGMEM = {
    0xc0, 0x00, 0xb0, 0x00, 0x88, 0x00, 0x87, 0xf0, 0x87, 0xf0, 0x88, 0x00, 0xb0, 0x00, 0xc0, 0x00
};

static const unsigned char enemy_lvl3_bmp[] PROGMEM = {
    0x0C, 0x00, 0x3F, 0x80, 0x4C, 0x60, 0xBF, 0x90, 0xBF, 0x90, 0x4C, 0x60, 0x3F, 0x80, 0x0C, 0x00
};

static const unsigned char b_addHealth_bmp[] PROGMEM = {
    0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0xFF, 0x80, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00
};

static const unsigned char b_diagonalBullets_bmp[] PROGMEM = {
    0x48, 0x00, 0x24, 0x00, 0x92, 0x00, 0x49, 0x00, 0x24, 0x80, 0x12, 0x00, 0x09, 0x00
};

static const unsigned char b_doubleBullets_bmp[] PROGMEM = {
    0x00, 0x00, 0xFF, 0x80, 0xFF, 0x80, 0x00, 0x00, 0xFF, 0x80, 0xFF, 0x80, 0x00, 0x00
};

static const unsigned char b_tripleBullets_bmp[] PROGMEM = {
    0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00, 0xFF, 0x80, 0x00, 0x00
};

static const char gameOver_message[] PROGMEM = "GAME OVER";

static const bool godMode = false; // true for godmode; false otherwise

class Game;
class Joystick;
class SpaceShip;
class Bullet;
class BulletPool;
class EnemyShipPool;
class Bonus;

enum ShipType { friendly, enemy };

// REMEMBER TO ADD NEW TYPES WHEN YOU ADD SPRITES
enum ShipBitmapType { normal_lvl1, enemy_lvl1, enemy_lvl2, enemy_lvl3 };

enum BulletDirection { straight, diagonalUp, diagonalDown };

enum BonusType { b_none, b_addHealth, b_diagonalBullets, b_doubleBullets, b_tripleBullets };

struct BonusData {
    BonusType type;
    uint8_t weight;
};

const BonusData BONUS_TABLE[] PROGMEM = {
    { b_none, 50 },
    { b_addHealth, 30 },
    { b_diagonalBullets, 25 },
    { b_doubleBullets, 15 },
    { b_tripleBullets, 10 }
};

const uint8_t BONUS_COUNT = sizeof(BONUS_TABLE) / sizeof(BONUS_TABLE[0]);

struct BulletsToShoot {
    uint8_t straightBullets, diagonalBullets;
};

struct DeathAnimation {
    bool inverted;
    int duration;
    unsigned long startTime;
    unsigned long flickerTime;
    int flicker_delay;
    bool isHappening;
};

struct GameOverAnimation {
    int8_t x, y;
    unsigned long readTime;
    uint8_t strLen;
    uint8_t outputCharCount;
};

struct WaveData {
    uint8_t simultaneousEnemyCount;
    uint8_t minEnemyLevel, maxEnemyLevel;
    uint8_t enemyLevelDistribution; // 1 (only min levels) - 10 (only max levels)
    uint32_t maxScore;
};

const WaveData WAVE_TABLE[] PROGMEM = {
    { 2, 1, 1, 10, 5 },
    { 3, 1, 2, 7, 10 },
    { 3, 1, 3, 6, 15 },
    { 2, 2, 3, 5, 20 }
    // ... continue
};

const WaveData LastWave PROGMEM = {
    3, 1, 3, 5, UINT32_MAX
};

const uint8_t WAVE_COUNT = sizeof(WAVE_TABLE) / sizeof(WAVE_TABLE[0]);

// uint8_t *getBmpByType(uint8_t width, uint8_t height, ShipBitmapType bmpType);
void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height);
int getBonusDuration(BonusType bonus);
int getTotalBonusWeight();
BonusType generateRandomBonus();
uint8_t getWidthForShip(uint8_t level, ShipType type);
uint8_t getHeightForShip(uint8_t level, ShipType type);
ShipBitmapType getBmpTypeForShip(uint8_t level, ShipType type);
uint8_t getRandomLevel(uint8_t minLvl, uint8_t maxLvl, uint8_t lvlDistr);

uint8_t setToTrue(uint8_t var, uint8_t bitmask);
uint8_t setToFalse(uint8_t var, uint8_t bitmask);
bool getBoolVal(uint8_t var, uint8_t bitmask);

class Game
{
    private:
	Adafruit_SSD1306 *display;
	Joystick *jstick;
	SpaceShip *spaceShip;
	BulletPool *bulletPool;
	EnemyShipPool *enemyPool;
	Bonus *bonus;

	GameOverAnimation gameOverAnimation;
	unsigned long waveTextStartTime;
	uint32_t score;
	uint8_t waveIndex;
	uint8_t boolVar;
    public:
	Game(Adafruit_SSD1306 *d, Joystick *j, SpaceShip *ss, BulletPool *bp, EnemyShipPool *ep) :
	    display(d), jstick(j), spaceShip(ss), bulletPool(bp), enemyPool(ep),
	    score(0), waveIndex(0) {
		gameOverAnimation.x = INITIAL_CHAR_X;
		gameOverAnimation.y = INITIAL_CHAR_Y;
		gameOverAnimation.readTime = 0;
		gameOverAnimation.outputCharCount = 0;
		boolVar = setToTrue(boolVar, G_FIRSTLOOP);
	    };
	void init();
	bool startScreen();
	void update();
	void drawGameOver();
	void drawHUD();
	void drawWaveText();
	void drawScore();
	void draw();
	void handleBonus();
	void handleWaves();
	void loadWave(int index);
	void updateScore(uint32_t add);
	void setGameOver(bool isGameOver);
	void setGameStarted(bool isGameStarted);
	uint32_t getScore();
	bool isGameOver();
	bool isGameStarted();
};

class Joystick
{
    private:
        int pin_x, pin_y, btnPin;
    public:
        Joystick(int pinX, int pinY, int pinButton)
            :   pin_x(pinX),
                pin_y(pinY),
                btnPin(pinButton)
        {
	    pinMode(btnPin, INPUT_PULLUP);
        }
        int getX();
        int getY();
        int getButtonState();
};

class SpaceShip
{
    private:
        int16_t x, y;
	int16_t x1, y1;
	uint8_t level;
        int8_t xSpeed, ySpeed;
        uint8_t width, height;
        int8_t maxHealth, health;
        uint8_t straightShotCount;
	uint8_t diagonalShotCount;
        unsigned long shotTime;
	unsigned long lastRandomMoveTime;
	unsigned long bonusStartTime; // also used as end time
	uint32_t killCount;
	uint8_t boolVar;
        ShipBitmapType bmpType;
        ShipType type;
        DeathAnimation deathAnimation;
	BulletsToShoot bulletsToShoot;
	BonusType activeBonus;
    public:
        SpaceShip() : SpaceShip(0, 0, enemy, 1) {};
        SpaceShip(int16_t posX, int16_t posY, ShipType type, uint8_t lvl)
            :   x(posX),
                y(posY),
		x1(posX), y1(posY),
                xSpeed(0),
                ySpeed(0),
		level(lvl),
                type(type),
		maxHealth(FIRST_LEVEL_HEALTH),
		health(maxHealth),
		width(getWidthForShip(level, type)),
		height(getHeightForShip(level, type)),
		bmpType(getBmpTypeForShip(level, type)),
                straightShotCount(0),
                diagonalShotCount(0),
                shotTime(millis()),
		lastRandomMoveTime(millis()),
		bonusStartTime(millis()),
		activeBonus(b_none) {
		    updateMaxHealth();

		    boolVar = setToTrue(boolVar, S_BURSTENDED);

		    deathAnimation.isHappening = false;
		    bulletsToShoot.straightBullets = 1;
		    bulletsToShoot.diagonalBullets = 0;
                };
        void draw(Adafruit_SSD1306 *display);
	void reset();
        int16_t getX();
        int16_t getY();
        int16_t getX1();
        int16_t getY1();
        uint8_t getWidth();
        uint8_t getHeight();
        bool getIsActive();
	bool getIsMoving();
	bool getIsBonusActive();
        int8_t getHealth();
	int8_t getMaxHealth();
	void updateMaxHealth();
	uint8_t getLevel();
	unsigned long getLastRandomMove();
	unsigned long getLastShotTime();
	unsigned long getBonusStartTime();
	uint8_t getStraightShotCount();
	uint8_t getDiagonalShotCount();
	bool getDeathAnimationStatus();
	ShipType getShipType();
	void randomMove(int16_t maxDistanceX, int16_t maxDistanceY);
        void setPosition(int16_t posX, int16_t posY);
	void setTargetPosition(int16_t posX1, int16_t posY1);
        void setBmpSettings(uint8_t bmpWidth, uint8_t bmpHeight, ShipBitmapType newBmpType);
        void setIsActive(bool isActive);
	void setIsMoving(bool moving);
	void setHealth(int8_t newHealth);
	void setMaxHealth(int8_t newMaxHealth);
	void setLastShotTime(unsigned long time);
	void addHealth(int8_t toAdd);
	void setBulletsToShoot(uint8_t straightBullets, uint8_t diagonalBullets);
	void resetHealth();
        void updateSpeed(int xJoystick, int yJoystick);
        void updatePosition();
	void setLevel(uint8_t lvl);
	void increaseLevel();
	void setSpeed(int8_t speedX, int8_t speedY);
        void gameUpdate(BulletPool *bp, Joystick *jstick);
        void shoot(BulletPool *bp);
        void shootStraight(BulletPool *bp, int straightBullets);
        void shootDiagonally(BulletPool *bp, int diagonalBullets);
        unsigned long getCooldown();
	bool isPointInside(int16_t px, int16_t py);
	bool isObjectInside(int16_t px0, int16_t py0, int16_t px1, int16_t py1);
        bool isHitByBullet(Bullet *b, ShipType bulletType);
        void startDeathAnimation(int duration, int flicker_delay);
	void handleBonus(BonusType bonus);
	void activateBonus(BonusType bonus);
	void resetBonus(BonusType bonus);
	void setRandomLastShotTime(unsigned int spread);
};


class EnemyShipPool
{
    private:
        int8_t poolSize;
        uint8_t nextAvailableIndex;
	int8_t activeEnemyCount;
	unsigned long lastShotEnemyTime;
        SpaceShip pool[3];
    public:
        EnemyShipPool() : poolSize(3), nextAvailableIndex(0), activeEnemyCount(0) {};
        void init();
	void createEnemy(int16_t x, int16_t y, uint8_t level);
        int gameUpdate(BulletPool *bp);
	void updateMaxHealth();
	void updatePosition();
	void randomMove();
        void draw(Adafruit_SSD1306 *display);
	void shoot(BulletPool *bp);
	int8_t getActiveEnemyCount();
	unsigned long getLastShotEnemyTime();
};

class Bullet
{
    public:
        int16_t x, y;
        int8_t xSpeed, ySpeed;
        int8_t damage;
        ShipType type;
        bool isActive;

        void draw(Adafruit_SSD1306 *display);
        void updatePosition();

};

class BulletPool
{
    private:
        uint8_t poolSize;
        uint8_t nextAvailableIndex;
        Bullet pool[BULLET_POOL_SIZE];
    public:
        BulletPool() : poolSize(BULLET_POOL_SIZE), nextAvailableIndex(0) {};
        void init();
        void fireBullet(int16_t x, int16_t y, ShipType type, BulletDirection direction);
        void gameUpdate();
        Bullet* getBulletByIndex(uint8_t i);
        void destroyBulletByIndex(uint8_t i);
        uint8_t getPoolSize();
        void draw(Adafruit_SSD1306 *display);
};

class Bonus
{
    private:
	int16_t x, y;
	unsigned long lastBonusTime, cooldown;
	bool isActive;
	BonusType type;
    public:
	void set(int16_t posX, int16_t posY, BonusType bonusType);
	void deactivate();
	void startCooldown();
	bool cooldownExpired();
	void resetTimer();
	bool shouldCreateBonus();
	void updatePosition();
	bool getIsActive();
	int16_t getX();
	int16_t getY();
	BonusType getType();
	void draw(Adafruit_SSD1306 *display);
};


#endif
