/* ------------------------------------------------------------------------------

	 Assignment 2: "The Computer Maze Assignment"
	 Issued By Martin Serpell
	 Based code Issued By Martin Serpell
	 Assignment Goal Achieved and Scripted by Charles Edwards

	 Assignment Start Date:				02/02/2020
	 Assignment Last Revision:			12/04/2020

	 Description:

	 Code Achieves 60 marks and stops gets roughly 189 score

------------------------------------------------------------------------------ */
#define _CRT_SECURE_NO_WARNINGS
#define  _WINSOCK_DEPRECATED_NO_WARNINGS 
#include <winsock2.h>
#include <time.h>
#include "Ws2tcpip.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <Windows.h>

#pragma comment(lib, "wsock32.lib")

#define STUDENT_NUMBER		"19008097"//"19003931"//"19008097"
#define STUDENT_FIRSTNAME	"Charles"//"Jacob"//"Charles"
#define STUDENT_FAMILYNAME	"Edwards"//"Allen"//"Edwards"

#define IP_ADDRESS_SERVER	"127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   5000
#define MAX_STRING_SIZE   200
#define MAX_NO_TOKENS     50

#define MAX_ITEMS_IN_ROOM		20
#define MAX_ITEMS_IN_BACKPACK	50

/********************* Define Options *********************/

#pragma region DEFINED OPTIONS

// move
#define OPTION_MOVE_NORTH 1
#define OPTION_MOVE_SOUTH 2
#define OPTION_MOVE_EAST 3
#define OPTION_MOVE_WEST 4
#define OPTION_MOVE_UP 5
#define OPTION_MOVE_DOWN 6

// unlocks
#define OPTION_UNLOCK_NORTH 7
#define OPTION_UNLOCK_SOUTH 8
#define OPTION_UNLOCK_EAST 9 
#define OPTION_UNLOCK_WEST 10
#define OPTION_UNLOCK_UP 11
#define OPTION_UNLOCK_DOWN 12

// actions
#define OPTION_BASE_FOR_READS 200
#define OPTION_BASE_FOR_PICKUPS 500
#define OPTION_BASE_FOR_DROPS 800
#define OPTION_BASE_FOR_DOS 1100
#define OPTION_BASE_FOR_EVENTS 1300

#pragma endregion

#pragma region ENUMS: DIRECTIONS, DIRECTION_POSSIBLE, ITEM TYPES, ITEM, STUDENT, ROOM, BACKPACK, TYPEOFROOM

enum directions
{
	DIRECTION_NORTH = 0,
	DIRECTION_SOUTH = 1,
	DIRECTION_EAST = 2,
	DIRECTION_WEST = 3,
	DIRECTION_UP = 4,
	DIRECTION_DOWN = 5
};

enum direction_possible
{
	DIRECTION_NOT_PRESENT = -1,
	DIRECTION_LOCKED = 0,
	DIRECTION_OPEN = 1
};

enum item_types
{
	ITEM_NONE = 0,
	ITEM_BOOK = 1,
	ITEM_JUNK = 2,
	ITEM_EQUIPMENT = 3,
	ITEM_MANUSCRIPT = 4,
	ITEM_TEST = 5,
	ITEM_OTHER = 10
};

struct Item
{
	int  number;
	int  value;
	int  volume;
};

struct Student
{
	int level;
	int rooms_visited;
	int doors_openned;
	int number_of_moves;
	int score;
};

struct Room
{
	char name[5];
	int  type;
	int  direction[6];
	int  number_of_keys;
	int  keys[4];
	int  number_of_items;
	Item items[MAX_ITEMS_IN_ROOM];
};

struct Backpack
{
	int number_of_items;
	Item items[MAX_ITEMS_IN_BACKPACK];
};

enum typeofroom
{
	ROOM_NONE = 0,
	ROOM_LECTURE_SMALL = 1,
	ROOM_LECTURE_MEDIUM = 2,
	ROOM_LECTURE_LARGE = 3,
	ROOM_CORRIDOR = 4,
	ROOM_LAB_SMALL = 5,
	ROOM_LAB_MEDIUM = 6,
	ROOM_LAB_LARGE = 7,
	ROOM_MEETING_ROOM = 8,
	ROOM_SEMINAR = 9,
	ROOM_HIVE = 10, //one per floor
	ROOM_COFFEESHOP = 11, //one
	ROOM_LIBRARY = 12, //only one
	ROOM_SHOP_SELL = 13,
	ROOM_SHOP_BUY = 14,
	ROOM_SHOP_BUYSELL = 15,
	ROOM_OFFICE = 16, //maybe only one door
	ROOM_LOBBY = 17, //Only one
	ROOM_EXIT = 18, //only one
	ROOM_STAIRS = 19,
	ROOM_ENTRANCE = 20 //only one
};

#pragma endregion

#pragma region CLIENT CODE VARIABLES

#define MAX_OPTIONS	50

int number_of_options;
int options[MAX_OPTIONS];

Student student;
Room room;
Backpack backpack;

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;
SOCKADDR_IN client_unity_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;

char InputBuffer[MAX_BUFFER_SIZE];

char Tokens[MAX_NO_TOKENS][MAX_STRING_SIZE];

char text_student[1000];
char text_backpack[1000];
char text_room[1000];
char text_keys[1000];
char text_items[1000];
char text_options[1000];

void sentOption(int option, int key)
{
	char buffer[100];

	sprintf(buffer, "Option %d, %x", option, key);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));
}

#pragma endregion

/*************************************************************/
/********* Your tactics code starts here *********************/
/*************************************************************/

std::string prefix = "[DEBUG-GENERAL]:";

// tracking rooms
#define MAX_ROOMS  500
char rooms_visited[MAX_ROOMS][5];
ULONG64 rooms_recorded = 0;

#pragma region doors attempted array

// struct used to store doors attempted data
struct doorsAttemptedData {

	// posistion of the door
	int floor;
	int x;
	int y;

