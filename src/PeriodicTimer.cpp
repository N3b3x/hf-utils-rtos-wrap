#include "PeriodicTimer.h"

PeriodicTimer::~PeriodicTimer() {
    if (created) {
        os_timer_deactivate_and_delete_ex(timer);
    }
}

