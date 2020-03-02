/*
Copyright (c) 2020 Mitch Coyer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
*/

#ifndef _MSC_VER
#error "This example of Pong requires the MSVC toolchain to compile and run"
#else

#ifndef __cplusplus
#error "This example of Pong requires a C++ compiler"
#else

#define _WIN32_WINNT 0x0501
#define TIME std::chrono::time_point<std::chrono::high_resolution_clock>
#define NOW chrono::high_resolution_clock::now()
#define DURATION(startTime, endTime) std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
#define CONSOLE_WHITE 15
#define CONSOLE_RED FOREGROUND_RED | FOREGROUND_INTENSITY
#define CONSOLE_GREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define CONSOLE_BLUE FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define CONSOLE_YELLOW FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define CONSOLE_AQUA FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
#define CONSOLE_MAGENTA FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY

#include <ctime>
#include <chrono>
#include <functional>
#include <iostream>
#include <windows.h>
#pragma comment(lib, "User32.lib")

using namespace std;
using namespace std::chrono;

class Game;
class Player;
class Ball;
class Shape;

class Shape {
 public:  // Typedefs
  typedef std::function<void(Shape *)> shapeCallback;

 public:  // Constructor
  Shape(int _width, int _height) : width(_width), height(_height) {}
  virtual ~Shape() {}

 public:  // Accessors
  void setVisibility(bool _visibility = true) {
    visible = _visibility;
  }
  void setXPosition(float _xPosition, bool _fireEvents = true) {
    if (_fireEvents && onBeforePositionEvent) onBeforePositionEvent(this);
    x = _xPosition;
    if (_fireEvents && onAfterPositionEvent) onAfterPositionEvent(this);
  }
  void setYPosition(float _yPosition, bool _fireEvents = true) {
    if (_fireEvents && onBeforePositionEvent) onBeforePositionEvent(this);
    y = _yPosition;
    if (_fireEvents && onAfterPositionEvent) onAfterPositionEvent(this);
  }
  void setAbsPosition(int _xPosition, int _yPosition, bool _fireEvents = true) {
    if (_fireEvents && onBeforePositionEvent) onBeforePositionEvent(this);

    x = _xPosition;
    position.X = _xPosition;
    y = _yPosition;
    position.Y = _yPosition;

    if (_fireEvents && onAfterPositionEvent) onAfterPositionEvent(this);
  }
  void setXVelocity(float _vx, bool _fireEvents = true) {
    if (_fireEvents && onBeforeVelocityEvent) onBeforeVelocityEvent(this);
    vx = _vx;
    if (_fireEvents && onAfterVelocityEvent) onAfterVelocityEvent(this);
  }
  void setYVelocity(float _vy, bool _fireEvents = true) {
    if (_fireEvents && onBeforeVelocityEvent) onBeforeVelocityEvent(this);
    vy = _vy;
    if (_fireEvents && onAfterVelocityEvent) onAfterVelocityEvent(this);
  }
  void setVelocities(float _vx, float _vy, bool _fireEvents = true) {
    if (_fireEvents && onBeforeVelocityEvent) onBeforeVelocityEvent(this);
    vx = _vx;
    vy = _vy;
    if (_fireEvents && onAfterVelocityEvent) onAfterVelocityEvent(this);
  }

 public:  // Events
  void onBeforePositionChange(shapeCallback _callback) {
    if (_callback) onBeforePositionEvent = _callback;
  };
  void onAfterPositionChange(shapeCallback _callback) {
    if (_callback) onAfterPositionEvent = _callback;
  };
  void onBeforeVelocityChange(shapeCallback _callback) {
    if (_callback) onBeforeVelocityEvent = _callback;
  };
  void onAfterVelocityChange(shapeCallback _callback) {
    if (_callback) onAfterVelocityEvent = _callback;
  };

 public:  // Draw Methods
  virtual void draw(HANDLE *_console) = 0;
  virtual void clear(HANDLE *_console) = 0;

 protected:  // Data
  bool          visible;
  int           height, width = 0;                            // The width and height of the object
  float         x, y = 0;                                     // The coordinates of the player used for calculating position
  float         vx, vy = 0;                                   // Velocity of the shape in space
  COORD         position;                                     // the onscreen position of the middle of the player
  shapeCallback onBeforePositionEvent = [](Shape *_this) {};  // Callback to call before the object has changed position
  shapeCallback onAfterPositionEvent = [](Shape *_this) {};   // Callback to call after the object has changed position
  shapeCallback onBeforeVelocityEvent = [](Shape *_this) {};  // Callback to call before the object has changed velocity
  shapeCallback onAfterVelocityEvent = [](Shape *_this) {};   // Callback to call after the object has changed velocity
};

class Player : public Shape {
 public:  // Friends
  friend Game;

 public:  // Constructor
  Player() : Shape(1, 5) {}
  ~Player() {}

 public:  // Player Movement Handles
  void moveUp() {
    setYPosition(y - vy);
  }
  void moveDown() {
    setYPosition(y + vy);
  }

