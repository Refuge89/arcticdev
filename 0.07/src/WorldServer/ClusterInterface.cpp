/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2009 Arctic Server Team
 * See COPYING for license details.
 */

#include "StdAfx.h"
#include "svn_revision.h"

#ifdef CLUSTERING

initialiseSingleton(ClusterInterface);
ClusterInterfaceHandler ClusterInterface::PHandlers[IMSG_NUM_TYPES];

//����������� � ���������� ��� ������������ ����� � ��������
void *ClusterInterface::operator new(size_t size)
{
	sLog.outDebug("New ClusterInterface dynamic string in object.");
	return malloc(size);
}

void ClusterInterface::operator delete(void* p)
{
	sLog.outDebug("Delete ClusterInterface dynamic string in object.");
	free(p);
}

void ClusterInterface::InitHandlers()
{
	memset(PHandlers, 0, sizeof(void*) * IMSG_NUM_TYPES);
	PHandlers[ISMSG_AUTH_REQUEST] = &ClusterInterface::HandleAuthRequest;
	PHandlers[ISMSG_AUTH_RESULT]  = &ClusterInterface::HandleAuthResult;
	PHandlers[ISMSG_REGISTER_RESULT] = &ClusterInterface::HandleRegisterResult;
	PHandlers[ISMSG_CREATE_INSTANCE] = &ClusterInterface::HandleCreateInstance;
	PHandlers[ISMSG_PLAYER_LOGIN] = &ClusterInterface::HandlePlayerLogin;
	PHandlers[ISMSG_WOW_PACKET] = &ClusterInterface::HandleWoWPacket;
}

ClusterInterface::ClusterInterface()
{
	ClusterInterface::InitHandlers();
	m_connected = false;
}

ClusterInterface::~ClusterInterface()
{

}

string ClusterInterface::GenerateVersionString()
{
	char str[200];
	snprintf(str, 200, "WoWArcTic r%u/%s-%s-%s", g_getRevision(), CONFIG, PLATFORM_TEXT, ARCH);
	return string(str);
}

void ClusterInterface::ForwardWoWPacket(uint16 opcode, uint32 size, const void * data, uint32 sessionid)
{
	OUT_DEBUG(FORMATSWOWPAKESTSAI, SERVERSTOWOWFORMAAI, LookupName(opcode, g_worldOpcodeNames));
	bool rv;
	uint32 size2 = 10 + size;
	uint16 opcode2 = ICMSG_WOW_PACKET;

	if(!_clientSocket) return;			// Shouldn't happen
	
	_clientSocket->BurstBegin();
	_clientSocket->BurstSend((const uint8*)&opcode2, 2);
	_clientSocket->BurstSend((const uint8*)&size2, 4);
	_clientSocket->BurstSend((const uint8*)&sessionid, 4);
	_clientSocket->BurstSend((const uint8*)&opcode, 2);
    rv=_clientSocket->BurstSend((const uint8*)&size, 4);
	if(size&&rv)
		rv=_clientSocket->BurstSend((const uint8*)data, size);

	if(rv) _clientSocket->BurstPush();
	_clientSocket->BurstEnd();
}

void ClusterInterface::ConnectToRealmServer()
{
	_lastConnectTime = UNIXTIME;
	string hostname;
	int port;
	string strkey;
	if(!Config.MainConfig.GetString("Cluster", "RSHostName", &hostname) || !Config.MainConfig.GetInt("Cluster", "RSPort", &port) || !Config.MainConfig.GetString("Cluster", "Key", &strkey))
	{
		Log.Error(CLUSTERINTERFACESAI, COUNTESNOTGETCONSAI);
		return;
	}

	/* hash the key */
	Sha1Hash k;
	k.UpdateData(strkey);
	k.Finalize();
	memcpy(key, k.GetDigest(), 20);

	Log.Notice(CLUSTERINTERFACESAI, ZCONNECTSPORTSWOWAI, hostname.c_str(), port);
    WSClient * s = ConnectTCPSocket<WSClient>(hostname.c_str(), port);
	if(!s)
	{
		Log.Error(CLUSTERINTERFACESAI, ZSDNNECTSPORTSWOWAI, hostname.c_str(), port);
		return;
	}

	Log.Success(CLUSTERINTERFACESAI, WOWCONNECTSPATCETAI, hostname.c_str(), port);

	_clientSocket = s;
	m_latency = getMSTime();
	m_connected = true;
}

