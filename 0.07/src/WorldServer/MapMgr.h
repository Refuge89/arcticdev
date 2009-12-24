/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2009 Arctic Server Team
 * See COPYING for license details.
 */
 
#ifndef __MAPMGR_H
#define __MAPMGR_H

class MapCell;
class Map;
class Object;
class MapScriptInterface;
class WorldSession;
class GameObject;
class Creature;
class Player;
class Pet;
class Vehicle;
class Transporter;
class Corpse;
class CBattleground;
class Instance;
class InstanceScript;


enum MapMgrTimers
{
	MMUPDATE_OBJECTS = 0,
	MMUPDATE_SESSIONS = 1,
	MMUPDATE_FIELDS = 2,
	MMUPDATE_IDLE_OBJECTS = 3,
	MMUPDATE_ACTIVE_OBJECTS = 4,
	MMUPDATE_COUNT = 5
};

enum ObjectActiveState
{
	OBJECT_STATE_NONE	 = 0,
	OBJECT_STATE_INACTIVE = 1,
	OBJECT_STATE_ACTIVE   = 2,
};

typedef unordered_set<ObjectPointer > ObjectSet;
typedef unordered_set<ObjectPointer > UpdateQueue;
typedef unordered_set<PlayerPointer  > PUpdateQueue;
typedef unordered_set<PlayerPointer  > PlayerSet;
typedef HM_NAMESPACE::hash_map<const uint32, ObjectPointer > StorageMap;
typedef unordered_set<uint64> CombatProgressMap;
typedef unordered_set<VehiclePointer> VehicleSet;
typedef unordered_set<CreaturePointer> CreatureSet;
typedef unordered_set<GameObjectPointer > GameObjectSet;
typedef HM_NAMESPACE::hash_map<const uint32, VehiclePointer> VehicleSqlIdMap;
typedef HM_NAMESPACE::hash_map<const uint32, CreaturePointer> CreatureSqlIdMap;
typedef HM_NAMESPACE::hash_map<const uint32, GameObjectPointer > GameObjectSqlIdMap;

#define MAX_TRANSPORTERS_PER_MAP 25

class Transporter;
#define RESERVE_EXPAND_SIZE 1024

#define CALL_INSTANCE_SCRIPT_EVENT( Mgr, Func ) if ( Mgr != NULL && Mgr->GetScript() != NULL ) Mgr->GetScript()->Func

class SERVER_DECL MapMgr : public CellHandler <MapCell>, public EventableObject,public ThreadContext, public std::tr1::enable_shared_from_this<MapMgr>
{
	friend class UpdateObjectThread;
	friend class ObjectUpdaterThread;
	friend class MapCell;
	friend class MapScriptInterface;
public:

	Mutex m_objectinsertlock;
	ObjectSet m_objectinsertpool;
	void AddObject(ObjectPointer);

//////////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of GameObjects         //
//////////////////////////////////////////////////////////////
	typedef HM_NAMESPACE::hash_map<const uint32, GameObjectPointer > GameObjectMap;
	GameObjectMap m_gameObjectStorage;
	uint32 m_GOHighGuid;
	GameObjectPointer CreateGameObject(uint32 entry);

	ARCTIC_INLINE uint32 GenerateGameobjectGuid()
	{
		m_GOHighGuid &= 0x00FFFFFF;
		return ++m_GOHighGuid;
	}

	ARCTIC_INLINE GameObjectPointer GetGameObject(uint32 guid)
	{
		GameObjectMap::iterator itr = m_gameObjectStorage.find(guid);
		return (itr != m_gameObjectStorage.end()) ? m_gameObjectStorage[guid] : NULLGOB;
	}

//////////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of Vehicles            //
//////////////////////////////////////////////////////////////
	uint32 m_VehicleArraySize;
	uint32 m_VehicleHighGuid;
	HM_NAMESPACE::unordered_map<const uint32,VehiclePointer> m_VehicleStorage;
	VehiclePointer CreateVehicle(uint32 entry);

	__inline VehiclePointer GetVehicle(uint32 guid)
	{
		return guid <= m_VehicleHighGuid ? m_VehicleStorage[guid] : NULLVEHICLE;
	}
//////////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of Creatures           //
//////////////////////////////////////////////////////////////
	uint32 m_CreatureArraySize;
	uint32 m_CreatureHighGuid;
	HM_NAMESPACE::unordered_map<const uint32,CreaturePointer> m_CreatureStorage;
	CreaturePointer CreateCreature(uint32 entry);

	__inline CreaturePointer GetCreature(uint32 guid)
	{
		return guid <= m_CreatureHighGuid ? m_CreatureStorage[guid] : NULLCREATURE;
	}
//////////////////////////////////////////////////////////////
// Local (mapmgr) storage/generation of DynamicObjects      //
//////////////////////////////////////////////////////////////
	uint32 m_DynamicObjectHighGuid;
	typedef HM_NAMESPACE::hash_map<const uint32, DynamicObjectPointer> DynamicObjectStorageMap;
	DynamicObjectStorageMap m_DynamicObjectStorage;
	DynamicObjectPointer CreateDynamicObject();
	
