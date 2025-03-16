#include <v8.h>

using v8::Local;
using v8::ObjectTemplate;

namespace done::textDecoder {
/*
    Initializes JS `textDecoder` buildins.
*/
void Initialize(Local<ObjectTemplate> globalObjTmpl);
} // namespace done::textDecoder
