#include "Actor.h"
#include "StudentWorld.h"

//
//	Actor
//

Actor::Actor(int imageID, double startX, double startY, StudentWorld * world, int dir, int depth)
	:GraphObject(imageID, startX, startY, dir, depth), m_world(world)
{
}

bool Actor::inBoundary(double x, double y) const
{
	if (getX() <= x && getX() + SPRITE_WIDTH - 1 >= x &&	// The point is within the Actor's boundary
		getY() <= y && getY() + SPRITE_HEIGHT - 1 >= y)
		return true;
	else
		return false;
}

bool Actor::move(int dir)
{
	setDirection(dir);	// Change the direction the Actor is facing

	int destX, destY;
	switch (dir)		// Determine destination
	{
	case left:
		destX = getX() - 4;
		destY = getY();
		break;
	case right:
		destX = getX() + 4;
		destY = getY();
		break;
	case up:
		destX = getX();
		destY = getY() + 4;
		break;
	case down:
		destX = getX();
		destY = getY() - 4;
		break;
	}

	if (getWorld()->blocked(destX, destY))	// The destination is blocked
		return false;
	else
	{
		moveTo(destX, destY);	// Make the move
		return true;
	}
}

//
//	Player
//

Player::Player(double startX, double startY, StudentWorld * world)
	:Actor(IID_PLAYER, startX, startY, world), m_alive(true), m_infected(false),
	m_infectedCount(0), m_nVacs(0), m_nFlames(0), m_nMines(0)
{
}

void Player::doSomething()
{
	if (!m_alive)	// Player dies
		return;

	if (m_infected)	// Player is infected
	{
		m_infectedCount++;
		if (m_infectedCount == 500)	// Player becomes a zombie
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
		case KEY_PRESS_ENTER:
		}
	}
}