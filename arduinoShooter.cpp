#include "Adafruit_SSD1306.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include <avr/pgmspace.h>
#include "arduinoShooter.h"

void Game::init()
{
    bulletPool->init();
    enemyPool->init();
}

bool Game::startScreen()
{
    display->clearDisplay();
    display->setCursor(0, 0);
    display->setTextColor(WHITE);
    display->setTextSize(2);
    display->println(F("ARDUINO"));
    display->println(F("SHOOTER"));
    display->display();
    for (;;) {
	jstick->loop();
	if (jstick->getButtonState() == PRESSED_BUTTON) {
	    return true;
	}
    }
    return false;
}

void Game::update()
{
    jstick->loop();

    int add = enemyPool->gameUpdate(bulletPool);
    if (add > 0) updateScore(add);
    enemyPool->shoot(bulletPool);

    if (!firstLoop && jstick->getButtonState() == PRESSED_BUTTON) {
        spaceShip->shoot(bulletPool);
    }
    firstLoop = false;
    if (enemyPool->getActiveEnemyCount() == 0 && millis() - enemyPool->getLastShotEnemyTime() >= 1200) {
	enemyPool->createEnemy(random(1, 10), random(1, 10), ENEMY_LVL1_WIDTH, ENEMY_LVL1_HEIGHT, enemy_lvl1);
    }

    if (score < 5) {
	spaceShip->setLevel(1);
    } else if (score < 10) {
	spaceShip->setLevel(2);
    } else if (score < 15){
	spaceShip->setLevel(3);
    } else {
	spaceShip->setLevel(5);
    }

    bulletPool->gameUpdate();
    spaceShip->gameUpdate(bulletPool, jstick);
    if (spaceShip->getHealth() <= 0 && !spaceShip->getDeathAnimationStatus()) {
	delay(1000);
	gameOver = true;
    }
}

void Game::drawGameOver()
{
    GameOverAnimation *g = &gameOverAnimation;
    display->setCursor(g->x, g->y);
    display->setTextColor(WHITE);
    display->setTextSize(2);
    g->strLen = strlen_P(gameOver_message);
    char currChar;
    byte k = 0;
    if (g->outputCharCount < g->strLen) {
	unsigned long currTime = millis();
	if (currTime - g->readTime >= GAME_OVER_PRINT_DELAY) {
	    byte k = g->outputCharCount;
	    currChar = pgm_read_byte_near(gameOver_message + k);
	    g->readTime = currTime;
	    display->print(currChar);

	    if (g->x >= INITIAL_CHAR_X + CHAR_WIDTH*4) {
		g->x = INITIAL_CHAR_X;
		g->y = NEXT_LINE_Y;
	    } else {
		g->x += CHAR_WIDTH;
	    }

	    g->outputCharCount++;
	}
    }
}

void Game::drawHUD()
{
    //display->drawRect(0, 0, 50, 10, WHITE);
    display->fillRect(0, 0, 85, 7, WHITE);
    display->setCursor(1, 0);
    display->setTextColor(BLACK);
    display->setTextSize(1);
    display->print("LVL:");
    display->print(spaceShip->getLevel());
    display->print(" S:");
    display->print(score);
    display->print(" H:");
    display->print(spaceShip->getHealth());
}

void Game::drawScore()
{
    display->setCursor(0,0);
    display->setTextColor(WHITE);
    display->setTextSize(1);
    display->println(score);
}

void Game::draw()
{
    spaceShip->draw(display);
    bulletPool->draw(display);
    enemyPool->draw(display);
    //drawScore();
}

void Game::updateScore(int add)
{
    score += add;
}

void Game::setGameOver(bool isGameOver)
{
    gameOver = isGameOver;
}

void Game::setGameStarted(bool isGameStarted)
{
    gameStarted = isGameStarted;
}

int Game::getScore()
{
    return score;
}

bool Game::isGameOver()
{
    return gameOver;
}

bool Game::isGameStarted()
{
    return gameStarted;
}

