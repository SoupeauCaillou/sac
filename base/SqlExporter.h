#pragma once

#include <string>

class SqlExporter {
	public:
		void init(const std::string& dbFile);
		void update();
		
	private:
		std::string dbFilename;
};