	// direction because there can be many doors on the same square
	int direction;

	// the key
	int keyValue;

	doorsAttemptedData(int f, int x, int y, int d, int k) : floor(f), x(x), y(y), direction(d), keyValue(k) { }

};

// vector array used to store attempted doors to not try at again
std::vector<doorsAttemptedData> attemptedDoors;

#pragma endregion

#pragma region Read Write Data key dat vector and struct data object

// key data struct that will store key information
struct keyDataObject {

	// posistion of the door
	int floor;
	int x;
	int y;

	// direction because there can be many doors on the same square
	int direction;

	// the key
	int keyValue;

	// constructor
	keyDataObject(int f, int x, int y, int d, int k) : floor(f), x(x), y(y), direction(d), keyValue(k) { }

};

// our vector array used to store our key data
std::vector<keyDataObject> keyData;

// method to return the keydata file name
std::string keyDataFileName()
{
	return "keydata.csv";
}

// this method is called when we want to check if the file name exists
// this is the fastest way to check if a file exists.
inline bool keyDatafileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

// method used to save key data vector array with object into a csv file
void saveToKeyDataFile()
{
	// check if the file exists
	if (keyDatafileExists(keyDataFileName()))
	{
		// delete the old file and write
		remove(keyDataFileName().c_str());

		// create an OFStream
		std::ofstream myfile;
		// open the file
		myfile.open(keyDataFileName(), std::ios_base::app);

		// for every item
		for (size_t i = 0; i < keyData.size(); i++)
		{
			// write save it a line
			keyDataObject KDO = keyData[i];
			myfile << KDO.floor << "," << KDO.x << "," << KDO.y << "," << KDO.direction << "," << KDO.keyValue << "\n";
		}

		// close the file
		myfile.close();
	}
	else
	{
		// new OFstream
		std::ofstream myfile;

		//open the file
		myfile.open(keyDataFileName(), std::ios_base::app);

		// for every line
		for (size_t i = 0; i < keyData.size(); i++)
		{
			// write
			keyDataObject KDO = keyData[i];
			myfile << KDO.floor << "," << KDO.x << "," << KDO.y << "," << KDO.direction << "," << KDO.keyValue << "\n";
		}

		// close
		myfile.close();
	}
}

#pragma endregion

// the general purpose key array used to brute force doors we couldnt find the key for
int generalPurposeKeyArray[20] = {

	0x200F,
	0xBCD5,
	0xE4A8,
	0x711A,

	0x29FC,
	0xA110,
	0xBE27,
	0x9F4A,

	0x8E28,
	0xD75B,
	0x090D,
	0x0172,

	0x8F32,
	0x1AE1,
	0x0EB9,
	0x43AB,

	0xD2E7,
	0xC567,
	0x8FD4,
	0xFD5A

};

// try to brute foce doors variables
int tryKey = -1;
int keyUnlockAttempt = 0;
int keyUnlockGeneralPurposeAttempt = 0;
bool isBruteForcing = false;

// direction debug text
std::string directionText;
std::string directionUnlockText;

#pragma region rooms_visited and initialize function

// int array of rooms
int rooms_visisted[5][10][10]; // bool

// called when we want to initialize a number for every 
void initialize_room_visits()
{
	int floor;
	int x;
	int y;

	// create the 3d array of a 10x10x5 grid
	for (int floor = 0; floor < 5; floor++)
	{
		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 10; y++)
			{
				// set the room vists to 0
				rooms_visisted[floor][x][y] = 0;
			}
		}
	}
}

#pragma endregion

#pragma region Fetch Location, currentFloor, currentX an Y

// main values that will be used to help compare or check where we are in the maze
int currentFloor = 0;
int currentX = 0;
int currentY = 0;

// fetch location method used to get the current room information or the bots location
void FetchLocation()
{
	// adjust the bot tracking value to detemrine what floor we are on
	int floor;
	int X;
	int Y;

	// on successful decode
	if (sscanf(room.name, "%1dY%1d%1d", &floor, &X, &Y) == 3)
	{
		// update values
		currentFloor = floor;
		currentX = X;
		currentY = Y;

		// update rooms visted
		rooms_visisted[floor][X][Y]++;

		// debug show location
		printf("%s Current location: Floor: %d X:%d Y:%d Room.name: %s Room.type: %d\n", prefix.c_str(), currentFloor, currentX, currentY, room.name, room.type);
	}
	else
	{
		// debug error
		printf("%s Unable to get room data (MISSING INFORMATION!)\n", prefix.c_str());
	}

}

#pragma endregion

#pragma region SendDesiredOption, last Action and desired direction state

// main boolean value used to prevent additional commands from being sent
bool haveWeSentOption = false;

// last action
int lastAction = -1;

// main value used to tell which direction our bot is facing
int desiredDirectionState = OPTION_MOVE_SOUTH;

