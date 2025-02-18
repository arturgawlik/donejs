#include <v8.h>

using v8::Local;
using v8::ObjectTemplate;

namespace done::syscall {
/*
    Initializes JS `syscall` buildins.
*/
void Initialize(Local<ObjectTemplate> globalObjTmpl);
} // namespace done::syscall
