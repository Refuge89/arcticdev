/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2009 Arctic Server Team
 * See COPYING for license details.
 */
 
#ifndef __ARCTIC_OPCODES_H
#define __ARCTIC_OPCODES_H

enum VoiceChatOpcodes
{
	VOICECHAT_CMSG_CREATE_CHANNEL			= 1,
	VOICECHAT_SMSG_CHANNEL_CREATED		= 2,
	VOICECHAT_CMSG_ADD_MEMBER				= 3,
	VOICECHAT_CMSG_REMOVE_MEMBER			= 4,
	VOICECHAT_CMSG_DELETE_CHANNEL			= 5,
	VOICECHAT_CMSG_PING						= 6,
	VOICECHAT_SMSG_PONG						= 7,
	VOICECHAT_NUM_OPCODES					= 8,
};

#endif

