#include "done.h"
#include "v8-external.h"
#include "v8-fast-api-calls.h"
#include "v8-function-callback.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-template.h"
#include "v8-value.h"

#include <cmath>
#include <cstdio>
#include <string>

std::string read_file(std::string path) {
  std::string fileContent;
  std::string tmp;

  std::ifstream file(path);
  while (getline(file, tmp)) {
    fileContent.append(tmp);
  }
  file.close();

  return fileContent;
}

int run_js(v8::Local<v8::Context> context, const char *jsFilePath) {
  v8::Isolate *isolate = v8::Isolate::GetCurrent();

  std::string indexJsFileContent = read_file(jsFilePath);
  if (!indexJsFileContent.length()) {
    return 1;
  }

  // Create a string containing the JavaScript source code.
  v8::Local<v8::String> source =
      v8::String::NewFromUtf8(isolate, indexJsFileContent.c_str())
          .ToLocalChecked();

  // Compile the source code.
  v8::Local<v8::Script> script =
      v8::Script::Compile(context, source).ToLocalChecked();

  // Run the script to get the result.
  v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

  // Convert the result to an UTF8 string and print it.
  v8::String::Utf8Value utf8(isolate, result);
  printf("%s\n", *utf8);

  return 0;
}

int done::Run(int argc, char **argv) {
  // Initialize V8.
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::Initialize();

  // Create a new Isolate and make it the current one.
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate *isolate = v8::Isolate::New(create_params);
  {
    v8::Isolate::Scope isolate_scope(isolate);

    // Create a stack-allocated handle scope.
    v8::HandleScope handle_scope(isolate);

    // Create a new context.
    v8::Local<v8::Context> context = v8::Context::New(isolate);

    // Enter the context for compiling and running the hello world script.
    v8::Context::Scope context_scope(context);
    {
      const char *doneJsFilePath = "./done.js";
      run_js(context, doneJsFilePath);

      char *indexJsFilePath = argv[1];
      run_js(context, indexJsFilePath);
    }
  }

  // Dispose the isolate and tear down V8.
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;

  return 0;
}
