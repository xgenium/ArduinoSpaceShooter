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
#define BULLET_POOL_SIZE 20
#define BULLET_BURST 5
#define BURST_COOLDOWN 500

#define SPACESHIP_BMP_HEIGHT 16
#define SPACESHIP_BMP_WIDTH 16

#define GAME_OVER_PRINT_DELAY 400
#define CHAR_WIDTH 12
#define CHAR_HEIGHT 18
#define INITIAL_CHAR_X (SCREEN_WIDTH/2 - CHAR_WIDTH*2)
#define INITIAL_CHAR_Y 0
#define NEXT_LINE_Y (INITIAL_CHAR_Y + CHAR_HEIGHT)

#define DEFAULT_DEATH_ANIMATION_DURATION 800
#define DEFAULT_FLICKER_DELAY 100

#define DEFAULT_HEALTH 3

#define INITIAL_X ((SCREEN_WIDTH / 4) * 3 - SPACESHIP_BMP_WIDTH)
#define INITIAL_Y ((SCREEN_HEIGHT / 4) * 3 - SPACESHIP_BMP_HEIGHT)

#define DEADZONE_X 15
#define DEADZONE_Y 15

#define SHOOTING_COOLDOWN 100

#define UNPRESSED_BUTTON HIGH
#define PRESSED_BUTTON LOW

enum ShipType { friendly, enemy };

enum ShipBitmapType { normal, xored };

static const unsigned char spaceShip_bmp[] PROGMEM = {0x00, 0x00, 0x00, 0x70, 0x01, 0x90, 0x02, 0x10, 0x1F, 0xF3, 0x3F, 0xFB, 0x60, 0x0F, 0x9F, 0xFF, 0x60,
 0x0F, 0x3F, 0xFB, 0x1F, 0xF3, 0x02, 0x10, 0x01, 0x90, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00};

static const unsigned char xoredShip_bmp[] PROGMEM = {0x1, 0x1, 0x1, 0x71, 0x0, 0x91, 0x3, 0x11, 0x1E, 0xF2, 0x3E, 0xFA, 0x61, 0xE, 0x9E, 0xFE, 0x61, 0xE, 0x3E, 0xFA,
  0x1E, 0xF2, 0x3, 0x11, 0x0, 0x91, 0x1, 0x71, 0x1, 0x1, 0x1, 0x1};

// Read as horizontal; flip ver; draw horizontal
static const unsigned char mirrored_xoredShip_bmp[] PROGMEM = {0x10, 0x10, 0x10, 0x10, 0x10, 0x71, 0x00, 0x91, 0x30, 0x11, 0x1e, 0xf2, 0x3e, 0xfa, 0x61, 0xe0, 
0x9e, 0xfe, 0x61, 0xe0, 0x3e, 0xfa, 0x1e, 0xf2, 0x30, 0x11, 0x00, 0x91, 0x10, 0x71, 0x10, 0x10};

static const char gameOver_message[] PROGMEM = "GAME OVER";

class Game;
class Joystick;
class SpaceShip;
class Bullet;
class BulletPool;
class EnemyShipPool;

struct DeathAnimation {
    bool inverted;
    int count;
    int duration;
    int startTime;
    int flickerTime;
    int flicker_delay;
    bool isHappening;
};

struct GameOverAnimation {
    int8_t x, y;
    int readTime;
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
	int level;
        int xSpeed, ySpeed;
        int width, height;
        int health, ammo;
        int shotTime;
        int shotCount;
	int killCount;
        bool burstEnded;
        bool isActive;
        ShipBitmapType bmpType;
        ShipType type;
        DeathAnimation deathAnimation;
    public:
        SpaceShip() : SpaceShip(0, 0, 0, 0, DEFAULT_HEALTH, xored, enemy, false) {};
        SpaceShip(int posX, int posY, int bmpWidth, int bmpHeight, int health, ShipBitmapType bmpType, ShipType type, bool isActive)
            :   x(posX),
                y(posY),
                xSpeed(0),
                ySpeed(0),
		level(1),
                width(bmpWidth),
                height(bmpHeight),
                health(health),
                bmpType(bmpType),
                type(type),
                isActive(isActive),
                shotCount(0),
                shotTime(0),
                burstEnded(true) {
		    deathAnimation.isHappening = false;
                };
        void draw(Adafruit_SSD1306 *display);
        int getX();
        int getY();
        int getWidth();
        int getHeight();
        bool getIsActive();
        int getHealth();
	int getLevel();
	bool getDeathAnimationStatus();
	ShipType getShipType();
        void setPosition(int posX, int posY);
        void setBmpSettings(int bmpWidth, int bmpHeight, ShipBitmapType newBmpType);
        void setIsActive(bool isActive);
	void setHealth(int newHealth);
        void updateSpeed(int xJoystick, int yJoystick);
        void updatePosition();
	void setLevel(int lvl);
        void gameUpdate(BulletPool *bp, Joystick *jstick);
        void shoot(BulletPool *bp);
        int getCooldown();
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
	int lastShotEnemyTime;
        SpaceShip pool[3];
    public:
        EnemyShipPool() : poolSize(3), nextAvailableIndex(0), activeEnemyCount(0) {};
        void init();
        void createEnemy(int x, int y, int width, int height, ShipBitmapType bmpType);
        int gameUpdate(BulletPool *bp);
        void draw(Adafruit_SSD1306 *display);
	void shoot(BulletPool *bp);
	int getActiveEnemyCount();
	int getLastShotEnemyTime();
};

class Bullet
{
    public:
        int8_t x, y;
        int8_t xSpeed;
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
        void fireBullet(int x, int y, ShipType type);
        void gameUpdate();
        Bullet* getBulletByIndex(int i);
        void destroyBulletByIndex(int i);
        int getPoolSize();
        void draw(Adafruit_SSD1306 *display);
};

void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height);

#endif
