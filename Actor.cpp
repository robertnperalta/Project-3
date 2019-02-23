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

Actor::Actor(int imageID, double startX, double startY, StudentWorld * world, int moveDist, int dir, int depth)
	:GraphObject(imageID, startX, startY, dir, depth), m_world(world), 
	m_moveDist(moveDist), m_alive(true), m_infected(false), m_infectedCount(0)
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

bool Actor::move(int dir)
{
	setDirection(dir);	// Change the direction the Actor is facing

	double destX, destY;
	switch (dir)		// Determine destination
	{
	case left:
		destX = getX() - m_moveDist;
		destY = getY();
		break;
	case right:
		destX = getX() + m_moveDist;
		destY = getY();
		break;
	case up:
		destX = getX();
		destY = getY() + m_moveDist;
		break;
	case down:
		destX = getX();
		destY = getY() - m_moveDist;
		break;
	}

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

//////////////////////////////
//		GAME OBJECTS		//
//////////////////////////////

//
//	Player
// 

Player::Player(double startX, double startY, StudentWorld * world)
	:Actor(IID_PLAYER, startX, startY, world, 4), m_nVacs(0), m_nFlames(0), m_nMines(0), m_finished(false)
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
		case KEY_PRESS_LEFT:	move(left);		break;
		case KEY_PRESS_RIGHT:	move(right);	break;
		case KEY_PRESS_UP:		move(up);		break;
		case KEY_PRESS_DOWN:	move(down);		break;
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
	:Actor(IID_EXIT, startX, startY, world, 0, right, 1)
{
}

void Exit::doSomething()
{
	list<Actor*> overlaps;
	getWorld()->overlap(getX(), getY(), overlaps, this);
	list<Actor*>::iterator it = overlaps.begin();
	while (it != overlaps.end())
	{
		if ((*it)->canExit())		// A Citizen is overlapping the Exit
		{
			(*it)->setToRemove();	// Remove the Citizen
			getWorld()->decCitizens();
		}
		it++;
	}

	if (getWorld()->nCitizens() == 0)	// No Citizens left on the level
		if (getWorld()->player()->overlapping(getX(), getY(), this))	// The Player is on the Exit
			getWorld()->player()->finishLevel();	// Finish the level
}
