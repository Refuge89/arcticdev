/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2014 Arctic Server Team
 * See COPYING for license details.
 */

#pragma once

class WorldRunnable : public ThreadContext
{
public:
	WorldRunnable();
	bool run();
};
