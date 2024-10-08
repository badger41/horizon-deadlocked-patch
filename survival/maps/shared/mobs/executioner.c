#include <libdl/stdio.h>
#include <libdl/game.h>
#include <libdl/collision.h>
#include <libdl/graphics.h>
#include <libdl/moby.h>
#include <libdl/random.h>
#include <libdl/radar.h>
#include <libdl/color.h>

#include "../../../include/game.h"
#include "../../../include/mob.h"
#include "../include/maputils.h"

int mobAmIOwner(Moby* moby);
int mobIsFrozen(Moby* moby);
void mobDoDamage(Moby* moby, float radius, float amount, int damageFlags, int friendlyFire, int jointId);
void mobSetAction(Moby* moby, int action);
void mobTransAnimLerp(Moby* moby, int animId, int lerpFrames, float startOff);
void mobTransAnim(Moby* moby, int animId, float startOff);
int mobHasVelocity(struct MobPVar* pvars);
void mobStand(Moby* moby);
int mobMoveCheck(Moby* moby, VECTOR outputPos, VECTOR from, VECTOR to);
void mobMove(Moby* moby);
void mobTurnTowards(Moby* moby, VECTOR towards, float turnSpeed);
void mobGetVelocityToTarget(Moby* moby, VECTOR velocity, VECTOR from, VECTOR to, float speed, float acceleration);
void mobPostDrawQuad(Moby* moby, int texId, u32 color);
void mobOnStateUpdate(Moby* moby, struct MobStateUpdateEventArgs e);

void executionerPreUpdate(Moby* moby);
void executionerPostUpdate(Moby* moby);
void executionerPostDraw(Moby* moby);
void executionerMove(Moby* moby);
void executionerOnSpawn(Moby* moby, VECTOR position, float yaw, u32 spawnFromUID, char random, struct MobSpawnEventArgs e);
void executionerOnDestroy(Moby* moby, int killedByPlayerId, int weaponId);
void executionerOnDamage(Moby* moby, struct MobDamageEventArgs e);
void executionerOnStateUpdate(Moby* moby, struct MobStateUpdateEventArgs e);
Moby* executionerGetNextTarget(Moby* moby);
enum MobAction executionerGetPreferredAction(Moby* moby);
void executionerDoAction(Moby* moby);
void executionerDoDamage(Moby* moby, float radius, float amount, int damageFlags, int friendlyFire);
void executionerForceLocalAction(Moby* moby, enum MobAction action);
short executionerGetArmor(Moby* moby);

void executionerPlayHitSound(Moby* moby);
void executionerPlayAmbientSound(Moby* moby);
void executionerPlayDeathSound(Moby* moby);
int executionerIsAttacking(struct MobPVar* pvars);
int executionerIsSpawning(struct MobPVar* pvars);
int executionerCanAttack(struct MobPVar* pvars);

struct MobVTable ExecutionerVTable = {
  .PreUpdate = &executionerPreUpdate,
  .PostUpdate = &executionerPostUpdate,
  .PostDraw = &executionerPostDraw,
  .Move = &executionerMove,
  .OnSpawn = &executionerOnSpawn,
  .OnDestroy = &executionerOnDestroy,
  .OnDamage = &executionerOnDamage,
  .OnStateUpdate = &executionerOnStateUpdate,
  .GetNextTarget = &executionerGetNextTarget,
  .GetPreferredAction = &executionerGetPreferredAction,
  .ForceLocalAction = &executionerForceLocalAction,
  .DoAction = &executionerDoAction,
  .DoDamage = &executionerDoDamage,
  .GetArmor = &executionerGetArmor,
};

SoundDef ExecutionerSoundDef = {
	0.0,	  // MinRange
	45.0,	  // MaxRange
	0,		  // MinVolume
	1228,		// MaxVolume
	-635,			// MinPitch
	635,			// MaxPitch
	0,			// Loop
	0x10,		// Flags
	0x17D,		// Index
	3			  // Bank
};

extern u32 MobPrimaryColors[];
extern u32 MobSecondaryColors[];
extern u32 MobLODColors[];

