#include "Actor.h"
#include <list>
using namespace std;

// Auxiliary functions

int randDir()
{
	switch (randInt(1, 4))
	{
	case 1:
		return GraphObject::right;
	case 2:
		return GraphObject::left;
	case 3:
		return GraphObject::up;
	case 4:
	default:	// Only so all control paths return a value
		return GraphObject::down;
	}
}

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
//	Activating
//

Activating::Activating(int imageID, double x, double y, StudentWorld * world, int dir, int depth)
	:Actor(imageID, x, y, world, dir, depth), m_playerOverlaps(false), m_stop(false)
{
}

void Activating::doSomething()
{
	something();	// For anything the Activating might need to do before checking for overlaps

	if (!alive() || m_stop)	// The Activating is dead or needs to skip this tick of doSomething()
	{
		m_stop = false;
		return;
	}

	getWorld()->overlap(getX(), getY(), m_overlaps, this);
	m_playerOverlaps = getWorld()->overlapsPlayer(getX(), getY(), this);

	list<Actor*>::iterator it = overlapsBegin();
	while (it != overlapsEnd())
	{
		tryActivate(*it);				// Try to activate on all overlapping Actors
		it++;
	}
	if (playerOverlaps())				// Try to activate on the Player, if it's overlapping
		tryActivate(getWorld()->player());

	m_overlaps.clear();
	m_playerOverlaps = false;
}

//
//	Projectile
//

Projectile::Projectile(int imageID, double x, double y, StudentWorld * world, int dir)
	:Activating(imageID, x, y, world, dir, 0), m_ticksAlive(0)
{
}

void Projectile::something()
{
	if (m_ticksAlive == 2)	// If the Pojectile has been around for 2 ticks, destroy it
	{
		setToRemove();
		return;
	}
	m_ticksAlive++;
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
	:Agent(IID_ZOMBIE, x, y, world, 1), m_plan(0)
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

	something();

	skipNextTurn();
}

void Zombie::something()
{
	// Attempt to vomit

	double vomitX, vomitY;
	findDest(getDirection(), SPRITE_WIDTH, SPRITE_HEIGHT, vomitX, vomitY);	// Find where the vomit would be
	list<Actor*> overlapsVomit;
	getWorld()->overlap(vomitX, vomitY, overlapsVomit, this);	// Get a list of Actors that would be hit by the Vomit
	if (getWorld()->overlapsPlayer(vomitX, vomitY, this))	// If Player would be hit, add it to the list
		overlapsVomit.push_back(getWorld()->player());

	bool foundTarget = false;
	list<Actor*>::iterator it = overlapsVomit.begin();
	while (it != overlapsVomit.end())	// See if any Actors would overlap with the vomit
	{
		if ((*it)->infectable())
		{
			foundTarget = true;
			break;
		}
		it++;
	}

	if (foundTarget && randInt(1, 3) == 1)	// There is a target and it passes the 1 in 3 chance
	{
		getWorld()->addActor(new Vomit(vomitX, vomitY, getWorld(), getDirection()));
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
}

//
//	Goodie
//

Goodie::Goodie(int imageID, double x, double y, StudentWorld * world)
	:Activating(imageID, x, y, world, right, 1)
{
}

void Goodie::tryActivate(Actor * a)
{
	if (playerOverlaps())
	{
		setToRemove();
		getWorld()->increaseScore(50);
		getWorld()->playSound(SOUND_GOT_GOODIE);
		incContains();
	}
}

//////////////////////////////
//		GAME OBJECTS		//
//////////////////////////////

//
//	Player
// 

Player::Player(double x, double y, StudentWorld * world)
	:Human(IID_PLAYER, x, y, world, 4), m_nVacs(0), m_nFlames(0), m_nMines(0), m_finished(false)
{
}

void Player::doSomething()
{
	if (!alive())	// Player died this turn
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

		case KEY_PRESS_SPACE:
			if (m_nFlames > 0)
			{
				makeFlameLine();
				m_nFlames--;
			}
			break;		
		case KEY_PRESS_TAB:
			if (m_nMines > 0)
			{
				getWorld()->addActor(new Landmine(getX(), getY(), getWorld()));
				m_nMines--;
			}
			break;
		case KEY_PRESS_ENTER:
			if (m_nVacs > 0)
			{
				vaccinate();
				m_nVacs--;
			}
			break;
		}
	}
}

