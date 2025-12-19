# Arduino Space Shooter

Fun little project

https://github.com/user-attachments/assets/4a13e35a-0188-418b-a5fd-61b472b1d2ee


## Requirements

- Arduino compatible board
- 128×32 SSD1306 OLED display
- Analog joystick with pushbutton

- Libraries:
  - [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
  - [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)

## Setup & Wiring

- Pins:
  - Joystick Y: `A0`
  - Joystick X: `A1`
  - Button: `3` (configured with `INPUT_PULLUP`)
- Display:
  - SSD1306 over I2C (address `0x3C` by default)

Adjust pin constants in `arduinoShooter.h` if your wiring differs:
```c
#define JOYSTICK_REAL_Y_PIN A0
#define JOYSTICK_REAL_X_PIN A1
#define JOYSTICK_BUTTON_PIN 3
#define SCREEN_ADDRESS 0x3C
```

## Controls

- Move: Analog joystick (deadzone applied for stability)
- Shoot: Press the joystick button
- Start game: Press the joystick button on the start screen

## Gameplay Features

- Shooting
  - Straight and diagonal bullets
  - Burst logic with cooldowns and limits (e.g., straight-bullet cap)
- Bonuses (each bonus chance can be easily modified)
  - Temporary shooting patterns (double/triple/diagonal)
  - Health boosts
- Enemies
  - Up to 3 active at once (pool-based)
  - Levels 1–3 with varying size and health
  - Random movement and timed shooting
- Waves and difficulty
  - Wave table defines simultaneous enemy count, level range/distribution, and a max score per wave
- Scoring and progression
  - Score increases based on killed enemy leve
  - Player level increases when the score hits configured thresholds
  - Health can be added once on score values that are multiples of 3
- HUD and UI
  - Displays player level, score, and health
  - Dedicated HUD drawing for clarity

## Code Architecture

I tried to make the code as easily modifiable and readable as I could. I'm a beginner in C/C++ though.

- `Game`
  - Orchestrates updates, drawing, waves, bonuses, scoring, state flags, and game-over animation
- `SpaceShip`
  - Player/enemy entity: movement, shooting (straight/diagonal), health/level, death animation, bonus handling
- `Bullet` / `BulletPool`
  - Fixed-size pool to avoid dynamic allocation
- `EnemyShipPool`
  - Fixed set of enemies: spawning, movement, shooting, drawing
- `Joystick`
  - Analog X/Y reads and debounced button state (uses `INPUT_PULLUP`)
- `Bonus`
  - Randomness, spawning, movement, cooldowns, activation

### Memory Optimizations

- Fixed-size pools (bullets, enemies) to avoid heap fragmentation
- Tables and strings stored in `PROGMEM`
- Bitmask flags (e.g., `G_GAMEOVER`, `S_ISBONUSACTIVE`) pack multiple booleans into a single byte

### Timing

- All time-based logic uses `millis()`:
  - Shooting cooldowns
  - Wave text visibility
  - Bonus durations
  - Death/game-over animations

### Graphics

- Rendered via Adafruit GFX/SSD1306
- Sprite selection through `ShipBitmapType` enums

### Configuration

Most tuning lives in `arduinoShooter.h`. Common constants:

- Screen, HUD, and sprite sizes
- Cooldowns and spreads (shooting, respawn, random movement)
- Bullet pool size and burst limits
- Leveling and scoring thresholds


## Build & Upload

1. Install required libraries (Adafruit GFX, Adafruit SSD1306)
2. Open `arduinoShooter.ino` in Arduino IDE (or use arduino-cli instead)
3. Select your board and port
4. Upload


## Future Work

- Optimize memory usage further
- Partial redraws/paging (only draw what changed)
- New shooting types (e.g., bombs)
- Bosses

## Notes

- A `godMode` flag exists in `arduinoShooter.h`
- Display size is assumed (128×32). Adjust constants for other resolutions but this can increase memory usage going over the limits.

## Inspiration

This project was inspired by “Masters of Doom” and by working within limited environments that encourage efficient code and hardware awareness. I'm also interested in both hardware and software development (this project is more about software, though).

## Contributing

Suggestions and recommendations on code or game design are welcome - feel free to reach out or open an issue/PR.
