# Pong - An interactive demo

![Main Menu](Images\example_pong.png)

## Overview

This is a side project I was asked to complete for evaluation. The requirement was to build Pong, a retro game from 1972 launched commissioned originally by atari as a training exercise then later put into production as one of the first successful video games. I wanted my recreation to feel "from the era", so to achieve this I built the game in the windows console complete with sound to achieve the look and feel of a retro game.

## How to compile

The game is contained in a single .cpp file called __`Pong.cpp`__ due to its simple nature. In this file are all of the class definitions, the logic of the game, and the windows functions used for drawing.

Since the game uses functions on a windows console to constantly refresh the current screen buffer, it is currently limited to windows environments as it uses windows specific functions from __windows.h__ and the __User32.lib__. If you wish to port this to a different OS you will have to change the various functions that are used to write to the conosle. These are mostly concentrated in the drawing utilities part of the Game class, however there are some scattered in the Player and Ball class as well.

For windows though, the compiler chosen to create the .exe in this repo was MSVC 64bit for x86 processors. This should allow the .exe to be opened on most modern windows computers. However if you want to compile from source the following command was used (from visual studio) to create the current .exe, modify this as you see fit.

``` cmd

cl.exe /Zi /EHsc /Fe: "Pong.exe" "Pong.cpp"

```

> Please note that you need to have either __Visual Studio__ or __Microsoft Build Tools__ installed and working to use the __`cl`__ command. An easy way to run the command is to use the [Developer Command Prompt](https://docs.microsoft.com/en-us/dotnet/framework/tools/developer-command-prompt-for-vs) to run the command above.

## How to Play

### Game Modes

![Main Menu](Images\Main-Menu.JPG)

The game has serval modes you can play. These are selected from the starting menu using the suggested keys. Each mode offers a unique playing experience from multiplayer to various computer based games. Below is the list of modes and a brief of how they operate.

- __Multiplayer__ - for playing against a friend.
- __Easy__ - Sets the computer to an easy to beat difficulty.
- __Medium__ - Sets the computer to a moderate to beat difficulty.
- __Hard__ - Sets the computer to hard to beat difficulty.
- __Survival__ - Sets the mode to impossible to beat, see how long you can last!

You can quit the game from the main menu by hitting __`ESC`__ or by closing the console window.

### Game Start and Controls

![PVP](Images\PVP.JPG)

Player 1 is to the left of the screen, while Player 2 is to the right. To move player 1, you need to press __`W`__ to go up and __`S`__ to go down, while Player 2 uses the __`UP ARROW`__ to go up and the __`DOWN ARROW`__ to go down. When playing against the computer in any computer based game mode, the player will be player one on the left of the screen.

At the beginning of the game you can start anytime by pressing the __`SPACE`__ button. This will randomly choose a direction for the ball and starts play.

During the play of the game you can press __`P`__ to pause play where it is or __`ESC`__ to exit back to the main menu.

### Scoring points

![PVP](Images\End.JPG)

In all modes except survival the aim is to hit the ball back towards your opponent and prevent it from hitting the wall behind you. To score a point, all you 1need to do is hit the ball past the other player into their the end zone. When a player or the computer has reached __5 points__ the game is won and the winner screen is shown.

For the survival mode there is no end. The score is how many times you are able to return the ball before missing it. The higher the better!
