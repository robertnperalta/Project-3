#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"
#include <list>

//////////////////////////////
//		BASE CLASSES		//
//////////////////////////////

//
//	Actor
//

class Actor : public GraphObject
{
public:
	Actor(int imageID, double startX, double startY, StudentWorld* world, int dir = right, int depth = 0);
	virtual ~Actor() {}

	virtual void doSomething() = 0;

	// Accessors

	StudentWorld* getWorld() const { return m_world; }
	bool alive() const { return m_alive; }

	// Mutators

	void setToRemove() { m_alive = false; }				// Sets for game to remove
	void kill() { setToRemove(); dyingAction(); }		// Does other actions (points, sounds, etc.)
	virtual void dyingAction() {}

	virtual void infect() {}

	// Check contact

	bool inBoundary(double x, double y, const Actor* moving) const;
	bool overlapping(double x, double y, const Actor* compare) const;

	// Characteristics

	virtual bool impassable() const { return false; }
	virtual bool blocksFire() const { return false; }
	virtual bool blocksVomit() const { return false; }
	virtual bool dieFromHazard() const { return false; }
	virtual bool infectable() const { return false; }
	virtual bool eatsBrains() const { return false; }

private:
	StudentWorld* m_world;
	bool m_alive;
};

//
//	Activating
//

class Activating : public Actor
{
public:
	Activating(int imageID, double x, double y, StudentWorld* world, int dir, int depth);
	virtual ~Activating() {}

	virtual void doSomething();
	virtual void something() {}
	virtual void tryActivate(Actor* a) = 0;

	// Accessors

	std::list<Actor*>::iterator overlapsBegin() { return m_overlaps.begin(); }
	std::list<Actor*>::iterator overlapsEnd() { return m_overlaps.end(); }
	bool playerOverlaps() const { return m_playerOverlaps; }

private:
	std::list<Actor*> m_overlaps;
	bool m_playerOverlaps;
};

//
//	Agent
//

class Agent : public Actor
{
public:
	Agent(int imageID, double x, double y, StudentWorld* world, int moveDist);
	virtual ~Agent() {}

	// Accessors

	int moveDist() { return m_moveDist; }
	bool paralyzed() { return m_paralyzed; }

	// Mutators

	void skipNextTurn() { m_paralyzed = true; }
	void takeNextTurn() { m_paralyzed = false; }

	// Characteristics

	virtual bool impassable() const { return true; }
	virtual bool dieFromHazard() const { return true; }

protected:
	void findDest(int dir, int distX, int distY, double& dest_x, double& dest_y);
	bool tryMove(int dir);

private:
	int m_moveDist;
	bool m_paralyzed;
};

//
//	Human
//

class Human : public Agent
{
public:
	Human(int imageID, double x, double y, StudentWorld* world, int moveDist);
	virtual ~Human() {}

	// Accessors

	bool infected() { return m_infected; }
	int infectedCount() { return m_infectedCount; }

	// Mutators

	virtual void infect() { whenInfected(); m_infected = true; }
	virtual void whenInfected() {}
	void incInfect() { m_infectedCount++; }
	void vaccinate() { m_infected = false; m_infectedCount = 0; }

	// Characteristics

	virtual bool infectable() const { return true; }

private:
	bool m_infected;
	int m_infectedCount;
};

class Zombie : public Agent
{
public:
	Zombie(double x, double y, StudentWorld* world);
	virtual ~Zombie() {}

	virtual void doSomething();
	void something();

	// Characteristics

	virtual bool eatsBrains() const { return true; }

private:
	int m_plan;
	virtual int pickDirection() = 0;
};

//
//	Goodie
//

class Goodie : public Activating
{
public:
	Goodie(int imageID, double x, double y, StudentWorld* world);
	virtual ~Goodie() {}

	virtual void tryActivate(Actor* a);
	virtual void incContains() = 0;

	virtual void dyingAction();

	// Characteristics

	virtual bool dieFromHazards() { return true; }
};

//////////////////////////////
//		GAME OBJECTS		//
//////////////////////////////

//
//	Player
//

class Player : public Human
{
public:
	Player(double x, double y, StudentWorld* world);
	virtual ~Player() {}

	virtual void doSomething();

	// Accessors

	int nVacs() const { return m_nVacs; }
	int nFlames() const { return m_nFlames; }
	int nMines() const { return m_nMines; }
	bool finished() const { return m_finished; }

	// Mutators

	void finishLevel() { m_finished = true; }
	virtual void dyingAction() { getWorld()->decLives(); getWorld()->playSound(SOUND_PLAYER_DIE); }
	void incVacs() { m_nVacs++; }
	void incFlames() { m_nFlames++; }
	void incMines() { m_nMines++; }

private:
	int m_nVacs;
	int m_nFlames;
	int m_nMines;
	bool m_finished;
};

//
//	Citizen
//

class Citizen : public Human
{
public:
	Citizen(double x, double y, StudentWorld* world);
	virtual ~Citizen() {}

	virtual void doSomething();
	void something();
	void turnIntoZombie();

	virtual void whenInfected();
	virtual void dyingAction();
};

//
//	DumbZombie
//

class DumbZombie : public Zombie
{
public:
	DumbZombie(double x, double y, StudentWorld* world);
	virtual ~DumbZombie() {}

	virtual void dyingAction();

private:
	virtual int pickDirection();
};

//
//	SmartZombie
//

class SmartZombie : public Zombie
{
public:
	SmartZombie(double x, double y, StudentWorld* world);
	virtual ~SmartZombie() {}

	virtual void dyingAction() { getWorld()->playSound(SOUND_ZOMBIE_DIE); getWorld()->increaseScore(2000); }

private:
	virtual int pickDirection();
};

//
//	Wall
//

class Wall : public Actor
{
public:
	Wall(double startX, double startY, StudentWorld* world);
	virtual ~Wall() {}

	virtual void doSomething() {}

	// Characteristics

	virtual bool impassable() const { return true; }
	virtual bool blocksFire() const { return true; }
	virtual bool blocksVomit() const { return true; }
};

//
//	Exit
//

class Exit : public Activating
{
public:
	Exit(double x, double y, StudentWorld* world);
	virtual ~Exit() {}

	virtual void tryActivate(Actor* a);

	// Characteristics

	virtual bool blocksFire() const { return true; }
};

//
//	Pit
//

class Pit : public Activating
{
public:
	Pit(double x, double y, StudentWorld* world);
	virtual ~Pit() {}

	virtual void tryActivate(Actor* a);
};

//
//	Flame
//

class Flame : public Activating
{
public:
	Flame(double x, double y, StudentWorld* world, int dir);
	virtual ~Flame() {}

	virtual void something();
	virtual void tryActivate(Actor* a);

private:
	int m_ticksAlive;
};

//
//	VaccineGoodie
//

class VaccineGoodie : public Goodie
{
public:
	VaccineGoodie(double x, double y, StudentWorld* world);
	virtual ~VaccineGoodie() {}

	virtual void incContains() { getWorld()->player()->incVacs(); }
};

//
//	GasCanGoodie
//

class GasCanGoodie : public Goodie
{
public:
	GasCanGoodie(double x, double y, StudentWorld* world);
	virtual ~GasCanGoodie() {}

	virtual void incContains() { getWorld()->player()->incFlames(); }
};

//
//	LandmineGoodie
//

class LandmineGoodie : public Goodie
{
public:
	LandmineGoodie(double x, double y, StudentWorld* world);
	virtual ~LandmineGoodie() {}

	virtual void incContains() { getWorld()->player()->incMines(); }
};

#endif // ACTOR_H_
