/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2010 Arctic Server Team
 * See COPYING for license details.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

// ArcTic Configuration Header File
// Feel free to customize to your needs.

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Enable/disable ArcTic world server debugging. 
// Enabling this will cause massive spam in the world server window, but will list
// packets recieved and certian player information.
// Default: Disabled

// #define _DEBUG 1

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Enable/disable Arctic world server optimized player saving. 
// It is not yet complete, things have to be updated, and player account data needs to be fixed.
// Player guilds need to be updated as well, not saving correctly :|
// Default: Disabled

// #define OPTIMIZED_PLAYER_SAVING 1

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Enable/disable ArcTic world server cluster support.
// Warning: ArcTic will no longer function independantly when built in cluster mode. 
// It will require a realmserver to function.
// As of last update, it is nowhere near complete, only enable for dev use.
// Default: Disabled

#ifdef ENABLE_CLUSTERING
#  error Sorry but clustering is not finished yet. dont bother to try to compile it. it wont work.
#  define CLUSTERING 1
#endif

// #define CLUSTERING 1

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Use memory mapping for map files for faster access (let OS take care of caching)
// (currently only available under windows)
// Only recommended under X64 builds, X86 builds will most likely run out of address space.
// Default: Disabled

// #define USE_MEMORY_MAPPING_FOR_MAPS

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Enable/disable ArcTic's Voice Chat support.
// While not completely finished (but close!) you can link your ArcTic server to a voice chat
// server (also compiled by default) and use the client's ingame voice chat function.
// At the moment it is only good for joining, you can't leave a voice channel yet :P
// Also, it only works for parties. Custom chat channels are not fully supported yet.
// However, if you want to play around with it, feel free to enable this define.
// Default: Disabled

// #define VOICE_CHAT 1

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Enable/disable movement compression.
// This allows the server to compress long-range creatures movement into a buffer and then flush
// it periodically, compressed with deflate. This can make a large difference to server bandwidth.
// Currently this sort of compression is only used for player and creature movement, although
// it may be expanded in the future.
// Default: Disabled

#define ENABLE_COMPRESSED_MOVEMENT 1
#define ENABLE_COMPRESSED_MOVEMENT_FOR_PLAYERS 1
#define ENABLE_COMPRESSED_MOVEMENT_FOR_CREATURES 1

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Default: Disabled

#ifndef ENABLE_IMPROVED_TICKETSYSTEM
#define GM_TICKET_MY_MASTER_COMPATIBLE
#endif
#undef GM_TICKET_MY_MASTER_COMPATIBLE

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Allow loading of unused testmaps.
// Default: Disabled
 
// #define EXCLUDE_TEST_MAPS 1

#endif // __CONFIG_H