int Joystick::getX()
{
    return analogRead(pin_x);
}

int Joystick::getY()
{
    return analogRead(pin_y);
}

int Joystick::getButtonState()
{
    return button.getState();
}

void Joystick::loop()
{
    button.loop();
}

void SpaceShip::draw(Adafruit_SSD1306 *display)
{
    const uint8_t *bmp;
    switch (bmpType) {
        case normal_lvl1: bmp = normal_lvl1_bmp; break;
        case enemy_lvl1: bmp = enemy_lvl1_bmp; break;
    }
    if (isActive && !deathAnimation.isHappening) {
        display->drawBitmap(x, y, bmp, width, height, 1);
    }
    if (deathAnimation.isHappening) {
        DeathAnimation *da = &deathAnimation;
        unsigned long currTime = millis();
        if (currTime - da->startTime <= da->duration) {
            if (currTime - da->flickerTime >= da->flicker_delay) {
                da->flickerTime = currTime;
		da->inverted = !da->inverted;
	    }
	    if (!da->inverted) {
		display->drawBitmap(x, y, bmp, width, height, 1);
	    } else if (da->inverted) {
		drawInvertedBitmap(display, x, y, bmp, width, height);
	    }
	} else {
	    da->isHappening = false;
	}

	return;
    }
}

void SpaceShip::reset()
{
    unsigned long currTime = millis();
    x = x1 = y = y1 = 0;
    xSpeed = ySpeed = 0;
    isActive = false;
    straightShotCount = 0;
    diagonalShotCount = 0;
    shotTime = currTime;
    lastRandomMove = currTime;
    burstEnded = true;
    deathAnimation.isHappening = false;
}

int SpaceShip::getX()
{
    return x;
}

int SpaceShip::getY()
{
    return y;
}

int SpaceShip::getX1()
{
    return x1;
}

int SpaceShip::getY1()
{
    return y1;
}

int SpaceShip::getWidth()
{
    return width;
}

int SpaceShip::getHeight()
{
    return height;
}

bool SpaceShip::getIsActive()
{
    return isActive;
}

bool SpaceShip::getIsMoving()
{
    return isMoving;
}

int SpaceShip::getHealth()
{
    return health;
}

int SpaceShip::getLevel()
{
    return level;
}

unsigned long SpaceShip::getLastRandomMove()
{
    return lastRandomMove;
}

unsigned long SpaceShip::getLastShotTime()
{
    return shotTime;
}

int SpaceShip::getStraightShotCount()
{
    return straightShotCount;
}

int SpaceShip::getDiagonalShotCount()
{
    return diagonalShotCount;
}

bool SpaceShip::getDeathAnimationStatus()
{
    return deathAnimation.isHappening;
}

ShipType SpaceShip::getShipType()
{
    return type;
}

void SpaceShip::randomMove(int maxDistanceX, int maxDistanceY)
{
    xSpeed = random(1, 3);
    ySpeed = random(1, 3);
    x1 = (int)random(1, maxDistanceX+1);
    y1 = (int)random(1, maxDistanceY+1);
    lastRandomMove = millis();
    isMoving = true;
}

void SpaceShip::setPosition(int posX, int posY)
{
    x = x1 = posX;
    y = y1 = posY;
}

void SpaceShip::setTargetPosition(int posX1, int posY1)
{
    x1 = posX1;
    y1 = posY1;
}

void SpaceShip::setBmpSettings(int bmpWidth, int bmpHeight, ShipBitmapType newBmpType)
{
    width = bmpWidth;
    height = bmpHeight;
    bmpType = newBmpType;
}

void SpaceShip::setIsActive(bool active)
{
    isActive = active;
}

void SpaceShip::setIsMoving(bool moving)
{
    isMoving = moving;
}

void SpaceShip::setHealth(int newHealth)
{
    health = newHealth;
}

