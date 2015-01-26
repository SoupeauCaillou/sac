/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <map>
#include <list>
#include <queue>

class IStorageProxy {
    public:
    virtual std::string getValue(const std::string& columnName) = 0;
    virtual void setValue(const std::string& columnName,
                          const std::string& value,
                          bool pushNewElement) = 0;
    virtual void pushAnElement() = 0;
    virtual void popAnElement() = 0;
    virtual bool isEmpty() = 0;
    virtual const std::string& getTableName() = 0;
    virtual const std::map<std::string, std::string>&
    getColumnsNameAndType() = 0;
};

template <class T> class StorageProxy : public IStorageProxy {
    public:
    virtual std::string getValue(const std::string& columnName) = 0;
    virtual void setValue(const std::string& columnName,
                          const std::string& value,
                          bool pushNewElement) = 0;

    void pushAnElement() { _queue.push(T()); }

    void popAnElement() { _queue.pop(); }

    bool isEmpty() { return (_queue.empty()); }

    const std::string& getTableName() { return _tableName; }

    const std::map<std::string, std::string>& getColumnsNameAndType() {
        return _columnsNameAndType;
    }

    public:
    std::queue<T> _queue;

    protected:
    std::string _tableName;
    std::map<std::string, std::string> _columnsNameAndType;
};
