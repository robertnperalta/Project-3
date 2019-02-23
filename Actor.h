#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

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
	virtual void dyingAction() = 0;

	virtual void infect() {}

	// Check contact

	bool inBoundary(double x, double y, const Actor* moving) const;
	bool overlapping(double x, double y, const Actor* compare) const;

	// Characteristics

	virtual bool impassable() const { return false; }
	virtual bool blocksProj() const { return false; }
	virtual bool dieFromFire() const { return false; }
	virtual bool dieFromPit() const { return false; }
	virtual bool infectable() const { return false; }
	virtual bool canExit() const { return false; }

private:
	StudentWorld* m_world;
	bool m_alive;
};

//
//	Agent
//

class Agent : public Actor
{
public:
	Agent(int imageID, double x, double y, StudentWorld* world, int moveDist);
	virtual ~Agent() {}

	// Characteristics

	virtual bool impassable() const { return true; }
	virtual bool dieFromFire() const { return true; }
	virtual bool dieFromPit() const { return true; }

protected:
	bool tryMove(int dir);

private:
	int m_moveDist;
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

	virtual void infect() { m_infected = true; }
	void incInfect() { m_infectedCount++; }
	void vaccinate() { m_infected = false; m_infectedCount = 0; }

	// Characteristics

	virtual bool infectable() const { return true; }
	virtual bool canExit() const { return true; }

private:
	bool m_infected;
	int m_infectedCount;
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
	Player(double startX, double startY, StudentWorld* world);
	virtual ~Player() {}

	virtual void doSomething();

	// Accessors

	int nVacs() const { return m_nVacs; }
	int nFlames() const { return m_nFlames; }
	int nMines() const { return m_nMines; }
	bool finished() const { return m_finished; }

	// Mutators

	void finishLevel() { m_finished = true; }
	virtual void dyingAction() { getWorld()->playSound(SOUND_PLAYER_DIE); }

private:
	int m_nVacs;
	int m_nFlames;
	int m_nMines;
	bool m_finished;
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

	virtual bool impassable() const { return true; }
	virtual void dyingAction() {}
};

//
//	Exit
//

class Exit : public Actor
{
public:
	Exit(double startX, double startY, StudentWorld* world);
	virtual ~Exit() {}

	virtual void doSomething();
	virtual void dyingAction() {}
};

#endif // ACTOR_H_
