#include <v8.h>

using v8::Local;
using v8::ObjectTemplate;

namespace done::textEncoder {
/*
    Initializes JS `textEncoder` buildins.
*/
void Initialize(Local<ObjectTemplate> globalObjTmpl);
} // namespace done::textEncoder
