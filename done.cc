#include "done.h"
#include "console.h"
#include "fetch.h"
#include "module.h"
#include "process.h"
#include "syscall-wrapper.h"
#include "text-decoder.h"

#include "v8-initialization.h"
#include "v8-object.h"
#include "v8.h"

using v8::Context;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::V8;

Local<ObjectTemplate> initializeDoneBuildins() {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> globalObjTmpl = ObjectTemplate::New(isolate);
  Local<ObjectTemplate> doneGlobalObjTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(String::NewFromUtf8Literal(isolate, "done"),
                     doneGlobalObjTmpl);

  done::console::Initialize(doneGlobalObjTmpl);
  done::fetch::Initialize(doneGlobalObjTmpl);
  done::syscall::Initialize(doneGlobalObjTmpl);
  done::textDecoder::Initialize(doneGlobalObjTmpl);

  return globalObjTmpl;
}

void initializeDoneBuildinsAfterContextCreation(Local<Object> globalObj,
                                                int argc, char **argv) {

  done::process::Initialize(globalObj, argc, argv);
}

int done::Run(int argc, char **argv) {
  // Initialize V8.
  V8::SetFlagsFromCommandLine(&argc, argv, true);
  V8::InitializeICUDefaultLocation(argv[0]);
  V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  V8::InitializePlatform(platform.get());
  V8::Initialize();

  // Create a new Isolate and make it the current one.
  Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  Isolate *isolate = v8::Isolate::New(create_params);
  {
    Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);

    // Create global object
    Local<ObjectTemplate> globalObjTmpl = initializeDoneBuildins();

    // Create a new context.
    Local<Context> context = Context::New(isolate, nullptr, globalObjTmpl);

    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);
    {
      Local<Object> doneObj =
          context->Global()
              ->Get(context, String::NewFromUtf8Literal(isolate, "done"))
              .ToLocalChecked()
              .As<Object>();
      initializeDoneBuildinsAfterContextCreation(doneObj, argc, argv);

      done::module::InitDynamicImports();

      const char *doneJsFilePath = "./done.js";
      int res = done::module::Run(doneJsFilePath);
      if (res)
        return res;

      const char *indexJsFilePath = argv[1];
      res = done::module::Run(indexJsFilePath);
      if (res)
        return res;
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;

  return 0;
}