void SpaceShip::updateSpeed(int xJoystick, int yJoystick)
{

    if (xJoystick > 500 + DEADZONE_X)
        ySpeed = 1;
    else if (xJoystick < 500 - DEADZONE_X)
        ySpeed = -1;
    else
        ySpeed = 0;

    if (yJoystick > 500 + DEADZONE_Y)
        xSpeed = 2;
    else if (yJoystick < 500 - DEADZONE_Y)
        xSpeed = -2;
    else
        xSpeed = 0;

}

void SpaceShip::updatePosition()
{
    if (type == enemy) {
	if (x < x1) {
	    x += xSpeed;
	    if (x > x1) x = x1;
	} else if (x > x1) {
	    x -= xSpeed;
	    if (x < x1) x = x1;
	} else {
	    xSpeed = 0;
	}

	if (y < y1) {
	    y += ySpeed;
	    if (y > y1) y = y1;
	} else if (y > y1) {
	    y -= ySpeed;
	    if (y < y1) y = y1;
	} else {
	    ySpeed = 0;
	}

	if (xSpeed == 0 && ySpeed == 0)
	    isMoving = false;
    } else {
	x += xSpeed;
	y += ySpeed;
    }

    if (x < 0) x = 0;
    if (x > SCREEN_WIDTH - width) x = SCREEN_WIDTH - width;
    if (y < 0) y = 0;
    if (y > SCREEN_HEIGHT - height) y = SCREEN_HEIGHT - height;
}

void SpaceShip::setLevel(int lvl)
{
    level = lvl;
}

void SpaceShip::setSpeed(int speedX, int speedY)
{
    xSpeed = speedX;
    ySpeed = speedY;
}

void SpaceShip::gameUpdate(BulletPool *bp, Joystick *jstick)
{
    if (jstick != NULL) updateSpeed(jstick->getX(), jstick->getY());

    updatePosition();

    if (type == enemy) return;

    int bpSize = bp->getPoolSize();
    for (int i = 0; i < bpSize; i++) {
        Bullet *b = bp->getBulletByIndex(i);
        if (b->isActive) {
	    if (isActive && isHitByBullet(b, enemy)) {
                bp->destroyBulletByIndex(i);
		startDeathAnimation(250, 50);
		if (health <= 0) {
		    isActive = false;
		    startDeathAnimation(DEFAULT_DEATH_ANIMATION_DURATION, DEFAULT_FLICKER_DELAY);
		}
	    }
        }
    }
}

void SpaceShip::shoot(BulletPool *bp)
{
    unsigned long currTime = millis();
    if (!isActive) return;

    switch (type) {
	case friendly:
	    if (currTime - shotTime >= BURST_COOLDOWN) {
		burstEnded = true;
		straightShotCount = 0;
		diagonalShotCount = 0;
	    }
	    break;
	case enemy:
	    if (currTime - shotTime >= random(800, 1200)) {
		burstEnded = true;
		straightShotCount = 0;
		diagonalShotCount = 0;
	    }
	    break;
    }

    // burstEnded means "Ready to start/continue burst"
    if (burstEnded && (currTime - shotTime >= SHOOTING_COOLDOWN)) {
	int bulletsToFire = level;
	int straightBullets;
	int diagonalBullets = bulletsToFire - MAX_STRAIGHT_BULLETS;

	if (bulletsToFire > MAX_STRAIGHT_BULLETS)
	    straightBullets = MAX_STRAIGHT_BULLETS;
	else
	    straightBullets = bulletsToFire;

	if (diagonalBullets < 0 || diagonalShotCount >= DIAGONAL_BURST
		||  diagonalBullets % 2 != 0) {
	    diagonalBullets = 0;
	}

	const int spread = 4;
	int centerY = y + height/2;
	BulletDirection direction;
	int i;

	for (i = 0; i < straightBullets; i++) {
	    direction = straight;
	    int bulletY;
	    if (straightBullets == 1) {
		bulletY = centerY;
	    } else {
		bulletY = centerY - spread / 2 + (spread * i) / (straightBullets - 1);
	    }
	    bp->fireBullet(x, bulletY, type, direction);
	}

	for (i = 0; i < diagonalBullets; i++) {
	    direction = (i%2==0) ? diagonalUp : diagonalDown;
	    bp->fireBullet(x, centerY, type, direction);
	}

        shotTime = currTime;
	straightShotCount++;
	diagonalShotCount += (diagonalBullets > 0) ? 1 : 0;
        if (straightShotCount >= BULLET_BURST && type == friendly) {
            burstEnded = false;
	} else if (straightShotCount >= random(1, BULLET_BURST+1) && type == enemy) {
            burstEnded = false;
	}
    }
}

