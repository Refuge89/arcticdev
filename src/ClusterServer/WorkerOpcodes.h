/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2014 Arctic Server Team
 * See COPYING for license details.
 */

#pragma once

enum WorkerServerOpcodes
{
	ISMSG_AUTH_REQUEST					        = 1,
	ICMSG_AUTH_REPLY					        = 2,
	ISMSG_AUTH_RESULT					        = 3,
	ICMSG_REGISTER_WORKER				        = 4,
	ISMSG_REGISTER_RESULT				        = 5,
	ISMSG_CREATE_INSTANCE				        = 6,
	ISMSG_DESTROY_INSTANCE				        = 7,
	ISMSG_PLAYER_LOGIN					        = 8,
	ISMSG_PLAYER_INFO					        = 9,
	ICMSG_PLAYER_INFO					        = 10,
	ISMSG_WOW_PACKET					        = 11,
	ICMSG_WOW_PACKET					        = 12,
	ICMSG_WHISPER						        = 13,
    ICMSG_CHAT							        = 14,
	ISMSG_WHISPER						        = 15,
	ISMSG_CHAT							        = 16,
	ISMSG_PACKED_PLAYER_INFO			        = 17,
	ICMSG_PLAYER_LOGIN_RESULT			        = 18,
	ICMSG_PLAYER_LOGOUT					        = 19,
	ISMSG_DESTROY_PLAYER_INFO			        = 20,
	ICMSG_TELEPORT_REQUEST				        = 21,
	ISMSG_TELEPORT_RESULT				        = 22,
	ISMSG_PLAYER_CHANGED_SERVERS		        = 23,
	ISMSG_PLAYER_CHANGE_INSTANCES		        = 24,
	ISMSG_CREATE_PLAYER					        = 25,
	ICMSG_PLAYER_CHANGE_SERVER_INFO		        = 26,
	ICMSG_ERROR_HANDLER					        = 27,
	ISMSG_SESSION_REMOVED				        = 28,
	ICMSG_SWITCH_SERVER					        = 29,
	ICMSG_SAVE_ALL_PLAYERS				        = 30,
	ISMSG_SAVE_ALL_PLAYERS				        = 31,
	ICMSG_TRANSPORTER_MAP_CHANGE		        = 32,
	ISMSG_TRANSPORTER_MAP_CHANGE		        = 33,
	ICMSG_PLAYER_TELEPORT				        = 34,
	ISMSG_PLAYER_TELEPORT				        = 35,
	ICMSG_CREATE_PLAYER					        = 36,
	ISMSG_CHANNEL_ACTION				        = 37,
	ICMSG_CHANNEL_ACTION				        = 38,
	ICMSG_CHANNEL_UPDATE				        = 39,
	ISMSG_CHANNEL_LFG_DUNGEON_STATUS_REQUEST    = 40,
	ICMSG_CHANNEL_LFG_DUNGEON_STATUS_REPLY      = 41,

	IMSG_NUM_TYPES,
};

enum msg_channel_action
{
	CHANNEL_JOIN,
	CHANNEL_PART,
	CHANNEL_SAY
};

enum cmsg_channel_update
{
	UPDATE_CHANNELS_ON_ZONE_CHANGE         = 1,
	PART_ALL_CHANNELS                      = 2,
	JOIN_ALL_CHANNELS                      = 3
};
