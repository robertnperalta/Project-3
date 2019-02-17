#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>

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

	bool overlap(double x, double y);
	bool blocked(double x, double y, const Actor* moving);

private:
	std::list<Actor*> m_actors;
	Player* m_player;
};

#endif // STUDENTWORLD_H_