// called when we want to send a desired option instead of the other one this has a safe guard check in place
void SendDesiredOption(int option, int sentKey = 0)
{
	// if we have sent an option then we dont want to send another this is to prevent overloading the server with to much data
	// it will also prevent our requests from being ignored
	if (!haveWeSentOption)
	{
		// set that we have sent an option to true
		haveWeSentOption = true;

		if (currentFloor == 3 && option == OPTION_MOVE_UP)
		{
			desiredDirectionState = OPTION_MOVE_DOWN;
			option = OPTION_MOVE_DOWN;
		}

		// set our last action variable
		if (option != -1)
		{
			lastAction = option;
		}



		// send the new option
		sentOption(option, sentKey);

		// switch case
		switch (option)
		{
			// move options
		case OPTION_MOVE_NORTH:
			directionText = "OPTION_MOVE_NORTH";
			break;
		case OPTION_MOVE_SOUTH:
			directionText = "OPTION_MOVE_SOUTH";
			break;
		case OPTION_MOVE_EAST:
			directionText = "OPTION_MOVE_EAST";
			break;
		case OPTION_MOVE_WEST:
			directionText = "OPTION_MOVE_WEST";
			break;
		case OPTION_MOVE_UP:
			directionText = "OPTION_MOVE_UP";
			break;
		case OPTION_MOVE_DOWN:
			directionText = "OPTION_MOVE_DOWN";
			break;

			// unlock options
		case OPTION_UNLOCK_NORTH:
			directionText = "OPTION_UNLOCK_NORTH";
			break;
		case OPTION_UNLOCK_SOUTH:
			directionText = "OPTION_UNLOCK_SOUTH";
			break;
		case OPTION_UNLOCK_EAST:
			directionText = "OPTION_UNLOCK_EAST";
			break;
		case OPTION_UNLOCK_WEST:
			directionText = "OPTION_UNLOCK_WEST";
			break;
		case OPTION_UNLOCK_UP:
			directionText = "OPTION_UNLOCK_UP";
			break;
		case OPTION_UNLOCK_DOWN:
			directionText = "OPTION_UNLOCK_DOWN";
			break;

			// misc
		case OPTION_BASE_FOR_READS:
			directionText = "OPTION_BASE_FOR_READS";
			break;
		case OPTION_BASE_FOR_PICKUPS:
			directionText = "OPTION_BASE_FOR_PICKUPS";
			break;
		case OPTION_BASE_FOR_DROPS:
			directionText = "OPTION_BASE_FOR_DROPS";
			break;
		case OPTION_BASE_FOR_DOS:
			directionText = "OPTION_BASE_FOR_DOS";
			break;
		case OPTION_BASE_FOR_EVENTS:
			directionText = "OPTION_BASE_FOR_EVENTS";
			break;

			// this is our own option meaning do nothing
		case -1:
			directionText = "OPTION_NONE no command was sent this move";
			break;

		}

		// debug log it
		printf("%s sent option: number: %d, move name: %s, last move: %d\n", prefix.c_str(), option, directionText.c_str(), lastAction);

	}
}

#pragma endregion

#pragma region Attend Event

// method to handle attending events
int HandleAttendEvent()
{
	int move = -1;

	int i;
	int option;
	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if (option >= OPTION_BASE_FOR_EVENTS)
		{
			move = option;
			return move;
		}
	}

	return move;
}

#pragma endregion

#pragma region Pick up stuff and itemValue, Read book

// used to check and item for its value by comparing to the room items array
int itemValue(int item)
{
	int value = -1;
	int i;

	for (i = 0; i < room.number_of_items; i++)
	{
		if (item == room.items[i].number)
		{
			value = room.items[i].value;
		}
	}

	return value;
}

// this function handles reading books
int HandleReadBook()
{
	int move = -1;
	int item;

	int i;
	int option;
	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];
		if ((option >= OPTION_BASE_FOR_READS) && (option < OPTION_BASE_FOR_PICKUPS))
		{
			item = option - OPTION_BASE_FOR_READS;

			// check the books value is greater than 2
			if (itemValue(item) > 2)
			{
				move = option;
				return move;
			}
		}
	}

	return move;
}

// this function handles pickups
int HandlePickup()
{
	int move = -1;

	int i;
	int option;
	int item;

	for (i = 0; i < number_of_options; i++)
	{
		option = options[i];

		if ((option >= OPTION_BASE_FOR_PICKUPS) && (option < OPTION_BASE_FOR_DROPS))
		{
			item = option - OPTION_BASE_FOR_PICKUPS;

			// check item value is greater than 2
			if (itemValue(item) > 2)
			{
				move = option;
				return move;
			}
		}
	}

	return move;
}

#pragma endregion

#pragma region Desired Direction, Validate Desired Direction, Randomize Direction

int ValidateDesiredDirection()
{
	std::string validation;
	std::string direction;
	int directionCheck = room.direction[desiredDirectionState - 1];

	switch (directionCheck)
	{
	case DIRECTION_OPEN:
		validation = "DIRECTION_OPEN";
		break;
	case DIRECTION_NOT_PRESENT:
		validation = "DIRECTION_NOT_PRESENT";
		break;
	case DIRECTION_LOCKED:
		validation = "DIRECTION_LOCKED";
		break;
	}

	switch (desiredDirectionState)
	{
		// move options
	case OPTION_MOVE_NORTH:
		direction = "OPTION_MOVE_NORTH";
		break;
	case OPTION_MOVE_SOUTH:
		direction = "OPTION_MOVE_SOUTH";
		break;
	case OPTION_MOVE_EAST:
		direction = "OPTION_MOVE_EAST";
		break;
	case OPTION_MOVE_WEST:
		direction = "OPTION_MOVE_WEST";
		break;
	case OPTION_MOVE_UP:
		direction = "OPTION_MOVE_UP";
		break;
	case OPTION_MOVE_DOWN:
		direction = "OPTION_MOVE_DOWN";
		break;
	}

	printf("%s Validating direction: %s result: %s\n", prefix.c_str(), direction.c_str(), validation.c_str());
	return directionCheck;
}

int lastRandomAction = -1;

void RandomizeDirection()
{
	int min = 1;
	int max = 4; // because we dont want to randomize up or down

	if (room.type == ROOM_STAIRS && student.rooms_visited >= 90)
		max = max + 2;

	int randNum = rand() % (max - min + 1) + min;

	// if we dare to move to the forth floor dont bother ignore the shadow realm
	if (room.type == ROOM_STAIRS && currentFloor == 3 && randNum == OPTION_MOVE_UP) //currentFloor == 3
		randNum = OPTION_MOVE_DOWN;

	desiredDirectionState = randNum;
	lastRandomAction = randNum;
}

int GetCurrentRoomNumberOfVisits()
{
	int visits = 1;
	visits = rooms_visisted[currentFloor][currentX][currentY];
	return visits;
}

