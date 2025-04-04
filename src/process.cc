#include "v8-container.h"
#include "v8-object.h"
#include "v8-primitive.h"
#include "v8-template.h"
#include <v8.h>

using v8::Array;
using v8::Context;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
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
    int exitCode = v8Num->Int32Value(context).FromJust();
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

Local<Array> GetArgv(int argc, char **argv) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Array> argvJsArray = Array::New(isolate);
  for (int i = 0; i < argc; i++) {
    char *argvItem = argv[i];
    Local<String> argvJs =
        String::NewFromUtf8(isolate, argvItem).ToLocalChecked();
    argvJsArray->Set(context, i, argvJs).ToChecked();
  }

  return argvJsArray;
}

void Initialize(Local<Object> globalObj, int argc, char **argv) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Object> processObj = Object::New(isolate);
  globalObj
      ->Set(context, String::NewFromUtf8Literal(isolate, "process"), processObj)
      .ToChecked();

  Local<v8::Function> exitFn =
      v8::Function::New(context, ExitSlow).ToLocalChecked();
  processObj
      ->Set(context, v8::String::NewFromUtf8Literal(isolate, "exit"), exitFn)
      .ToChecked();

  Local<Array> argvJsArray = GetArgv(argc, argv);
  processObj
      ->Set(context, v8::String::NewFromUtf8Literal(isolate, "argv"),
            argvJsArray)
      .ToChecked();
}
} // namespace done::process
