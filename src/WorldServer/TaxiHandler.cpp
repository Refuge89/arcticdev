/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2010 Arctic Server Team
 * See COPYING for license details.
 */

#include "StdAfx.h"

void WorldSession::HandleTaxiNodeStatusQueryOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	DEBUG_LOG( "WORLD"," Received CMSG_TAXINODE_STATUS_QUERY" );

	uint64 guid;
	uint32 curloc;
	uint8 field;
	uint32 submask;

	recv_data >> guid;

	curloc = sTaxiMgr.GetNearestTaxiNode( GetPlayer( )->GetPositionX( ), GetPlayer( )->GetPositionY( ), GetPlayer( )->GetPositionZ( ), GetPlayer( )->GetMapId( ) );

	field = (uint8)((curloc - 1) / 32);
	submask = 1<<((curloc-1)%32);

	WorldPacket data(9);
	data.Initialize( SMSG_TAXINODE_STATUS );
	data << guid;

	// Check for known nodes
	if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
	{   
		data << uint8( 0 );	
	}
	else
	{
		data << uint8( 1 );
	}	

	SendPacket( &data );
	DEBUG_LOG( "WORLD"," Sent SMSG_TAXINODE_STATUS" );
}


void WorldSession::HandleTaxiQueryAvaibleNodesOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	DEBUG_LOG( "WORLD"," Received CMSG_TAXIQUERYAVAILABLENODES" );
	uint64 guid;
	recv_data >> guid;
	Creature* pCreature = _player->GetMapMgr()->GetCreature(GET_LOWGUID_PART(guid));
	if(!pCreature) return;

	SendTaxiList(pCreature);
}

void WorldSession::SendTaxiList(Creature* pCreature)
{
	uint32 curloc;
	uint8 field;
	uint32 TaxiMask[12];
	uint32 submask;
	uint64 guid = pCreature->GetGUID();

	curloc = pCreature->m_TaxiNode;
	if (!curloc)
		return;

	field = (uint8)((curloc - 1) / 32);
	submask = 1<<((curloc-1)%32);

	// Check for known nodes
	if (!(GetPlayer( )->GetTaximask(field) & submask))
	{
		GetPlayer()->SetTaximask(field, (submask | GetPlayer( )->GetTaximask(field)) );

		OutPacket(SMSG_NEW_TAXI_PATH);

		//Send packet
		WorldPacket update(SMSG_TAXINODE_STATUS, 9);
		update << guid << uint8( 1 );
		SendPacket( &update );
	}

	//Set Mask
	memset(TaxiMask, 0, sizeof(uint32)*12);
	sTaxiMgr.GetGlobalTaxiNodeMask(curloc, TaxiMask);
	TaxiMask[field] |= 1 << ((curloc-1)%32);

	//Remove nodes unknown to player
	for(int i = 0; i < 12; ++i)
	{
		TaxiMask[i] &= GetPlayer( )->GetTaximask(i);
	}

	WorldPacket data(48);
	data.Initialize( SMSG_SHOWTAXINODES );
	data << uint32( 1 ) << guid;
	data << uint32( curloc );
	for(int i = 0; i < 12; ++i)
	{
		data << TaxiMask[i];
	}
	SendPacket( &data );

	DEBUG_LOG( "WORLD"," Sent SMSG_SHOWTAXINODES" );
}

