#include "Adafruit_SSD1306.h"
#include "Arduino.h"
#include <avr/pgmspace.h>
#include "arduinoShooter.h"

void Game::init()
{
    bulletPool->init();
    spaceShip->setIsActive(true);
    enemyPool->init();
    bonus = new Bonus();
    bonus->deactivate();

    boolVar = setToTrue(boolVar, G_SHOULDDRAWWAVE);
    waveTextStartTime = millis();
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
	if (jstick->getButtonState() == PRESSED_BUTTON) {
	    return true;
	}
    }
    return false;
}

void Game::update()
{
    int add = enemyPool->gameUpdate(bulletPool);
    if (add > 0) updateScore(add);

    handleWaves();
    handleBonus();
    enemyPool->shoot(bulletPool);

    if (!getBoolVal(boolVar, G_FIRSTLOOP) && jstick->getButtonState() == PRESSED_BUTTON) {
        spaceShip->shoot(bulletPool);
    }
    boolVar = setToFalse(boolVar, G_FIRSTLOOP);

    if (score >= 3 && score % 3 == 0) {
	if (getBoolVal(boolVar, G_SHOULDADDHEALTH)) {
	    int8_t toAdd;
	    if (spaceShip->getHealth() <= (spaceShip->getMaxHealth()-3))
		toAdd = 3;
	    else if (spaceShip->getHealth() >= spaceShip->getMaxHealth())
		toAdd = 0;
	    else
		toAdd = spaceShip->getMaxHealth() - spaceShip->getHealth();
	    spaceShip->addHealth(toAdd);
	    boolVar = setToFalse(boolVar, G_SHOULDADDHEALTH);
	}
    } else {
	boolVar = setToTrue(boolVar, G_SHOULDADDHEALTH);
    }

    if (score >= SCORE_FOR_NEXT_LEVEL && (score % SCORE_FOR_NEXT_LEVEL == 0)) {
	if (getBoolVal(boolVar, G_CANINCREASELVL)) {
	    spaceShip->increaseLevel();
	    boolVar = setToFalse(boolVar, G_CANINCREASELVL);
	}
    } else {
	boolVar = setToTrue(boolVar, G_CANINCREASELVL);
    }

    bulletPool->gameUpdate();
    spaceShip->gameUpdate(bulletPool, jstick);
    if (spaceShip->getHealth() <= 0 && !spaceShip->getDeathAnimationStatus()) {
	delay(1000);
	boolVar = setToTrue(boolVar, G_GAMEOVER);
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
    } else {
	delay(1000);
	constexpr int width = 50;
	constexpr int height = 10;
	constexpr int x = (SCREEN_WIDTH/2) - width/2;
	constexpr int y = (SCREEN_HEIGHT/2) - height/2;
	display->fillRect(x, y, width, height, BLACK);
	display->setCursor(x+1, y+1);
	display->setTextSize(1);
	display->print(F("score: "));
	display->print(score);
    }
}

void Game::drawHUD()
{
    //display->drawRect(0, 0, 50, 10, WHITE);
    display->fillRect(0, 0, HUD_WIDTH, HUD_HEIGHT, WHITE);
    display->setCursor(1, 0);
    display->setTextColor(BLACK);
    display->setTextSize(1);
    display->print(F("LVL"));
    display->print(spaceShip->getLevel());
    display->print(F(" S:"));
    display->print(score);
    display->print(F(" H:"));
    display->print(spaceShip->getHealth());
}

void Game::drawWaveText()
{
    if (millis() - waveTextStartTime >= WAVE_TEXT_DURATION) {
	boolVar = setToFalse(boolVar, G_SHOULDDRAWWAVE);
    }

    constexpr int16_t x = SCREEN_WIDTH/2 - WAVE_RECT_WIDTH/2;
    constexpr int16_t y = SCREEN_HEIGHT/2 - WAVE_RECT_HEIGHT/2;
    display->fillRect(x, y, WAVE_RECT_WIDTH, WAVE_RECT_HEIGHT, WHITE);
    display->setCursor(x+1, y+1);
    display->setTextSize(1);
    display->setTextColor(BLACK);
    display->print(F("Wave "));
    if (waveIndex >= WAVE_COUNT)
	display->print("X");
    else
	display->print(waveIndex + 1);
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
    bonus->draw(display);
    if (getBoolVal(boolVar, G_SHOULDDRAWWAVE)) {
	drawWaveText();
    }
    //drawScore();
}

