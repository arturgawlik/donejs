#include "v8-primitive.h"
#include "v8-template.h"
#include <v8.h>

using v8::Context;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::ObjectTemplate;
using v8::Value;

namespace done::process {

void ExitSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> exitCode = args[0].As<Value>();

  if (exitCode->IsNumber()) {
    Local<Number> v8Num = exitCode.As<Number>();
    int exitCode = 0;
    v8Num->Int32Value(context).FromMaybe(exitCode);
    // TODO: probably it's not proper way just exiting process
    // I think that maybe I should before that cleanup some V8 resources
    // buuut maybe for now it's totally okay
    exit(exitCode);
  } else {
    // throw JS exception
  }
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<FunctionTemplate> exitFnTmpl = FunctionTemplate::New(isolate, ExitSlow);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "exit"),
                     exitFnTmpl);
}

} // namespace done::process