void Player::makeFlameLine()
{
	for (int k = 0; k < 3; k++)	// For each Flame in the line
	{
		double destX, destY;
		switch (getDirection())					// Find where it would be allocated
		{
		case GraphObject::left:
			destX = getX() - SPRITE_WIDTH * (k + 1);
			destY = getY();
			break;
		case GraphObject::right:
			destX = getX() + SPRITE_WIDTH * (k + 1);
			destY = getY();
			break;
		case GraphObject::up:
			destX = getX();
			destY = getY() + SPRITE_HEIGHT * (k + 1);
			break;
		case GraphObject::down:
			destX = getX();
			destY = getY() - SPRITE_HEIGHT * (k + 1);
			break;
		}

		list<Actor*> overlaps;
		getWorld()->overlap(destX, destY, overlaps, nullptr);
		list<Actor*>::iterator it = overlaps.begin();
		bool doubleBreak = false;
		while (it != overlaps.end())	// Check if the Flame would be overlapping something that blocks it
		{
			if ((*it)->blocksFire())
			{
				doubleBreak = true;
				break;
			}
			it++;
		}
		if (doubleBreak)
			break;
		else
			getWorld()->addActor(new Flame(destX, destY, getWorld(), getDirection()));	// Allocate the Flame if nothing is blocking it
	}
}

//
//	Citizen
//

Citizen::Citizen(double x, double y, StudentWorld * world)
	:Human(IID_CITIZEN, x, y, world, 2)
{
}

void Citizen::doSomething()
{
	if (!alive())	// Citizen died this turn
		return;

	if (infected())	// Citizen is infected
	{
		incInfect();
		if (infectedCount() == 500)	// Citizen becomes a zombie
		{
			turnIntoZombie();
			return;
		}
	}

	if (paralyzed())	// The Citizen is paralyzed this turn
	{
		takeNextTurn();
		return;
	}

	something();

	skipNextTurn();
}

void Citizen::something()
{
	double closestZombie = getWorld()->distToZombie(getX(), getY());
	double closestPlayer = getWorld()->distToPlayer(getX(), getY());

	if ((closestZombie == VIEW_HEIGHT * 2 ||		// No Zombies on the level
		closestPlayer < closestZombie) &&			// Player is closer than the closest Zombie
		closestPlayer <= 80)						// Player is 80 pixels away or closer
	{
		bool blocked = false;
		if (getX() == getWorld()->player()->getX())			// Citizen is in the same column as Player
		{
			if (getY() < getWorld()->player()->getY())		// Directly below Player
				blocked = tryMove(up);
			else if (getY() > getWorld()->player()->getY())	// Directly above Player
				blocked = tryMove(down);

			if (!blocked)	// The Citizen successfully moved
				return;
		}
		else if (getY() == getWorld()->player()->getY())	// Citizen is in the same row as Player
		{
			if (getX() < getWorld()->player()->getX())		// Directly to the left of Player
				blocked = tryMove(right);
			else if (getX() > getWorld()->player()->getX())	// Directly to the right of Player
				blocked = tryMove(left);

			if (!blocked)	// The Citizen successfully moved
				return;
		}

		if (!blocked)	// Only try a different path to the Player if the Citizen hasn't already moved/tried to move
		{
			int horz, vert;	// Determines the two best directions to move to get to the player

			if (getY() < getWorld()->player()->getY())		// Below Player
				vert = up;
			else if (getY() > getWorld()->player()->getY())	// Above Player
				vert = down;

			if (getX() < getWorld()->player()->getX())		// To the left of Player
				horz = right;
			else if (getX() > getWorld()->player()->getX())	// To the right of Player
				horz = left;

			int dir;
			if (randInt(0, 1))	// Randomly pick a direction to try
				dir = vert;
			else
				dir = horz;

			if (tryMove(dir))	// The move succeeds
				return;
			else				// Try moving in the other direction
				if (dir == vert)
				{
					if (tryMove(horz))
						return;
				}
				else if (tryMove(vert))
					return;
		}
	}

	if (closestZombie <= 80)
	{
		double bestDist = closestZombie;
		int bestDir = -1;
		list<int> dirs = { up, down, left, right };
		list<int>::iterator it = dirs.begin();
		while (it != dirs.end())	// For each direction
		{
			double destX, destY;
			findDest(*it, moveDist(), moveDist(), destX, destY);

			list<Actor*> blocking;
			getWorld()->blocked(destX, destY, blocking, this);
			if (blocking.empty())	// If the motion isn't blocked
			{
				double aCloseZombie = getWorld()->distToZombie(destX, destY);
				if ( aCloseZombie > bestDist)	// If moving in that direction will put the Citizen farther from the closest Zombie
				{
					bestDist = aCloseZombie;
					bestDir = *it;
				}
			}
			it++;	// If a move in a direction is blocked, ignore it and move on
		}

		if (bestDir >= 0)	// There is a best move to make
		{
			tryMove(bestDir);
			return;
		}
	}
	return;
}

