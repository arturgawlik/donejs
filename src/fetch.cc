#include <v8.h>

using v8::Context;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

namespace done::fetch {

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  // TODO: this module will be implemented in js land instead of c++
}

} // namespace done::fetch