 private:  // Drawing Overrides
  virtual void draw(HANDLE *_console) override {
    if (visible) {
      // Update Player Position from the x y values
      position.Y = int(y);
      COORD drawPosition = position;
      drawPosition.Y -= 2;

      // Adjust the draw position and colour to draw the player
      SetConsoleTextAttribute(*_console, CONSOLE_AQUA);
      for (int i = 0; i < height; i++) {
        SetConsoleCursorPosition(*_console, drawPosition);
        cout << 'I';
        drawPosition.Y += 1;
      }
      SetConsoleTextAttribute(*_console, CONSOLE_WHITE);
    }
  }
  virtual void clear(HANDLE *_console) override {
    // Update Player Position
    COORD drawPosition = position;
    drawPosition.Y -= 2;
    for (int i = 0; i < height; i++) {
      SetConsoleCursorPosition(*_console, drawPosition);
      cout << ' ';
      drawPosition.Y += 1;
    }
  }

 private:                      // Private Data
  int  score = 0;              // The current score of the player
  bool lostLastPoint = false;  // Keep track of if the player lost their last point
};

class Ball : public Shape {
 public:  // Friends
  friend Game;

 public:  // Constructors
  Ball() : Shape(1, 1) {}
  ~Ball() {}

 private:  // Calculations
  void calculatePosition() {
    if (onBeforePositionEvent) onBeforePositionEvent(this);
    x += vx;
    y += vy;
    if (onAfterPositionEvent) onAfterPositionEvent(this);
  }

 private:  // Drawing Overrides
  virtual void draw(HANDLE *_console) override {
    // Update the position
    position.X = int(x);
    position.Y = int(y);
    SetConsoleCursorPosition(*_console, position);

    // Draw on console
    SetConsoleTextAttribute(*_console, CONSOLE_GREEN);
    cout << 'O';
    SetConsoleTextAttribute(*_console, CONSOLE_WHITE);
  }
  virtual void clear(HANDLE *_console) override {
    SetConsoleCursorPosition(*_console, position);
    cout << ' ';
  }
};

class Game {
 public:  // Enums
  enum class GameMode {
    NOT_STARTED = -1,
    MULTIPLAYER = 0,
    EASY = 1,
    MEDIUM = 2,
    HARD = 3,
    IMPOSSIBLE = 4
  };
  enum class GameState {
    NOT_STARTED = -1,
    PAUSED = 0,
    IN_PLAY = 1,
    PLAYER_1_WINNER = 2,
    PLAYER_2_WINNER = 3,
    CPU_WINNER = 4
  };

 public:  // Constructor
  Game(int _width, int _height) : width(_width), height(_height) {
    // Adjust the height if set to 0
    if (!height)
      height = 35;
    if (!width)
      width = 79;

    // Set up the players and the ball
    initGame();
  }

 private:  // Game Initializer
  void initGame() {
    // Seed the random number generator
    srand(unsigned int(time(NULL)));

    // Get the output console object and set its size
    setGameArea(height, width);

    // Hid the cursor for the whole Game
    hideCursor();

    // Create generic functors for player movement
    Shape::shapeCallback playerBeforeCallback = [=](Shape *_this) {
      beforePlayerChangeCallback(_this);
    };
    Shape::shapeCallback playerAfterCallback = [=](Shape *_this) {
      afterPlayerChangeCallback(_this);
    };

    // Init the players
    player1.setYVelocity(1);
    player1.onBeforePositionChange(playerBeforeCallback);
    player1.onAfterPositionChange(playerAfterCallback);

    player2.setYVelocity(1);
    player2.onBeforePositionChange(playerBeforeCallback);
    player2.onAfterPositionChange(playerAfterCallback);

    cpu.onBeforePositionChange(playerBeforeCallback);
    cpu.onAfterPositionChange(playerAfterCallback);

    // Init the ball
    ball.onBeforePositionChange([=](Shape *_this) {
      beforeBallChangeCallback(_this);
    });
    ball.onAfterPositionChange([=](Shape *_this) {
      afterBallChangeCallback(_this);
    });
    ball.onAfterVelocityChange([=](Shape *_this) {
      afterBallVelocityCallback(_this);
    });
  }

