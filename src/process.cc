#include "v8-primitive.h"
#include "v8-template.h"
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

namespace done::process {

void ExitSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> exitCode = args[0].As<Value>();

  if (exitCode->IsNumber()) {
    Local<Number> v8Num = exitCode.As<Number>();
    int exitCode = 0;
    v8Num->Int32Value(context).FromMaybe(exitCode);
    // It seems that simple `exit()` sys call is right here - e.g. in nodejs
    // before exit appropriate callback `exit` (event) is called, but for my
    // implementation it seems right thing to do right now.
    exit(exitCode);
  } else {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'exitCode' must be an number when calling 'exit()' function")));
  }
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<FunctionTemplate> exitFnTmpl = FunctionTemplate::New(isolate, ExitSlow);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "exit"),
                     exitFnTmpl);
}

} // namespace done::process