void Game::handleBonus()
{
    if (bonus->shouldCreateBonus()) {
	int16_t x = -BONUS_ICON_WIDTH;
	int16_t y = random(HUD_HEIGHT, SCREEN_HEIGHT-BONUS_ICON_HEIGHT);
	BonusType type = generateRandomBonus();
	bonus->set(x, y, type);
    }

    if (bonus->getIsActive() &&
	    spaceShip->isObjectInside(bonus->getX(), bonus->getY(),
				    bonus->getX() + BONUS_ICON_WIDTH,
				    bonus->getY() + BONUS_ICON_HEIGHT)) {
	spaceShip->handleBonus(bonus->getType());
	bonus->deactivate();
	bonus->startCooldown();
    }

    bonus->updatePosition();
}

void Game::handleWaves()
{
    uint32_t maxScore = pgm_read_dword(&(WAVE_TABLE[waveIndex].maxScore));
    if (score >= maxScore) {
	waveIndex++;
	boolVar = setToTrue(boolVar, G_SHOULDDRAWWAVE);
	waveTextStartTime = millis();
	return;
    }

    uint8_t simCount = pgm_read_dword(&(WAVE_TABLE[waveIndex].simultaneousEnemyCount));
    uint8_t minLvl = pgm_read_dword(&(WAVE_TABLE[waveIndex].minEnemyLevel));
    uint8_t maxLvl = pgm_read_dword(&(WAVE_TABLE[waveIndex].maxEnemyLevel));
    uint8_t lvlDistr = pgm_read_dword(&(WAVE_TABLE[waveIndex].enemyLevelDistribution));

    if (enemyPool->getActiveEnemyCount() < simCount
	    && millis() - enemyPool->getLastShotEnemyTime() >= ENEMY_RESPAWN_COOLDOWN) {
	// Use appropriate sizes for each level
	uint8_t randomLvl = getRandomLevel(minLvl, maxLvl, lvlDistr);
	int randomX = random(0, SCREEN_WIDTH/2 - getWidthForShip(randomLvl, enemy));
	int randomY = random(0, SCREEN_HEIGHT - getHeightForShip(randomLvl, enemy));
	enemyPool->createEnemy(randomX, randomY, randomLvl);
    }
}

void Game::updateScore(uint32_t add)
{
    score += add;
}

void Game::setGameOver(bool isGameOver)
{
    boolVar = isGameOver ? setToTrue(boolVar, G_GAMEOVER)
	: setToFalse(boolVar, G_GAMEOVER);
}

void Game::setGameStarted(bool isGameStarted)
{
    boolVar = isGameStarted ? setToTrue(boolVar, G_GAMESTARTED)
	: setToFalse(boolVar, G_GAMESTARTED);
}

uint32_t Game::getScore()
{
    return score;
}

bool Game::isGameOver()
{
    return getBoolVal(boolVar, G_GAMEOVER);
}

bool Game::isGameStarted()
{
    return getBoolVal(boolVar, G_GAMESTARTED);
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
    return digitalRead(btnPin);
}

