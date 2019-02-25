#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h"
#include "Actor.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_player(nullptr), m_nCitizens(0)
{
}

StudentWorld::~StudentWorld()
{
	cleanUp();
}

int StudentWorld::init()
{
	Level lev(assetPath());

	if (getLevel() > 99)
		return GWSTATUS_PLAYER_WON;

	string levelFile = "level";
	if (getLevel() < 10)			// Add an extra 0 if the level is one digit
		levelFile += "0";
	levelFile += getLevel() + '0';	// Append the level number
	levelFile += ".txt";			// Append the file type

	Level::LoadResult result = lev.loadLevel(levelFile);
	if (result == Level::load_fail_file_not_found)
	{
		cerr << "Cannot find" << levelFile << "data file" << endl;
		return GWSTATUS_PLAYER_WON;
	}
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
					newActor = new Exit(x * SPRITE_WIDTH, y * SPRITE_HEIGHT, this);
					m_actors.push_back(newActor);
					break;
				case Level::wall:
					newActor = new Wall(x * SPRITE_WIDTH, y * SPRITE_HEIGHT, this);
					m_actors.push_back(newActor);
					break;
				case Level::pit:
					break;
				case Level::citizen:
					newActor = new Citizen(x * SPRITE_WIDTH, y * SPRITE_HEIGHT, this);
					m_actors.push_back(newActor);
					m_nCitizens++;
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
	if (!(m_player->alive()))
		return GWSTATUS_PLAYER_DIED;

	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())
	{
		if (!(*it)->alive())			// Remove an Actor if it's dead
		{
			delete *it;
			it = m_actors.erase(it);
			continue;
		}

		(*it)->doSomething();			// Every Actor takes a turn

		if (!(m_player->alive()))		// Player died during that turn
			return GWSTATUS_PLAYER_DIED;

		if (m_player->finished())		// Player finished level during that turn
		{
			playSound(SOUND_LEVEL_FINISHED);
			return GWSTATUS_FINISHED_LEVEL;
		}

		it++;
	}

	ostringstream oss;	// Set up and print the status text
	oss << "Score: ";
	oss.fill('0');
	oss << setw(6) << getScore() << "  Level: " << getLevel()
		<< "  Lives: " << getLives() << "  Vaccines: " << m_player->nVacs()
		<< "  Flames: " << m_player->nFlames() << "  Mines: " << m_player->nMines()
		<< "  Infected: " << m_player->infectedCount();
	string status = oss.str();
	setGameStatText(status);

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{
	delete m_player;
	m_player = nullptr;		// Prevents undefined behavior in repeated calls of cleanUp()

	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())
	{
		delete *it;
		it++;
	}

	m_actors.clear();		// Prevents undefined behavior in repeated calls of cleanUp()
}

void StudentWorld::overlap(double x, double y, list<Actor*> & trues, const Actor * compare)
{
	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())	// Check every Actor in the game right now
	{
		if ((*it)->overlapping(x, y, compare))	// The point is close enough to the Actor to overlap sprites
			trues.push_back(*it);		// Add the Actor to a list of objects overlapping
		it++;
	}
	return;
}

bool StudentWorld::overlapsPlayer(double x, double y, const Actor * compare)
{
	return m_player->inBoundary(x, y, compare);
}

void StudentWorld::blocked(double x, double y, list<Actor*> & trues, const Actor * moving)
{
	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())	// Check every Actor in the game right now
	{
		if ((*it)->impassable())			// The Actor is impassable
			if ((*it)->inBoundary(x, y, moving))	// The point is in the Actor's boundary box
				trues.push_back(*it);	// Add the Actor to a list of objects blocking
		it++;
	}

	if (m_player->inBoundary(x, y, moving))	// Check the player
		trues.push_back(m_player);

	return;
}

double StudentWorld::distToZombie(double x, double y)
{
	double min = 0;
	list<Actor*>::iterator it = m_actors.begin();
	while (it != m_actors.end())
	{
		if ((*it)->eatsBrains())	// The Actor is a Zombie
			min = fmin(min, sqrt(pow((*it)->getX() - x, 2) + pow((*it)->getY() - y, 2)));	// Set min to the shortest distance to a Zombie
		it++;
	}
	return min;	// Since two Agents can only get within 16 pixels of each other, min == 0 will mean no Zombies found
}

double StudentWorld::distToPlayer(double x, double y)
{
	return sqrt(pow(m_player->getX() - x, 2) + pow(m_player->getY() - y, 2));	// Returns distance to player from a point
}
