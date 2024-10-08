#ifndef SURVIVAL_MOB_H
#define SURVIVAL_MOB_H

#include <tamtypes.h>
#include <libdl/moby.h>
#include <libdl/math.h>
#include <libdl/time.h>
#include <libdl/player.h>
#include <libdl/sound.h>
#include "zombie.h"
#include "executioner.h"
#include "tremor.h"
#include "game.h"

enum MobAction
{
	MOB_ACTION_SPAWN,
	MOB_ACTION_IDLE,
	MOB_ACTION_JUMP,
	MOB_ACTION_WALK,
	MOB_ACTION_FLINCH,
	MOB_ACTION_BIG_FLINCH,
	MOB_ACTION_LOOK_AT_TARGET,
  MOB_ACTION_DIE,
	MOB_ACTION_ATTACK,
	MOB_ACTION_TIME_BOMB,
  MOB_ACTION_TIME_BOMB_EXPLODE
};

enum MobAttributeType
{
	MOB_ATTRIBUTE_NONE = 0,
	MOB_ATTRIBUTE_FREEZE,
	MOB_ATTRIBUTE_ACID,
	MOB_ATTRIBUTE_GHOST,
  MOB_ATTRIBUTE_EXPLODE,
	MOB_ATTRIBUTE_COUNT
};

enum MobEvent
{
	MOB_EVENT_SPAWN,
	MOB_EVENT_DESTROY,
	MOB_EVENT_DAMAGE,
	MOB_EVENT_STATE_UPDATE,
	MOB_EVENT_TARGET_UPDATE,
	MOB_EVENT_OWNER_UPDATE,
};

// what kind of spawn a mob can have
enum MobSpawnType {
	SPAWN_TYPE_DEFAULT_RANDOM = 0,
	SPAWN_TYPE_SEMI_NEAR_PLAYER = 1,
	SPAWN_TYPE_NEAR_PLAYER = 2,
	SPAWN_TYPE_ON_PLAYER = 4,
	SPAWN_TYPE_NEAR_HEALTHBOX = 8,
};

// 
enum MobUnreliableMsgId {
  MOB_UNRELIABLE_MSG_ID_STATE_UPDATE
};

struct MobDamageEventArgs;
struct MobSpawnEventArgs;
struct MobStateUpdateEventArgs;

typedef void (*MobGenericCallback_func)(Moby* moby);
typedef Moby* (*MobGetNextTarget_func)(Moby* moby);
typedef enum MobAction (*MobGetPreferredAction_func)(Moby* moby);
typedef void (*MobOnSpawn_func)(Moby* moby, VECTOR position, float yaw, u32 spawnFromUID, char random, struct MobSpawnEventArgs* e);
typedef void (*MobOnDestroy_func)(Moby* moby, int killedByPlayerId, int weaponId);
typedef void (*MobOnDamage_func)(Moby* moby, struct MobDamageEventArgs* e);
typedef void (*MobOnStateUpdate_func)(Moby* moby, struct MobStateUpdateEventArgs* e);
typedef void (*MobForceLocalAction_func)(Moby* moby, enum MobAction action);
typedef void (*MobDoDamage_func)(Moby* moby, float radius, float amount, int damageFlags, int friendlyFire);
typedef short (*MobGetArmor_func)(Moby* moby);

struct MobVTable {
  MobGenericCallback_func PreUpdate;
  MobGenericCallback_func PostUpdate;
  MobGenericCallback_func PostDraw;
  MobGenericCallback_func Move;
  MobOnSpawn_func OnSpawn;
  MobOnDestroy_func OnDestroy;
  MobOnDamage_func OnDamage;
  MobOnStateUpdate_func OnStateUpdate;
  MobGetNextTarget_func GetNextTarget;
  MobGetPreferredAction_func GetPreferredAction;
  MobForceLocalAction_func ForceLocalAction;
  MobGenericCallback_func DoAction;
  MobDoDamage_func DoDamage;
  MobGetArmor_func GetArmor;
};

struct MobConfig {
	int Bolts;
	float Damage;
	float MaxDamage;
  float DamageScale;
	float Speed;
	float MaxSpeed;
  float SpeedScale;
	float Health;
	float MaxHealth;
  float HealthScale;
	float AttackRadius;
	float HitRadius;
  float CollRadius;
	u16 Bangles;
	u8 Xp;
	u8 MaxCostMutation;
	u8 ReactionTickCount;
	u8 AttackCooldownTickCount;
	char MobAttribute;
};

struct MobSpawnParams {
	int Cost;
	int MinRound;
	int CooldownTicks;
	float Probability;
  enum MobStatId StatId;
	enum MobSpawnType SpawnType;
	char Name[32];
	struct MobConfig Config;
};

struct Knockback {
	short Angle;
	u8 Power;
	u8 Ticks;
  char Force;
};

struct MobMoveVars {
  VECTOR LastPosition;
  VECTOR NextPosition;
  VECTOR Velocity;
  VECTOR AddVelocity;
  VECTOR LastJumpPosition;
  VECTOR SumPositionDelta;
  VECTOR LastTargetPos;
  float SumSpeedOver;
  float WallSlope;
  float PathEdgeAlpha;
	u16 StuckTicks;
  char Grounded;
  char HitWall;
  char IsStuck;
  u8 UngroundedTicks;
  u8 StuckCheckTicks;
  u8 StuckJumpCount;
  u8 MoveSkipTicks;