int GetDesiredDirectionPredictedRoomVisits()
{
	int visits = 0;

	// depending on the option currently being sent we want to return the number of visits ahead of its direction/current posistion
	switch (desiredDirectionState)
	{
	case OPTION_MOVE_NORTH:
		visits = rooms_visisted[currentFloor][currentX - 1][currentY];
		break;
	case OPTION_MOVE_SOUTH:
		visits = rooms_visisted[currentFloor][currentX + 1][currentY];
		break;
	case OPTION_MOVE_EAST:
		visits = rooms_visisted[currentFloor][currentX][currentY + 1];
		break;
	case OPTION_MOVE_WEST:
		visits = rooms_visisted[currentFloor][currentX][currentY - 1];
		break;
	case OPTION_MOVE_UP:
		visits = rooms_visisted[currentFloor + 1][currentX][currentY];
		break;
	case OPTION_MOVE_DOWN:
		visits = rooms_visisted[currentFloor - 1][currentX][currentY];
		break;
	}

	return visits;
}

#pragma endregion

/*

	MAIN BOT PROPERTIES AND ACTIONS SWITCHS
	THESE BOOLEANS CHANGE THE WAY THIS BOT OPERATORS
	exploreModeOnly == true:BOT WILL NOT BRUTE FORCE

*/

// are we allowed to brute froce?
bool developedModeEnabled = false; // this is off when we have finished the program to prevent any marks being deducted
bool exploreModeOnly = true;
bool performRoomAnalysis = false; // set to true when we have made a dead end so we want to check all directions
bool useGeneralPurposeKeys = true;

bool taskCompleted = false; // do not touch used to stop the code if it has reached the mark desired
int nextMoveDelay = 5; // this is the delay value used to wait for next cycle lower this value to increase bot completetion speed increase to have accuracte display in maze visualizer

#pragma region HaveWeTriedThisDoor, CheckAttemptedDoorExists

// used to check if we have attempted to open this door
bool HaveWeTriedThisDoor()
{
	int i;
	for (int i = 0; i < attemptedDoors.size(); i++)
	{
		if ((attemptedDoors[i].floor == currentFloor) && (attemptedDoors[i].x == currentX) && (attemptedDoors[i].y == currentY) && (attemptedDoors[i].direction == desiredDirectionState))
		{
			return true;
		}
	}

	return false;
}

// used to check if we have added this item to the array list of attempted doors
bool CheckAttemptedDoorExists(int f, int x, int y, int d)
{
	int i;
	for (int i = 0; i < attemptedDoors.size(); i++)
	{
		if ((attemptedDoors[i].floor == f) && (attemptedDoors[i].x == x) && (attemptedDoors[i].y == y) && (attemptedDoors[i].direction == d))
		{
			return true;
		}
	}

	return false;
}

#pragma endregion

#pragma region HandleDoorObstacle

// used to handle door unlocking when we encounter a locked door
void HandleDoorObstacle()
{

	int keyArraySize = keyData.size() - 1;//sizeof(keyArray) / sizeof(keyArray[0]);
	int generalPurposeKeyArraySize = sizeof(generalPurposeKeyArray) / sizeof(generalPurposeKeyArray[0]);

	if (keyUnlockAttempt <= keyArraySize && !HaveWeTriedThisDoor())
	{
		if ((keyData[keyUnlockAttempt].floor == currentFloor) && (keyData[keyUnlockAttempt].x = currentX) && (keyData[keyUnlockAttempt].y == currentY) && (keyData[keyUnlockAttempt].direction == desiredDirectionState))
		{
			SendDesiredOption(6 + desiredDirectionState, keyData[keyUnlockAttempt].keyValue);
			printf("UNLOCKING DOOR IN DESIRED DIRECTION KEY VALUE %04X\nWITH ATTEMPT: %d\nAT ROOM COORDS: F:%d X:%d Y:%d\nKEY DATA COORDS: F:%d X:%d Y:%d\nCURDIRECTION:%d\nKEYDIRECTION:%d\n\n", keyData[keyUnlockAttempt].keyValue, keyUnlockAttempt, currentFloor, currentX, currentY, keyData[keyUnlockAttempt].floor, keyData[keyUnlockAttempt].x, keyData[keyUnlockAttempt].y, desiredDirectionState, keyData[keyUnlockAttempt].direction);

			keyUnlockAttempt++;
			return;
		}

		/*if (keyArray[keyUnlockAttempt][0] != -1)
		{
			if (keyArray[keyUnlockAttempt][0] == currentFloor && keyArray[keyUnlockAttempt][1] == currentX && keyArray[keyUnlockAttempt][2] == currentY)
			{
				SendDesiredOption(6 + desiredDirectionState, keyArray[keyUnlockAttempt][3]);
				printf("UNLOCKING DOOR IN DESIRED DIRECTION KEY VALUE %04X\n\n", keyArray[keyUnlockAttempt][3]);

				keyUnlockAttempt++;
				return;
			}

		}
		else
		{
			if (keyArray[keyUnlockAttempt][0] == -1 && keyArray[keyUnlockAttempt][1] == -1 && keyArray[keyUnlockAttempt][2] == -1)
			{
				SendDesiredOption(6 + desiredDirectionState, keyArray[keyUnlockAttempt][3]);
				printf("ATTEMPTING UNLOCKING DOOR WITH GENERAL PURPOSE KEY IN DESIRED DIRECTION: VALUE %04X\n\n", keyArray[keyUnlockAttempt][3]);
				keyUnlockAttempt++;
				return;
			}
		}*/

		keyUnlockAttempt++;
	}
	else
	{
		if (exploreModeOnly == false)
		{
			printf("BRUTE FORCING DOOR IN DESIRED DIRECTION: %s, UNLOCK_DIR: %s, BRUTE FORCE VALUE %04X\n\n", directionText.c_str(), directionUnlockText.c_str(), tryKey);

			isBruteForcing = true;

			// cheers mate the calvary is here
			// Bombs Away!
			tryKey++;
			SendDesiredOption(6 + desiredDirectionState, tryKey);
		}
		else {

			// if we are allowed to use general purpose keys
			if (useGeneralPurposeKeys)
			{
				// attempt to use general purpose keys
				if (keyUnlockGeneralPurposeAttempt <= generalPurposeKeyArraySize && !HaveWeTriedThisDoor())
				{
					// randomize attempt to unlock with general purpose keys
					printf("ATTEMPTING TO UNLOCK DOOR WITH GENERAL PURPOSE KEY: %s, UNLOCK_DIR: %s, KEY VALUE %04X\n\n", directionText.c_str(), directionUnlockText.c_str(), generalPurposeKeyArray[keyUnlockGeneralPurposeAttempt]);
					SendDesiredOption(6 + desiredDirectionState, generalPurposeKeyArray[keyUnlockGeneralPurposeAttempt]);
					keyUnlockGeneralPurposeAttempt++;
				}
				else
				{
					// if we have already added this door to our attempted if not add it
					if (!CheckAttemptedDoorExists(currentFloor, currentX, currentY, desiredDirectionState))
						attemptedDoors.push_back(doorsAttemptedData(currentFloor, currentX, currentY, desiredDirectionState, 0));

					// randomize direction
					printf("EXPLORE MODE ENABLED NOT BRUTE FORCING DOOR CANT UNLOCK IT NAVIGATING SOMEWHERE ELSE!\n\n");
					RandomizeDirection();
					printf("EXPLORE MODE ENABLED SETTING RANDOM DIRECTION! \n\n");
				}
			}
			else // if we dont want to use general purpose keys and want to use the CSV key data file to open our doors
			{
				// if we have already added this door to our attempted if not add it
				if (!CheckAttemptedDoorExists(currentFloor, currentX, currentY, desiredDirectionState))
					attemptedDoors.push_back(doorsAttemptedData(currentFloor, currentX, currentY, desiredDirectionState, 0));

				// randomize direction
				printf("EXPLORE MODE ENABLED NOT BRUTE FORCING DOOR CANT UNLOCK IT NAVIGATING SOMEWHERE ELSE!\n\n");
				RandomizeDirection();
				printf("EXPLORE MODE ENABLED SETTING RANDOM DIRECTION! \n\n");
			}
		}
	}
}