void Citizen::turnIntoZombie()
{
	setToRemove();
	getWorld()->decCitizens();
	getWorld()->playSound(SOUND_ZOMBIE_BORN);
	getWorld()->increaseScore(-1000);
	if (randInt(1, 10) < 4)		// 30% chance of a SmartZombie
		getWorld()->addActor(new SmartZombie(getX(), getY(), getWorld()));
	else
		getWorld()->addActor(new DumbZombie(getX(), getY(), getWorld()));
}

void Citizen::whenInfected()
{
	if (!infected())		// The Citizen isn't already infected
		getWorld()->playSound(SOUND_CITIZEN_INFECTED);
}

void Citizen::dyingAction()
{
	setToRemove();
	getWorld()->decCitizens();
	getWorld()->playSound(SOUND_CITIZEN_DIE);
	getWorld()->increaseScore(-1000);
}

//
//	DumbZombie
//

DumbZombie::DumbZombie(double x, double y, StudentWorld * world)
	:Zombie(x, y, world)
{
}

void DumbZombie::dyingAction()
{
	getWorld()->playSound(SOUND_ZOMBIE_DIE);
	getWorld()->increaseScore(1000);
	if (randInt(1, 10) == 1)
	{
		double destX, destY;
		findDest(randDir(), SPRITE_WIDTH, SPRITE_HEIGHT, destX, destY);
		list<Actor*> overlaps;
		getWorld()->overlap(destX, destY, overlaps, this);
		if (overlaps.empty() && !getWorld()->overlapsPlayer(destX, destY, this))
		{
			getWorld()->addActor(new VaccineGoodie(destX, destY, getWorld()));
		}
	}
}

int DumbZombie::pickDirection()
{
	return randDir();
}

//
//	SmartZombie
//

SmartZombie::SmartZombie(double x, double y, StudentWorld * world)
	:Zombie(x, y, world)
{
}

int SmartZombie::pickDirection()
{
	Actor* closest = nullptr;
	double closestDist = getWorld()->distToCitizen(getX(), getY(), closest);
	if (getWorld()->distToPlayer(getX(), getY()) <= closestDist)
	{
		closestDist = getWorld()->distToPlayer(getX(), getY());
		closest = getWorld()->player();
	}

	if (closestDist > 80)	// If the closest Human is too far, pick a random direction
		return randDir();

	if (getX() == closest->getX())			// SmartZombie is in the same column as target
	{
		if (getY() < closest->getY())		// Directly below target
			return up;
		else if (getY() > closest->getY())	// Directly above target
			return down;
	}
	else if (getY() == closest->getY())	// SmartZombie is in the same row as target
	{
		if (getX() < closest->getX())		// Directly to the left of target
			return right;
		else if (getX() > closest->getX())	// Directly to the right of target
			return left;
	}

	int horz, vert;	// Determines the two best directions to move to get to the target
	if (getY() < closest->getY())		// Below target
		vert = up;
	else if (getY() > closest->getY())	// Above target
		vert = down;

	if (getX() < closest->getX())		// To the left of target
		horz = right;
	else if (getX() > closest->getX())	// To the right of target
		horz = left;

	if (randInt(0, 1))	// Randomly pick a direction
		return vert;
	else
		return horz;
}

//
//	Wall
//

Wall::Wall(double startX, double startY, StudentWorld * world)
	:Actor(IID_WALL, startX, startY, world)
{
}

//
//	Exit
//