//--------------------------------------------------------------------------
int executionerCreate(int spawnParamsIdx, VECTOR position, float yaw, int spawnFromUID, int freeAgent, struct MobConfig *config)
{
	struct MobSpawnEventArgs args;
  
	// create guber object
	GuberEvent * guberEvent = 0;
	guberMobyCreateSpawned(EXECUTIONER_MOBY_OCLASS, sizeof(struct MobPVar), &guberEvent, NULL);
	if (guberEvent)
	{
    if (MapConfig.PopulateSpawnArgsFunc) {
      MapConfig.PopulateSpawnArgsFunc(&args, config, spawnParamsIdx, spawnFromUID == -1, freeAgent);
    }

		u8 random = (u8)rand(100);

    position[2] += 1; // spawn slightly above point
		guberEventWrite(guberEvent, position, 12);
		guberEventWrite(guberEvent, &yaw, 4);
		guberEventWrite(guberEvent, &spawnFromUID, 4);
		guberEventWrite(guberEvent, &freeAgent, 4);
		guberEventWrite(guberEvent, &random, 1);
		guberEventWrite(guberEvent, &args, sizeof(struct MobSpawnEventArgs));
	}
	else
	{
		DPRINTF("failed to guberevent mob\n");
	}
  
  return guberEvent != NULL;
}

//--------------------------------------------------------------------------
void executionerPreUpdate(Moby* moby)
{
  if (!moby || !moby->PVar)
    return;
    
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
  if (mobIsFrozen(moby))
    return;

	if (!pvars->MobVars.AmbientSoundCooldownTicks) {
		executionerPlayAmbientSound(moby);
		pvars->MobVars.AmbientSoundCooldownTicks = randRangeInt(EXECUTIONER_AMBSND_MIN_COOLDOWN_TICKS, EXECUTIONER_AMBSND_MAX_COOLDOWN_TICKS);
	}

  // decrement path target pos ticker
  decTimerU8(&pvars->MobVars.MoveVars.PathTicks);
  decTimerU8(&pvars->MobVars.MoveVars.PathCheckNearAndSeeTargetTicks);
  decTimerU8(&pvars->MobVars.MoveVars.PathNewTicks);
}

//--------------------------------------------------------------------------
void executionerPostUpdate(Moby* moby)
{
  if (!moby || !moby->PVar)
    return;
    
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;

  // adjust animSpeed by speed and by animation
	float animSpeed = 0.6 * (pvars->MobVars.Config.Speed / MOB_BASE_SPEED);
  if (moby->AnimSeqId == EXECUTIONER_ANIM_JUMP) {
    animSpeed = 1 * (1 - powf(moby->AnimSeqT / 21, 1));
    if (pvars->MobVars.MoveVars.Grounded) {
      animSpeed = 0.6;
    }
  }

	if (mobIsFrozen(moby) || (moby->DrawDist == 0 && pvars->MobVars.Action == MOB_ACTION_WALK)) {
		moby->AnimSpeed = 0;
	} else {
		moby->AnimSpeed = animSpeed;
	}
}

//--------------------------------------------------------------------------
void executionerPostDraw(Moby* moby)
{
  if (!moby || !moby->PVar)
    return;
    
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
  u32 color = MobLODColors[pvars->MobVars.SpawnParamsIdx] | (moby->Opacity << 24);
  mobPostDrawQuad(moby, 127, color);
}

//--------------------------------------------------------------------------
void executionerAlterTarget(VECTOR out, Moby* moby, VECTOR forward, float amount)
{
	VECTOR up = {0,0,1,0};
	
	vector_outerproduct(out, forward, up);
	vector_normalize(out, out);
	vector_scale(out, out, amount);
}

//--------------------------------------------------------------------------
void executionerMove(Moby* moby)
{
  mobMove(moby);
}

//--------------------------------------------------------------------------
void executionerOnSpawn(Moby* moby, VECTOR position, float yaw, u32 spawnFromUID, char random, struct MobSpawnEventArgs e)
{
  
	struct MobPVar* pvars = (struct MobPVar*)moby->PVar;

  // set scale
  moby->Scale = 0.6;

  // colors by mob type
	moby->GlowRGBA = MobSecondaryColors[pvars->MobVars.SpawnParamsIdx];
	moby->PrimaryColor = MobPrimaryColors[pvars->MobVars.SpawnParamsIdx];

  // targeting
	pvars->TargetVars.targetHeight = 3.5;
  pvars->MobVars.BlipType = 5;
}