	ARCTIC_INLINE DynamicObjectPointer GetDynamicObject(uint32 guid)
	{
		DynamicObjectStorageMap::iterator itr = m_DynamicObjectStorage.find(guid);
		return (itr != m_DynamicObjectStorage.end()) ? m_DynamicObjectStorage[guid] : NULLDYN;
	}

//////////////////////////////////////////////////////////////
// Local (mapmgr) storage of pets                           //
//////////////////////////////////////////////////////////////
	typedef HM_NAMESPACE::hash_map<const uint32, PetPointer> PetStorageMap;
	PetStorageMap m_PetStorage;
	__inline PetPointer GetPet(uint32 guid)
	{
		PetStorageMap::iterator itr = m_PetStorage.find(guid);
		return (itr != m_PetStorage.end()) ? m_PetStorage[guid] : NULLPET;
	}

//////////////////////////////////////////////////////////////
// Local (mapmgr) storage of players for faster lookup      //
//////////////////////////////////////////////////////////////
    
    // double typedef lolz// a compile breaker..
	typedef HM_NAMESPACE::hash_map<const uint32, PlayerPointer> PlayerStorageMap;

	Mutex PlayerStorageMaplock;

	PlayerStorageMap m_PlayerStorage;
	__inline PlayerPointer GetPlayer(uint32 guid)
	{
		PlayerStorageMaplock.Acquire();
		PlayerStorageMap::iterator itr = m_PlayerStorage.find(guid);
		if (itr != m_PlayerStorage.end())
		{
			PlayerStorageMaplock.Release();
			return m_PlayerStorage[guid];
		}
		PlayerStorageMaplock.Release();
		return NULLPLR;
	}

//////////////////////////////////////////////////////////////
// Local (mapmgr) storage of combats in progress            //
//////////////////////////////////////////////////////////////
	CombatProgressMap _combatProgress;
	void AddCombatInProgress(uint64 guid)
	{
		_combatProgress.insert(guid);
	}
	void RemoveCombatInProgress(uint64 guid)
	{
		_combatProgress.erase(guid);
	}
	ARCTIC_INLINE bool IsCombatInProgress()
	{ 
		// temporary disabled until AI updates list correctly.
		return false;

		//if all players are out, list should be empty.
		if(!HasPlayers())
			_combatProgress.clear();
		return (_combatProgress.size() > 0);
	}

//////////////////////////////////////////////////////////////
// Lookup Wrappers                                          //
//////////////////////////////////////////////////////////////
	UnitPointer GetUnit(const uint64 & guid);
	ObjectPointer _GetObject(const uint64 & guid);

	bool run();
	bool Do();

	MapMgr(Map *map, uint32 mapid, uint32 instanceid);
	~MapMgr();

	//����������� � ���������� ��� ������������ ����� � ��������
	void *operator new(size_t);
	void operator delete(void*);

	void Init();
	void Destructor();

	void PushObject(ObjectPointer obj);
	void PushStaticObject(ObjectPointer obj);
	void RemoveObject(ObjectPointer obj, bool free_guid);
	void ChangeObjectLocation(ObjectPointer obj); // update inrange lists
	void ChangeFarsightLocation(PlayerPointer plr, UnitPointer farsight, bool apply);
	void ChangeFarsightLocation(PlayerPointer plr, float X, float Y, bool apply);

	// Mark object as updated
	void ObjectUpdated(ObjectPointer obj);
	void UpdateCellActivity(uint32 x, uint32 y, int radius);

	// Terrain Functions
	ARCTIC_INLINE float  GetLandHeight(float x, float y) { return GetBaseMap()->GetLandHeight(x, y); }
	ARCTIC_INLINE float  GetWaterHeight(float x, float y) { return GetBaseMap()->GetWaterHeight(x, y); }
	ARCTIC_INLINE uint8  GetWaterType(float x, float y) { return GetBaseMap()->GetWaterType(x, y); }
	ARCTIC_INLINE uint8  GetWalkableState(float x, float y) { return GetBaseMap()->GetWalkableState(x, y); }
	ARCTIC_INLINE uint16 GetAreaID(float x, float y) { return GetBaseMap()->GetAreaID(x, y); }

	ARCTIC_INLINE uint32 GetMapId() { return _mapId; }

	void AddForcedCell(MapCell * c);
	void RemoveForcedCell(MapCell * c);

	void PushToProcessed(PlayerPointer plr);

	ARCTIC_INLINE bool HasPlayers() { return (m_PlayerStorage.size() > 0); }
	void TeleportPlayers();

	ARCTIC_INLINE uint32 GetInstanceID() { return m_instanceID; }
	ARCTIC_INLINE MapInfo *GetMapInfo() { return pMapInfo; }

	ARCTIC_INLINE void SetCollision(bool enable) { collision = enable; }
	ARCTIC_INLINE bool IsCollisionEnabled() { return collision; }

	ARCTIC_INLINE MapScriptInterface * GetInterface() { return ScriptInterface; }
	virtual int32 event_GetInstanceID() { return m_instanceID; }

