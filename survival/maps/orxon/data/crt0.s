# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: crt0.s 1066 2005-04-29 10:25:23Z pixel $
# Standard startup file.


.set noat
.set noreorder

.global _start

	.ent _start
_start:


#.word MOB_PATHFINDING_NODES_COUNT
#.word MOB_PATHFINDING_EDGES_COUNT
#.word MOB_PATHFINDING_NODES
#.word MOB_PATHFINDING_NODES_CORNERING
#.word MOB_PATHFINDING_EDGES
#.word MOB_PATHFINDING_EDGES_PATHFIT
#.word MOB_PATHFINDING_EDGES_JUMPPADSPEED
#.word MOB_PATHFINDING_EDGES_JUMPPADAT
#.word MOB_PATHFINDING_PATHS_MAX_PATH_LENGTH
#.word MOB_PATHFINDING_PATHS