//--------------------------------------------------------------------------
void executionerOnDestroy(Moby* moby, int killedByPlayerId, int weaponId)
{
  VECTOR expOffset;
  if (!moby || !moby->PVar)
    return;
    
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;

	// set colors before death so that the corn has the correct color
	moby->PrimaryColor = MobPrimaryColors[pvars->MobVars.SpawnParamsIdx];

  // spawn explosion
  u32 expColor = 0x801E70D6;
  vector_copy(expOffset, moby->Position);
  expOffset[2] += pvars->TargetVars.targetHeight;
  u128 expPos = vector_read(expOffset);
  mobySpawnExplosion
    (expPos, 1, 0, 0, 0, 0, 16, 0, 16, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, expColor, expColor, expColor, expColor, expColor, expColor, expColor, expColor,
    0, 0, 0, 0, 0);
}

//--------------------------------------------------------------------------
void executionerOnDamage(Moby* moby, struct MobDamageEventArgs e)
{
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
	float damage = e.DamageQuarters / 4.0;
  float newHp = pvars->MobVars.Health - damage;

	int canFlinch = pvars->MobVars.Action != MOB_ACTION_FLINCH 
            && pvars->MobVars.Action != MOB_ACTION_BIG_FLINCH
            && pvars->MobVars.Action != MOB_ACTION_TIME_BOMB 
            && pvars->MobVars.FlinchCooldownTicks == 0;

  int isShock = e.DamageFlags & 0x40;

	// destroy
	if (newHp <= 0) {
    executionerForceLocalAction(moby, MOB_ACTION_DIE);
    pvars->MobVars.LastHitBy = e.SourceUID;
    pvars->MobVars.LastHitByOClass = e.SourceOClass;
	}

	// knockback
	if (e.Knockback.Power > 0 && (canFlinch || e.Knockback.Force))
	{
		memcpy(&pvars->MobVars.Knockback, &e.Knockback, sizeof(struct Knockback));
	}

  // flinch
	if (mobAmIOwner(moby))
	{
		float damageRatio = damage / pvars->MobVars.Config.Health;
    if (canFlinch) {
      if (isShock) {
        mobSetAction(moby, MOB_ACTION_FLINCH);
      }
      else if (e.Knockback.Force || randRangeInt(0, 10) < e.Knockback.Power) {
        mobSetAction(moby, MOB_ACTION_BIG_FLINCH);
      }
      else if (randRange(0, 1) < (EXECUTIONER_FLINCH_PROBABILITY * damageRatio)) {
        mobSetAction(moby, MOB_ACTION_FLINCH);
      }
    }
	}
}

//--------------------------------------------------------------------------
void executionerOnStateUpdate(Moby* moby, struct MobStateUpdateEventArgs e)
{
  mobOnStateUpdate(moby, e);
}

//--------------------------------------------------------------------------
Moby* executionerGetNextTarget(Moby* moby)
{
  asm (".set noreorder;");

  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
	Player ** players = playerGetAll();
	int i;
	VECTOR delta;
	Moby * currentTarget = pvars->MobVars.Target;
	Player * closestPlayer = NULL;
	float closestPlayerDist = 100000;

	for (i = 0; i < GAME_MAX_PLAYERS; ++i) {
		Player * p = *players;
		if (p && p->SkinMoby && !playerIsDead(p) && p->Health > 0 && p->SkinMoby->Opacity >= 0x80) {
			vector_subtract(delta, p->PlayerPosition, moby->Position);
			float distSqr = vector_sqrmag(delta);

			if (distSqr < 300000) {
				// favor existing target
				if (p->SkinMoby == currentTarget)
					distSqr *= (1 / EXECUTIONER_TARGET_KEEP_CURRENT_FACTOR);
				
				// pick closest target
				if (distSqr < closestPlayerDist) {
					closestPlayer = p;
					closestPlayerDist = distSqr;
				}
			}
		}

		++players;
	}

	if (closestPlayer) {
    return closestPlayer->SkinMoby;
  }

	return NULL;
}

//--------------------------------------------------------------------------
enum MobAction executionerGetPreferredAction(Moby* moby)
{
	struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
	VECTOR t;

	// no preferred action
	if (executionerIsAttacking(pvars))
		return -1;

	if (executionerIsSpawning(pvars))
		return -1;

	if (pvars->MobVars.Action == MOB_ACTION_JUMP && !pvars->MobVars.MoveVars.Grounded) {
		return MOB_ACTION_WALK;
  }

  // wait for grounded to stop flinch
  if ((pvars->MobVars.Action == MOB_ACTION_FLINCH || pvars->MobVars.Action == MOB_ACTION_BIG_FLINCH) && !pvars->MobVars.MoveVars.Grounded)
    return -1;

