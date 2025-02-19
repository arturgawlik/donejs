#include "v8-container.h"
#include "v8-isolate.h"
#include "v8-primitive.h"
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <v8.h>

using v8::Array;
using v8::Boolean;
using v8::Context;
using v8::Exception;
using v8::External;
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
        isolate, "'domain', 'type', 'protocol' must be an number when calling "
                 "'socket' syscall")));
    return;
  }

  int domain = domainLocal->Int32Value(context).ToChecked();
  int type = typeLocal->Int32Value(context).ToChecked();
  int protocol = protocolLocal->Int32Value(context).ToChecked();

  int fd = socket(domain, type, protocol);
  if (fd == -1) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "socket creation by calling `socket` syscall failed")));
    return;
  }

  args.GetReturnValue().Set(fd);
}

void hostentAccessor(v8::Local<v8::Name> property,
                     const v8::PropertyCallbackInfo<v8::Value> &info) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Object> self = info.Holder();
  void *ptr = Local<External>::Cast(self->GetInternalField(0))->Value();
  hostent *hostentPtr = static_cast<hostent *>(ptr);

  bool isHAddr =
      property->Equals(context, String::NewFromUtf8Literal(isolate, "h_addr"))
          .FromJust();
  if (isHAddr) {
    char *addr = hostentPtr->h_addr_list[0];
    std::string cppStr(addr);
    Local<String> addrLocal =
        String::NewFromUtf8(isolate, cppStr.c_str()).ToLocalChecked();
    info.GetReturnValue().Set(addrLocal);
  }

  bool isHLength =
      property->Equals(context, String::NewFromUtf8Literal(isolate, "h_length"))
          .FromJust();
  if (isHLength) {
    int length = hostentPtr->h_length;
    info.GetReturnValue().Set(length);
  }
}

void GetHostByNameSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> name = args[0].As<Value>();

  if (!name->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'name' must be an string when calling 'gethostbyname' syscall")));
    return;
  }

  Local<String> domainLocal = name->ToString(context).ToLocalChecked();
  String::Utf8Value domain(context->GetIsolate(), domainLocal);

  hostent *hostent = gethostbyname(*domain);
  if (hostent == nullptr) {
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8Literal(isolate, "gethostbyname syscall failed")));
    return;
  }

  //   Local<Object> hostentLocal = Object::New(isolate);
  Local<ObjectTemplate> hostentTmpl = ObjectTemplate::New(isolate);
  hostentTmpl->SetInternalFieldCount(1);
  hostentTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "h_addr"), hostentAccessor);
  hostentTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "h_length"), hostentAccessor);
  Local<Object> hostentLocal =
      hostentTmpl->NewInstance(context).ToLocalChecked();
  hostentLocal->SetInternalField(0, External::New(isolate, hostent));

  args.GetReturnValue().Set(hostentLocal);
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> consoleObjTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "syscall"),
                     consoleObjTmpl);

  Local<FunctionTemplate> socketFnTmpl =
      FunctionTemplate::New(isolate, SocketSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "socket"),
                      socketFnTmpl);

  Local<FunctionTemplate> gethostbynameFnTmpl =
      FunctionTemplate::New(isolate, GetHostByNameSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "gethostbyname"),
                      gethostbynameFnTmpl);
}

} // namespace done::syscall