void ClusterInterface::HandleAuthRequest(WorldPacket & pck)
{
	uint32 x;
	pck >> x;
	OUT_DEBUG(CLUSTERINTERFACESAI, INOCONMINTGSAUGRSAI, _clientSocket->GetRemoteIP().c_str(), x);

	WorldPacket data(ICMSG_AUTH_REPLY, 50);
	data.append(key, 20);
	data << uint32(g_getRevision());
	data << GenerateVersionString();
	SendPacket(&data);

	m_latency = getMSTime() - m_latency;
	Log.Notice(CLUSTERINTERFACESAI, LATESTNCEBEWEENSWAI, m_latency);
}

void ClusterInterface::HandleAuthResult(WorldPacket & pck)
{
	uint32 res;
	pck >> res;
	OUT_DEBUG(CLUSTERINTERFACESAI, WOAUTYSWOWRESIULNAI, res);
	if(!res)
	{
		Log.Error(CLUSTERINTERFACESAI, AUTERHEARTICATIONAI);
		_clientSocket->Disconnect();
		_clientSocket = 0;
		return;
	}

	/* hardcoded to perfer 0, 530 */
	WorldPacket data(ICMSG_REGISTER_WORKER, 4 + 12);
	data << uint32(69);
	data << uint32(1);
	data << uint32(0);
	SendPacket(&data);
}


void ClusterInterface::HandleRegisterResult(WorldPacket & pck)
{
	uint32 res;
	pck >> res;
	OUT_DEBUG(CLUSTERINTERFACESAI, REGISHEARTICATIONAI, res);
}

void ClusterInterface::HandleCreateInstance(WorldPacket & pck)
{
	uint32 mapid, instanceid;
	pck >> mapid >> instanceid;
	OUT_DEBUG(CLUSTERINTERFACESAI, CREAYIONSTINSTALCAI, instanceid, mapid);
	Map * pMap = sWorldCreator.GetMap(mapid);
	pMap->CreateMapMgrInstance(instanceid);
}

void ClusterInterface::HandleDestroyInstance(WorldPacket & pck)
{

}

void ClusterInterface::HandlePlayerLogin(WorldPacket & pck)
{
	/* player x logging into instance y */
	uint32 guid, instance, mapid;
	uint32 accountid, accountflags, sessionid;
	string gmpermissions, accountname;
	pck >> guid >> mapid >> instance >> accountid >> accountflags >> sessionid >> gmpermissions >> accountname;

	/* find the instance */
	Map * ma = sWorldCreator.GetMap(mapid);
	ASSERT(ma);
	MapMgrPointer mm = ma->GetInstance(instance);
	ASSERT(mm);

	/* create the session */
	WorldSession * s = sWorld.FindSession(accountid);
	ASSERT(!s);

	/* create the socket */
	WorldSocket * so = new WorldSocket(sessionid);
	s = new WorldSession(accountid, accountname, so);
	_sessions[sessionid] = s;
	sWorld.AddSession(s);

	bool login_result = s->PlayerLogin(guid, mapid, instance);
	if(login_result)
	{
		/* login was ok. send a message to the realm server telling him to distribute our info to all other realm server */
		WorldPacket data(ICMSG_PLAYER_LOGIN_RESULT, 5);
		data << guid << sessionid <<  uint8(1);
		SendPacket(&data);
	}
	else
	{
		/* for some reason the login failed */
		WorldPacket data(ICMSG_PLAYER_LOGIN_RESULT, 5);
		data << guid << sessionid << uint8(0);
		SendPacket(&data);

		/* tell the client his login failed before deleting the session */
		data.Initialize(SMSG_CHARACTER_LOGIN_FAILED);
		data << uint8(62);
		so->SendPacket(&data);

		/* destroy the session */
		DestroySession(sessionid);
	}
}

