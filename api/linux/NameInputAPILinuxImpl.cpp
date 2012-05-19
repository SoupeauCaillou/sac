#include "NameInputAPILinuxImpl.h"
#include "base/Log.h"
#include <iostream>

void NameInputAPILinuxImpl::show() {
    __log_enabled = false;
    std::cout << "Enter your name (oui Gautier, j'ai tout cassé)" << std::endl;
}

bool NameInputAPILinuxImpl::done(std::string& name) {
    std::cin >> name;
    return true;
}

void NameInputAPILinuxImpl::hide() {
    __log_enabled = true;
}
