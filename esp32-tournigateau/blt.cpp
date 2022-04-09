#include "blt.h"

#include <Arduino.h>

#include "BleKeyboard.h"

namespace blt {

// ----------------------------------
// const - types
// ----------------------------------

// ----------------------------------
// local variable
// ----------------------------------

static BleKeyboard* keyboard = nullptr;

// ----------------------------------
// public variable
// ----------------------------------

// ----------------------------------
// local services
// ----------------------------------

// ----------------------------------
// public services
// ----------------------------------

void initialize() {
    keyboard = new BleKeyboard("Tournigateau", "Petou Company", 100);
    keyboard->setDelay(50);
    keyboard->begin();
}

void sendVolumeUp() {
    if (isConnected()) {
        keyboard->releaseAll();
        delay(20);
        keyboard->write(KEY_MEDIA_VOLUME_DOWN);
        delay(20);
        keyboard->releaseAll();
    }
}

bool isConnected() {
    if (keyboard == nullptr) {
        return false;
    }
    return keyboard->isConnected();
}

}  // namespace blt