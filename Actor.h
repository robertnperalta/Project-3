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
	Actor(int imageID, double startX, double startY, StudentWorld* world, 
		int moveDist = 0, int dir = 0, int depth = 0);
	virtual ~Actor() {}

	virtual void doSomething() = 0;

	// Characteristics

	virtual bool impassable() const { return false; }
	virtual bool blocksProj() const { return false; }
	virtual bool dieFromFire() const { return false; }
	virtual bool dieFromPit() const { return false; }
	virtual bool infectable() const { return false; }

	// Accessors

	StudentWorld* getWorld() const { return m_world; }
	int moveDist() const { return m_moveDist; }
	bool alive() const { return m_alive; }
	bool infected() const { return m_infected; }
	int infectedCount() const { return m_infectedCount; }

	// Mutators

	void setToRemove() { m_alive = false; }		// Sets for game to remove
	virtual void kill() { setToRemove(); }		// Does other actions (points, sounds, etc.)

	void infect() { m_infected = true; }
	void incInfect() { m_infected++; }
	void vaccinate() { m_infected = false; m_infectedCount = 0; }

	// Check contact

	bool inBoundary(double x, double y, const Actor* moving) const;
	bool overlapping(double x, double y, const Actor* compare) const;

protected:
	bool move(int dir);

private:
	StudentWorld* m_world;
	int m_moveDist;
	bool m_alive;
	bool m_infected;
	int m_infectedCount;
};

//////////////////////////////
//		GAME OBJECTS		//
//////////////////////////////

//
//	Player
//

class Player : public Actor
{
public:
	Player(double startX, double startY, StudentWorld* world);
	virtual ~Player() {}

	virtual void doSomething();

	// Characteristics

	virtual bool impassable() const { return true; }
	virtual bool dieFromFire() const { return true; }
	virtual bool dieFromPit() const { return true; }
	virtual bool infectable() const { return true; }

	int nVacs() const { return m_nVacs; }
	int nFlames() const { return m_nFlames; }
	int nMines() const { return m_nMines; }

	virtual void kill() { setToRemove(); getWorld()->playSound(SOUND_PLAYER_DIE); }

private:
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
	virtual bool impassable() const { return true; }
};

//
//	Exit
//

class Exit : public Actor
{
public:
	Exit()
};

#endif // ACTOR_H_
