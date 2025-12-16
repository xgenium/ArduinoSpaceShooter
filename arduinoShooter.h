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
    byte strLen;
    byte outputCharCount;
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
	int score;
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
	void updateScore(int add);
	void setGameOver(bool isGameOver);
	void setGameStarted(bool isGameStarted);
	int getScore();
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
        int x, y;
	int x1, y1;
	int level;
        int xSpeed, ySpeed;
        int width, height;
        int health, ammo;
        unsigned long shotTime;
        int straightShotCount;
	int diagonalShotCount;
	int killCount;
	unsigned long lastRandomMove;
        bool burstEnded;
        bool isActive;
	bool isMoving;
        ShipBitmapType bmpType;
        ShipType type;
        DeathAnimation deathAnimation;
    public:
        SpaceShip() : SpaceShip(0, 0, 0, 0, DEFAULT_HEALTH, enemy_lvl1, enemy, false) {};
        SpaceShip(int posX, int posY, int bmpWidth, int bmpHeight, int health, ShipBitmapType bmpType, ShipType type, bool isActive)
            :   x(posX),
                y(posY),
		x1(posX), y1(posY),
                xSpeed(0),
                ySpeed(0),
		level(1),
                width(bmpWidth),
                height(bmpHeight),
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
        int getX();
        int getY();
        int getX1();
        int getY1();
        int getWidth();
        int getHeight();
        bool getIsActive();
	bool getIsMoving();
        int getHealth();
	int getLevel();
	unsigned long getLastRandomMove();
	unsigned long getLastShotTime();
	int getStraightShotCount();
	int getDiagonalShotCount();
	bool getDeathAnimationStatus();
	ShipType getShipType();
	void randomMove(int maxDistanceX, int maxDistanceY);
        void setPosition(int posX, int posY);
	void setTargetPosition(int posX1, int posY1);
        void setBmpSettings(int bmpWidth, int bmpHeight, ShipBitmapType newBmpType);
        void setIsActive(bool isActive);
	void setIsMoving(bool moving);
	void setHealth(int newHealth);
        void updateSpeed(int xJoystick, int yJoystick);
        void updatePosition();
	void setLevel(int lvl);
	void setSpeed(int speedX, int speedY);
        void gameUpdate(BulletPool *bp, Joystick *jstick);
        void shoot(BulletPool *bp);
        unsigned long getCooldown();
	bool isPointInside(int px, int py);
        bool isHitByBullet(Bullet *b, ShipType bulletType);
        void startDeathAnimation(int duration, int flicker_delay);
};


class EnemyShipPool
{
    private:
        int poolSize;
        int nextAvailableIndex;
	int activeEnemyCount;
	unsigned long lastShotEnemyTime;
        SpaceShip pool[3];
    public:
        EnemyShipPool() : poolSize(3), nextAvailableIndex(0), activeEnemyCount(0) {};
        void init();
        void createEnemy(int x, int y, int width, int height, ShipBitmapType bmpType);
        int gameUpdate(BulletPool *bp);
	void updatePosition();
	void randomMove();
        void draw(Adafruit_SSD1306 *display);
	void shoot(BulletPool *bp);
	int getActiveEnemyCount();
	unsigned long getLastShotEnemyTime();
};

class Bullet
{
    public:
        int8_t x, y;
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
        int poolSize;
        int nextAvailableIndex;
        Bullet pool[BULLET_POOL_SIZE];
    public:
        BulletPool() : poolSize(BULLET_POOL_SIZE), nextAvailableIndex(0) {};
        void init();
        void fireBullet(int x, int y, ShipType type, BulletDirection direction);
        void gameUpdate();
        Bullet* getBulletByIndex(int i);
        void destroyBulletByIndex(int i);
        int getPoolSize();
        void draw(Adafruit_SSD1306 *display);
};

void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height);

#endif