Exit::Exit(double x, double y, StudentWorld * world)
	:Activating(IID_EXIT, x, y, world, right, 1)
{
}

void Exit::tryActivate(Actor * a)
{
	if (a == getWorld()->player())					// Player is on the exit and there are no more Citizens
	{
		if (getWorld()->nCitizens() == 0)
		{
			getWorld()->player()->finishLevel();	// End the level
			return;
		}
	}
	else if (a->infectable())
	{
		a->setToRemove();	// Remove the Citizen
		getWorld()->decCitizens();
		getWorld()->increaseScore(500);
		getWorld()->playSound(SOUND_CITIZEN_SAVED);
	}
}

//
//	Pit
//

Pit::Pit(double x, double y, StudentWorld * world)
	:Activating(IID_PIT, x, y, world, right, 0)
{
}

void Pit::tryActivate(Actor * a)
{
	if (a->dieFromHazard())
		a->kill();
}

//
//	Landmine
//

Landmine::Landmine(double x, double y, StudentWorld * world)
	:Activating(IID_LANDMINE, x, y, world, right, 1), m_safetyTicks(30), m_armed(false)
{
}

void Landmine::something()
{
	if (m_safetyTicks >= 0)
	{
		if (m_safetyTicks == 0)
			m_armed = true;

		m_safetyTicks--;	// If the Landmine is armed, m_safetyTicks < 0, so this code will never execute again
		stop();
		return;
	}
}

void Landmine::tryActivate(Actor * a)
{
	if (a->infectable() || a->eatsBrains() || a->burns())
	{
		kill();
	}
}

void Landmine::dyingAction()
{
	getWorld()->playSound(SOUND_LANDMINE_EXPLODE);
																							// Allocate Flames:
	getWorld()->addActor(new Flame(getX(), getY(), getWorld(), up));								// At point
	getWorld()->addActor(new Flame(getX(), getY() + SPRITE_HEIGHT, getWorld(), up));				// Top
	getWorld()->addActor(new Flame(getX(), getY() - SPRITE_HEIGHT, getWorld(), up));				// Bottom
	getWorld()->addActor(new Flame(getX() - SPRITE_WIDTH, getY(), getWorld(), up));					// Left
	getWorld()->addActor(new Flame(getX() + SPRITE_WIDTH, getY(), getWorld(), up));					// Right
	getWorld()->addActor(new Flame(getX() + SPRITE_WIDTH, getY() + SPRITE_HEIGHT, getWorld(), up));	// Top right
	getWorld()->addActor(new Flame(getX() - SPRITE_WIDTH, getY() + SPRITE_HEIGHT, getWorld(), up));	// Top left
	getWorld()->addActor(new Flame(getX() + SPRITE_WIDTH, getY() - SPRITE_HEIGHT, getWorld(), up));	// Bottom right
	getWorld()->addActor(new Flame(getX() - SPRITE_WIDTH, getY() - SPRITE_HEIGHT, getWorld(), up));	// Bottom left

	getWorld()->addActor(new Pit(getX(), getY(), getWorld()));	// Allocate Pit
}

//
//	Flame
//

Flame::Flame(double x, double y, StudentWorld * world, int dir)
	:Projectile(IID_FLAME, x, y, world, dir)
{
}

void Flame::tryActivate(Actor * a)
{
	if (a->dieFromHazard())
		a->kill();
}

//
//	Vomit
//

Vomit::Vomit(double x, double y, StudentWorld * world, int dir)
	:Projectile(IID_VOMIT, x, y, world, dir)
{
}

void Vomit::tryActivate(Actor * a)
{
	if (a->infectable())
		a->infect();
}

//
//	VaccineGoodie
//

VaccineGoodie::VaccineGoodie(double x, double y, StudentWorld * world)
	:Goodie(IID_VACCINE_GOODIE, x, y, world)
{
}

//
//	GasCanGoodie
//

GasCanGoodie::GasCanGoodie(double x, double y, StudentWorld * world)
	:Goodie(IID_GAS_CAN_GOODIE, x, y, world)
{
}

//
//	LandmineGoodie
//

LandmineGoodie::LandmineGoodie(double x, double y, StudentWorld * world)
	:Goodie(IID_LANDMINE_GOODIE, x, y, world)
{
}
