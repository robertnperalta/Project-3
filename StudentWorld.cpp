#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "Actor.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_player(nullptr)
{
}

StudentWorld::~StudentWorld()
{
	cleanUp();
}

int StudentWorld::init()
{
	Level lev(assetPath());

	string levelFile = "level01.txt";
	Level::LoadResult result = lev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found)
		cerr << "Cannot find level01.txt data file" << endl;
	else if (result == Level::load_fail_bad_format)
		cerr << "Your level was improperly formatted" << endl;
	else if (result == Level::load_success)
	{
		cerr << "Successfully loaded level" << endl;

		for (int x = 0; x < LEVEL_WIDTH; x++)
		{
			for (int y = 0; y < LEVEL_HEIGHT; y++)
			{
				Level::MazeEntry ge = lev.getContentsOf(x, y);	// TODO: ALLOCATE NEW ACTORS
				switch (ge)
				{
				case Level::empty:
					cout << "Location 80,160 is empty" << endl;
					break;
				case Level::smart_zombie:
					cout << "Location 80,160 starts with a smart zombie" << endl;
					break;
				case Level::dumb_zombie:
					cout << "Location 80,160 starts with a dumb zombie" << endl;
					break;
				case Level::player:
					cout << "Location 80,160 is where Penelope starts" << endl;
					break;
				case Level::exit:
					cout << "Location 80,160 is where an exit is" << endl;
					break;
				case Level::wall:
					cout << "Location 80,160 holds a Wall" << endl;
					break;
				case Level::pit:
					cout << "Location 80,160 has a pit in the ground" << endl;
					break;
				case Level::citizen:
					cout << "Location 80,160 has a pit in the ground" << endl;
					break;
				case Level::vaccine_goodie:
					cout << "Location 80,160 has a pit in the ground" << endl;
					break;
				case Level::gas_can_goodie:
					cout << "Location 80,160 has a pit in the ground" << endl;
					break;
				case Level::landmine_goodie:
					cout << "Location 80,160 has a pit in the ground" << endl;
					break;
				}
			}
		}
		return GWSTATUS_CONTINUE_GAME;
	}
	return GWSTATUS_LEVEL_ERROR;
}

int StudentWorld::move()
{
    // This code is here merely to allow the game to build, run, and terminate after you hit enter.
    // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
    decLives();
    return GWSTATUS_PLAYER_DIED;
}

void StudentWorld::cleanUp()
{
}

bool StudentWorld::overlap(double x, double y)
{
	return false;
}

bool StudentWorld::blocked(double x, double y)
{
	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())	// Check every Actor in the game right now
	{
		if ((*it)->impassable())			// The Actor is impassable
			if ((*it)->inBoundary(x, y))	// The point is in the Actor's boundary box
				return true;				// The point is blocked
		it++;
	}
	return false;
}