	void LoadAllCells();
	ARCTIC_INLINE size_t GetPlayerCount() { return m_PlayerStorage.size(); }

	void DespawnEvent(uint8 eventToRemove);
	void SpawnEvent(uint8 eventId);

	void _PerformObjectDuties();
	uint32 mLoopCounter;
	uint32 lastGameobjectUpdate;
	uint32 lastUnitUpdate;
	void EventCorpseDespawn(uint64 guid);

	time_t InactiveMoveTime;
    uint32 iInstanceMode;

	void UnloadCell(uint32 x,uint32 y);
	void EventRespawnVehicle(VehiclePointer v, MapCell * p);
	void EventRespawnCreature(CreaturePointer c, MapCell * p);
	void EventRespawnGameObject(GameObjectPointer o, MapCell * c);
	void SendMessageToCellPlayers(ObjectPointer obj, WorldPacket * packet, uint32 cell_radius = 2);
	void SendChatMessageToCellPlayers(ObjectPointer obj, WorldPacket * packet, uint32 cell_radius, uint32 langpos, int32 lang, WorldSession * originator);

	Instance * pInstance;
	void BeginInstanceExpireCountdown();
	void HookOnAreaTrigger(PlayerPointer plr, uint32 id);
	
	// better hope to clear any references to us when calling this :P
	void InstanceShutdown()
	{
		pInstance = NULL;
		Terminate();
	}

	// kill the worker thread only
	void KillThread()
	{
		pInstance=NULL;
		thread_kill_only = true;
		Terminate();
		while(thread_running)
		{
			Sleep(100);
		}
	}

protected:

	bool m_sharedPtrDestructed;
	// Collect and send updates to clients
	void _UpdateObjects();

private:
	// Objects that exist on map
 
	uint32 _mapId;
	set<ObjectPointer > _mapWideStaticObjects;

	bool _CellActive(uint32 x, uint32 y);
	void UpdateInRangeSet(ObjectPointer obj, PlayerPointer plObj, MapCell* cell);

public:
	// Distance a Player can "see" other objects and receive updates from them (!! ALREADY dist*dist !!)
	float m_UpdateDistance;

private:
	// Update System.
	FastMutex m_updateMutex;		// use a user-mode mutex for extra speed
	UpdateQueue _updates;
	PUpdateQueue _processQueue;

	// Sessions..
	Mutex MapSessionsMutex;//FOR Maps !
	SessionSet MapSessions;

	// Map Information.
	MapInfo *pMapInfo;
	uint32 m_instanceID;

	MapScriptInterface * ScriptInterface;

	bool collision;

public:
#ifdef WIN32
	DWORD threadid;
#endif

	GameObjectSet activeGameObjects;
	CreatureSet activeCreatures;
	VehicleSet activeVehicles;
	EventableObjectHolder eventHolder;
	BattlegroundPointer m_battleground;
	set<CorpsePointer > m_corpses;
	VehicleSqlIdMap _sqlids_vehicles;
	CreatureSqlIdMap _sqlids_creatures;
	GameObjectSqlIdMap _sqlids_gameobjects;

	VehiclePointer GetSqlIdVehicle(uint32 sqlid);
	CreaturePointer GetSqlIdCreature(uint32 sqlid);
	GameObjectPointer GetSqlIdGameObject(uint32 sqlid);
	deque<uint32> _reusable_guids_creature;
	deque<uint32> _reusable_guids_vehicle;

	bool forced_expire;
	bool thread_kill_only;
	bool thread_running;

	// world state manager stuff
	WorldStateManager* m_stateManager;

	// bytebuffer caching
	ByteBuffer m_updateBuffer;
	ByteBuffer m_createBuffer;
	ByteBuffer m_updateBuildBuffer;
	ByteBuffer m_compressionBuffer;

public:

	// get!
	ARCTIC_INLINE WorldStateManager& GetStateManager() { return *m_stateManager; }

	// send packet functions for state manager
	void SendPacketToPlayers(int32 iZoneMask, int32 iFactionMask, WorldPacket *pData);
	void SendPacketToPlayers(int32 iZoneMask, int32 iFactionMask, StackPacket *pData);
	void SendPvPCaptureMessage(int32 iZoneMask, uint32 ZoneId, const char * Format, ...);

	// auras :< (world pvp)
	void RemoveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId);
	void RemovePositiveAuraFromPlayers(int32 iFactionMask, uint32 uAuraId);
	void CastSpellOnPlayers(int32 iFactionMask, uint32 uSpellId);


public:

	// stored iterators for safe checking
	PetStorageMap::iterator __pet_iterator;
	PlayerStorageMap::iterator __player_iterator;

	VehicleSet::iterator __vehicle_iterator;
	CreatureSet::iterator __creature_iterator;
	GameObjectSet::iterator __gameobject_iterator;
	
	SessionSet::iterator __session_iterator_1;
	SessionSet::iterator __session_iterator_2;

	// Script related
	InstanceScript* GetScript() { return mInstanceScript; };
	void LoadInstanceScript();
	void CallScriptUpdate();

protected:

	InstanceScript* mInstanceScript;
};

#endif