  // jump if we've hit a slope and are grounded
  if (pvars->MobVars.MoveVars.Grounded && pvars->MobVars.MoveVars.WallSlope > EXECUTIONER_MAX_WALKABLE_SLOPE) {
    return MOB_ACTION_JUMP;
  }

  // jump if we've hit a jump point on the path
  if (pathShouldJump(moby)) {
    return MOB_ACTION_JUMP;
  }

	// prevent action changing too quickly
	if (pvars->MobVars.ActionCooldownTicks)
		return -1;

	// get next target
	Moby * target = executionerGetNextTarget(moby);
	if (target) {
		vector_copy(t, target->Position);
		vector_subtract(t, t, moby->Position);
		float distSqr = vector_sqrmag(t);
		float attackRadiusSqr = pvars->MobVars.Config.AttackRadius * pvars->MobVars.Config.AttackRadius;

		if (distSqr <= attackRadiusSqr) {
			if (executionerCanAttack(pvars))
				return MOB_ACTION_ATTACK;
			return MOB_ACTION_WALK;
		} else {
			return MOB_ACTION_WALK;
		}
	}
	
	return MOB_ACTION_IDLE;
}

//--------------------------------------------------------------------------
void executionerDoAction(Moby* moby)
{
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
	Moby* target = pvars->MobVars.Target;
	VECTOR t, t2;
  int i;
  float difficulty = 1;
  float turnSpeed = pvars->MobVars.MoveVars.Grounded ? EXECUTIONER_TURN_RADIANS_PER_SEC : EXECUTIONER_TURN_AIR_RADIANS_PER_SEC;
  float acceleration = pvars->MobVars.MoveVars.Grounded ? EXECUTIONER_MOVE_ACCELERATION : EXECUTIONER_MOVE_AIR_ACCELERATION;

  if (MapConfig.State)
    difficulty = MapConfig.State->Difficulty;

	switch (pvars->MobVars.Action)
	{
		case MOB_ACTION_SPAWN:
		{
      mobTransAnim(moby, EXECUTIONER_ANIM_SPAWN, 0);
      mobStand(moby);
			break;
		}
		case MOB_ACTION_FLINCH:
		case MOB_ACTION_BIG_FLINCH:
		{
      int animFlinchId = pvars->MobVars.Action == MOB_ACTION_BIG_FLINCH ? EXECUTIONER_ANIM_BIG_FLINCH : EXECUTIONER_ANIM_FLINCH;

      mobTransAnim(moby, animFlinchId, 0);
      
			if (pvars->MobVars.Knockback.Ticks > 0) {
				float power = PLAYER_KNOCKBACK_BASE_POWER * pvars->MobVars.Knockback.Power;
				vector_fromyaw(t, pvars->MobVars.Knockback.Angle / 1000.0);
				t[2] = 1.0;
				vector_scale(t, t, power * 1 * MATH_DT);
				vector_add(pvars->MobVars.MoveVars.AddVelocity, pvars->MobVars.MoveVars.AddVelocity, t);
			} else if (pvars->MobVars.MoveVars.Grounded) {
        mobStand(moby);
      }
			break;
		}
		case MOB_ACTION_IDLE:
		{
			mobTransAnim(moby, EXECUTIONER_ANIM_IDLE, 0);
      mobStand(moby);
			break;
		}
		case MOB_ACTION_JUMP:
			{
        // move
        if (target) {
          pathGetTargetPos(t, moby);
          mobTurnTowards(moby, t, turnSpeed);
          mobGetVelocityToTarget(moby, pvars->MobVars.MoveVars.Velocity, moby->Position, t, pvars->MobVars.Config.Speed, acceleration);
        } else {
          mobStand(moby);
        }

        // handle jumping
        if (pvars->MobVars.MoveVars.Grounded) {
			    mobTransAnim(moby, EXECUTIONER_ANIM_JUMP, 5);

          // check if we're near last jump pos
          // if so increment StuckJumpCount
          if (pvars->MobVars.MoveVars.IsStuck) {
            if (pvars->MobVars.MoveVars.StuckJumpCount < 255)
              pvars->MobVars.MoveVars.StuckJumpCount++;
          }

          // use delta height between target as base of jump speed
          // with min speed
          float jumpSpeed = pathGetJumpSpeed(moby);
          if (jumpSpeed <= 0 && target) {
            jumpSpeed = 8; //clamp(2 + (target->Position[2] - moby->Position[2]) * fabsf(pvars->MobVars.MoveVars.WallSlope) * 2, 3, 15);
          }

          pvars->MobVars.MoveVars.Velocity[2] = jumpSpeed * MATH_DT;
          pvars->MobVars.MoveVars.Grounded = 0;
        }
				break;
			}
		case MOB_ACTION_LOOK_AT_TARGET:
    {
      mobStand(moby);
      if (target) {
        mobTurnTowards(moby, target->Position, turnSpeed);
      }
      break;
    }
    case MOB_ACTION_WALK:
		{
      int walkBackwards = 0;

			if (target) {

				float dir = ((pvars->MobVars.ActionId + pvars->MobVars.Random) % 3) - 1;

				// determine next position
				vector_copy(t, target->Position);
				vector_subtract(t, t, moby->Position);
				float dist = vector_length(t);

        // walk backwards if too close
        if (dist < EXECUTIONER_TOO_CLOSE_TO_TARGET_RADIUS) {
          walkBackwards = 1;

          mobTurnTowards(moby, target->Position, turnSpeed);
          mobGetVelocityToTarget(moby, pvars->MobVars.MoveVars.Velocity, moby->Position, target->Position, -pvars->MobVars.Config.Speed, acceleration);
        }
        else if (dist > (pvars->MobVars.Config.AttackRadius - pvars->MobVars.Config.HitRadius)) {

          pathGetTargetPos(t, moby);
				  vector_subtract(t, t, moby->Position);
				  float dist = vector_length(t);
          if (dist < 10.0) {
            executionerAlterTarget(t2, moby, t, clamp(dist, 0, 10) * 0.3 * dir);
            vector_add(t, t, t2);
          }
          vector_scale(t, t, 1 / dist);
          vector_add(t, moby->Position, t);

          mobTurnTowards(moby, t, turnSpeed);
          mobGetVelocityToTarget(moby, pvars->MobVars.MoveVars.Velocity, moby->Position, t, pvars->MobVars.Config.Speed, acceleration);
        } else {
          mobStand(moby);
        }
      } else {
        // stand
        mobStand(moby);
			}

			// 
      if (moby->AnimSeqId == EXECUTIONER_ANIM_JUMP && !pvars->MobVars.MoveVars.Grounded) {
        // wait for jump to land
      }
			else if (mobHasVelocity(pvars))
				mobTransAnim(moby, walkBackwards ? EXECUTIONER_ANIM_WALK_BACKWARD : EXECUTIONER_ANIM_RUN, 0);
			else
				mobTransAnim(moby, EXECUTIONER_ANIM_IDLE, 0);
			break;
		}
    case MOB_ACTION_DIE:
    {
      mobTransAnimLerp(moby, EXECUTIONER_ANIM_BIG_FLINCH, 5, 0);

      if (moby->AnimSeqId == EXECUTIONER_ANIM_BIG_FLINCH && moby->AnimSeqT > 3) {
        pvars->MobVars.Destroy = 1;
      }

      mobStand(moby);
      break;
    }
		case MOB_ACTION_ATTACK:
		{
      int attack1AnimId = EXECUTIONER_ANIM_SWING;
			mobTransAnim(moby, attack1AnimId, 0);

			float speedMult = 0; // (moby->AnimSeqId == attack1AnimId && moby->AnimSeqT < 5) ? (difficulty * 2) : 1;
			int swingAttackReady = moby->AnimSeqId == attack1AnimId && moby->AnimSeqT >= 4 && moby->AnimSeqT < 10;
			u32 damageFlags = 0x00081801;

			if (target) {
        mobTurnTowards(moby, target->Position, turnSpeed);
        mobGetVelocityToTarget(moby, pvars->MobVars.MoveVars.Velocity, moby->Position, target->Position, speedMult * pvars->MobVars.Config.Speed, acceleration);
			} else {
				// stand
				mobStand(moby);
			}

			// attribute damage
			switch (pvars->MobVars.Config.MobAttribute)
			{
				case MOB_ATTRIBUTE_FREEZE:
				{
					damageFlags |= 0x00800000;
					break;
				}
				case MOB_ATTRIBUTE_ACID:
				{
					damageFlags |= 0x00000080;
					break;
				}
			}
			
			if (swingAttackReady && damageFlags) {
				executionerDoDamage(moby, pvars->MobVars.Config.HitRadius, pvars->MobVars.Config.Damage, damageFlags, 0);
			}
			break;
		}
	}

  pvars->MobVars.CurrentActionForTicks ++;
}

