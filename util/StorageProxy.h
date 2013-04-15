/*
	This file is part of RecursiveRunner.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	RecursiveRunner is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	RecursiveRunner is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <list>
#include <stack>
#include <string>
#include <map>

class StorageProxy {
    public:
        StorageProxy(const std::string & structName, const std::list<std::string> & columnsName)
            : _tableName(structName), _columnsName(columnsName)
            {}
        //Return the columns' names of the database table
        const std::list<std::string> & getColumnsName() const;

        //get the value from the table
        std::string getValue(const std::string& columnName);

        //set the value into the table
        void setValue(const std::string& columnName, const std::string& value);

    public:
        std::stack<std::map<std::string, std::string> > _stack;
        std::string _tableName;
        std::list<std::string> _columnsName;
};