void ClusterInterface::HandlePackedPlayerInfo(WorldPacket & pck)
{
	RPlayerInfo * pi;
	// todo: uncompress
	uint32 count;
	pck >> count;
	for(uint32 i = 0; i < count; ++i)
	{
		pi = new RPlayerInfo;
		pi->Unpack(pck);
		_onlinePlayers[pi->Guid] = pi;
	}
}

void ClusterInterface::Update()
{
	if(!m_connected && UNIXTIME >= (_lastConnectTime + 3))
		ConnectToRealmServer();

	WorldPacket * pck;
	uint16 opcode;
	while((pck = _pckQueue.Pop()))
	{
		opcode = pck->GetOpcode();
		if(opcode < IMSG_NUM_TYPES && ClusterInterface::PHandlers[opcode] != 0)
			(this->*ClusterInterface::PHandlers[opcode])(*pck);
		else
			Log.Error(CLUSTERINTERFACESAI, WOWPAUNHADLESRWOWAI, opcode);
	}
}

void ClusterInterface::DestroySession(uint32 sid)
{
	WorldSession * s = _sessions[sid];
	_sessions[sid] = 0;
	if(s)
	{
		/* todo: replace this with an event so we don't remove from the wrong thread */
		if(s->GetPlayer())
			s->LogoutPlayer(true);

		delete s->GetSocket();
		delete s;
	}
}


void ClusterInterface::HandleWoWPacket(WorldPacket & pck)
{
	uint32 size, sid;
	uint16 opcode;
	pck >> sid >> opcode >> size;

	if(!_sessions[sid])
	{
		Log.Error(HANDERWOWARCTICPACI, SQERYIONSTINSTALCAI, sid);
		return;
	}

	OUT_DEBUG(HANDERWOWARCTICPACI, SERVERSTOWOWFORMAAI, LookupName(opcode, g_worldOpcodeNames));

	WorldPacket * npck = new WorldPacket(opcode, size);
	npck->resize(size);
	memcpy((void*)npck->contents(), pck.contents()+10, size);
	_sessions[sid]->QueuePacket(npck);
}

void ClusterInterface::HandlePlayerChangedServers(WorldPacket & pck)
{
	uint32 sessionid, dsid;
	pck >> sessionid >> dsid;
	if(!_sessions[dsid])
	{
		Log.Error(COUNTESNOTGETCDASAI, SESSIONWOWINVALIDAI, sessionid);
		return;
	}

	WorldSession * s = _sessions[sessionid];
	PlayerPointer plr = s->GetPlayer();



	/* build the packet with the players information */
	WorldPacket data(ICMSG_PLAYER_CHANGE_SERVER_INFO, 1000);
	data << sessionid << plr->GetGUIDLow();

	/* pack */
	//data << plr->
	
	/* remove the player from our world. */
	sEventMgr.AddEvent(plr, &Player::EventRemoveAndDelete, EVENT_GAMEOBJECT_EXPIRE	/* meh :P */, 1000, 1);

	/* dereference the session */
}

void ClusterInterface::RequestTransfer(PlayerPointer plr, uint32 MapId, uint32 InstanceId, LocationVector & vec)
{
	WorldPacket data(ICMSG_TELEPORT_REQUEST, 32);
	data << plr->GetSession()->GetSocket()->GetSessionId() << MapId << InstanceId << vec << vec.o;
	SendPacket(&data);
}

#endif
