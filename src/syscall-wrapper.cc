#include <sys/socket.h>
#include <v8.h>

using v8::Boolean;
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

namespace done::syscall {

void SocketSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> domainLocal = args[0].As<Value>();
  Local<Value> typeLocal = args[1].As<Value>();
  Local<Value> protocolLocal = args[2].As<Value>();

  if (!domainLocal->IsNumber() || !typeLocal->IsNumber() ||
      !protocolLocal->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'domain', 'type', 'protocol' must be an number when creating "
                 "socket")));
    return;
  }

  int domain = domainLocal->Int32Value(context).ToChecked();
  int type = typeLocal->Int32Value(context).ToChecked();
  int protocol = protocolLocal->Int32Value(context).ToChecked();

  int fd = socket(domain, type, protocol);
  if (fd == -1) {
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8Literal(isolate, "socket creation failed")));
    return;
  }

  args.GetReturnValue().Set(fd);
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> consoleObjTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "syscall"),
                     consoleObjTmpl);

  Local<FunctionTemplate> logFnTmpl =
      FunctionTemplate::New(isolate, SocketSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "socket"),
                      logFnTmpl);
}

} // namespace done::syscall
