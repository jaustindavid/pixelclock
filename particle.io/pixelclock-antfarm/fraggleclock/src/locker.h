#pragma once

/*
 * Usage: 
 *    if (acqiure(LOCK, my_id)) {
 *      .. do things
 *      release(LOCK);
 *    }
 */

int _locks[256];
#define UNLOCKED -1


// true if holder_id has lock_id
bool acquire(byte lock_id, int holder_id) {
  if (_locks[lock_id] == UNLOCKED) {
    _locks[lock_id] = holder_id;
  }
  return _locks[lock_id] == holder_id;
} // bool acquire(lock_id, holder_id)


// releases lock_id
void release(byte lock_id) {
  _locks[lock_id] = UNLOCKED;
}


void locker_setup() {
  for (int i = 0; i < 256; i++) {
    release((byte)i);
  }
} // setup_locks()
