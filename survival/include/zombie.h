#ifndef SURVIVAL_MOB_ZOMBIE_H
#define SURVIVAL_MOB_ZOMBIE_H

#define ZOMBIE_BASE_REACTION_TICKS						(0.25 * TPS)
#define ZOMBIE_BASE_ATTACK_COOLDOWN_TICKS			(2 * TPS)
#define ZOMBIE_BASE_EXPLODE_RADIUS						(5)
#define ZOMBIE_MELEE_HIT_RADIUS								(1.75)
#define ZOMBIE_EXPLODE_HIT_RADIUS							(5)
#define ZOMBIE_MELEE_ATTACK_RADIUS						(5)

#define ZOMBIE_TARGET_KEEP_CURRENT_FACTOR     (10.0)

#define ZOMBIE_MAX_WALKABLE_SLOPE             (40 * MATH_DEG2RAD)
#define ZOMBIE_BASE_STEP_HEIGHT								(2)
#define ZOMBIE_TURN_RADIANS_PER_SEC           (45 * MATH_DEG2RAD)
#define ZOMBIE_TURN_AIR_RADIANS_PER_SEC       (15 * MATH_DEG2RAD)
#define ZOMBIE_MOVE_ACCELERATION              (25)
#define ZOMBIE_MOVE_AIR_ACCELERATION          (5)

#define ZOMBIE_ANIM_ATTACK_TICKS							(30)
#define ZOMBIE_TIMEBOMB_TICKS									(60 * 2)
#define ZOMBIE_FLINCH_COOLDOWN_TICKS					(60 * 7)
#define ZOMBIE_ACTION_COOLDOWN_TICKS					(30)
#define ZOMBIE_RESPAWN_AFTER_TICKS						(60 * 30)
#define ZOMBIE_BASE_COLL_RADIUS								(0.5)
#define ZOMBIE_MAX_COLL_RADIUS								(4)
#define ZOMBIE_AMBSND_MIN_COOLDOWN_TICKS    	(60 * 2)
#define ZOMBIE_AMBSND_MAX_COOLDOWN_TICKS    	(60 * 3)
#define ZOMBIE_FLINCH_PROBABILITY             (1.0)

enum ZombieAnimId
{
	ZOMBIE_ANIM_UNDERGROUND,
	ZOMBIE_ANIM_IDLE,
	ZOMBIE_ANIM_2,
	ZOMBIE_ANIM_WALK,
	ZOMBIE_ANIM_AGGRO_WALK,
	ZOMBIE_ANIM_RUN,
	ZOMBIE_ANIM_JUMP,
	ZOMBIE_ANIM_CROUCH,
	ZOMBIE_ANIM_ELECTROCUTED,
	ZOMBIE_ANIM_CRAWL_OUT_OF_GROUND,
	ZOMBIE_ANIM_FLINCH,
	ZOMBIE_ANIM_THROW_HEAD,
	ZOMBIE_ANIM_TAUNT,
	ZOMBIE_ANIM_CAT_PASS,
	ZOMBIE_ANIM_BIG_FLINCH,
	ZOMBIE_ANIM_SLAP,
	ZOMBIE_ANIM_IDLE_2
};

enum ZombieBangles {
	ZOMBIE_BANGLE_HEAD_1 =  	(1 << 0),
	ZOMBIE_BANGLE_HEAD_2 =  	(1 << 1),
	ZOMBIE_BANGLE_HEAD_3 =  	(1 << 2),
	ZOMBIE_BANGLE_HEAD_4 =  	(1 << 3),
	ZOMBIE_BANGLE_HEAD_5 =  	(1 << 4),
	ZOMBIE_BANGLE_TORSO_1 = 	(1 << 5),
	ZOMBIE_BANGLE_TORSO_2 = 	(1 << 6),
	ZOMBIE_BANGLE_TORSO_3 = 	(1 << 7),
	ZOMBIE_BANGLE_TORSO_4 = 	(1 << 8),
	ZOMBIE_BANGLE_HIPS = 			(1 << 9),
	ZOMBIE_BANGLE_LLEG = 			(1 << 10),
	ZOMBIE_BANGLE_RFOOT = 		(1 << 11),
	ZOMBIE_BANGLE_RLEG = 			(1 << 12),
	ZOMBIE_BANGLE_RARM = 			(1 << 13),
	ZOMBIE_BANGLE_LARM = 			(1 << 14),
	ZOMBIE_BANGLE_HIDE_BODY =	(1 << 15)
};

#endif // SURVIVAL_MOB_ZOMBIE_H