 private:  // Player and ball callbacks
  void beforePlayerChangeCallback(Shape *_this) {
    _this->clear(&console);
  }
  void afterPlayerChangeCallback(Shape *_this) {
    // Cast back up to Player object
    Player *player = static_cast<Player *>(_this);

    // Check if there are collisions with the wall
    int playerHalfHeight = int(player->height / 2);
    int playerTop = player->y - playerHalfHeight;
    int playerBottom = player->y + playerHalfHeight;
    if (playerTop < 3) {
      player->setYPosition(3 + playerHalfHeight, false);
    } else if (playerBottom > height - 1) {
      player->setYPosition(height - 1 - playerHalfHeight, false);
    }

    // Draw Player
    player->draw(&console);
  }
  void beforeBallChangeCallback(Shape *_this) {
    _this->clear(&console);
  }
  void afterBallChangeCallback(Shape *_this) {
    // Cast back up to Player object
    Ball *c_ball = static_cast<Ball *>(_this);

    // Make a reset play bool
    bool playNeedsReset = false;

    // Check if it will collide with a wall
    if (c_ball->y < 3) {
      if (c_ball->vy < 0) {
        c_ball->setAbsPosition(c_ball->x, 3, false);
        c_ball->setVelocities(c_ball->vx, -c_ball->vy);
      }
    } else if (c_ball->y > height - 1) {
      if (c_ball->vy > 0) {
        c_ball->setAbsPosition(c_ball->x, height - 1, false);
        c_ball->setVelocities(c_ball->vx, -c_ball->vy);
      }
    }

    // Check if it will collide with player 1
    if (c_ball->x < 1) {
      // Get position relative to player1
      int ballRelPos = player1.position.Y - (int)c_ball->y;
      int playerHalfHeight = (int)player1.height / 2;

      if (ballRelPos <= playerHalfHeight && ballRelPos >= -playerHalfHeight) {
        // Adjust Ball position
        c_ball->setAbsPosition(1, c_ball->y, false);

        // Invert ball x velocity
        if (c_ball->vx < 0) {
          float newXVelocity = randomFloat(minXSpeed, maxXSpeed);
          float newYVelocity = c_ball->vy + ballRelPos / 4 + randomFloat(-0.5, 0.5);
          c_ball->setVelocities(newXVelocity, newYVelocity);
          Beep(300, 50);
        }

        // If in impossible mode add to score each hit
        if (gameMode == GameMode::IMPOSSIBLE) {
          player1.score += 1;
          drawScore();
        }
      } else {
        // add score to CPU or player 2
        c_ball->setAbsPosition(0, c_ball->y, false);
        player1.lostLastPoint = true;
        getOpponent()->score += 1;
        playNeedsReset = true;
      }
    }

    // Check if it will collide with player 2 or the CPU
    if (c_ball->x > width - 1) {
      // Get the opponent
      Player *opponent = getOpponent();

      // Get position relative to opponent
      int ballRelPos = (int)c_ball->y - opponent->position.Y;
      int playerHalfHeight = (int)opponent->height / 2;

      if (ballRelPos <= playerHalfHeight && ballRelPos >= -playerHalfHeight) {
        // Adjust Ball position
        c_ball->setAbsPosition(width - 2, c_ball->y, false);

        // Invert ball x velocity
        if (c_ball->vx > 0) {
          float newXVelocity = randomFloat(-maxXSpeed, -minXSpeed);
          float newYVelocity = c_ball->vy + ballRelPos / 4 + randomFloat(-0.5, 0.5);
          c_ball->setVelocities(newXVelocity, newYVelocity);
          Beep(300, 50);
        }
      } else {
        // add score to CPU or player 2
        c_ball->setAbsPosition(width - 1, c_ball->y, false);
        opponent->lostLastPoint = true;
        player1.score += 1;
        playNeedsReset = true;
      }
    }

    c_ball->draw(&console);
    if (playNeedsReset) resetPlay();
  }
  void afterBallVelocityCallback(Shape *_this) {
    // Cast back to ball
    Ball *c_ball = static_cast<Ball *>(_this);

    // Check if the velocities have breached their maximums or minimums
    c_ball->setXVelocity((c_ball->vx > maxXSpeed) ? maxXSpeed : c_ball->vx, false);
    c_ball->setXVelocity((c_ball->vx < -maxXSpeed) ? -maxXSpeed : c_ball->vx, false);
    c_ball->setXVelocity((c_ball->vx > -minXSpeed && c_ball->vx < 0) ? -minXSpeed : c_ball->vx, false);
    c_ball->setXVelocity((c_ball->vx < minXSpeed && c_ball->vx > 0) ? minXSpeed : c_ball->vx, false);
    c_ball->setYVelocity((c_ball->vy > maxYSpeed) ? maxYSpeed : c_ball->vy, false);
    c_ball->setYVelocity((c_ball->vy < -maxYSpeed) ? -maxYSpeed : c_ball->vy, false);
  }

 private:  // Game Logic Methods
  Player *getOpponent() {
    Player *opponent;
    if (gameMode == GameMode::MULTIPLAYER) {
      opponent = &player2;
    } else {
      opponent = &cpu;
    }
    return opponent;
  }
  void checkInputs() {
    if (isActiveWindow()) {
      if (GetAsyncKeyState(0x57))
        player1.moveUp();
      if (GetAsyncKeyState(0x53))
        player1.moveDown();
      if (GetAsyncKeyState(VK_UP))
        player2.moveUp();
      if (GetAsyncKeyState(VK_DOWN))
        player2.moveDown();
    }
  }
  void calculateCpuPosition() {
    // Shortcut the impossible mode
    if (gameMode == GameMode::IMPOSSIBLE) {
      cpu.setYPosition(ball.y);
    }

    // Only judge where the ball will be when if coming towards the CPU
    else if (ball.vx > 0) {
      // Get the difference between the ball and the center of the cpu
      float deltaCpu = float(cpu.y - ball.y);

      // Applied a weighted multiplier based on the difficulty
      switch (gameMode) {
      case GameMode::EASY:
        cpu.vy -= deltaCpu / 10.0f;
        cpu.vy *= 0.60f;
        cpu.setYPosition(cpu.y + cpu.vy);
        break;
      case GameMode::MEDIUM:
        cpu.vy -= deltaCpu / 10.0f;
        cpu.vy *= 0.70f;
        cpu.setYPosition(cpu.y + cpu.vy);
        break;
      case GameMode::HARD:
        cpu.vy -= deltaCpu / 10.0f;
        cpu.vy *= 0.80f;
        cpu.setYPosition(cpu.y + cpu.vy);
        break;
      default:
        break;
      }
    }
  }
  void checkScore() {
    if (gameMode != GameMode::IMPOSSIBLE) {
      Player *opponent = getOpponent();
      if (player1.score >= 5) {
        gameState = GameState::PLAYER_1_WINNER;
      } else if (opponent->score >= 5) {
        if (gameMode == GameMode::MULTIPLAYER)
          gameState = GameState::PLAYER_2_WINNER;
        else
          gameState = GameState::CPU_WINNER;
      }
    }
  }
  void waitForStart() {
    //  Wait till the game mode is selected
    if (isActiveWindow()) {
      if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        gameMode = GameMode::MULTIPLAYER;
      } else if (GetAsyncKeyState(0x31) & 0x8000) {
        gameMode = GameMode::EASY;
      } else if (GetAsyncKeyState(0x32) & 0x8000) {
        gameMode = GameMode::MEDIUM;
      } else if (GetAsyncKeyState(0x33) & 0x8000) {
        gameMode = GameMode::HARD;
      } else if (GetAsyncKeyState(0x34) & 0x8000) {
        gameMode = GameMode::IMPOSSIBLE;
      }
    }

