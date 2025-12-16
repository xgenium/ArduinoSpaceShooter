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

#define JOYSTICK_REAL_Y_PIN A0
#define JOYSTICK_REAL_X_PIN A1
#define JOYSTICK_BUTTON_PIN 3

#define BULLET_WIDTH 3
#define BULLET_HEIGHT 2 // if ySpeed == 2, then change HEIGHT to 3

#define BULLET_POOL_SIZE 20
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

enum ShipType { friendly, enemy };

enum ShipBitmapType { normal_lvl1, enemy_lvl1 };

enum BulletDirection { straight, diagonalUp, diagonalDown };

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
        unsigned long shotTime;
        uint8_t straightShotCount;
	uint8_t diagonalShotCount;
	uint32_t killCount;
	unsigned long lastRandomMove;
        bool burstEnded;
        bool isActive;
	bool isMoving;
        ShipBitmapType bmpType;
        ShipType type;
        DeathAnimation deathAnimation;
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
                straightShotCount(0),
                diagonalShotCount(0),
                shotTime(millis()),
		lastRandomMove(millis()),
		isMoving(false),
                burstEnded(true) {
		    deathAnimation.isHappening = false;
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
        int8_t getHealth();
	uint8_t getLevel();
	unsigned long getLastRandomMove();
	unsigned long getLastShotTime();
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
	void resetHealth();
        void updateSpeed(int xJoystick, int yJoystick);
        void updatePosition();
	void setLevel(uint8_t lvl);
	void setSpeed(int8_t speedX, int8_t speedY);
        void gameUpdate(BulletPool *bp, Joystick *jstick);
        void shoot(BulletPool *bp);
        unsigned long getCooldown();
	bool isPointInside(int16_t px, int16_t py);
        bool isHitByBullet(Bullet *b, ShipType bulletType);
        void startDeathAnimation(int duration, int flicker_delay);
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

void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height);

#endif