  char PathEdgeCount;
  char PathEdgeCurrent;
  char PathHasReachedStart;
  char PathHasReachedEnd;
  u8 PathStartEndNodes[2];
  u8 PathTicks;
  u8 PathNewTicks;
  u8 PathCheckNearAndSeeTargetTicks;
  u8 CurrentPath[20];
};

struct MobVars {
	struct MobConfig Config;
	struct Knockback Knockback;
  struct MobMoveVars MoveVars;
  int SpawnParamsIdx;
  int FreeAgent;
  VECTOR TargetPosition;
	int Action;
	int NextAction;
	int LastAction;
	float Health;
	float ClosestDist;
	float LastSpeed;
	Moby * Target;
	int LastHitBy;
	u16 LastHitByOClass;
	u16 NextCheckActionDelayTicks;
	u16 NextActionDelayTicks;
	u16 ActionCooldownTicks;
	u16 AttackCooldownTicks;
	u16 ScoutCooldownTicks;
	u16 FlinchCooldownTicks;
	u16 AutoDirtyCooldownTicks;
	u16 AmbientSoundCooldownTicks;
	u16 TimeBombTicks;
	u16 MovingTicks;
	u16 CurrentActionForTicks;
	u16 TimeLastGroundedTicks;
	u8 ActionId;
	u8 LastActionId;
	char Owner;
	char IsTraversing;
	char AnimationLooped;
	char AnimationReset;
	char OpacityFlickerDirection;
	char Destroy;
	char Respawn;
	char Dirty;
	char Destroyed;
	char Order;
	char Random;
  char BlipType;
};

// warning: multiple differing types with the same name, only one recovered
struct ReactVars {
	/*   0 */ int flags;
	/*   4 */ int lastReactFrame;
	/*   8 */ Moby* pInfectionMoby;
	/*   c */ float acidDamage;
	/*  10 */ char eternalDeathCount;
	/*  11 */ char doHotSpotChecks;
	/*  12 */ char unchainable;
	/*  13 */ char isShielded;
	/*  14 */ char state;
	/*  15 */ char deathState;
	/*  16 */ char deathEffectState;
	/*  17 */ char deathStateType;
	/*  18 */ signed char knockbackRes;
	/*  19 */ char padC;
	/*  1a */ char padD;
	/*  1b */ char padE;
	/*  1c */ char padF;
	/*  1d */ char padG;
	/*  1e */ char padH;
	/*  1f */ char padI;
	/*  20 */ float minorReactPercentage;
	/*  24 */ float majorReactPercentage;
	/*  28 */ float deathHeight;
	/*  2c */ float bounceDamp;
	/*  30 */ int deadlyHotSpots;
	/*  34 */ float curUpGravity;
	/*  38 */ float curDownGravity;
	/*  3c */ float shieldDamageReduction;
	/*  40 */ float damageReductionSameOClass;
	/*  44 */ int deathCorn;
	/*  48 */ int deathType;
	/*  4c */ short int deathSound;
	/*  4e */ short int deathSound2;
	/*  50 */ float peakFrame;
	/*  54 */ float landFrame;
	/*  58 */ float drag;
	/*  5c */ short unsigned int effectStates;
	/*  5e */ short unsigned int effectPrimMask;
	/*  60 */ short unsigned int effectTimers[16];
};

struct MobPVar {
	struct TargetVars * TargetVarsPtr;
	char _pad0[0x0C];
	struct ReactVars * ReactVarsPtr;
	char _pad1[0x08];
  struct MoveVars_V2 * MoveVarsPtr;
	char _pad2[0x14];
 	struct FlashVars * FlashVarsPtr;
	char _pad3[0x18];

	struct TargetVars TargetVars;
	struct ReactVars ReactVars;
	struct FlashVars FlashVars;
	struct MobVars MobVars;
  struct MobVTable* VTable;
};

struct MobDamageEventArgs
{
	struct Knockback Knockback;
	int SourceUID;
  u32 DamageFlags;
	u16 DamageQuarters;
	u16 SourceOClass;
};

struct MobActionUpdateEventArgs
{
	int Action;
	u8 ActionId;
};

struct MobStateUpdateEventArgs
{
	VECTOR Position;
	int TargetUID;
  u8 PathStartNodeIdx;
  u8 PathEndNodeIdx;
  u8 PathCurrentEdgeIdx;
  char PathHasReachedStart;
  char PathHasReachedEnd;
};

struct MobSpawnEventArgs
{
	int Bolts;
	u16 StartHealth;
	u16 Bangles;
	u16 SpeedEighths;
	u16 Damage;
	char MobType;
	char MobAttribute;
  u8 SpawnParamsIdx;
	u8 Xp;
	u8 AttackRadiusEighths;
	u8 HitRadiusEighths;
  u8 CollRadiusEighths;
	u8 ReactionTickCount;
	u8 AttackCooldownTickCount;
};

struct MobUnreliableBaseMsgArgs
{
  int MsgId;
  u32 MobUID;
};

struct MobUnreliableMsgStateUpdateArgs
{
  struct MobUnreliableBaseMsgArgs Base;
  struct MobStateUpdateEventArgs StateUpdate;
};

int mobOnUnreliableMsgRemote(void* connection, void* data);
void mobReactToExplosionAt(int byPlayerId, VECTOR position, float damage, float radius);
void mobNuke(int killedByPlayerId);
int mobCreate(int spawnParamsIdx, VECTOR position, float yaw, int spawnFromUID, int freeAgent, struct MobConfig *config);
void mobInitialize(void);
void mobTick(void);

#endif // SURVIVAL_MOB_H
