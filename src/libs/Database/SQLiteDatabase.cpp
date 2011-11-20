/*
 * Arctic MMORPG Server Software
 * Copyright (c) 2008-2011 Arctic Server Team
 * See COPYING for license details.
 */

#include "DatabaseEnv.h"
#include "../CrashHandler.h"
#include "../NGLog.h"

#if defined(ENABLE_DATABASE_SQLITE) 

#include "SQLiteDatabase.h"

SQLiteDatabase::~SQLiteDatabase()
{
	delete Connections;
}

SQLiteDatabase::SQLiteDatabase() : Database()
{

}

void SQLiteDatabase::_BeginTransaction(DatabaseConnection * conn)
{
	//_SendQuery( conn, "BEGIN TRANSACTION", false );
}

void SQLiteDatabase::_EndTransaction(DatabaseConnection * conn)
{
	//_SendQuery( conn, "COMMIT TRANSACTION", false );
}

bool SQLiteDatabase::Initialize(const char* Hostname, unsigned int port, const char* Username, const char* Password, const char* DatabaseName, uint32 ConnectionCount, uint32 BufferSize)
{
	//int result;
	//SQLiteDatabaseConnection * con;

	mHostname = string(Hostname);
	mConnectionCount = ConnectionCount;
	mUsername = string(Username);
	mPassword = string(Password);

	Log.Notice("SQLiteDatabase", "Opening `%s`...", Hostname);
/*	Connections = new SQLiteDatabaseConnection;
	con = static_cast<SQLiteDatabaseConnection*>(Connections);
	
	result = sqlite3_open( mHostname.c_str(), &con->handle );
	if( con->handle == NULL || result != SQLITE_OK )
	{
		Log.Error("SQLiteDatabase", "Could not open %s.", Hostname);
		return false;
	}
*/
	return true;
}

string SQLiteDatabase::EscapeString(string Escape)
{
	char a2[16384] = {0};
	sqlite3_snprintf( 16384, a2, Escape.c_str() );
	return string(a2);
}

void SQLiteDatabase::EscapeLongString(const char * str, uint32 len, stringstream& out)
{
	char a2[65536*3] = {0};
	sqlite3_snprintf( 65536*3, a2, str );
}

string SQLiteDatabase::EscapeString(const char * esc, DatabaseConnection * con)
{
	char a2[16384] = {0};
	sqlite3_snprintf( 16384, a2, esc );
	return string(a2);
}

void SQLiteDatabase::Shutdown()
{
//	sqlite3_close( static_cast<SQLiteDatabaseConnection*>(Connections)->handle );
}

bool SQLiteDatabase::_SendQuery(DatabaseConnection *con, const char* Sql, bool Self)
{
	SQLiteDatabaseConnection * db = static_cast<SQLiteDatabaseConnection*>( con );
	char * err = NULL;
	int result = sqlite3_get_table( db->handle, Sql, &db->tabledata, &db->rows, &db->cols, &err );

	if( result != SQLITE_OK )
	{
		printf("SQLite: Query failed with error message: %s", err ? err : "NULL");
		return false;
	}
	return true;
}

QueryResult * SQLiteDatabase::_StoreQueryResult(DatabaseConnection * con)
{
	return NULL;
}

}

#endif