void WorldSession::HandleActivateTaxiOpcode( WorldPacket & recv_data )
{
	if(!_player->IsInWorld()) return;
	DEBUG_LOG( "WORLD"," Received CMSG_ACTIVATETAXI" );

	uint64 guid;
	uint32 sourcenode, destinationnode;
	int32 newmoney;
	uint32 curloc;
	uint8 field;
	uint32 submask;
	WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);

	recv_data >> guid >> sourcenode >> destinationnode;

	if(GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
		return;

	TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(sourcenode, destinationnode);
	TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(sourcenode);

	if( !taxinode || !taxipath )
		return;

	curloc = taxinode->id;
	field = (uint8)((curloc - 1) / 32);
	submask = 1<<((curloc-1)%32);

	// Check for known nodes
	if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
	{   
		data << uint32( 1 );
		SendPacket( &data );
		return;
	}

	// Check for valid node
	if (!taxinode)
	{
		data << uint32( 1 );
		SendPacket( &data );
		return;
	}

	if (!taxipath || !taxipath->GetNodeCount())
	{
		data << uint32( 2 );
		SendPacket( &data );
		return;
	}

	if (taxipath->GetID() == 766 || taxipath->GetID() == 767 || taxipath->GetID() == 771 || taxipath->GetID() == 772)
	{
		data << uint32( 2 );
		SendPacket( &data );
		return;
	}

	// Check for gold
	newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - taxipath->GetPrice());
	if(newmoney < 0 )
	{
		data << uint32( 3 );
		SendPacket( &data );
		return;
	}

	// MOUNTDISPLAYID
	// bat: 1566
	// gryph: 1147
	// wyvern: 295
	// hippogryph: 479

	uint32 modelid = 0;
	if( _player->GetTeam() )
	{
		if( taxinode->horde_mount == 2224 )
			modelid = 295; // In case it's a wyvern
		else
			modelid = 1566; // In case it's a bat or a bad id
	}
	else
	{
		if( taxinode->alliance_mount == 3837 )
			modelid = 479; // In case it's an hippogryph
		else
			modelid = 1147; // In case it's a gryphon or a bad id
	}

	//GetPlayer( )->setDismountCost( newmoney );

	data << uint32( 0 );
	// 0 Ok
	// 1 Unspecified Server Taxi Error
	// 2.There is no direct path to that direction
	// 3 Not enough Money
	SendPacket( &data );
	DEBUG_LOG( "WORLD"," Sent SMSG_ACTIVATETAXIREPLY" );

	// 0x001000 seems to make a mount visible
	// 0x002000 seems to make you sit on the mount, and the mount move with you
	// 0x000004 locks you so you can't move, no msg_move updates are sent to the server
	// 0x000008 seems to enable detailed collision checking

	// check for a summon -> if we do, remove.
    // Check if the player is casting, obviously they should not be able to cast on a taxi 
 	if( _player->GetCurrentSpell() != NULL ) 
 	_player->GetCurrentSpell()->cancel(); 
	if(_player->GetSummon() != NULL)
	{
		if(_player->GetSummon()->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
			_player->GetSummon()->Dismiss(false);						   // warlock summon -> dismiss
		else
			_player->GetSummon()->Remove(false, true, true);					  // hunter pet -> just remove for later re-call
	}

	_player->taxi_model_id = modelid;
	GetPlayer()->TaxiStart(taxipath, modelid, 0);
	GetPlayer()->GetAchievementInterface()->HandleAchievementCriteriaFlightPathsTaken();
	
	//sLog.outString("TAXI: Starting taxi trip. Next update in %d msec.", first_node_time);
}