void SpaceShip::draw(Adafruit_SSD1306 *display)
{
    const uint8_t *bmp;
    // REMEMBER TO CHANGE THIS IF YOU ADD NEW SPRITES!!!!
    switch (bmpType) {
	case normal_lvl1: bmp = normal_lvl1_bmp; break;
	case enemy_lvl1: bmp = enemy_lvl1_bmp; break;
	case enemy_lvl2: bmp = enemy_lvl2_bmp; break;
	case enemy_lvl3: bmp = enemy_lvl3_bmp; break;
     }
    if (getBoolVal(boolVar, S_ISACTIVE) && !deathAnimation.isHappening) {
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
    boolVar = setToFalse(boolVar, S_ISACTIVE);
    health = maxHealth = level * LEVEL_HEALTH_SPREAD;
    straightShotCount = 0;
    diagonalShotCount = 0;
    shotTime = currTime;
    lastRandomMoveTime = currTime;
    boolVar = setToTrue(boolVar, S_BURSTENDED);
    deathAnimation.isHappening = false;
}

int16_t SpaceShip::getX()
{
    return x;
}

int16_t SpaceShip::getY()
{
    return y;
}

int16_t SpaceShip::getX1()
{
    return x1;
}

int16_t SpaceShip::getY1()
{
    return y1;
}

uint8_t SpaceShip::getWidth()
{
    return width;
}

uint8_t SpaceShip::getHeight()
{
    return height;
}

bool SpaceShip::getIsActive()
{
    return getBoolVal(boolVar, S_ISACTIVE);
}

bool SpaceShip::getIsMoving()
{
    return getBoolVal(boolVar, S_ISMOVING);
}

bool SpaceShip::getIsBonusActive()
{
    return getBoolVal(boolVar, S_ISBONUSACTIVE);
}

int8_t SpaceShip::getHealth()
{
    return health;
}

int8_t SpaceShip::getMaxHealth()
{
    return maxHealth;
}

void SpaceShip::updateMaxHealth()
{
    if (type == friendly)
	maxHealth = FIRST_LEVEL_HEALTH + level * LEVEL_HEALTH_SPREAD;
    else
	maxHealth = ENEMY_FIRST_LEVEL_HEALTH + level * ENEMY_LEVEL_HEALTH_SPREAD;
}

uint8_t SpaceShip::getLevel()
{
    return level;
}

unsigned long SpaceShip::getLastRandomMove()
{
    return lastRandomMoveTime;
}

unsigned long SpaceShip::getLastShotTime()
{
    return shotTime;
}

unsigned long SpaceShip::getBonusStartTime()
{
    return bonusStartTime;
}

uint8_t SpaceShip::getStraightShotCount()
{
    return straightShotCount;
}

uint8_t SpaceShip::getDiagonalShotCount()
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
    lastRandomMoveTime = millis();
    boolVar = setToTrue(boolVar, S_ISMOVING);
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

void SpaceShip::setBmpSettings(uint8_t bmpWidth, uint8_t bmpHeight, ShipBitmapType newBmpType)
{
    width = bmpWidth;
    height = bmpHeight;
    bmpType = newBmpType;
}

void SpaceShip::setIsActive(bool active)
{
    boolVar = active ? setToTrue(boolVar, S_ISACTIVE) : setToFalse(boolVar, S_ISACTIVE);
}

void SpaceShip::setIsMoving(bool moving)
{
    boolVar = moving ? setToTrue(boolVar, S_ISMOVING) : setToFalse(boolVar, S_ISMOVING);
}

void SpaceShip::setHealth(int8_t newHealth)
{
    health = newHealth;
}

void SpaceShip::setMaxHealth(int8_t newMaxHealth)
{
    maxHealth = newMaxHealth;
}

void SpaceShip::setLastShotTime(unsigned long time)
{
    shotTime = time;
}

void SpaceShip::addHealth(int8_t toAdd)
{
    health += (health >= maxHealth) ? 0 : toAdd;
}

void SpaceShip::setBulletsToShoot(uint8_t straightBullets, uint8_t diagonalBullets)
{
    bulletsToShoot.straightBullets = straightBullets;
    bulletsToShoot.diagonalBullets = diagonalBullets;
}

void SpaceShip::resetHealth()
{
    health = maxHealth;
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
	    boolVar = setToFalse(boolVar, S_ISMOVING);
    } else {
	x += xSpeed;
	y += ySpeed;
	if (x <= SCREEN_WIDTH/2) x -= xSpeed;
    }

    if (x < 0) x = 0;
    if (x > SCREEN_WIDTH - width) x = SCREEN_WIDTH - width;
    if (y < 0) y = 0;
    if (y > SCREEN_HEIGHT - height) y = SCREEN_HEIGHT - height;
}

void SpaceShip::setLevel(uint8_t lvl)
{
    level = lvl;
    updateMaxHealth();
    health = maxHealth;
}

void SpaceShip::increaseLevel()
{
    if (level >= MAX_LEVEL) return;
    level++;
    updateMaxHealth();
    health = maxHealth;
}

void SpaceShip::setSpeed(int8_t speedX, int8_t speedY)
{
    xSpeed = speedX;
    ySpeed = speedY;
}

