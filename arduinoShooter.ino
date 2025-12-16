#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ezButton.h>
#include "arduinoShooter.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Joystick jstick(JOYSTICK_REAL_Y_PIN, JOYSTICK_REAL_X_PIN, JOYSTICK_BUTTON_PIN);
SpaceShip spaceShip(INITIAL_X, INITIAL_Y, NORMAL_LVL1_WIDTH, NORMAL_LVL1_HEIGHT, 10, normal_lvl1, friendly, true);
BulletPool bulletPool;
EnemyShipPool enemyPool;

Game game(&display, &jstick, &spaceShip, &bulletPool, &enemyPool);

void setup() {
    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    while (!game.startScreen());
    delay(200);
    game.init();
    Serial.print(F("free memory: ")); Serial.println(freeMemory());
}

void loop() {
    display.clearDisplay();

    if (game.isGameOver()) {
	gameOver();
    }

    game.update();
    game.draw();
    game.drawHUD();

    display.display();
}

void gameOver()
{
    for (;;) {
	game.drawGameOver();
	display.display();
    }
}

int freeMemory()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