void WorldSession::HandleMultipleActivateTaxiOpcode(WorldPacket & recvPacket)
{
	if(!_player->IsInWorld()) return;
	DEBUG_LOG( "WORLD"," Received CMSG_ACTIVATETAXI" );

	uint64 guid;
	uint32 nodecount;
	vector<uint32> pathes;
	int32 newmoney;
	uint32 curloc;
	uint8 field;
	uint32 submask;
	WorldPacket data(SMSG_ACTIVATETAXIREPLY, 4);

	recvPacket >> guid >> nodecount;
	if(nodecount < 2)
		return;

	if(nodecount>12)
	{
		printf("CMSG_ACTIVATETAXI: Client disconnected, nodecount: %u", nodecount);
		Disconnect();
		return;
	}

	for(uint32 i = 0; i < nodecount; ++i)
		pathes.push_back( recvPacket.read<uint32>() );

	if(GetPlayer()->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_LOCK_PLAYER))
		return;

	// get first trip
	TaxiPath* taxipath = sTaxiMgr.GetTaxiPath(pathes[0], pathes[1]);
	TaxiNode* taxinode = sTaxiMgr.GetTaxiNode(pathes[0]);

	curloc = taxinode->id;
	field = (uint8)((curloc - 1) / 32);
	submask = 1<<((curloc-1)%32);

	// Check for known nodes
	if ( (GetPlayer( )->GetTaximask(field) & submask) != submask )
	{   
		data << uint32( 1 );
		SendPacket( &data );
		return;
	}

	// Check for valid node
	if (!taxinode)
	{
		data << uint32( 1 );
		SendPacket( &data );
		return;
	}

	if (!taxipath || !taxipath->GetNodeCount())
	{
		data << uint32( 2 );
		SendPacket( &data );
		return;
	}

	if (taxipath->GetID() == 766 || taxipath->GetID() == 767 || taxipath->GetID() == 771 || taxipath->GetID() == 772)
	{
		data << uint32( 2 );
		SendPacket( &data );
		return;
	}

	uint32 totalcost = taxipath->GetPrice();
	for(uint32 i = 2; i < nodecount; ++i)
	{
		TaxiPath * np = sTaxiMgr.GetTaxiPath(pathes[i-1], pathes[i]);
		if(!np) return;
		totalcost += np->GetPrice();
	}

	// Check for gold
	newmoney = ((GetPlayer()->GetUInt32Value(PLAYER_FIELD_COINAGE)) - totalcost);
	if(newmoney < 0 )
	{
		data << uint32( 3 );
		SendPacket( &data );
		return;
	}

	// MOUNTDISPLAYID
	// bat: 1566
	// gryph: 1147
	// wyvern: 295
	// hippogryph: 479

	uint32 modelid = 0;
	if( _player->GetTeam() )
	{
		if( taxinode->horde_mount == 2224 )
			modelid = 295; // In case it's a wyvern
		else
			modelid = 1566; // In case it's a bat or a bad id
	}
	else
	{
		if( taxinode->alliance_mount == 3837 )
			modelid = 479; // In case it's an hippogryph
		else
			modelid = 1147; // In case it's a gryphon or a bad id
	}

	//GetPlayer( )->setDismountCost( newmoney );

	data << uint32( 0 );
	// 0 Ok
	// 1 Unspecified Server Taxi Error
	// 2.There is no direct path to that direction
	// 3 Not enough Money
	SendPacket( &data );
	DEBUG_LOG( "WORLD"," Sent SMSG_ACTIVATETAXIREPLY" );

	// 0x001000 seems to make a mount visible
	// 0x002000 seems to make you sit on the mount, and the mount move with you
	// 0x000004 locks you so you can't move, no msg_move updates are sent to the server
	// 0x000008 seems to enable detailed collision checking

	// check for a summon -> if we do, remove.
	if(_player->GetSummon() != NULL)
	{
		if(_player->GetSummon()->GetUInt32Value(UNIT_CREATED_BY_SPELL) > 0)
			_player->GetSummon()->Dismiss(false);						   // warlock summon -> dismiss
		else
			_player->GetSummon()->Remove(false, true, true);			   // hunter pet -> just remove for later re-call
	}

	_player->taxi_model_id = modelid;
	
	// build the rest of the path list
	for(uint32 i = 2; i < nodecount; ++i)
	{
		TaxiPath * np = sTaxiMgr.GetTaxiPath(pathes[i-1], pathes[i]);
		if(!np) return;

		if (np->GetID() == 766 || np->GetID() == 767 || np->GetID() == 771 || np->GetID() == 772)
		{
			_player->m_taxiPaths.clear();
			return;
		}

		// add to the list.. :)
		_player->m_taxiPaths.push_back(np);
	}

	// start the first trip :)
	GetPlayer()->TaxiStart(taxipath, modelid, 0);
}
