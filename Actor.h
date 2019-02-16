#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include "StudentWorld.h"

//
//	Actor
//

class Actor : public GraphObject
{
public:
	Actor(int imageID, double startX, double startY,
		StudentWorld* world, int dir = 0, int depth = 0);
	virtual ~Actor() {}

	virtual void doSomething() = 0;
	virtual bool impassable() const { return false; }

	StudentWorld* getWorld() const { return m_world; }
	bool inBoundary(double x, double y) const;
	bool move(int dir);

private:
	StudentWorld* m_world;
};

//
//	Player
//

class Player : public Actor
{
public:
	Player(double startX, double startY, StudentWorld* world);
	virtual ~Player() {}

	virtual void doSomething();
	virtual bool impassable() { return true; }

	bool isAlive() const { return m_alive; }
	int infectedCount() const { return m_infectedCount; }
	int nVacs() const { return m_nVacs; }
	int nFlames() const { return m_nFlames; }
	int nMines() const { return m_nMines; }

	void kill() { m_alive = false; getWorld()->playSound(SOUND_PLAYER_DIE); }
	void infect() { m_infected = true; }

private:
	bool m_alive;
	bool m_infected;
	int m_infectedCount;
	int m_nVacs;
	int m_nFlames;
	int m_nMines;
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
	virtual bool impassable() { return true; }
};

#endif // ACTOR_H_