unsigned long SpaceShip::getCooldown()
{
    return (millis() - shotTime);
}

bool SpaceShip::isPointInside(int px, int py)
{
  return (
        px >= x &&
        px <= x+width &&
        py >= y &&
        py <= y+height
      );
}

bool SpaceShip::isHitByBullet(Bullet *b, ShipType bulletType)
{
    bool isHit = isPointInside(b->x, b->y) && b->type == bulletType;
    if (isHit) {
	health -= b->damage;
    }
    return isHit;
}

void SpaceShip::startDeathAnimation(int duration, int flicker_delay)
{
    unsigned long currTime = millis();
    deathAnimation.isHappening = true;
    deathAnimation.inverted = false;
    deathAnimation.startTime = currTime;
    deathAnimation.flickerTime = currTime;
    deathAnimation.flicker_delay = flicker_delay;
    deathAnimation.duration = duration;
}

void EnemyShipPool::init()
{
    for (int i = 0; i < poolSize; i++) {
        SpaceShip newShip;
        newShip.setIsActive(false);
        pool[i] = newShip;
    }
}

void EnemyShipPool::createEnemy(int x, int y, int width, int height, ShipBitmapType bmpType)
{
    int originalNextIndex = nextAvailableIndex;
    int i = originalNextIndex;
    do {
	SpaceShip *enemyShip = &pool[i];
        if (!enemyShip->getIsActive() && !enemyShip->getDeathAnimationStatus()) {
	    enemyShip->reset();
	    activeEnemyCount++;

	    enemyShip->setIsActive(true);
	    enemyShip->setHealth(DEFAULT_HEALTH);
	    enemyShip->setPosition(x, y);
	    enemyShip->randomMove(
			    SCREEN_WIDTH / 3 - enemyShip->getWidth(),
			    SCREEN_HEIGHT - enemyShip->getHeight()
			);
	    enemyShip->setBmpSettings(width, height, bmpType);

            nextAvailableIndex = (i + 1) % poolSize;
            return;
        }
        i = (i + 1) % poolSize;
    } while (i != originalNextIndex);
}

int EnemyShipPool::gameUpdate(BulletPool *bp)
{
    randomMove();
    updatePosition();

    int bpSize = bp->getPoolSize();
    for (int i = 0; i < bpSize; i++) {
	Bullet *b = bp->getBulletByIndex(i);
	if (b->isActive) {
	    for (int j = 0; j < poolSize; j++) {
                SpaceShip *enemyShip = &pool[j];
                if (enemyShip->getIsActive() && enemyShip->isHitByBullet(b, friendly)) {
		    bp->destroyBulletByIndex(i);
		    if (enemyShip->getHealth() <= 0) {
			lastShotEnemyTime = millis();
			activeEnemyCount--;
			enemyShip->setIsActive(false);
			enemyShip->startDeathAnimation(DEFAULT_DEATH_ANIMATION_DURATION, DEFAULT_FLICKER_DELAY);
			return 1;
		    }
		}
	    }
	}
    }
    return 0;
}

void EnemyShipPool::updatePosition()
{
    for (int i = 0; i < poolSize; i++) {
	SpaceShip *ship = &pool[i];
	if (ship->getIsActive())
	    ship->updatePosition();
    }
}

