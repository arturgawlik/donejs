#include <v8.h>

using v8::Local;
using v8::ObjectTemplate;

namespace done::fetch {
/*
    Initializes JS `process` buildins.
*/
void Initialize(Local<ObjectTemplate> globalObjTmpl);
} // namespace done::fetch
