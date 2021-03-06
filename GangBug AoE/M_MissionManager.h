#ifndef _M_MISION_MANAGER_
#define _M_MISION_MANAGER_

#include "j1Timer.h"
#include "Module.h"

enum Mission_State 
{
	M_INTRO,
	M_TOWNATTACK,
	M_TOWNREPAIR,
	M_WAVES,
	M_BOSS,
	M_VICTORY,
	M_DEFEAT,
	M_STANDBY
};
#define MISION_TIME 1

#define TOWNREPAIR_TIME 130

#define TROOPS_ONTOWN 1 

#define ENEMIES_TO_DEFEAT_WAVES 260

class M_MissionManager : public Module
{
public:
	M_MissionManager(bool startEnabled = true);
	virtual ~M_MissionManager();

	bool Start();
	update_status Update(float dt)override;
	void DrawDebug()override{}
	bool CleanUp() override;


	uint GetEnemyUnits()const;
	std::string GetStateName()const;
	std::string GetObjectiveState1()const { return ObjectiveState1; }
	std::string GetObjectiveState2()const { return ObjectiveState2; }
	std::string GetObjectiveState3()const { return ObjectiveState3; }
	std::string GetObjectiveState4()const { return ObjectiveState4; }

	Timer GetMisionTime()const;
	float GetMisionTimeleftf()const;
	bool GetBossState()const;
	uint GetEnemyDeadUnits()const;


	void AddStartUnit();
	void AddEnemyDeadUnit();
	void SetBossState(bool state);
	void TheTownCenterIsDead();
	void TheTownCenterInsAlive();
	Mission_State GetState()const;

	void SetState(Mission_State _newstate);

	//Easy / Hard Mode system
	bool getHardModeStatus();
	void setGameToHardMode(bool mode);

	bool Load(pugi::xml_node&);
	bool Save(pugi::xml_node&) const;

public:

	Timer misionTimer;

private:
	Mission_State State;

private://troops states

	uint enemyTroopCounter;
	uint enemyStartTroops;
	uint aliveTroopWave;
	uint enemyDeadUnits;

	std::string stateName;

	std::string ObjectiveState1;
	std::string ObjectiveState2;
	std::string ObjectiveState3;
	std::string ObjectiveState4;
	

	bool bossIsAlive;
	bool townCenterIsAlive;
	bool isHardModeActive;
};


#endif 