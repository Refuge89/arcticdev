/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2014 Arctic Server Team
 * See COPYING for license details.
 */

#pragma once

struct ItemPrototype;
class MapMgr;
class Player;
class LootRoll : public EventableObject
{
public:
	LootRoll();
	~LootRoll();
	void Init(uint32 timer, uint32 groupcount, uint64 guid, uint32 slotid, uint32 itemid, uint32 randomsuffixid, uint32 randompropertyid, MapMgr* mgr);
	void PlayerRolled(Player* player, uint8 choice);
	void Finalize();

	int32 event_GetInstanceID();

private:
	Mutex mLootLock;
	std::map<uint32, uint32> m_NeedRolls;
	std::map<uint32, uint32> m_GreedRolls;
	set<uint32> m_passRolls;
	uint32 _groupcount;
	uint32 _slotid;
	uint32 _itemid;
	uint32 _randomsuffixid;
	uint32 _randompropertyid;
	uint32 _remaining;
	uint64 _guid;
	MapMgr* _mgr;
};

typedef vector<pair<RandomProps*, float> > RandomPropertyVector;
typedef vector<pair<ItemRandomSuffixEntry*, float> > RandomSuffixVector;

typedef struct _LootItem
{
	ItemPrototype * itemproto;
	uint32 displayid;
}_LootItem;

typedef std::set<uint32> LooterSet;

typedef struct __LootItem
{
	_LootItem item;
	uint32 iItemsCount;
	RandomProps * iRandomProperty;
	ItemRandomSuffixEntry * iRandomSuffix;
	LootRoll* roll;
	bool passed;
	LooterSet has_looted;
	uint32 ffa_loot;
}__LootItem;


typedef struct StoreLootItem
{
	_LootItem item;
	float chance;
	float chance2;
	uint32 mincount;
	uint32 maxcount;
	uint32 ffa_loot;
}StoreLootItem;


typedef struct StoreLootList
{
	uint32 count;
	StoreLootItem *items;
}StoreLootList;

struct Loot
{
	std::vector<__LootItem> items;
	uint32 gold;
	LooterSet looters;
	bool HasItems(Player* Looter);
	bool HasLoot(Player* Looter);
};

struct tempy
{
	uint32 itemid;
	float chance;
	float chance_2;
	uint32 mincount;
	uint32 maxcount;
	uint32 ffa_loot;
};

typedef HM_NAMESPACE::hash_map<uint32, StoreLootList > LootStore;

enum PARTY_LOOT
{
	PARTY_LOOT_FFA		= 0,
	PARTY_LOOT_RR		= 1,
	PARTY_LOOT_MASTER	= 2,
	PARTY_LOOT_GROUP	= 3,
	PARTY_LOOT_NBG		= 4,
};
enum PARTY_ROLL
{
	NEED	= 1,
	GREED	= 2,
};

class SERVER_DECL LootMgr : public Singleton < LootMgr >
{
public:
	LootMgr();
	~LootMgr();

	void AddLoot(Loot * loot, uint32 itemid, uint32 mincount, uint32 maxcount, uint32 ffa_loot);
	void FillCreatureLoot(Loot * loot,uint32 loot_id, bool heroic);
	void FillGOLoot(Loot * loot,uint32 loot_id, bool heroic);
	void FillItemLoot(Loot *loot, uint32 loot_id);
	void FillFishingLoot(Loot * loot,uint32 loot_id);
	void FillGatheringLoot(Loot * loot,uint32 loot_id);
	void FillPickpocketingLoot(Loot *loot, uint32 loot_id);
	void FillDisenchantingLoot(Loot *loot, uint32 loot_id);
	void FillProspectingLoot(Loot *loot, uint32 loot_id);
	void FillMillingLoot(Loot *loot, uint32 loot_id);

	bool CanGODrop(uint32 LootId,uint32 itemid);
	bool IsPickpocketable(uint32 creatureId);
	bool IsSkinnable(uint32 creatureId);
	bool IsFishable(uint32 zoneid);

	void LoadLoot();
	void LoadDelayedLoot();
	void LoadLootProp();

	LootStore CreatureLoot;
	LootStore FishingLoot;
	LootStore GatheringLoot;
	LootStore GOLoot;
	LootStore ItemLoot;
	LootStore ProspectingLoot;
	LootStore DisenchantingLoot;
	LootStore PickpocketingLoot;
	LootStore MillingLoot;
	std::map<uint32, std::set<uint32> > quest_loot_go;

	RandomProps * GetRandomProperties(ItemPrototype * proto);
	ItemRandomSuffixEntry * GetRandomSuffix(ItemPrototype * proto);

	bool is_loading;

	void FillObjectLootMap(map<uint32, vector<uint32> > *dest);

private:
	void LoadLootTables(const char * szTableName,LootStore * LootTable);
	void PushLoot(StoreLootList *list,Loot * loot, bool heroic, bool disenchant);

	map<uint32, RandomPropertyVector> _randomprops;
	map<uint32, RandomSuffixVector> _randomsuffix;
};

#define lootmgr LootMgr::getSingleton()
