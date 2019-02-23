#include "Actor.h"
#include "StudentWorld.h"
#include <list>
using namespace std;

//////////////////////////////
//		BASE CLASSES		//
//////////////////////////////

//
//	Actor
//

Actor::Actor(int imageID, double startX, double startY, StudentWorld * world, int dir, int depth)
	:GraphObject(imageID, startX, startY, dir, depth), m_world(world), m_alive(true)
{
}

bool Actor::inBoundary(double x, double y, const Actor * moving) const
{
	if (moving == this)	// This is the Actor trying to move
		return false;	// An Actor can't block itself

	if ((getX() <= x && getX() + SPRITE_WIDTH - 1 >= x &&	// Any of the corners of the boundary box including the point is within the Actor's boundary
		getY() <= y && getY() + SPRITE_HEIGHT - 1 >= y) ||											// Bottom left corner
		(getX() <= x + SPRITE_WIDTH - 1 && getX() + SPRITE_WIDTH - 1 >= x + SPRITE_WIDTH - 1 &&
		getY() <= y + SPRITE_WIDTH - 1 && getY() + SPRITE_WIDTH - 1 >= y + SPRITE_WIDTH - 1) ||		// Top right corner
		(getX() <= x + SPRITE_WIDTH - 1 && getX() + SPRITE_WIDTH - 1 >= x + SPRITE_WIDTH - 1 &&
		getY() <= y && getY() + SPRITE_WIDTH - 1 >= y) ||											// Bottom right corner
		(getX() <= x && getX() + SPRITE_WIDTH - 1 >= x &&
		getY() <= y + SPRITE_WIDTH - 1 && getY() + SPRITE_WIDTH - 1 >= y + SPRITE_WIDTH - 1))		// Top left corner
		return true;
	else
		return false;
}

bool Actor::overlapping(double x, double y, const Actor * compare) const
{
	if (compare == this)	// This is the Actor that the function is comparing against
		return false;		// An Actor can't overlap with itself

	if ((getX() - x) * (getX() - x) + (getY() - y) * (getY() - y) <= 100)	// The point and the anchor pixel of this Actor are within 10 pixels of each other
		return true;
	return false;
}

//
//	Agent
//


Agent::Agent(int imageID, double x, double y, StudentWorld * world, int moveDist)
	:Actor(imageID, x, y, world), m_moveDist(moveDist), m_paralyzed(false)
{
}

void Agent::findDest(int dir, int distX, int distY, double & destX, double & destY)
{
	switch (dir)
	{
	case left:
		destX = getX() - distX;
		destY = getY();
		break;
	case right:
		destX = getX() + distX;
		destY = getY();
		break;
	case up:
		destX = getX();
		destY = getY() + distY;
		break;
	case down:
		destX = getX();
		destY = getY() - distY;
		break;
	}
}

bool Agent::tryMove(int dir)
{
	setDirection(dir);	// Change the direction the Actor is facing

	double destX, destY;
	findDest(dir, m_moveDist, m_moveDist, destX, destY);	// Determine the Actor's destination

	list<Actor*> blocking;
	getWorld()->blocked(destX, destY, blocking, this);
	if (blocking.size() != 0)	// The destination is blocked
		return false;
	else
	{
		moveTo(destX, destY);	// Make the move
		return true;
	}
}

//
//	Human
//

Human::Human(int imageID, double x, double y, StudentWorld * world, int moveDist)
	:Agent(imageID, x, y, world, moveDist), m_infected(false), m_infectedCount(0)
{
}

//
//	Zombie
//

Zombie::Zombie(double x, double y, StudentWorld * world)
	:Agent(IID_ZOMBIE, x, y, world, 1)
{
}

void Zombie::doSomething()
{
	if (!alive())		// The Zombie is dead
		return;

	if (paralyzed())	// The Zombie is paralyzed this turn
	{
		takeNextTurn();
		return;
	}

	// Attempt to vomit

	double vomitX, vomitY;
	findDest(getDirection(), SPRITE_WIDTH, SPRITE_HEIGHT, vomitX, vomitY);	// Find where the vomit would be
	list<Actor*> overlapsVomit;
	getWorld()->overlap(vomitX, vomitY, overlapsVomit, this);

	bool foundTarget = false;
	list<Actor*>::iterator it = overlapsVomit.begin();
	while (it != overlapsVomit.end())	// See if any Actors would overlap with the vomit
	{
		if ((*it)->infectable())
		{
			foundTarget = true;
			break;
		}
	}

	if (foundTarget && randInt(1, 3) == 1)	// There is a target and it passes the 1 in 3 chance
	{
		// TODO: ALLOCATE VOMIT, ADD TO STUDENTWORLD
		return;
	}

	// Attempt movement

	if (m_plan == 0)	// The Zombie doesn't have a plan
	{
		m_plan = randInt(3, 10);	// Pick a new plan
		setDirection(pickDirection());
	}

	double destX, destY;
	findDest(getDirection(), moveDist(), moveDist(), destX, destY);
	if (!tryMove(getDirection()))	// The motion is blocked
		m_plan = 0;					// Pick a new plan next turn
	else
		m_plan--;
	return;
}


//////////////////////////////
//		GAME OBJECTS		//
//////////////////////////////

//
//	Player
// 

Player::Player(double startX, double startY, StudentWorld * world)
	:Human(IID_PLAYER, startX, startY, world, 4), m_nVacs(0), m_nFlames(0), m_nMines(0), m_finished(false)
{
}

void Player::doSomething()
{
	if (!alive())	// Player dies
		return;

	if (infected())	// Player is infected
	{
		incInfect();
		if (infectedCount() == 500)	// Player becomes a zombie
		{
			kill();
			return;
		}
	}

	int ch;
	if (getWorld()->getKey(ch))	// User gives an input
	{
		switch (ch)
		{
		case KEY_PRESS_LEFT:	tryMove(left);		break;
		case KEY_PRESS_RIGHT:	tryMove(right);		break;
		case KEY_PRESS_UP:		tryMove(up);		break;
		case KEY_PRESS_DOWN:	tryMove(down);		break;
		case KEY_PRESS_SPACE:				// TODO: IMPLEMENT OTHER FUNCTIONS
		case KEY_PRESS_TAB:
		case KEY_PRESS_ENTER:	break;
		}
	}
}

//
//	Wall
//

Wall::Wall(double startX, double startY, StudentWorld* world)
	:Actor(IID_WALL, startX, startY, world)
{
}

//
//	Exit
//

Exit::Exit(double startX, double startY, StudentWorld * world)
	:Actor(IID_EXIT, startX, startY, world, right, 1)
{
}

void Exit::doSomething()
{
	list<Actor*> overlaps;
	getWorld()->overlap(getX(), getY(), overlaps, this);
	list<Actor*>::iterator it = overlaps.begin();
	while (it != overlaps.end())
	{
		if ((*it)->infectable())		// A Citizen is overlapping the Exit
		{
			(*it)->setToRemove();	// Remove the Citizen
			getWorld()->decCitizens();
			getWorld()->increaseScore(500);
			getWorld()->playSound(SOUND_CITIZEN_SAVED);
		}
		it++;
	}

	if (getWorld()->nCitizens() == 0)	// No Citizens left on the level
		if (getWorld()->player()->overlapping(getX(), getY(), this))	// The Player is on the Exit
			getWorld()->player()->finishLevel();	// Finish the level
}