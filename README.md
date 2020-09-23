# The Computer Maze Assignment
Using code and creating an external tool such as a 3d game engine to hep solve a computer maze that is randomly generated.

## Introduction
At my university we were given a new assignment and to my surprise it was a randomly generated computer maze that would generate a random maze depending on the given seed. In this case the seed would be your student ID.

This maze would generate a 10x10x5 grid a total of 500 rooms to visit and each direction/corridor being locked behind doors.

![Visualization of the full maze empty](https://i.imgur.com/bijxKH6.png)

Doors can be unlocked by finding keys on the floor and then interacting with them to pick up them up to be stored within your backpack. Or you can brute force the door and determine its key.

![Image Sample Maze Visualizer not empty](https://i.imgur.com/ImBZX8Q.png)

There are events that occur during the maze that you can attend or ignore. Anything from lectures to reading books on certain topics. Reading books such as Philosophy will level you down while reading a book in AI would level you up.

You have access to the bot’s/student information that roams this maze.
Each move is logged and each action you do takes a move.
You are to take control of the bots decision making and determine how it moves and reacts to the environment. 

## The goal
The goal is to get the highest amount of points or score in this maze with the littlest number of moves made possible and levelling up your bot (student) to the highest it can be.

![Goal Sample Vizualiser](https://i.imgur.com/KunXgnX.png)

## Extras
Traversing the maze is no easy task as there are teleport zones on the outside the bounds of the maze, they will teleport your bot(student) to a random floor. The highest floor on the maze will not report back any information of your location, You must devise a way of navigating it.

There are also one way paths where that your bot can get trapped and must find a different exit.

The only way for a bot to move up or down on a floor is on the 4 corners of the maze. These stairs provide limited access and can only let you go up or down on the maze from certain floors.

## Giving this ago
You can download this project and edit the visual studio project file named “Computer Maze Completed.cpp” and editing the function named YourMove()
Most of the code implemented in this was put in at incredible speed due to timeline constraints and had to skip a few things.

## Running my solution

If you want to run my solution you can clone the repo by doing:

`git clone https://github.com/Auzlex/The-Computer-Maze-Assignment/`

Once all the files have downloaded open these files in the following order:

1. Run the "Setup UDP Server And ComputerMaze.bat" this will run the server and then the client visualizer

**Make sure you wait for the visualizer to load first or it will not have time to initialize the maze on the visualizer**

2. Once it has fully loaded you can then begin to run the “runresolver.bat” which will run the exe that I have made to solve the maze.

One last thing the ports **Required** is that ports that need to be open for loopback traffic on your machine is **6532** and **6533**. Used for communication between server, resolver and visualizer
