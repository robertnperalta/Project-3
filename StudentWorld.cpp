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
				Level::MazeEntry ge = lev.getContentsOf(x, y);

				Actor* newActor;
				switch (ge)	// TODO: ALLOCATE OTHER ACTORS
				{
				case Level::empty:
					break;
				case Level::smart_zombie:
					break;
				case Level::dumb_zombie:
					break;
				case Level::player:
					m_player = new Player(x * SPRITE_WIDTH, y * SPRITE_HEIGHT, this);
					break;
				case Level::exit:
					break;
				case Level::wall:
					newActor = new Wall(x * SPRITE_WIDTH, y * SPRITE_HEIGHT, this);
					break;
				case Level::pit:
					break;
				case Level::citizen:
					break;
				case Level::vaccine_goodie:
					break;
				case Level::gas_can_goodie:
					break;
				case Level::landmine_goodie:
					break;
				}
			}
		}
		return GWSTATUS_CONTINUE_GAME;
	}
	return GWSTATUS_LEVEL_ERROR;
}

int StudentWorld::move()	// TODO: COMPLETE
{
	m_player->doSomething();		// Player takes its turn
	if (!(m_player->isAlive()))
		return GWSTATUS_PLAYER_DIED;

	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())
	{
		(*it)->doSomething();			// Every Actor takes a turn

		if (!(m_player->isAlive()))		// Check if Player died during that turn
			return GWSTATUS_PLAYER_DIED;

		it++;
	}
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	delete m_player;

	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())
		delete *it;
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