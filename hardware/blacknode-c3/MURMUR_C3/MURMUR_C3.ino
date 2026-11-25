// MURMUR BLACKNODE C3 1.0.0
// Complete implementation lives in Node.h to avoid Arduino automatic prototypes.
// Keep this .ino and all three headers in the MURMUR_C3 sketch folder.
#include "Node.h"
void setup() { node::begin(); }
void loop() { node::tick(); }
