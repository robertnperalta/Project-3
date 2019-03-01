#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>
#include <cmath>

class Actor;
class Player;
class Activating;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetPath);
	virtual ~StudentWorld();

    virtual int init();
    virtual int move();
    virtual void cleanUp();

	int nCitizens() const { return m_nCitizens; }
	void decCitizens() { m_nCitizens--; }

	// Interfacing with Player

	double playerX() const;	// Defined in .cpp file although trivial
	double playerY() const;	// to prevent circular dependency
	void playerFinish();
	void addFlames();
	void addVacs();
	void addMines();

	int randDir();
	void addActor(Actor* a) { m_actors.push_back(a); }
	double distToZombie(double x, double y);
	double distToPlayer(double x, double y);
	bool blocked(double x, double y, const Actor* moving);
	int bestDirToTarget(Actor* searching);

	// Conditional overlaps

	bool overlapsPlayer(double x, double y, const Actor* compare);
	bool isOverlapped(double x, double y, const Actor* compare);
	bool foundTarget(double x, double y, const Actor* a);
	bool flameBlocked(double x, double y, const Actor* a);
	void activateOnOverlaps(Activating* checking);

private:
	void overlap(double x, double y, std::list<Actor*>& trues, const Actor* compare);
	double distToCitizen(double x, double y, Actor*& closest);
	std::list<Actor*> m_actors;
	Player* m_player;
	int m_nCitizens;
};

#endif // STUDENTWORLD_H_