void SpaceShip::gameUpdate(BulletPool *bp, Joystick *jstick)
{
    if (jstick != NULL) updateSpeed(jstick->getX(), jstick->getY());

    updatePosition();
    updateMaxHealth();

    if (type == enemy) return;

    handleBonus(b_none);

    int bpSize = bp->getPoolSize();
    for (int i = 0; i < bpSize; i++) {
        Bullet *b = bp->getBulletByIndex(i);
        if (b->isActive) {
	    if (getBoolVal(boolVar, S_ISACTIVE) && isHitByBullet(b, enemy)) {
                bp->destroyBulletByIndex(i);
		startDeathAnimation(250, 50);
		if (health <= 0) {
		    boolVar = setToFalse(boolVar, S_ISACTIVE);
		    startDeathAnimation(DEFAULT_DEATH_ANIMATION_DURATION, DEFAULT_FLICKER_DELAY);
		}
	    }
        }
    }
}

void SpaceShip::shoot(BulletPool *bp)
{
    if (!getBoolVal(boolVar, S_ISACTIVE)) return;

    unsigned long currTime = millis();

    switch (type) {
	case friendly:
	    if (currTime - shotTime >= BURST_COOLDOWN) {
		boolVar = setToTrue(boolVar, S_BURSTENDED);
		straightShotCount = 0;
		diagonalShotCount = 0;
	    }
	    break;
	case enemy:
	    if (currTime - shotTime >= random(800, 1200)) {
		boolVar = setToTrue(boolVar, S_BURSTENDED);
		straightShotCount = 0;
		diagonalShotCount = 0;
	    }
	    break;
    }

    // burstEnded means "Ready to start/continue burst"
    if (getBoolVal(boolVar, S_BURSTENDED) && (currTime - shotTime >= SHOOTING_COOLDOWN)) {
	int straightBullets;
	int diagonalBullets;

	straightBullets = bulletsToShoot.straightBullets;
	diagonalBullets = bulletsToShoot.diagonalBullets;

	// Old logic by level:
	// int bulletsToFire = level;
	// diagonalBullets =  bulletsToFire - MAX_STRAIGHT_BULLETS;

	// if (bulletsToFire > MAX_STRAIGHT_BULLETS)
	//     straightBullets = MAX_STRAIGHT_BULLETS;
	// else
	//     straightBullets = bulletsToFire;

	// if (diagonalBullets < 0 || diagonalShotCount >= DIAGONAL_BURST) {
	//     diagonalBullets = 0;
	//     }
	// }

	if (straightBullets > 0)
	    shootStraight(bp, straightBullets);
	if (diagonalBullets > 0 && diagonalShotCount <= DIAGONAL_BURST)
	    shootDiagonally(bp, diagonalBullets);

        shotTime = currTime;
        if (straightShotCount >= BULLET_BURST && type == friendly) {
	    boolVar = setToFalse(boolVar, S_BURSTENDED);
	} else if (straightShotCount >= random(1, BULLET_BURST-1) && type == enemy) {
	    boolVar = setToFalse(boolVar, S_BURSTENDED);
	}
    }
}

void SpaceShip::shootStraight(BulletPool *bp, int straightBullets)
{
    int centerY = y + height/2;
    for (int i = 0; i < straightBullets; i++) {
	int bulletY;
	if (straightBullets == 1) {
	    bulletY = centerY;
	} else {
	    bulletY = centerY - BULLET_SPREAD / 2 + (BULLET_SPREAD * i) / (straightBullets - 1);
	}
	bp->fireBullet(x, bulletY, type, straight);
    }
    straightShotCount++;
}

void SpaceShip::shootDiagonally(BulletPool *bp, int diagonalBullets)
{
    for (int i = 0; i < diagonalBullets; i++) {
	int offsetY;
	if (diagonalBullets == 1) {
	    offsetY = 0;
	} else {
	    int spreadFactor = (i*10) / (diagonalBullets-1);
	    int shift = (spreadFactor - 5) * BULLET_SPREAD;
	    offsetY = shift/10;
	}

	int centerY = y + height/2;
	bp->fireBullet(x, centerY - offsetY, type, diagonalUp);
	bp->fireBullet(x, centerY + offsetY, type, diagonalDown);
    }
    diagonalShotCount += (diagonalBullets > 0) ? 1 : 0;
}

unsigned long SpaceShip::getCooldown()
{
    return (millis() - shotTime);
}

bool SpaceShip::isPointInside(int16_t px, int16_t py)
{
  return (
        px >= x &&
        px <= x+width &&
        py >= y &&
        py <= y+height
      );
}

