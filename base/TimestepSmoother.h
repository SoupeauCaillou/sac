#pragma once

#define HISTORY_SIZE 11

class TimestepSmoother {
    public:
        TimestepSmoother();

        float smooth(float dt);

    private:
        float timestepHistory[HISTORY_SIZE];
        int next;
};