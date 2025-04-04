#include <v8.h>

using v8::Local;
using v8::Object;

namespace done::process {
/*
    Initializes JS `process` buildins.
*/
void Initialize(Local<Object> globalObj, int argc, char **argv);
} // namespace done::process