#pragma endregion

// used for perform room analysis
int directionCycleCheckValue = 0;

// called on every move i need to make
void yourMove()
{

	// if the score is 60 or above we have completed our task
	if (student.score >= 60 && taskCompleted == false)
	{
		// set task completed
		taskCompleted = true;

		printf("\n\n");
		printf("TASK COMPLETED Charles zoomba roomba has achieved a score of: %d and has reached level: %d!! \n\n\n", student.score, student.level);
	}

	// if task completed stop
	if (taskCompleted)
		return;

	// spaceer used to seperate each print info
	printf("\n\n");

	// reset the have we sent option at the start of every cycle
	haveWeSentOption = false;

	// every cycle we want to fetch the location and know our posistion
	FetchLocation();

	#pragma region Auto Attend/Drop/PickUp Functions

	// display student leve and score
	printf("STUDENT LEVE: %d, STUDENT SCORE: %d\n", student.level, student.score);

	// attend events
	int moveForEvent = HandleAttendEvent();
	if (moveForEvent != -1) // && student.number_of_moves <= 
	{
		// debug and do option
		printf("FOUND EVENT ATTENDING IT!!!\n\n");
		SendDesiredOption(moveForEvent, 0);
	}

	// handle books
	int moveForBook = HandleReadBook();
	if (moveForBook != -1)
	{
		// debug and do option
		printf("FOUND BOOK WITH VALUE GREATER THAN ZERO PICKING IT UP!!!\n\n");
		SendDesiredOption(moveForBook, 0);
	}

	// handle pick up items
	int moveForItem = HandlePickup();
	if (moveForItem != -1)
	{
		// debug and do option
		printf("FOUND ITEM WITH VALUE GREATER THAN ZERO PICKING IT UP!!!\n\n");
		SendDesiredOption(moveForItem, 0);
	}

	#pragma endregion

	// set our start desired direction
	//desiredDirectionState = OPTION_MOVE_SOUTH;

	#pragma region Desired Direction Safe Guard

	// safe guard check to make sure we dont exceed option directions
	if (desiredDirectionState > 6)
	{
		desiredDirectionState = 1;
	}

	// safe guard check to make sure we dont exceed option directions
	if (desiredDirectionState < 1)
	{
		desiredDirectionState = 6;
	}

	#pragma endregion

	// are we going to perform a room analysis
	if (performRoomAnalysis)
	{
		// validate direction cycle check value
		if (directionCycleCheckValue < 6)
		{

			// switch to tell debug information of what direction we are facing during this analysis check
			std::string directionPRA = "N/A";

			switch (directionCycleCheckValue)
			{
			case 0:
				directionPRA = "NORTH";
				break;
			case 1:
				directionPRA = "EAST";
				break;
			case 2:
				directionPRA = "SOUTH";
				break;
			case 3:
				directionPRA = "WEST";
				break;
			case 4:
				directionPRA = "UP";
				break;
			case 5:
				directionPRA = "DOWN";
				break;
			}

			// debug output
			printf("%s PERFORMING ROOM ANALYSIS PHASE: %d %s\n\n", prefix.c_str(), directionCycleCheckValue, directionPRA.c_str());

			// check each direction for number of visits
			int numberOfRoomVisits = GetDesiredDirectionPredictedRoomVisits();

			// for each direction validate next desired move and compare to current room of vists
			switch (directionCycleCheckValue) // i have used numbers because i want it to check NORTH,EAST,SOUTH,WEST
			{
			case 0:

				// we check the direction is open
				if (room.direction[DIRECTION_NORTH] == DIRECTION_OPEN)
				{
					// if its less than the current room of vists then move there
					if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // if its visits value is less than get current number of vists then its a must need
					{
						performRoomAnalysis = false;
						directionCycleCheckValue = 0;
						desiredDirectionState = OPTION_MOVE_NORTH;
					}
				}

				break;
			case 1:

				// we check the direction is open
				if (room.direction[DIRECTION_EAST] == DIRECTION_OPEN)
				{
					// if its less than the current room of vists then move there
					if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // if its visits value is less than get current number of vists then its a must need
					{
						performRoomAnalysis = false;
						directionCycleCheckValue = 0;
						desiredDirectionState = OPTION_MOVE_EAST;
					}
				}

				break;
			case 2:

				// we check the direction is open
				if (room.direction[DIRECTION_SOUTH] == DIRECTION_OPEN)
				{
					// if its less than the current room of vists then move there
					if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // if its visits value is less than get current number of vists then its a must need
					{
						performRoomAnalysis = false;
						directionCycleCheckValue = 0;
						desiredDirectionState = OPTION_MOVE_SOUTH;
					}
				}

				break;
			case 3:

				// we check the direction is open
				if (room.direction[DIRECTION_WEST] == DIRECTION_OPEN)
				{
					// if its less than the current room of vists then move there
					if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // if its visits value is less than get current number of vists then its a must need
					{
						performRoomAnalysis = false;
						directionCycleCheckValue = 0;
						desiredDirectionState = OPTION_MOVE_WEST;
					}
				}

				break;
			case 4:

				// we check the direction is open
				if (room.direction[DIRECTION_UP] == DIRECTION_OPEN)
				{
					// if its less than the current room of vists then move there
					if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // if its visits value is less than get current number of vists then its a must need
					{
						performRoomAnalysis = false;
						directionCycleCheckValue = 0;
						desiredDirectionState = OPTION_MOVE_UP;
					}
				}

				break;
			case 5:

				// we check the direction is open
				if (room.direction[DIRECTION_DOWN] == DIRECTION_OPEN)
				{
					// if its less than the current room of vists then move there
					if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // if its visits value is less than get current number of vists then its a must need
					{
						performRoomAnalysis = false;
						directionCycleCheckValue = 0;
						desiredDirectionState = OPTION_MOVE_DOWN;
					}
				}

				break;
			}

			// cycle direction check value
			directionCycleCheckValue++;
		}
		else
		{
			// if we are not looking at room analysis randomize direction
			RandomizeDirection();

			// send option
			SendDesiredOption(desiredDirectionState);

			// make sure room anayls is off and reset its cycle value
			performRoomAnalysis = false;
			directionCycleCheckValue = 0;
		}
	}
	else
	{
		// debug direciton and room visits
		printf("DESIRED DIRECTION VISISTS: %d\nNUMBER OF CURRENT ROOM VISISTS: %d\n\n", GetDesiredDirectionPredictedRoomVisits(), GetCurrentRoomNumberOfVisits());

		// what ever direction we are facing is it open or locked or not valid?
		switch (ValidateDesiredDirection())
		{
		case DIRECTION_OPEN:

			// debug info telling we are moving in open direction
			printf("%s moving to desired direction its open!\n", prefix.c_str());

			// reset values used to unlock doors and stuff
			directionCycleCheckValue = 0;
			keyUnlockAttempt = 0;
			keyUnlockGeneralPurposeAttempt = 0;

			// if we where brute forcing
			if (tryKey != -1)
			{
				// output KEY
				printf("#################################################\n");
				printf("#################################################\n");
				printf("#################################################\n");
				printf("Brute Force Success Key = %04X\n", tryKey);
				printf("#################################################\n");
				printf("#################################################\n");
				printf("#################################################\n");
				//getchar();

				// save to the data key array as an object that saves floor, x, y, direciton and the key
				keyData.push_back(keyDataObject(currentFloor, currentX, currentY, desiredDirectionState, tryKey));

				// save that key to the data file CSV
				saveToKeyDataFile();

				// reset key value back to -1 to begin brute forcing
				tryKey = -1;
				isBruteForcing = false;
				keyUnlockAttempt = 0;
				keyUnlockGeneralPurposeAttempt = 0;
			}

			// is the desired direction open
			if (room.direction[desiredDirectionState - 1] == DIRECTION_OPEN)
			{
				// get room vists of the desired direction ahead
				int numberOfRoomVisits = GetDesiredDirectionPredictedRoomVisits();

				// compare to current room
				if (numberOfRoomVisits < GetCurrentRoomNumberOfVisits()) // was < only
				{
					// send desired direction state option
					SendDesiredOption(desiredDirectionState);
				}
				else
				{
					// randomize desired direction
					RandomizeDirection();

					// send desired direction state option
					SendDesiredOption(desiredDirectionState);
				}
			}

			break;
		case DIRECTION_LOCKED:

			// debug room is locked
			printf("%s Cant move in this direction its locked!\n", prefix.c_str());

			// compute unlock door code
			HandleDoorObstacle();

			break;
		case DIRECTION_NOT_PRESENT:

			// if we cant find a way we were going in desired direction perform a room analysis
			performRoomAnalysis = true;

			// debug info
			printf("%s Cant move in this direction it does not exist! PERFORMING ROOM ANALYSIS\n", prefix.c_str());

			break;
		}
	}

	// we send option one so this code can loop at the end of each cycle
	// this wont be called if we have already sent an option before it
	SendDesiredOption(-1);

}