void EnemyShipPool::randomMove()
{
    for (int i = 0; i < poolSize; i++) {
	SpaceShip *ship = &pool[i];
	if (ship->getIsActive()) {
	    if (!ship->getIsMoving() && millis() - ship->getLastRandomMove() >= RANDOM_MOVE_COOLDOWN) {
		ship->randomMove(
			    SCREEN_WIDTH / 3 - ship->getWidth(),
			    SCREEN_HEIGHT - ship->getHeight()
			);
		}
	}
    }
}

void EnemyShipPool::draw(Adafruit_SSD1306 *display)
{
     for (int i = 0; i < poolSize; i++) {
        SpaceShip *enemyShip = &pool[i];
        enemyShip->draw(display);
    }
}

int EnemyShipPool::getActiveEnemyCount()
{
    return activeEnemyCount;
}

unsigned long EnemyShipPool::getLastShotEnemyTime()
{
    return lastShotEnemyTime;
}

void EnemyShipPool::shoot(BulletPool *bp)
{
    unsigned long currTime = millis();
    for (int i = 0; i < poolSize; i++) {
	SpaceShip *enemy = &pool[i];
	if (enemy->getIsActive()) {
	    enemy->shoot(bp);
	}
    }
}

void Bullet::draw(Adafruit_SSD1306 *display)
{
    //display->drawFastHLine(x, y, BULLET_WIDTH, 1);
    int y1;
    if (ySpeed < 0)
	y1 = (type == friendly) ? y+BULLET_HEIGHT : y-BULLET_HEIGHT;
    else if (ySpeed > 0)
	y1 = (type == friendly) ? y-BULLET_HEIGHT : y+BULLET_HEIGHT;
    else
	y1 = y;
    display->drawLine(x, y, x+BULLET_WIDTH, y1, WHITE);
}

void Bullet::updatePosition()
{
    switch (type) {
        case friendly: x -= xSpeed; break;
        case enemy: x += xSpeed; break;
    }
    y += ySpeed;
    if (y >= SCREEN_HEIGHT || y <= 0)
	ySpeed = -ySpeed;
}

void BulletPool::init()
{
    for (int i = 0; i < poolSize; i++) {
        Bullet newBullet;
        newBullet.damage = 1;
        newBullet.isActive = false;
        pool[i] = newBullet;
    }
}

void BulletPool::fireBullet(int x, int y, ShipType type, BulletDirection direction)
{
    int originalNextIndex = nextAvailableIndex;
    int i = originalNextIndex;
    do {
        if (!pool[i].isActive) {
            Bullet *b = &pool[i];
            b->isActive = true;
            b->x = x;
            b->y = y;
            b->xSpeed = 3;
	    switch (direction) {
		case straight: b->ySpeed = 0; break;
		case diagonalUp: b->ySpeed = -1; break;
		case diagonalDown: b->ySpeed = 1; break;
	    }
            b->type = type;
            nextAvailableIndex = (i + 1) % poolSize;
            return;
        }
        i = (i + 1) % poolSize;
    } while (i != originalNextIndex);
}

void BulletPool::gameUpdate()
{
    for (int i = 0; i < poolSize; i++) {
        Bullet *b = &pool[i];
        if (b->isActive) {
            b->updatePosition();
            if ((b->x + BULLET_WIDTH) < 0 || (b->x - BULLET_WIDTH) > SCREEN_WIDTH) {
                b->isActive = false;
            }
        }
    }
}

Bullet* BulletPool::getBulletByIndex(int i)
{
    return &pool[i];
}

void BulletPool::destroyBulletByIndex(int i)
{
    pool[i].isActive = false;
}

int BulletPool::getPoolSize()
{
    return poolSize;
}

void BulletPool::draw(Adafruit_SSD1306 *display)
{
    for (int i = 0; i < poolSize; i++) {
        Bullet *b = &pool[i];
        if (b->isActive)
            b->draw(display);
    }
}

void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height)
{
    uint8_t buf[width*height/8];
    for (uint8_t i = 0; i < sizeof(buf); i++)
	buf[i] = ~pgm_read_byte(bitmap + i);
    display->drawBitmap(x, y, buf, width, height, 1);
}
