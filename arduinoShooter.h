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

#define HUD_WIDTH 90
#define HUD_HEIGHT 7

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

#define ENEMY_LVL1_WIDTH 10
#define ENEMY_LVL1_HEIGHT 10

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

#define DEFAULT_HEALTH 3

#define INITIAL_X ((SCREEN_WIDTH / 4) * 3 - NORMAL_LVL1_WIDTH)
#define INITIAL_Y ((SCREEN_HEIGHT / 4) * 3 - NORMAL_LVL1_HEIGHT)

#define DEADZONE_X 20
#define DEADZONE_Y 20

#define SHOOTING_COOLDOWN 100
#define RANDOM_MOVE_COOLDOWN 2000

#define UNPRESSED_BUTTON HIGH
#define PRESSED_BUTTON LOW

#define BONUS_ICON_SIZE 5
#define BONUS_XSPEED 1

#define B_DIAGONALBULLETS_DURATION 4000
#define B_MULTIPLEBULLETS_DURATION 4000

static const unsigned char normal_lvl1_bmp[] PROGMEM = {
0x00, 0x00, 0x07, 0x80, 0x08, 0x40, 0x16, 0x40, 0xE9, 0xC0, 0xE9, 0xC0, 0x16, 0x40, 0x08, 0x40, 0x07, 0x80, 0x00, 0x00
};

// enemy ships need to be mirrored
// image2cpp settings: read as horizontal, flip horizontally, output horizontal
static const unsigned char enemy_lvl1_bmp[] PROGMEM = {
0x70, 0x00, 0x8c, 0x00, 0xf3, 0x00, 0x8c, 0x80, 0xf3, 0x40, 0xf3, 0x40, 0x8c, 0x80, 0xf3, 0x00, 
0x8c, 0x00, 0x70, 0x00
};

static const char gameOver_message[] PROGMEM = "GAME OVER";

class Game;
class Joystick;
class SpaceShip;
class Bullet;
class BulletPool;
class EnemyShipPool;
class Bonus;

enum ShipType { friendly, enemy };

enum ShipBitmapType { normal_lvl1, enemy_lvl1 };

enum BulletDirection { straight, diagonalUp, diagonalDown };

enum BonusType { b_none, b_resetHealth, b_diagonalBullets, b_doubleBullets, b_tripleBullets };

struct BonusData {
    BonusType type;
    uint8_t weight;
};

const BonusData BONUS_TABLE[] PROGMEM = {
    { b_none, 50 },
    { b_resetHealth, 30 },
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

struct Wave {
    uint8_t enemyCount;
    uint8_t minEnemyLevel, maxEnemyLevel;
    uint8_t enemyLevelDistribution; // 1 (only min levels) - 10 (only max levels)
    int maxScore;
};

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
	uint32_t score;
	bool gameOver;
	bool firstLoop;
	bool gameStarted;
    public:
	Game(Adafruit_SSD1306 *d, Joystick *j, SpaceShip *ss, BulletPool *bp, EnemyShipPool *ep) :
	    display(d), jstick(j), spaceShip(ss), bulletPool(bp), enemyPool(ep),
	    score(0), firstLoop(true), gameOver(false), gameStarted(false) {
		gameOverAnimation.x = INITIAL_CHAR_X;
		gameOverAnimation.y = INITIAL_CHAR_Y;
		gameOverAnimation.readTime = 0;
		gameOverAnimation.outputCharCount = 0;
	    };
	void init();
	bool startScreen();
	void update();
	void drawGameOver();
	void drawHUD();
	void drawScore();
	void draw();
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
        ezButton button;
    public:
        Joystick(int pinX, int pinY, int pinButton)
            :   pin_x(pinX),
                pin_y(pinY),
                btnPin(pinButton),
                button(btnPin, INPUT_PULLUP)
        {
            button.setDebounceTime(20);
        }
        int getX();
        int getY();
        int getButtonState();
        void loop();
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
        bool burstEnded;
        bool isActive;
	bool isMoving;
	bool isBonusActive;
        ShipBitmapType bmpType;
        ShipType type;
        DeathAnimation deathAnimation;
	BulletsToShoot bulletsToShoot;
	BonusType activeBonus;
    public:
        SpaceShip() : SpaceShip(0, 0, 0, 0, DEFAULT_HEALTH, enemy_lvl1, enemy, false) {};
        SpaceShip(int16_t posX, int16_t posY, uint8_t bmpWidth, uint8_t bmpHeight, int8_t health, ShipBitmapType bmpType, ShipType type, bool isActive)
            :   x(posX),
                y(posY),
		x1(posX), y1(posY),
                xSpeed(0),
                ySpeed(0),
		level(1),
                width(bmpWidth),
                height(bmpHeight),
                maxHealth(health),
                health(health),
                bmpType(bmpType),
                type(type),
                isActive(isActive),
		isBonusActive(false),
                straightShotCount(0),
                diagonalShotCount(0),
                shotTime(millis()),
		lastRandomMoveTime(millis()),
		bonusStartTime(millis()),
		isMoving(false),
		activeBonus(b_none),
                burstEnded(true) {
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
	void setBulletsToShoot(uint8_t straightBullets, uint8_t diagonalBullets);
	void resetHealth();
        void updateSpeed(int xJoystick, int yJoystick);
        void updatePosition();
	void setLevel(uint8_t lvl);
	void setSpeed(int8_t speedX, int8_t speedY);
        void gameUpdate(BulletPool *bp, Joystick *jstick);
        void shoot(BulletPool *bp);
        void shootStraight(BulletPool *bp, int straightBullets);
        void shootDiagonally(BulletPool *bp, int diagonalBullets);
        unsigned long getCooldown();
	bool isPointInside(int16_t px, int16_t py);
        bool isHitByBullet(Bullet *b, ShipType bulletType);
        void startDeathAnimation(int duration, int flicker_delay);
	void handleBonus(BonusType bonus);
	void activateBonus(BonusType bonus);
	void resetBonus(BonusType bonus);
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
        void createEnemy(int16_t x, int16_t y, uint8_t width, uint8_t height, ShipBitmapType bmpType);
        int gameUpdate(BulletPool *bp);
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

void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height);
int getBonusDuration(BonusType bonus);
int getTotalBonusWeight();
BonusType generateRandomBonus();

#endif
