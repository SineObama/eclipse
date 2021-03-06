/*
 * SimpleTiming.cpp
 *
 *  Created on: 2015��12��13��
 *      Author: Sine
 */

#include "SimpleTiming.h"
#include <ctime>

namespace Sine {

SimpleTiming::SimpleTiming() {
    newTime = oldTime = -1;
}

void SimpleTiming::operator()() {
    oldTime = newTime;
    newTime = clock();
}

int SimpleTiming::get() {
    if (oldTime == -1)
        return 0;
    return int(newTime - oldTime);
}

}
