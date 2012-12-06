#if 0
#include "SqlExporter.h"
#include "SqlExportGenerated.h"
#include "EntityManager.h"
#include "systems/System.h"
#include <sqlite3.h>

static void queryCreateEntityTable(sqlite3 *db) {
	char *zErrMsg = 0;
	std::string query = "create table entity(e number(16) default '0')";
	int rc = sqlite3_exec(db, query.c_str(), 0, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		LOGI("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
}

void SqlExporter::init(const std::string& dbFile) {
	char *zErrMsg = 0;
	this->dbFilename = dbFile;

	int rc = sqlite3_open(dbFilename.c_str(), &db);

	// execute 'create Entity table query'
	queryCreateEntityTable();

	std::vector<std::string> allSystems = ComponentSystem::registeredSystemNames();
	for (int i=0; i<allSystems.size(); i++) {
		// execute 'create this system table query'
		queryCreateSystemTable(allSystems[i]);
	}
	sqlite3_close(db);
}

void SqlExporter::update() {
	// browse every entities
	std::vector<Entity> entities = allEntities();
	// create missing entities
	std::vector<Entity> dbEntities = queryRetrieveAllEntities();
	for (int i=0; i<entities.size()) {
		if (std::find(dbEntities.begin(), dbEntities.end(), entities[i] == -1) {
			queryCreateEntity(entities[i]);
		}
	}
	// remove deleted entities
	for (int i=0; i<dbEntities.size()) {
		if (std::find(entities.begin(), entities.end(), dbEntities[i] == -1) {
			queryRemoveEntity(dbEntities[i]);
		}
	}

	// update components
	std::vector<std::string> allSystems = ComponentSystem::registeredSystemNames();
	for (int i=0; i<allSystems.size(); i++) {
		ComponentSystem::Named(it->first)
		std::vector<Entity> e = allSystems[i].RetrieveAllEntityWithComponent();
	}
}
#endif
