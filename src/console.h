#include <v8.h>

using v8::Local;
using v8::ObjectTemplate;

namespace done::console {
void Initialize(Local<ObjectTemplate> globalObjTmpl);
}