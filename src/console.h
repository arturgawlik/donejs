#include <v8.h>

using v8::Local;
using v8::ObjectTemplate;

namespace done::console {
/*
    Initializes JS `console` buildins.
*/
void Initialize(Local<ObjectTemplate> globalObjTmpl);
} // namespace done::console
