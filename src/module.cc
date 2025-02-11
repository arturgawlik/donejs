#include "v8-local-handle.h"
#include "v8-value.h"
#include "v8.h"

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
using v8::ScriptOrModule;
using v8::String;
using v8::Value;

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
  String::Utf8Value str(context->GetIsolate(), specifier);

  // Return unchecked module
  return load_module(*str, read_file(*str));
}

MaybeLocal<Module> instantiate_module(Local<Context> context,
                                      const char *jsFilePath) {
  std::string jsFileContent = read_file(jsFilePath);
  if (!jsFileContent.length()) {
    return MaybeLocal<Module>();
  }

  Local<Module> module = load_module(jsFilePath, jsFileContent);
  module->InstantiateModule(context, call_resolve).ToChecked();

  return module;
}

int run_js(Local<Context> context, const char *jsFilePath) {
  Local<Module> module =
      instantiate_module(context, jsFilePath).ToLocalChecked();
  module->Evaluate(context).ToLocalChecked();

  return 0;
}

MaybeLocal<Promise> dynamic_import(Local<Context> context,
                                   Local<Data> host_defined_options,
                                   Local<Value> resource_name,
                                   Local<String> specifier,
                                   Local<FixedArray> import_attributes) {
  Local<Promise::Resolver> resolver =
      Promise::Resolver::New(context).ToLocalChecked();
  MaybeLocal<Promise> promise(resolver->GetPromise());

  String::Utf8Value name(context->GetIsolate(), specifier);
  Local<Module> module = instantiate_module(context, *name).ToLocalChecked();
  Local<Value> moduleAsValue = module.As<Value>();

  resolver->Resolve(context, moduleAsValue).ToChecked();
  return promise;
}

void InitDynamicImports() {
  Isolate *isolate = Isolate::GetCurrent();

  isolate->SetHostImportModuleDynamicallyCallback(dynamic_import);
}

int Run(const char *jsFilePath) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  return run_js(context, jsFilePath);
}
} // namespace done::module