/*************************************************************/
/********* Your tactics code ends here ***********************/
/*************************************************************/

int getTokens(char* instring, char seperator)
{
	int  number_of_tokens;
	char chr;
	int  state;
	int  i;
	int  j;


	for (i = 0; i < MAX_NO_TOKENS; i++)
	{
		for (j = 0; j < MAX_STRING_SIZE; j++)
		{
			Tokens[i][j] = '\0';
		}
	}

	number_of_tokens = -1;
	chr = instring[0];
	state = 0;
	i = 0;

	while (chr != '\0')
	{
		switch (state)
		{
		case 0:  // Initial state
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 1:  // Leading white space
			if (chr == seperator)
			{
				number_of_tokens++;
				state = 1;
			}
			else if ((chr == ' ') || (chr == '\t') || (chr == '\n'))
			{
				state = 1;
			}
			else
			{
				number_of_tokens++;
				j = 0;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		case 2:  // Collecting chars
			if (chr == seperator)
			{
				//number_of_tokens++;
				state = 1;
			}
			else
			{
				j++;
				Tokens[number_of_tokens][j] = chr;
				Tokens[number_of_tokens][j + 1] = '\0';
				state = 2;
			}
			break;

		default:
			break;
		}

		i++;
		chr = instring[i];
	}

	return (number_of_tokens + 1);
}

bool getline(FILE* fp, char* buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}

void printRoom()
{
	/*int i;

	printf("Room\n");
	printf("Room = %s, Room type = %d\n", room.name, room.type);

	printf("Directions = ");
	for (i = 0; i < 6; i++)
	{
		printf("%d  ", room.direction[i]);
	}
	printf("\n");

	if (room.number_of_keys > 0)
	{
		printf("Keys = ");
		for (i = 0; i < room.number_of_keys; i++)
		{
			printf("0x%X  ", room.keys[i]);
		}
		printf("\n");
	}
	else
	{
		printf("No keys in this room\n");
	}

	if (room.number_of_items > 0)
	{
		for (i = 0; i < room.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", room.items[i].number, room.items[i].value, room.items[i].volume);
		}
	}
	else
	{
		printf("No items in this room\n");
	}

	printf("\n");*/
}

void printStudent()
{
	/*printf("Student\n");
	printf("Level=%d,  Number of rooms visited = %d,  Number of doors openned = %d,  Number of moves = %d,  Score = %d\n", student.level, student.rooms_visited, student.doors_openned, student.number_of_moves, student.score);
	printf("\n");*/
}

void printBackpack()
{
	/*int i;

	printf("Backpack\n");

	if (backpack.number_of_items > 0)
	{
		for (i = 0; i < backpack.number_of_items; i++)
		{
			printf("Item=%d, Value=%d, Volume=%d\n", backpack.items[i].number, backpack.items[i].value, backpack.items[i].volume);
		}
	}
	else
	{
		printf("Your backpack is empty\n");
	}
	printf("\n");*/
}

void printOptions()
{
	/*int i;

	printf("Options\n");
	printf("Options = ");
	for (i = 0; i < number_of_options; i++)
	{
		printf("%d  ", options[i]);
	}
	printf("\n");
	printf("\n");*/
}

// i have hidden the print options lol xd
void communicate_with_server()
{
	char buffer[4096];
	int  len = sizeof(SOCKADDR);
	int  i;
	char* p;
	int	 number_of_tokens;


	sprintf_s(buffer, "Register  %s %s %s", STUDENT_NUMBER, STUDENT_FIRSTNAME, STUDENT_FAMILYNAME);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&server_addr, sizeof(SOCKADDR));

	while (true)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (SOCKADDR*)&client_addr, &len) != SOCKET_ERROR)
		{

			// send to the Unity Game Client
			sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR*)&client_unity_addr, sizeof(SOCKADDR));

			p = ::inet_ntoa(client_addr.sin_addr);


			if ((strcmp(IP_ADDRESS_SERVER, "127.0.0.1") == 0) || (strcmp(IP_ADDRESS_SERVER, p) == 0))
			{
				//printf("%s\n\n", buffer);

				number_of_tokens = getTokens(buffer, '|');

				if (number_of_tokens == 6)
				{
					strcpy(text_student, Tokens[0]);
					strcpy(text_backpack, Tokens[1]);
					strcpy(text_room, Tokens[2]);
					strcpy(text_keys, Tokens[3]);
					strcpy(text_items, Tokens[4]);
					strcpy(text_options, Tokens[5]);

					/*printf("Student  = '%s'\n", text_student);
					printf("Backpack = '%s'\n", text_backpack);
					printf("Room     = '%s'\n", text_room);
					printf("Keys     = '%s'\n", text_keys);
					printf("Items    = '%s'\n", text_items);
					printf("Options  = '%s'\n", text_options);*/

					printf("Options  = '%s'\n", text_options);

					student.level = -1;
					student.rooms_visited = -1;
					student.doors_openned = -1;
					student.number_of_moves = -1;

					if (sscanf(text_student, "%d,%d,%d,%d,%d", &student.level, &student.rooms_visited, &student.doors_openned, &student.number_of_moves, &student.score) == 5)
					{
					}

					if (strlen(text_backpack) > 0)
					{
						backpack.number_of_items = getTokens(text_backpack, '&');

						if (backpack.number_of_items > 0)
						{
							for (i = 0; i < backpack.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_BACKPACK)
								{
									backpack.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &backpack.items[i].number, &backpack.items[i].value, &backpack.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						backpack.number_of_items = 0;
					}

					sscanf(text_room, "%s ,%d, %d, %d, %d, %d, %d, %d", &room.name, &room.type, &room.direction[DIRECTION_NORTH], &room.direction[DIRECTION_SOUTH], &room.direction[DIRECTION_EAST], &room.direction[DIRECTION_WEST], &room.direction[DIRECTION_UP], &room.direction[DIRECTION_DOWN]);

					if (strlen(text_keys) > 0)
					{
						room.number_of_keys = getTokens(text_keys, '&');

						if (room.number_of_keys > 0)
						{
							for (i = 0; i < room.number_of_keys; i++)
							{
								if (i < 4)
								{
									room.keys[i] = -1;

									if (sscanf(Tokens[i], "%x", &room.keys[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_keys = 0;
					}

					if (strlen(text_items) > 0)
					{
						room.number_of_items = getTokens(text_items, '&');

						if (room.number_of_items > 0)
						{
							for (i = 0; i < room.number_of_items; i++)
							{
								if (i < MAX_ITEMS_IN_ROOM)
								{
									room.items[i].number = -1;

									if (sscanf(Tokens[i], "%d, %d, %d", &room.items[i].number, &room.items[i].value, &room.items[i].volume) == 3)
									{
									}
								}
							}
						}
					}
					else
					{
						room.number_of_items = 0;
					}

					if (strlen(text_options) > 0)
					{
						number_of_options = getTokens(text_options, ',');

						if (number_of_options > 0)
						{
							for (i = 0; i < number_of_options; i++)
							{
								if (i < MAX_OPTIONS)
								{
									options[i] = -1;

									if (sscanf(Tokens[i], "%d", &options[i]) == 1)
									{
									}
								}
							}
						}
					}
					else
					{
						number_of_options = 0;
					}
				}

				printStudent();
				printBackpack();
				printRoom();
				printOptions();

				if (isBruteForcing == false)
				{
					if (taskCompleted)
					{
						// sleep 30 seconds
						Sleep(30000);
					}
					else
					{
						//thread sleep every 5 miliseconds
						Sleep(nextMoveDelay); //10
					}
				}

				yourMove();
			}
		}
		else
		{
			printf_s("recvfrom error = %d\n", WSAGetLastError());
		}
	}

	printf_s("Student %s\n", STUDENT_NUMBER);
}

bool fetchValidKeys()
{

	bool allowtoprogress = false;

	if (keyDatafileExists(keyDataFileName()))
	{
		printf("KEY DATA FILE EXISTS READING!\n");

		std::ifstream input(keyDataFileName());

		std::vector<int> row;
		std::string line, word, temp;

		for (std::string line; getline(input, line); )
		{

			printf("LINE DATA FROM KEY DATA FILE: %s \n", line.c_str());


			// clear the row
			row.clear();

			//while (getline(line, word, ',')) {

			//	// add all the column data 
			//	// of a row to a vector 
			//	row.push_back(word);
			//}

			std::stringstream ss(line);

			for (int i; ss >> i;) {
				row.push_back(i);
				if (ss.peek() == ',')
					ss.ignore();
			}

			//keyData.push_back(keyDataObject(std::stoi(row[0]), std::stoi(row[1]), std::stoi(row[2]), std::stoi(row[3]), std::stoi(row[4])));
			keyData.push_back(keyDataObject(row[0], row[1], row[2], row[3], row[4]));
		}

		printf("PRINTING KEY ARRAY FROM TEXT FILE THAT IS IN VECTOR: \n\n\n\n");

		for (size_t i = 0; i < keyData.size(); i++)
		{
			keyDataObject KDO = keyData[i];
			printf("VECTOR %d, %d, %d, %d, %d \n", KDO.floor, KDO.x, KDO.y, KDO.direction, KDO.keyValue);
		}

		allowtoprogress = true;

	}
	else
	{
		if (developedModeEnabled)
		{
			printf("KEY DATA FILE DOES NOT EXIST WRITING!\\n");

			std::ofstream file;
			file.open(keyDataFileName(), std::ios_base::app);
			//file << key << "," << unlocks << std::endl;
			file.close();

			if (exploreModeOnly)
			{
				printf("exploreModeOnly = true; BRUTE FORCING IS DISABLED THERE WILL BE NO CACHED KEY DATA TO OPEN DOORS IN EXPLORE MARKS WILL BE DEDUCTED WARNING!\\n");
				printf("exploreModeOnly = true; BRUTE FORCING IS DISABLED THERE WILL BE NO CACHED KEY DATA TO OPEN DOORS IN EXPLORE MARKS WILL BE DEDUCTED WARNING!\\n");
				printf("exploreModeOnly = true; BRUTE FORCING IS DISABLED THERE WILL BE NO CACHED KEY DATA TO OPEN DOORS IN EXPLORE MARKS WILL BE DEDUCTED WARNING!\\n");
				printf("exploreModeOnly = true; BRUTE FORCING IS DISABLED THERE WILL BE NO CACHED KEY DATA TO OPEN DOORS IN EXPLORE MARKS WILL BE DEDUCTED WARNING!\\n");
				printf("PROGRAM RESUMING in 30 SECONDS...\\n");
				Sleep(30 * 1000);
			}

			allowtoprogress = true;

		}
		else
		{
			allowtoprogress = false;
		}

	}

	return allowtoprogress;
}

int main()
{
	char chr = '\0';

	printf("\n");
	printf("The Computer Maze Student Program\n");
	printf("UWE Computer and Network Systems Assignment 2 \n");
	printf("\n");

	bool canContinue = fetchValidKeys();

	if (!canContinue)
	{
		printf("\n\nTHE KEYDATA.CSV FILE IS MISSING PLEASE MAKE SURE THE KEYDATA.CSV (5KB in size) IS ATTACHED TO THIS PROGRAM TO GET 60 MARKS\nYOU MAY OVERRIDE THIS WARNING BY TURNING DEVELOPER MODE ON BUT WILL BE CAPED AT 45 MARKS.\n\nMAKE SURE THE ITS LOCATED IN THE SAME LOCATION AS THE EXE IF YOUR RUNNING THIS APPLICATION STANDALONE.\n\nIF YOU ARE RUNNING THIS USING VISUAL STUDIO MAKE SURE ITS IN THE SAME FOLDER AS 'Computer Maze Completed.cpp'\n");
		while (!canContinue)
		{
			chr = getchar();
		}

	}

	
	initialize_room_visits();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).

	if (!sock)
	{
		// Creation failed! 
	}

	/*int reuse = 1;
	int opt = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

	if (opt == -1)
	{
		perror("Failed to set socket option");
	}*/

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	memset(&client_unity_addr, 0, sizeof(SOCKADDR_IN));
	client_unity_addr.sin_family = AF_INET;
	client_unity_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_unity_addr.sin_port = htons(PORT_CLIENT);

	//bind(sock, (SOCKADDR*)&client_addr, sizeof(client_addr));

	communicate_with_server();

	closesocket(sock);
	WSACleanup();

	while (chr != '\n')
	{
		chr = getchar();
	}

	return 0;
}

