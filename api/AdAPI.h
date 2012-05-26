#pragma once

class AdAPI {
    public:
        virtual void showAd() {}
        virtual bool done() { return true; }
};