bool SpaceShip::isObjectInside(int16_t px0, int16_t py0, int16_t px1, int16_t py1)
{
  return (
        px1 >= x &&
        px0 <= x+width &&
        py1 >= y &&
        py0 <= y+height
      );
}

bool SpaceShip::isHitByBullet(Bullet *b, ShipType bulletType)
{
    int16_t bx, by;
    bx = (b->type == enemy) ? b->x + BULLET_WIDTH : b->x;
    if (b->type == enemy) {
	if (b->ySpeed < 0) by = b->y - BULLET_HEIGHT;
	else if (b->ySpeed > 0) by = b->y + BULLET_HEIGHT;
	else by = b->y;
    }
    bool isHit = isPointInside(b->x, b->y) && b->type == bulletType;
    if (type == friendly && godMode)
	return isHit;

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

void SpaceShip::handleBonus(BonusType bonus)
{
    unsigned long currTime = millis();
    if (activeBonus == b_none && bonus == b_none) return;
    if (!getIsBonusActive()) {
	activateBonus(bonus);
    } else {
	if (currTime - bonusStartTime >= getBonusDuration(activeBonus)) {
	    resetBonus(activeBonus);
	}
    }
}

void SpaceShip::activateBonus(BonusType bonus)
{
    boolVar = setToTrue(boolVar, S_ISBONUSACTIVE);
    bonusStartTime = millis();
    activeBonus = bonus;
    switch (bonus) {
	case b_addHealth:
	    if (health <= (maxHealth - B_ADDHEALTH_AMOUNT)) {
		health += B_ADDHEALTH_AMOUNT;
	    }
	    else if (health < maxHealth) {
		health += maxHealth - health;
	    }
	    boolVar = setToFalse(boolVar, S_ISBONUSACTIVE);
	    activeBonus = b_none;
	    break;
	case b_diagonalBullets:
	    bulletsToShoot.diagonalBullets = bulletsToShoot.straightBullets;
	    break;
	case b_doubleBullets:
	    bulletsToShoot.diagonalBullets *= 2;
	    bulletsToShoot.straightBullets *= 2;
	    break;
	case b_tripleBullets:
	    bulletsToShoot.diagonalBullets *= 3;
	    bulletsToShoot.straightBullets *= 3;
	    break;
    }
}

void SpaceShip::resetBonus(BonusType bonus)
{
    boolVar = setToFalse(boolVar, S_ISBONUSACTIVE);
    activeBonus = b_none;
    switch (bonus) {
	case b_diagonalBullets:
	    bulletsToShoot.diagonalBullets = 0;
	    break;
	case b_doubleBullets:
	    bulletsToShoot.straightBullets /= 2;
	    bulletsToShoot.diagonalBullets /= 2;
	    break;
	case b_tripleBullets:
	    bulletsToShoot.straightBullets /= 3;
	    bulletsToShoot.diagonalBullets /= 3;
	    break;
    }
}

void SpaceShip::setRandomLastShotTime(unsigned int spread)
{
    unsigned long currTime = millis();
    shotTime = random(currTime-spread, currTime);
}

void SpaceShip::setRandomLastRandomMoveTime(unsigned int spread)
{
    unsigned long currTime = millis();
    lastRandomMoveTime = random(currTime-spread, currTime);
}


void EnemyShipPool::init()
{
    for (int i = 0; i < poolSize; i++) {
        SpaceShip newShip;
        newShip.setIsActive(false);
        pool[i] = newShip;
    }
}

void EnemyShipPool::createEnemy(int16_t x, int16_t y, uint8_t level)
{
    unsigned long currTime = millis();
    uint8_t width, height;
    ShipBitmapType bmpType;
    width = getWidthForShip(level, enemy);
    height = getHeightForShip(level, enemy);
    bmpType = getBmpTypeForShip(level, enemy);

    int originalNextIndex = nextAvailableIndex;
    int i = originalNextIndex;
    do {
	SpaceShip *enemyShip = &pool[i];
        if (!enemyShip->getIsActive() && !enemyShip->getDeathAnimationStatus()) {
	    enemyShip->reset();
	    activeEnemyCount++;

	    enemyShip->setLevel(level);
	    enemyShip->setIsActive(true);
	    enemyShip->setBulletsToShoot(1, 0);
	    // dont make them should simultaneously
	    enemyShip->setRandomLastShotTime(ENEMY_SHOOTING_SPREAD);
	    enemyShip->setPosition(x, y);
	    enemyShip->randomMove(
			    SCREEN_WIDTH / 3 - enemyShip->getWidth(),
			    SCREEN_HEIGHT - enemyShip->getHeight()
			);
	    enemyShip->setRandomLastRandomMoveTime(RANDOM_MOVE_COOLDOWN);
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
    updateMaxHealth();

    int8_t bpSize = bp->getPoolSize();
    for (int8_t i = 0; i < bpSize; i++) {
	Bullet *b = bp->getBulletByIndex(i);
	if (b->isActive) {
	    for (int8_t j = 0; j < poolSize; j++) {
                SpaceShip *enemyShip = &pool[j];
                if (enemyShip->getIsActive() && enemyShip->isHitByBullet(b, friendly)) {
		    bp->destroyBulletByIndex(i);
		    if (enemyShip->getHealth() <= 0) {
			// I know that it updates every time enemy is shot
			// idc though, at least it works
			lastShotEnemyTime = millis();
			activeEnemyCount--;
			enemyShip->setIsActive(false);
			enemyShip->startDeathAnimation(DEFAULT_DEATH_ANIMATION_DURATION, DEFAULT_FLICKER_DELAY);
			return enemyShip->getLevel();
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

void EnemyShipPool::updateMaxHealth()
{
    for (int i = 0; i < poolSize; i++) {
	SpaceShip *ship = &pool[i];
	if (ship->getIsActive())
	    ship->updateMaxHealth();
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

int8_t EnemyShipPool::getActiveEnemyCount()
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

Bullet* BulletPool::getBulletByIndex(uint8_t i)
{
    return &pool[i];
}

void BulletPool::destroyBulletByIndex(uint8_t i)
{
    pool[i].isActive = false;
}

uint8_t BulletPool::getPoolSize()
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

void Bonus::set(int16_t posX, int16_t posY, BonusType bonusType)
{
    x = posX;
    y = posY;
    type = bonusType;
    isActive = true;
}

void Bonus::deactivate()
{
    isActive = false;
    x = BONUS_ICON_WIDTH;
    y = BONUS_ICON_HEIGHT;
    type = b_none;
}

void Bonus::startCooldown()
{
    lastBonusTime = millis();
    cooldown = random(5000, 10000);
}

bool Bonus::cooldownExpired()
{
    return (millis() - lastBonusTime) >= cooldown;
}

void Bonus::resetTimer()
{
    lastBonusTime = millis();
}

bool Bonus::shouldCreateBonus()
{
    if (!isActive && cooldownExpired())
	return true;
    else
	return false;
}

void Bonus::updatePosition()
{
    if (isActive) {
	x++;
	if (x >= SCREEN_WIDTH) {
	    deactivate();
	    startCooldown();
	}
    }
}

bool Bonus::getIsActive()
{
    return isActive;
}

int16_t Bonus::getX()
{
    return x;
}

int16_t Bonus::getY()
{
    return y;
}

BonusType Bonus::getType()
{
    return type;
}

void Bonus::draw(Adafruit_SSD1306 *display)
{
    if (!isActive || type == b_none) return;
    const uint8_t *bmp;
    switch (type) {
	case b_addHealth: bmp = b_addHealth_bmp; break;
	case b_diagonalBullets: bmp = b_diagonalBullets_bmp; break;
	case b_doubleBullets: bmp = b_doubleBullets_bmp; break;
	case b_tripleBullets: bmp = b_tripleBullets_bmp; break;
    }
    display->drawBitmap(x, y, bmp, BONUS_ICON_WIDTH, BONUS_ICON_HEIGHT, WHITE);
    display->drawRect(x, y, BONUS_ICON_WIDTH+1, BONUS_ICON_HEIGHT+1, WHITE);
}

void drawInvertedBitmap(Adafruit_SSD1306 *display, uint8_t x, uint8_t y, const uint8_t *bitmap, uint8_t width, uint8_t height)
{
    uint8_t buf[width*height/8];
    for (uint8_t i = 0; i < sizeof(buf); i++)
	buf[i] = ~pgm_read_byte(bitmap + i);
    display->drawBitmap(x, y, buf, width, height, 1);
}

// uint8_t *getBmpByType(uint8_t width, uint8_t height, ShipBitmapType bmpType)
// {
//     const uint8_t *bmp;
//     switch (bmpType) {
//	    case normal_lvl1: bmp = normal_lvl1_bmp; break;
//	    case enemy_lvl1: bmp = enemy_lvl1_bmp; break;
//	    case enemy_lvl2: bmp = enemy_lvl2_bmp; break;
//     }
//     uint8_t buf[width*height/8];
//     for (uint8_t i = 0; i < sizeof(buf); i++)
// 	buf[i] = pgm_read_byte(bmp + i);
//     return buf;
// }

int getBonusDuration(BonusType bonus)
{
    switch (bonus) {
	case b_diagonalBullets:
	    return B_DIAGONALBULLETS_DURATION;
	case b_doubleBullets: case b_tripleBullets:
	    return B_MULTIPLEBULLETS_DURATION;
    }
}

int getTotalBonusWeight()
{
    int totalWeight = 0;
    for (int i = 0; i < BONUS_COUNT; i++) {
	BonusData current;
	memcpy_P(&current, &BONUS_TABLE[i], sizeof(BonusData));
	totalWeight += current.weight;
    }
    return totalWeight;
}

BonusType generateRandomBonus()
{
    int totalWeight = getTotalBonusWeight();
    if (totalWeight == 0) return b_none;

    int roll = random(0, totalWeight);
    int currentLimit = 0;

    for (int i = 0; i < BONUS_COUNT; i++) {
	BonusData current;
	memcpy_P(&current, &BONUS_TABLE[i], sizeof(BonusData));
	currentLimit += current.weight;
	if (roll < currentLimit) return current.type;
    }
    return b_none;
}

uint8_t setToTrue(uint8_t var, uint8_t bitmask)
{
    return var | bitmask;
}


uint8_t setToFalse(uint8_t var, uint8_t bitmask)
{
    return var & ~bitmask;
}

bool getBoolVal(uint8_t var, uint8_t bitmask)
{
    return var & bitmask;
}

uint8_t getWidthForShip(uint8_t level, ShipType type)
{
    if (type == friendly) {
	switch (level) {
	    case 1: default:
		return NORMAL_LVL1_WIDTH;
	}
    } else {
	switch (level) {
	    case 1: return ENEMY_LVL1_WIDTH;
	    case 2: return ENEMY_LVL2_WIDTH;
	    case 3: return ENEMY_LVL3_WIDTH;
	}
    }
}

uint8_t getHeightForShip(uint8_t level, ShipType type)
{
    if (type == friendly) {
	switch (level) {
	    case 1: default:
		return NORMAL_LVL1_HEIGHT;
	}
    } else {
	switch (level) {
	    case 1: return ENEMY_LVL1_HEIGHT;
	    case 2: return ENEMY_LVL2_HEIGHT;
	    case 3: return ENEMY_LVL3_HEIGHT;
	}
    }
}

ShipBitmapType getBmpTypeForShip(uint8_t level, ShipType type)
{
    if (type == friendly) {
	switch (level) {
	    case 1: default:
		return normal_lvl1;
	}
    } else {
	switch (level) {
	    case 1: return enemy_lvl1;
	    case 2: return enemy_lvl2;
	    case 3: return enemy_lvl3;
	}
    }
}

// maybe change later because I don't even remember how it works
uint8_t getRandomLevel(uint8_t minLvl, uint8_t maxLvl, uint8_t lvlDistr)
{
    if (minLvl == maxLvl) return minLvl;
    if (lvlDistr == 10) return maxLvl;
    else if (lvlDistr == 1) return minLvl;
    uint8_t lvlCount = (maxLvl - minLvl) + 1;
    uint8_t weights[lvlCount];
    int totalWeight = 0;

    // Map intensity (1-10) to a position in our level array (0 to numLevels-1);
    // use 100 as a multiplier to keep precision without floats
    int targetPosFixed = ((lvlDistr-1) * (lvlCount-1) * 100) / 9;
    for (int i = 0; i < lvlCount; i++) {
	int currPosFixed = i*100;
	int distance = abs(targetPosFixed - currPosFixed);

	int w = 100 - (distance/2);
	if (w < 1) w = 1; // every level has at least tiny chance
	weights[i] = w;
	totalWeight += w;
    }

    int roll = random(1, totalWeight);
    unsigned long currSum = 0;

    for (int i = 0; i < lvlCount; i++) {
	currSum += weights[i];
	if (roll < currSum) return minLvl + i;
    }

    return maxLvl;
}
