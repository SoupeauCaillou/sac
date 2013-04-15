#include "StorageProxy.h"

const std::list<std::string> & StorageProxy::getColumnsName() const {
    return _columnsName;
}

//get the value from the table
std::string StorageProxy::getValue(const std::string& columnName) {
    return _stack.top()[columnName];
}

//set the value into the table
void StorageProxy::setValue(const std::string& columnName, const std::string& value) {
    _stack.top()[columnName] = value;
}