//--------------------------------------------------------------------------
void executionerDoDamage(Moby* moby, float radius, float amount, int damageFlags, int friendlyFire)
{
  mobDoDamage(moby, radius, amount, damageFlags, friendlyFire, 6);
}

//--------------------------------------------------------------------------
void executionerForceLocalAction(Moby* moby, enum MobAction action)
{
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
  float difficulty = 1;

  if (MapConfig.State)
    difficulty = MapConfig.State->Difficulty;

	// from
	switch (pvars->MobVars.Action)
	{
		case MOB_ACTION_SPAWN:
		{
			// enable collision
			moby->CollActive = 0;
			break;
		}
		case MOB_ACTION_DIE:
		{
      // can't undie
      return;
		}
	}

	// to
	switch (action)
	{
		case MOB_ACTION_SPAWN:
		{
			// disable collision
			moby->CollActive = 1;
			break;
		}
		case MOB_ACTION_WALK:
		{
			
			break;
		}
		case MOB_ACTION_DIE:
		{
			
			break;
		}
		case MOB_ACTION_ATTACK:
		{
			pvars->MobVars.AttackCooldownTicks = pvars->MobVars.Config.AttackCooldownTickCount;
			break;
		}
		case MOB_ACTION_TIME_BOMB:
		{
			pvars->MobVars.OpacityFlickerDirection = 4;
			pvars->MobVars.TimeBombTicks = EXECUTIONER_TIMEBOMB_TICKS / clamp(difficulty, 0.5, 2);
			break;
		}
		case MOB_ACTION_FLINCH:
		case MOB_ACTION_BIG_FLINCH:
		{
			executionerPlayHitSound(moby);
			pvars->MobVars.FlinchCooldownTicks = EXECUTIONER_FLINCH_COOLDOWN_TICKS;
			break;
		}
		default:
		{
			break;
		}
	}

	// 
  if (action != pvars->MobVars.Action)
    pvars->MobVars.CurrentActionForTicks = 0;

	pvars->MobVars.Action = action;
	pvars->MobVars.NextAction = -1;
	pvars->MobVars.ActionCooldownTicks = EXECUTIONER_ACTION_COOLDOWN_TICKS;
}

