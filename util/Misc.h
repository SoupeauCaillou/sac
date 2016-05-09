#pragma once

/* Returns shortest angle to aim /target/ from /position/, given current /rotation/ */
extern float aim(const glm::vec2& position, float rotation, const glm::vec2& target);
