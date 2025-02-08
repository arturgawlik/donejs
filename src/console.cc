#include "v8-primitive.h"
#include "v8-template.h"
#include <v8.h>

using v8::Boolean;
using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

namespace done::console {

void LogSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> valToLog = args[0].As<Value>();

  if (valToLog->IsString()) {
    Local<String> v8Str = valToLog.As<String>();
    String::Utf8Value utf8(isolate, v8Str);
    char *str = *utf8;
    printf("%s\n", str);
  } else if (valToLog->IsNumber()) {
    Local<Number> v8Number = valToLog.As<Number>();
    int integer = v8Number->Int32Value(context).ToChecked();
    printf("%d\n", integer);
  } else if (valToLog->IsBoolean()) {
    Local<Boolean> v8Boolean = valToLog.As<Boolean>();
    bool boolean = v8Boolean->BooleanValue(isolate);
    printf("%s\n", boolean ? "true" : "false");
  }
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> consoleObjTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "console"),
                     consoleObjTmpl);

  Local<FunctionTemplate> logFnTmpl = FunctionTemplate::New(isolate, LogSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "log"),
                      logFnTmpl);
}

} // namespace done::console
