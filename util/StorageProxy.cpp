#include "StorageProxy.h"

#include <sstream>

std::string StorageProxy::int2sql(int value) {
    std::stringstream ss;

    ss << value;

    return ss.str();
}

int StorageProxy::sql2int(const std::string & value) {
    std::istringstream iss(value);

    int res;
    iss >> res;

    return res;
}

