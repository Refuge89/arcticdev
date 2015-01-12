/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2014 Arctic Server Team
 * See COPYING for license details.
 */

#include "StdAfx.h"
#include "Setup.h"

// Archmage Malin
#define GOSSIP_ARCHMAGE_MALIN    "Can you send me to Theramore? I have an urgent message for Lady Jaina from Highlord Bolvar."

class ArchmageMalin_Gossip : public GossipScript
{
public:
    void GossipHello(Object* pObject, Player* plr, bool AutoSend)
    {
        GossipMenu *Menu;
        objmgr.CreateGossipMenuForPlayer(&Menu, pObject->GetGUID(), 11469, plr);

		if(plr->GetQuestLogForEntry(11223))
        Menu->AddItem( 0, GOSSIP_ARCHMAGE_MALIN, 1);
        
        if(AutoSend)
            Menu->SendTo(plr);
    }

    void GossipSelectOption(Object* pObject, Player* plr, uint32 Id, uint32 IntId, const char * Code)
    {
		Creature* pCreature = (pObject->GetTypeId()==TYPEID_UNIT)?(TO_CREATURE(pObject)):NULL;
		if(pObject->GetTypeId()!=TYPEID_UNIT)
			return;
		
		switch(IntId)
        {
        case 1:
			{
				plr->Gossip_Complete();
				pCreature->CastSpell(plr, dbcSpell.LookupEntry(42711), true);
            }break;
		}
    }

    void Destroy()
    {
        delete this;
    }
};


void SetupStormwindGossip(ScriptMgr * mgr)
{
	GossipScript * ArchmageMalinGossip = (GossipScript*) new ArchmageMalin_Gossip;

	mgr->register_gossip_script(2708, ArchmageMalinGossip); // Archmage Malin

}
