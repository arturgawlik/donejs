#include "v8-container.h"
#include "v8-isolate.h"
#include "v8-object.h"
#include "v8-primitive.h"
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <v8.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

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

void ConnectSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> socketfdLocal = args[0].As<Value>();
  Local<Value> aiAddrLocalVal = args[1].As<Value>();
  Local<Value> aiAddrLenLocal = args[2].As<Value>();

  if (!socketfdLocal->IsNumber() || !aiAddrLenLocal->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'socket_fd', 'ai_addrlen' must be an number when calling "
                 "'connect' syscall")));
    return;
  }
  if (!aiAddrLocalVal->IsObject()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'ai_addr' must be an object when calling "
                 "'connect' syscall")));
    return;
  }

  int socketfd = socketfdLocal->Int32Value(context).ToChecked();
  int aiAddrLen = aiAddrLenLocal->Int32Value(context).ToChecked();
  Local<Object> aiAddrLocal =
      aiAddrLocalVal->ToObject(context).ToLocalChecked();
  void *ptr = Local<External>::Cast(aiAddrLocal->GetInternalField(0))->Value();
  struct sockaddr *sockaddr = static_cast<struct sockaddr *>(ptr);

  int connection_result = connect(socketfd, sockaddr, aiAddrLen);
  if (connection_result == -1) {
    const char *errmsg = gai_strerror(connection_result);
    std::string msg = std::string("connect syscall failed with msg: ") + errmsg;

    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str()).ToLocalChecked()));
    return;
  }

  args.GetReturnValue().Set(connection_result);
}

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
    const char *errmsg = gai_strerror(fd);
    std::string msg =
        std::string(
            "socket creation by calling `socket` syscall failed with msg: ") +
        errmsg;
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str()).ToLocalChecked()));
    return;
  }

  args.GetReturnValue().Set(fd);
}

void getaddrinfoResultAccessor(
    v8::Local<v8::Name> property,
    const v8::PropertyCallbackInfo<v8::Value> &info) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Object> self = info.Holder();
  void *ptr = Local<External>::Cast(self->GetInternalField(0))->Value();

  struct addrinfo *addrinfo = static_cast<struct addrinfo *>(ptr);

  if (property
          ->Equals(context, String::NewFromUtf8Literal(isolate, "ai_family"))
          .FromJust()) {
    info.GetReturnValue().Set(addrinfo->ai_family);
  }

  if (property
          ->Equals(context, String::NewFromUtf8Literal(isolate, "ai_socktype"))
          .FromJust()) {
    info.GetReturnValue().Set(addrinfo->ai_socktype);
  }

  if (property
          ->Equals(context, String::NewFromUtf8Literal(isolate, "ai_protocol"))
          .FromJust()) {
    info.GetReturnValue().Set(addrinfo->ai_protocol);
  }

  if (property
          ->Equals(context, String::NewFromUtf8Literal(isolate, "ai_addrlen"))
          .FromJust()) {
    info.GetReturnValue().Set(addrinfo->ai_addrlen);
  }

  if (property->Equals(context, String::NewFromUtf8Literal(isolate, "ai_addr"))
          .FromJust()) {
    // idea here is to when this object will be used in another syscall
    // then `ai_addr` should be picked from internal field
    //
    // sidenote: maybe other data reused in syscall should be handled also this
    // way? thanks to that there is no need to switch between js<=>native types
    Local<ObjectTemplate> ai_addrTmpl = ObjectTemplate::New(isolate);
    ai_addrTmpl->SetInternalFieldCount(1);
    Local<Object> ai_addrLocal =
        ai_addrTmpl->NewInstance(context).ToLocalChecked();
    ai_addrLocal->SetInternalField(0,
                                   External::New(isolate, addrinfo->ai_addr));
    info.GetReturnValue().Set(ai_addrLocal);
  }
}

void GetAddrInfoSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> hostArgs = args[0].As<Value>();
  Local<Value> portArgs = args[1].As<Value>();
  Local<Value> hintsArgs = args[2].As<Value>();
  // Local<Value> resultsArgs = args[3].As<Value>();

  if (!hostArgs->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'host' must be an string when calling 'getaddrinfo' syscall")));
    return;
  }
  if (!portArgs->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'port' must be an string when calling 'getaddrinfo' syscall")));
    return;
  }
  if (!hintsArgs->IsObject()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'hints' must be an object when calling 'getaddrinfo' syscall")));
    return;
  }
  // TODO: for now I don't know how to implement setting Internal Filed
  // in received object from js land
  // if (!resultsArgs->IsObject()) {
  //   isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
  //       isolate,
  //       "'results' must be an object when calling 'getaddrinfo' syscall")));
  //   return;
  // }

  Local<String> hostLocal = hostArgs->ToString(context).ToLocalChecked();
  Local<String> portLocal = portArgs->ToString(context).ToLocalChecked();
  Local<Object> hintsLocal = hintsArgs->ToObject(context).ToLocalChecked();
  // Local<Object> resultsLocal =
  // resultsArgs->ToObject(context).ToLocalChecked();

  String::Utf8Value hostUtf8(context->GetIsolate(), hostLocal);
  String::Utf8Value portUtf8(context->GetIsolate(), portLocal);
  Local<Value> hintsAiFamily =
      hintsLocal->Get(context, String::NewFromUtf8Literal(isolate, "ai_family"))
          .ToLocalChecked();
  Local<Value> hintsAiSockType =
      hintsLocal
          ->Get(context, String::NewFromUtf8Literal(isolate, "ai_socktype"))
          .ToLocalChecked();
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);
  if (hintsAiFamily->StrictEquals(
          String::NewFromUtf8Literal(isolate, "AF_UNSPEC"))) {
    hints.ai_family = AF_UNSPEC;
  }
  if (hintsAiSockType->StrictEquals(
          String::NewFromUtf8Literal(isolate, "SOCK_STREAM"))) {
    hints.ai_socktype = SOCK_STREAM;
  }

  struct addrinfo *result;

  int errCode = getaddrinfo(*hostUtf8, *portUtf8, &hints, &result);
  if (errCode != 0) {
    const char *errmsg = gai_strerror(errCode);
    std::string msg =
        std::string("getaddrinfo syscall failed with msg: ") + errmsg;

    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str()).ToLocalChecked()));
    return;
  }

  Local<ObjectTemplate> resultTmpl = ObjectTemplate::New(isolate);
  resultTmpl->SetInternalFieldCount(1);
  // resultTmpl->SetNativeDataProperty(
  //     String::NewFromUtf8Literal(isolate, "ai_flags"),
  //     getaddrinfoResultAccessor);
  resultTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "ai_family"),
      getaddrinfoResultAccessor);
  resultTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "ai_socktype"),
      getaddrinfoResultAccessor);
  resultTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "ai_protocol"),
      getaddrinfoResultAccessor);

  // for client only those are needed
  resultTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "ai_addrlen"),
      getaddrinfoResultAccessor);
  resultTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "ai_addr"),
      getaddrinfoResultAccessor);

  // resultTmpl->SetNativeDataProperty(
  //     String::NewFromUtf8Literal(isolate, "ai_canonname"),
  //     getaddrinfoResultAccessor);
  // resultTmpl->SetNativeDataProperty(
  //     String::NewFromUtf8Literal(isolate, "ai_next"),
  //     getaddrinfoResultAccessor);

  Local<Object> hostentLocal =
      resultTmpl->NewInstance(context).ToLocalChecked();
  hostentLocal->SetInternalField(0, External::New(isolate, result));

  // TODO: figure out how to set this on "resultsLocal"
  args.GetReturnValue().Set(hostentLocal);
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> consoleObjTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "syscall"),
                     consoleObjTmpl);

  Local<FunctionTemplate> gethostbynameFnTmpl =
      FunctionTemplate::New(isolate, GetAddrInfoSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "getaddrinfo"),
                      gethostbynameFnTmpl);

  Local<FunctionTemplate> socketFnTmpl =
      FunctionTemplate::New(isolate, SocketSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "socket"),
                      socketFnTmpl);

  Local<FunctionTemplate> connectFnTmpl =
      FunctionTemplate::New(isolate, ConnectSlow);
  consoleObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "connect"),
                      connectFnTmpl);
}

} // namespace done::syscall
