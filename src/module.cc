#include "v8-exception.h"
#include "v8-isolate.h"
#include "v8-primitive.h"
#include "v8-script.h"
#include "v8.h"

#include <cstdio>
#include <fstream>

using std::string;

using v8::Context;
using v8::Data;
using v8::FixedArray;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Module;
using v8::Promise;
using v8::ScriptCompiler;
using v8::ScriptOrigin;
using v8::String;
using v8::Value;

#define ANSI_COLOR_RED "\x1b[31m"
// #define ANSI_COLOR_GREEN   "\x1b[32m"
// #define ANSI_COLOR_YELLOW  "\x1b[33m"
// #define ANSI_COLOR_BLUE    "\x1b[34m"
// #define ANSI_COLOR_MAGENTA "\x1b[35m"
// #define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

namespace done::module {

std::string read_file(string path) {
  std::string fileContent;
  std::string tmp;

  std::ifstream file(path);
  while (getline(file, tmp)) {
    fileContent.append("\n" + tmp);
  }
  file.close();

  return fileContent;
}

MaybeLocal<Module> call_resolve(Local<Context> context, Local<String> specifier,
                                Local<FixedArray> import_attributes,
                                Local<Module> referrer);

Local<Module> load_module(string jsFileName, string jsFileContent) {
  Isolate *isolate = Isolate::GetCurrent();

  ScriptOrigin origin(
      String::NewFromUtf8(isolate, jsFileName.c_str()).ToLocalChecked(), 0, 0,
      false, -1, Local<Value>(), false, false, true, Local<Data>());
  ScriptCompiler::Source *source = new ScriptCompiler::Source(
      String::NewFromUtf8(isolate, jsFileContent.c_str()).ToLocalChecked(),
      origin);

  // Compile the source code.
  Local<Module> compiledModule =
      ScriptCompiler::CompileModule(isolate, source).ToLocalChecked();

  return compiledModule;
}

MaybeLocal<Module> call_resolve(Local<Context> context, Local<String> specifier,
                                Local<FixedArray> import_attributes,
                                Local<Module> referrer) {

  // Get module name from specifier (given name in import args)
  String::Utf8Value utf8(context->GetIsolate(), specifier);
  string str = *utf8;

  // build in modules should be imported with "done:" prefix
  if (str.contains("done:")) {
    str.replace(0, sizeof("done:") - 1, "./lib/");
  }

  // Return unchecked module
  return load_module(str, read_file(str));
}

MaybeLocal<Module> instantiate_module(Local<Context> context,
                                      const char *jsFilePath) {
  Isolate *isolate = Isolate::GetCurrent();

  std::string jsFileContent = read_file(jsFilePath);
  if (!jsFileContent.length()) {
    return MaybeLocal<Module>();
  }

  Local<Module> module = load_module(jsFilePath, jsFileContent);
  if (!module->InstantiateModule(context, call_resolve).IsJust()) {
    Module::Status moduleStatus = module->GetStatus();
    if (moduleStatus == Module::Status::kErrored) {
      Local<Value> exception = module->GetException();
      v8::String::Utf8Value exceptionStr(isolate, exception);
      printf(ANSI_COLOR_RED
             "Module instantiation failed: %s\n" ANSI_COLOR_RESET,
             *exceptionStr);
    }
  }

  return module;
}

int run_js(Local<Context> context, const char *jsFilePath) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Module> module =
      instantiate_module(context, jsFilePath).ToLocalChecked();

  module->Evaluate(context).ToLocalChecked();

  Module::Status status = module->GetStatus();
  if (status == Module::Status::kErrored) {
    Local<Value> exception = module->GetException();
    v8::String::Utf8Value exceptionStr(isolate, exception);
    printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, *exceptionStr);
  }

  return 0;
}

MaybeLocal<Promise> dynamic_import_cb(Local<Context> context,
                                      Local<Data> host_defined_options,
                                      Local<Value> resource_name,
                                      Local<String> specifier,
                                      Local<FixedArray> import_attributes) {

  Isolate *isolate = Isolate::GetCurrent();
  Local<Promise::Resolver> resolver =
      Promise::Resolver::New(context).ToLocalChecked();
  String::Utf8Value name(isolate, specifier);
  Local<Module> module = instantiate_module(context, *name).ToLocalChecked();
  module->Evaluate(context).ToLocalChecked();
  Local<Value> moduleNamespace = module->GetModuleNamespace();
  resolver->Resolve(context, moduleNamespace).ToChecked();

  return resolver->GetPromise();
}

void InitDynamicImports() {
  Isolate *isolate = Isolate::GetCurrent();

  isolate->SetHostImportModuleDynamicallyCallback(dynamic_import_cb);
}

int Run(const char *jsFilePath) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<Context> context = isolate->GetCurrentContext();

  return run_js(context, jsFilePath);
}
} // namespace done::module