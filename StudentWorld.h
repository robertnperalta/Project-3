#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>
#include <cmath>

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class Actor;
class Player;

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

	Player* player() const { return m_player; }

	void overlap(double x, double y, std::list<Actor*>& trues, const Actor* compare);
	bool overlapsPlayer(double x, double y, const Actor* compare);
	void blocked(double x, double y, std::list<Actor*>& trues, const Actor* moving);
	void addActor(Actor* a) { m_actors.push_back(a); }
	double distToZombie(double x, double y);
	double distToPlayer(double x, double y);

private:
	std::list<Actor*> m_actors;
	Player* m_player;
	int m_nCitizens;
};

#endif // STUDENTWORLD_H_
