# Remote-Chess
Connecting friends across great distances during this pandemic has become a challenge due to lockdowns and quarantine.
Remote Chess is an IoT chessboard that allows for online play with anyone, anywhere, that keeps tactile feel of a real 
physical chessboard. Connect with friends who share your love for chess, sharpen your skills against an AI, or learn the 
game for the first time, all while preserving the much-desired hands-on experience. 

![Title Image](/Images/title_image.jpg)
### Video
https://youtu.be/B_T1OviKhvA

## Overview
A Remote-Chessboard contains an array of hall sensors underneath the surface of the
board. These hall sensors, combined with magnets contained in each of the pieces, allows for the
board to detect movement.
Once a user submits their move, LEDs will light up on the opponent’s board representing the move
just made, and they will then be prompted to “make your move” on their board.

## Features
We designed the Remote Chessboards with the spirit of chess in mind, aiming to create an intuitive
and satisfying user experience:
- When picking up a piece, receive immediate visual feedback as to where it can move
- Board will light up when opponent makes their move, signaling which piece was moved and to where
- King square will light up when in check
- Supports castling, en passant, and promotions
- At the end of a game, the board will light up indicating a win, a loss, or if a player resigned
- Play versus Stockfish (an AI opponent)
- Supports local play

![Example of all legal moves for the black queen](/Images/moves.jpg)

In order to support being able to play with anyone, anywhere, Remote-Chess has various social features 
helping you to connect to your opponent and start playing:
- Built-in friends system, invite anyone else with a board to become your friend
- Able to view and accept or decline friend invites
- Send game invites to any friend, they will receive a notification to play which they can accept or decline
- Can create a game and receive a code to send to someone who is not your friend
- If given a game code, able to enter it using a built-in keyboard to join a game

## Design
To make remote chess a reality, we underwent an intense development process to fabricate and write all the physical parts and software necessary. 
<br />

![A Solidworks rendering of a Remote Chessboard](/Images/solidworks.jpg)
### Hardware
The boards have 8 custom white PCBs containing 64 Hall sensors to detect where pieces are on the board.
These PCBs sit inside of an interlocking cell structure holding the sensors in the center of the squares,
illuminated from below by the LEDs. The cell structure is held in place by a sheet of reflective white acrylic
that is fastened to the bottom plate of the enclosure by 4 rubber feet. To the side is space for the user
interaction elements (LCD and buttons), as well as the controlling electronics. Dozens of wires connect the
8 PCBs, LEDs, buttons, and LCD to each other and the MSP432. 

We chose a 4x20 LCD screen with 5 buttons to make a flexible and intuitive interface in spite of its tiny size.
Additionally on the side is a port for DC power and a latched switch to turn it on and off. The housing is fully enclosed by the diffuse acrylic layer
and glossy black plastic, with etchings on the edges to show the file/rank notation (A-H, 1-8) on the side of
the play area.
### Software
We used a fully custom ARM RTOS to allow for multithreading and resource management on
the provided MSP432. We programmed the board logic in C++14 with small amounts in C and assembly.
An event-based logic system handles how the board reacts to pieces moving - when a piece moves, a
long series of complicated conditions are performed to calculate which squares should be lit and how. 

We wrote a hardware interface layer for the sensors, LEDs, buttons, and LCD, which includes sub-microsecond
timing on the LEDs and complicated control logic for the LCD. Additionally, we created a large controlling
finite state machine that handles the network calls, menus, button presses, and user LCD display. 

For the server-side implementation, we deployed a Python Flask server on an AWS EC2 instance. The MSP432
communicates with the server by sending HTTP requests via the CC3100 Wi-Fi module. When sending a
request, the MSP432 calls a function from our "ChessServer" networking interface and fills it with
appropriate parameters. After receiving a request, the server will ensure the operation is allowed, then
return an appropriate response. The MSP432 then receives this response, parses it, copies any requested
information into a string reference, and finally returns a value depending on the success of the operation.
This value will contain data that was retrieved from the server, such as legal moves, checkmate status,
king location, etc. 

To implement how pieces move on the chess board, our server uses the Python chess
library, which performs move validation, provides game state information, and helps us make a list of all
legal moves. To store user information and keep track of game states, we used Google Firestore as our
database. If a board disconnects or turns off, all the information is stored in the cloud, allowing the board
to return to its the correct state upon reconnecting or powering back on.

## Acknowledgements and Credits
This project was created to compete in the IoT Design Competition 2021 hosted by the IoT Students Club at the University of Florida. 
We wanted to thank them and their sponsors for making this competition possible!

Sponsors:
- Warren B. Nelms Institute for the Connected World
- Texas Instruments
- Microsoft

![End of a local play game](/Images/local_endgame.jpg)

#### Designed by Dylan Ferris and Austin Welch
