

#ifndef SURVIVAL_MAP_UTILS_H
#define SURVIVAL_MAP_UTILS_H

#include <tamtypes.h>
#include <libdl/moby.h>
#include <libdl/math.h>
#include <libdl/time.h>
#include <libdl/player.h>
#include <libdl/sound.h>
#include "game.h"


extern struct SurvivalMapConfig MapConfig;

Moby * spawnExplosion(VECTOR position, float size, u32 color);
void playPaidSound(Player* player);
int tryPlayerInteract(Moby* moby, Player* player, char* message, int boltCost, int tokenCost, int actionCooldown, float sqrDistance);
GuberEvent* guberCreateEvent(Moby* moby, u32 eventType);

float getSignedSlope(VECTOR forward, VECTOR normal);

u8 decTimerU8(u8* timeValue);
u16 decTimerU16(u16* timeValue);
u32 decTimerU32(u32* timeValue);

void pushSnack(int localPlayerIdx, char* string, int ticksAlive);

int isInDrawDist(Moby* moby);

int mobyIsMob(Moby* moby);

#endif // SURVIVAL_MAP_UTILS_H