    // Show the correct players
    switch (gameMode) {
    case GameMode::NOT_STARTED:
      player1.visible = false;
      player2.visible = false;
      cpu.visible = false;
      break;
    case GameMode::MULTIPLAYER:
      player1.visible = true;
      player2.visible = true;
      cpu.visible = false;
      break;
    default:
      player1.visible = true;
      player2.visible = false;
      cpu.visible = true;
      break;
    }
  }
  void waitForPlay() {
    if (isActiveWindow()) {
      if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        //  Clear the play area
        clearPlayArea();

        // Draw the players and ball back in
        player1.draw(&console);
        getOpponent()->draw(&console);
        ball.draw(&console);

        // Set the game state to inplay
        gameState = GameState::IN_PLAY;
      } else {
        gameState = GameState::PAUSED;
      }
    }
  }

 public:  // Game Progression Methods
  bool runGame() {
    // Draw the Title and setup game
    resetGame();
    drawTitleScreen();

    // Play theme song
    playThemeSong();

    // Handle the front options menu
    while (gameMode == GameMode::NOT_STARTED) {
      // Exit the console if on the main screen
      if (isActiveWindow() && GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        FreeConsole();
        return false;
      }

      // Wait for user input for game mode
      waitForStart();
      Sleep(50);
    }

    // Show game mode when selected
    drawGameModeScreen();

    // Reset the game play
    resetPlay();

    // Loop through the main game functions until there is a result
    while (gameState <= GameState::IN_PLAY) {
      // Use the esc key to escape the game
      if (isActiveWindow() && GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        Sleep(50);
        return true;
      }

      // If window is not active or p button is pressed pause the game
      if ((!isActiveWindow() || GetAsyncKeyState(0x50) & 0x8000) && gameState != GameState::PAUSED) {
        gameState = GameState::PAUSED;
        drawPauseScreen();
        continue;
      }

      // Handle the start of paused play
      if (gameState == GameState::PAUSED) {
        waitForPlay();
        Sleep(50);
      }

      // Run the game continuously
      else if (gameState == GameState::IN_PLAY) {
        // Check the keyboard inputs
        checkInputs();

        // Calculate the new CPU position
        calculateCpuPosition();

        // Calculate the new ball position
        ball.calculatePosition();

        // Check the score
        checkScore();

        // Delay for visuals
        TIME loopEndTime = NOW;
        auto ms = DURATION(loopStartTime, loopEndTime);
        if (ms < 30)
          Sleep(30 - ms);
        loopStartTime = NOW;
      }
    }

    // Show the winner
    drawWinnerScreen();
    playWinningSong();
    Sleep(3000);
    return true;
  }
  void resetPlay() {
    // Get the opponent
    Player *opponent = getOpponent();

    // If the game mode was impossible reset player 1 score
    if (gameMode == GameMode::IMPOSSIBLE && player1.score > 0) {
      drawImpossibleModeScore();
      Sleep(1000);
      player1.score = 0;
    }

    // Add small delay to see ball before reset
    Sleep(500);

    // Reset the game to start conditions
    player1.setAbsPosition(0, height / 2);
    opponent->setAbsPosition(width - 1, height / 2);
    ball.setAbsPosition(width / 2, height / 2);

    // Serve the ball towards the winner
    if (player1.lostLastPoint) {
      ball.setVelocities(randomFloat(minXSpeed / 2, maxXSpeed / 2), randomFloat(-maxYSpeed / 3, maxYSpeed / 3));
      player1.lostLastPoint = false;
    } else if (opponent->lostLastPoint) {
      ball.setVelocities(randomFloat(-maxXSpeed / 2, -minXSpeed / 2), randomFloat(-maxYSpeed / 3, maxYSpeed / 3));
      opponent->lostLastPoint = false;
    } else {
      ball.setVelocities(randomFloat(-maxXSpeed / 2, maxXSpeed / 2), randomFloat(-maxYSpeed / 3, maxYSpeed / 3));
    }

    // Draw the start screen
    drawGameStartScreen();

    // Set the game state
    gameState = GameState::PAUSED;
  }
  void resetGame() {
    // Reset the scores
    player1.score = 0;
    player2.score = 0;
    cpu.score = 0;

    // Reset the states
    gameMode = GameMode::NOT_STARTED;
    gameState = GameState::NOT_STARTED;
  }

 private:  // Game Draw Methods
  void drawBorder(int borderColour = CONSOLE_WHITE) {
    // Set the colour of the text
    SetConsoleTextAttribute(console, borderColour);

    // Draw the top border
    setCursorPosition(0, 0);
    drawOverWidth('-');

    // Draw the score border
    setCursorPosition(0, 2);
    drawOverWidth('-');

    // Draw the bottom
    setCursorPosition(0, height);
    drawOverWidth('-');

    // Reset the colour
    SetConsoleTextAttribute(console, CONSOLE_WHITE);
  }
  void drawScore() {
    // Put the cursor at the top
    setCursorPosition(0, 1);

    // Get the opponent
    Player *    opponent = getOpponent();
    const char *opponentName = (gameMode == GameMode::MULTIPLAYER) ? "P2" : "CPU";

    // Draw the score board
    if (gameMode == GameMode::NOT_STARTED) {
      SetConsoleTextAttribute(console, CONSOLE_GREEN);
      padToMiddle("   Waiting for Game Start   \n");
      SetConsoleTextAttribute(console, CONSOLE_WHITE);
    } else if (gameMode != GameMode::IMPOSSIBLE) {
      // Create the score strings
      char p1Score[12];
      char opponentScore[14];
      sprintf(p1Score, "P1 Score: %d", player1.score);
      sprintf(opponentScore, "%s Score: %d ", opponentName, opponent->score);

      // Print P1 score
      if (player1.score > opponent->score) {
        SetConsoleTextAttribute(console, CONSOLE_GREEN);
      } else if (player1.score < opponent->score) {
        SetConsoleTextAttribute(console, CONSOLE_RED);
      } else {
        SetConsoleTextAttribute(console, CONSOLE_YELLOW);
      }
      padToWidth(p1Score, width / 2 - 12);

      // Print the divider
      SetConsoleTextAttribute(console, CONSOLE_WHITE);
      cout << " | ";

      // Print the opponent score
      if (player1.score < opponent->score) {
        SetConsoleTextAttribute(console, CONSOLE_GREEN);
      } else if (player1.score > opponent->score) {
        SetConsoleTextAttribute(console, CONSOLE_RED);
      } else {
        SetConsoleTextAttribute(console, CONSOLE_YELLOW);
      }
      cout << opponentScore;

      // Reset the colour
      SetConsoleTextAttribute(console, CONSOLE_WHITE);
    } else if (gameMode == GameMode::IMPOSSIBLE) {
      // Create the score strings
      char p1Score[12];
      sprintf(p1Score, "     P1 Score: %d     ", player1.score);

      // Print P1 score
      SetConsoleTextAttribute(console, CONSOLE_GREEN);
      padToMiddle(p1Score);
      SetConsoleTextAttribute(console, CONSOLE_WHITE);

      // Reset the colour
      SetConsoleTextAttribute(console, CONSOLE_WHITE);
    }
  }
  void drawTitleScreen() {
    // Draw the border and score
    drawBorder();
    drawScore();
    clearPlayArea();

    // Put the cursor in the right place
    setCursorPosition(0, 9);

    // Draw the welcome
    SetConsoleTextAttribute(console, CONSOLE_BLUE);
    padToMiddle(" Welcome to:                             \n");

    // Draw the Game title
    SetConsoleTextAttribute(console, CONSOLE_WHITE);
    padToMiddle(" _______  _______  __    _  _______  __  \n");
    padToMiddle("|       ||       ||  |  | ||       ||  | \n");
    padToMiddle("|    _  ||   _   ||   |_| ||    ___||  | \n");
    padToMiddle("|   |_| ||  | |  ||       ||   | __ |  | \n");
    padToMiddle("|    ___||  |_|  ||  _    ||   ||  ||__| \n");
    padToMiddle("|   |    |       || | |   ||   |_| | __  \n");
    padToMiddle("|___|    |_______||_|  |__||_______||__| \n");

    // Draw the instructions
    setCursorPosition(0, 20);
    padToMiddle("Hit SPACE for Multiplayer");
    setCursorPosition(0, 22);
    padToMiddle("or");
    setCursorPosition(0, 24);
    padToMiddle("Enter difficulty of Single Player:");
    drawOverWidth(' ');

    // Draw the multiplayer options
    setCursorPosition(0, 26);
    SetConsoleTextAttribute(console, CONSOLE_GREEN);
    padToWidth("Easy : 1", 14);
    SetConsoleTextAttribute(console, CONSOLE_WHITE);
    cout << " | ";
    SetConsoleTextAttribute(console, CONSOLE_YELLOW);
    cout << "Medium : 2";
    SetConsoleTextAttribute(console, CONSOLE_WHITE);
    cout << " | ";
    SetConsoleTextAttribute(console, CONSOLE_RED);
    cout << "Hard : 3";
    SetConsoleTextAttribute(console, CONSOLE_WHITE);
    cout << " | ";
    SetConsoleTextAttribute(console, CONSOLE_MAGENTA);
    cout << "Survival : 4";
    SetConsoleTextAttribute(console, CONSOLE_WHITE);
  }
  void drawGameModeScreen() {
    // Reset the border
    drawScore();
    clearPlayArea();

    // Draw the appropriate game mode in the right colour
    switch (gameMode) {
    case GameMode::MULTIPLAYER:
      // Draw Text
      drawBorder();
      setCursorPosition(0, height / 2 - 3);
      padToMiddle(" _______  __   __  _______  __  \n");
      padToMiddle("|       ||  | |  ||       ||  | \n");
      padToMiddle("|    _  ||  |_|  ||    _  ||  | \n");
      padToMiddle("|   |_| ||       ||   |_| ||  | \n");
      padToMiddle("|    ___||       ||    ___||__| \n");
      padToMiddle("|   |     |     | |   |     __  \n");
      padToMiddle("|___|      |___|  |___|    |__| \n");

      // Play Song
      Beep(247, 300);
      Beep(330, 300);
      Beep(330, 300);
      Beep(370, 300);
      Beep(555, 300);

      break;
    case GameMode::EASY:
      // Draw Text
      drawBorder(CONSOLE_GREEN);
      setCursorPosition(0, height / 2 - 3);
      SetConsoleTextAttribute(console, CONSOLE_GREEN);
      padToMiddle(" _______  _______  _______  __   __  __  \n");
      padToMiddle("|       ||   _   ||       ||  | |  ||  | \n");
      padToMiddle("|    ___||  |_|  ||  _____||  |_|  ||  | \n");
      padToMiddle("|   |___ |       || |_____ |       ||  | \n");
      padToMiddle("|    ___||       ||_____  ||_     _||__| \n");
      padToMiddle("|   |___ |   _   | _____| |  |   |   __  \n");
      padToMiddle("|_______||__| |__||_______|  |___|  |__| \n");
      SetConsoleTextAttribute(console, CONSOLE_WHITE);

      // Play Song
      Beep(494, 300);
      Beep(440, 300);
      Beep(392, 200);
      Beep(440, 200);
      Beep(494, 200);
      Beep(440, 800);
      break;
    case GameMode::MEDIUM:
      // Draw Text
      drawBorder(CONSOLE_YELLOW);
      setCursorPosition(0, height / 2 - 3);
      SetConsoleTextAttribute(console, CONSOLE_YELLOW);
      padToMiddle(" __   __  _______  ______   ___   __   __  __   __  __  \n");
      padToMiddle("|  |_|  ||       ||      | |   | |  | |  ||  |_|  ||  | \n");
      padToMiddle("|       ||    ___||  _    ||   | |  | |  ||       ||  | \n");
      padToMiddle("|       ||   |___ | | |   ||   | |  |_|  ||       ||  | \n");
      padToMiddle("|       ||    ___|| |_|   ||   | |       ||       ||__| \n");
      padToMiddle("| ||_|| ||   |___ |       ||   | |       || ||_|| | __  \n");
      padToMiddle("|_|   |_||_______||______| |___| |_______||_|   |_||__| \n");
      SetConsoleTextAttribute(console, CONSOLE_WHITE);

      // Play Song
      Beep(440, 300);
      Beep(494, 300);
      Beep(440, 300);
      Beep(392, 800);
      break;
    case GameMode::HARD:
      // Draw Text
      drawBorder(CONSOLE_RED);
      setCursorPosition(0, height / 2 - 3);
      SetConsoleTextAttribute(console, CONSOLE_RED);
      padToMiddle(" __   __  _______  ______    ______   __  \n");
      padToMiddle("|  | |  ||   _   ||    _ |  |      | |  | \n");
      padToMiddle("|  |_|  ||  |_|  ||   | ||  |  _    ||  | \n");
      padToMiddle("|       ||       ||   |_||_ | | |   ||  | \n");
      padToMiddle("|       ||       ||    __  || |_|   ||__| \n");
      padToMiddle("|   _   ||   _   ||   |  | ||       | __  \n");
      padToMiddle("|__| |__||__| |__||___|  |_||______| |__|\n");
      SetConsoleTextAttribute(console, CONSOLE_WHITE);

      // Play Song
      Beep(392, 800);
      Beep(392, 300);
      Beep(370, 300);
      Beep(278, 600);
      break;
    case GameMode::IMPOSSIBLE:
      // Draw Text
      drawBorder(CONSOLE_MAGENTA);
      setCursorPosition(0, height / 2 - 3);
      SetConsoleTextAttribute(console, CONSOLE_MAGENTA);
      padToMiddle(" ______   _______  _______  _______  __   __  __  \n");
      padToMiddle("|      | |       ||   _   ||       ||  | |  ||  | \n");
      padToMiddle("|  _    ||    ___||  |_|  ||_     _||  |_|  ||  | \n");
      padToMiddle("| | |   ||   |___ |       |  |   |  |       ||  | \n");
      padToMiddle("| |_|   ||    ___||       |  |   |  |       ||__| \n");
      padToMiddle("|       ||   |___ |   _   |  |   |  |   _   | __  \n");
      padToMiddle("|______| |_______||__| |__|  |___|  |__| |__||__|\n");
      SetConsoleTextAttribute(console, CONSOLE_WHITE);

      // Play Song
      Beep(494, 800);
      Beep(440, 800);
      Beep(392, 1600);
      break;
    }
  }
  void drawGameStartScreen() {
    // Refresh the area
    drawScore();
    clearPlayArea();

    // Draw the relevant players
    player1.draw(&console);
    getOpponent()->draw(&console);
    ball.draw(&console);

    // Draw the press to start button
    drawPauseScreen();
  }
  void drawPauseScreen() {
    // Show the game menu
    setCursorPosition(0, 6);
    padToMiddle("Press SPACE to start");
    setCursorPosition(0, 7);
    padToMiddle("Press P to pause");
    setCursorPosition(0, 8);
    padToMiddle("Press ESC to exit");

    // Show how to win
    setCursorPosition(0, 10);
    SetConsoleTextAttribute(console, CONSOLE_GREEN);
    if (gameMode != GameMode::IMPOSSIBLE) {
      padToMiddle("First to 5 wins!");
    } else {
      padToMiddle("Try return the ball as many times as you can!");
    }
    SetConsoleTextAttribute(console, CONSOLE_WHITE);

    // Show the player controls
    setCursorPosition(0, 28);
    padToMiddle("Player 1 Controls: W = up, S = down");
    if (gameMode == GameMode::MULTIPLAYER) {
      setCursorPosition(0, 29);
      padToMiddle("Player 2 Controls: UP ARROW = up, DOWN ARROW = down");
    }
  }
  void drawWinnerScreen() {
    // Reset the screen
    clearPlayArea();

    // Put the cursor back in the middle
    setCursorPosition(0, 11);

    // Draw the Winner Sketch
    switch (gameState) {
    case GameState::PLAYER_1_WINNER:
      padToWidth("     _     _  ___   __    _  __    _  _______  ______    ___         \n", width / 2 - 34);
      padToWidth("    | | _ | ||   | |  |  | ||  |  | ||       ||    _ |  |   |        \n", width / 2 - 34);
      padToWidth("    | || || ||   | |   |_| ||   |_| ||    ___||   | ||  |___|        \n", width / 2 - 34);
      padToWidth("    |       ||   | |       ||       ||   |___ |   |_||_  ___         \n", width / 2 - 34);
      padToWidth("    |       ||   | |  _    ||  _    ||    ___||    __  ||   |        \n", width / 2 - 34);
      padToWidth("    |   _   ||   | | | |   || | |   ||   |___ |   |  | ||___|        \n", width / 2 - 34);
      padToWidth("    |__| |__||___| |_|  |__||_|  |__||_______||___|  |_|             \n", width / 2 - 34);
      SetConsoleTextAttribute(console, CONSOLE_GREEN);
      padToWidth(" _______  ___      _______  __   __  _______  ______      ____   __  \n", width / 2 - 34);
      padToWidth("|       ||   |    |   _   ||  | |  ||       ||    _ |    |    | |  | \n", width / 2 - 34);
      padToWidth("|    _  ||   |    |  |_|  ||  |_|  ||    ___||   | ||     |   | |  | \n", width / 2 - 34);
      padToWidth("|   |_| ||   |    |       ||       ||   |___ |   |_||_    |   | |  | \n", width / 2 - 34);
      padToWidth("|    ___||   |___ |       ||_     _||    ___||    __  |   |   | |__| \n", width / 2 - 34);
      padToWidth("|   |    |       ||   _   |  |   |  |   |___ |   |  | |   |   |  __  \n", width / 2 - 34);
      padToWidth("|___|    |_______||__| |__|  |___|  |_______||___|  |_|   |___| |__| \n", width / 2 - 34);
      SetConsoleTextAttribute(console, CONSOLE_WHITE);
      break;
    case GameState::PLAYER_2_WINNER:
      padToWidth("     _     _  ___   __    _  __    _  _______  ______    ___           \n", width / 2 - 34);
      padToWidth("    | | _ | ||   | |  |  | ||  |  | ||       ||    _ |  |   |          \n", width / 2 - 34);
      padToWidth("    | || || ||   | |   |_| ||   |_| ||    ___||   | ||  |___|          \n", width / 2 - 34);
      padToWidth("    |       ||   | |       ||       ||   |___ |   |_||_  ___           \n", width / 2 - 34);
      padToWidth("    |       ||   | |  _    ||  _    ||    ___||    __  ||   |          \n", width / 2 - 34);
      padToWidth("    |   _   ||   | | | |   || | |   ||   |___ |   |  | ||___|          \n", width / 2 - 34);
      padToWidth("    |__| |__||___| |_|  |__||_|  |__||_______||___|  |_|               \n", width / 2 - 34);
      SetConsoleTextAttribute(console, CONSOLE_GREEN);
      padToWidth(" _______  ___      _______  __   __  _______  ______      _______  __  \n", width / 2 - 34);
      padToWidth("|       ||   |    |   _   ||  | |  ||       ||    _ |    |       ||  | \n", width / 2 - 34);
      padToWidth("|    _  ||   |    |  |_|  ||  |_|  ||    ___||   | ||    |____   ||  | \n", width / 2 - 34);
      padToWidth("|   |_| ||   |    |       ||       ||   |___ |   |_||_    ____|  ||  | \n", width / 2 - 34);
      padToWidth("|    ___||   |___ |       ||_     _||    ___||    __  |  | ______||__| \n", width / 2 - 34);
      padToWidth("|   |    |       ||   _   |  |   |  |   |___ |   |  | |  | |_____  __  \n", width / 2 - 34);
      padToWidth("|___|    |_______||__| |__|  |___|  |_______||___|  |_|  |_______||__| \n", width / 2 - 34);
      SetConsoleTextAttribute(console, CONSOLE_WHITE);
      break;
    case GameState::CPU_WINNER:
      padToMiddle(" _     _  ___   __    _  __    _  _______  ______    ___  \n");
      padToMiddle("| | _ | ||   | |  |  | ||  |  | ||       ||    _ |  |   | \n");
      padToMiddle("| || || ||   | |   |_| ||   |_| ||    ___||   | ||  |___| \n");
      padToMiddle("|       ||   | |       ||       ||   |___ |   |_||_  ___  \n");
      padToMiddle("|       ||   | |  _    ||  _    ||    ___||    __  ||   | \n");
      padToMiddle("|   _   ||   | | | |   || | |   ||   |___ |   |  | ||___| \n");
      padToMiddle("|__| |__||___| |_|  |__||_|  |__||_______||___|  |_|      \n");
      SetConsoleTextAttribute(console, CONSOLE_RED);
      padToMiddle("           _______  _______  __   __  __                  \n");
      padToMiddle("          |       ||       ||  | |  ||  |                 \n");
      padToMiddle("          |       ||    _  ||  | |  ||  |                 \n");
      padToMiddle("          |       ||   |_| ||  |_|  ||  |                 \n");
      padToMiddle("          |      _||    ___||       ||__|                 \n");
      padToMiddle("          |     |_ |   |    |       | __                  \n");
      padToMiddle("          |_______||___|    |_______||__|                 \n");
      SetConsoleTextAttribute(console, CONSOLE_WHITE);

      break;
    }
  }
  void drawImpossibleModeScore() {
    // Create the string
    char p1Score[21];
    sprintf(p1Score, "Your Score was: %d", player1.score);

    // Print the string
    setCursorPosition(0, 9);
    padToMiddle(p1Score);
  }

 private:  // Drawing Utilities
  void drawOverWidth(char inputChar) {
    for (int i = 0; i < width; i++) {
      cout << inputChar;
    }
  }
  void padToWidth(const char *inputString, int padding) {
    if (inputString) {
      for (int i = 0; i < padding; i++) {
        cout << ' ';
      }
      cout << inputString;
    }
  }
  void padToMiddle(const char *inputString) {
    int length = strlen(inputString);
    padToWidth(inputString, (width / 2) - (strlen(inputString) / 2));
  }
  void clearPlayArea() {
    setCursorPosition(0, 3);
    for (int i = 3; i < height; ++i) {
      drawOverWidth(' ');
      setCursorPosition(0, i + 1);
    }
  }

 private:  // Songs
  void playThemeSong() {
    Beep(220, 300);
    Beep(294, 300);
    Beep(294, 300);
    Beep(370, 300);
    Beep(494, 300);
    Beep(370, 300);
    Beep(440, 800);
  }
  void playWinningSong() {
    if (gameState == GameState::PLAYER_1_WINNER || gameState == GameState::PLAYER_2_WINNER) {
      Beep(440, 300);
      Beep(494, 300);
      Beep(440, 300);
      Beep(370, 300);
      Beep(392, 300);
      Beep(370, 300);
      Beep(330, 800);
    } else {
      Beep(392, 300);
      Beep(370, 300);
      Beep(247, 1600);
    }
  }

 private:  // Console Utility Methods
  void setGameArea(int height, int width) {
    // Get the output console object and set its size
    SMALL_RECT consoleRectangle = {short(0), short(0), (short)width, short(height)};
    console = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleWindowInfo(console, TRUE, &consoleRectangle);

    // Set the max size of the window buffer
    COORD consoleSize;
    consoleSize.X = width;
    consoleSize.Y = height;
    SetConsoleScreenBufferSize(console, consoleSize);

    // Set the title of the console
    SetConsoleTitle("Pong! - A fun interactive demo by Mitch Coyer");

    // Get the HWND of the console
    windowsHandle = GetForegroundWindow();
  }
  bool isActiveWindow() {
    HWND currentWindow = GetForegroundWindow();
    if (currentWindow == windowsHandle) {
      return true;
    } else {
      return false;
    }
  }
  void hideCursor() {
    // Hide Console Cursor
    CONSOLE_CURSOR_INFO cursor;
    cursor.dwSize = 10;
    cursor.bVisible = false;
    SetConsoleCursorInfo(console, &cursor);
  }
  void setCursorPosition(int _x, int _y) {
    COORD cursor;
    cursor.X = _x;
    cursor.Y = _y;
    SetConsoleCursorPosition(console, cursor);
  }
  float randomFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
  }

 public:  // Data
  //  Game Data
  int       width, height;                       // The width and height of the console
  HANDLE    console;                             // The handle to the current console
  HWND      windowsHandle;                       // The HWND handle to the console window
  GameMode  gameMode = GameMode::NOT_STARTED;    // Enum to track the game mode
  GameState gameState = GameState::NOT_STARTED;  // Enum to keep track of the play state
  TIME      loopStartTime = NOW;                 // A timer stamp to keep track of the execution loop

  // Player Objects
  Player player1;
  Player player2;
  Player cpu;

  // Ball object
  Ball  ball;
  float maxXSpeed = 3;    // Maximum ball speed in x direction
  float minXSpeed = 2;    // Minimum ball speed in x direction
  float maxYSpeed = 1.5;  // Maximum ball speed in y direction
  float minYSpeed = 0;    // Maximum ball speed in y direction
};

int main() {
  // Grid starts in the top left at (0,0) and ends at (79,35)
  // Playable area is 79 x 31 starting at (2,0) and ending at (79, 34)

  Game game(79, 35);
  while (game.runGame()) {
  }
}

#endif  // __cplusplus
#endif  //_MSC_VER