//--------------------------------------------------------------------------
short executionerGetArmor(Moby* moby)
{
  struct MobPVar* pvars = (struct MobPVar*)moby->PVar;
	float t = pvars->MobVars.Health / pvars->MobVars.Config.MaxHealth;
  int bangles = pvars->MobVars.Config.Bangles;

  if (t < 0.3)
    return 0x0000;
  else if (t < 0.7)
    return bangles & 0x1f; // remove torso bangle

	return bangles;
}

//--------------------------------------------------------------------------
void executionerPlayHitSound(Moby* moby)
{
	ExecutionerSoundDef.Index = 0x17D;
	soundPlay(&ExecutionerSoundDef, 0, moby, 0, 0x400);
}	

//--------------------------------------------------------------------------
void executionerPlayAmbientSound(Moby* moby)
{
  const int ambientSoundIds[] = { 0x17A, 0x179 };
	ExecutionerSoundDef.Index = ambientSoundIds[rand(2)];
	soundPlay(&ExecutionerSoundDef, 0, moby, 0, 0x400);
}

//--------------------------------------------------------------------------
void executionerPlayDeathSound(Moby* moby)
{
	ExecutionerSoundDef.Index = 0x171;
	soundPlay(&ExecutionerSoundDef, 0, moby, 0, 0x400);
}

//--------------------------------------------------------------------------
int executionerIsAttacking(struct MobPVar* pvars)
{
	return pvars->MobVars.Action == MOB_ACTION_TIME_BOMB || (pvars->MobVars.Action == MOB_ACTION_ATTACK && !pvars->MobVars.AnimationLooped);
}

//--------------------------------------------------------------------------
int executionerIsSpawning(struct MobPVar* pvars)
{
	return pvars->MobVars.Action == MOB_ACTION_SPAWN && !pvars->MobVars.AnimationLooped;
}

//--------------------------------------------------------------------------
int executionerCanAttack(struct MobPVar* pvars)
{
	return pvars->MobVars.AttackCooldownTicks == 0;
}
