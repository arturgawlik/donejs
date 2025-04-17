#include "v8-array-buffer.h"
#include "v8-container.h"
#include "v8-exception.h"
#include "v8-function-callback.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-object.h"
#include "v8-primitive.h"
#include "v8-typed-array.h"
#include "v8-value.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <glob.h>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <v8.h>

#include <netdb.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

using v8::Array;
using v8::ArrayBuffer;
using v8::Boolean;
using v8::Context;
using v8::Exception;
using v8::External;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Int32;
using v8::Int32Array;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

namespace done::syscall {

void WaitPidSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'pid' must be an number when calling "
                 "'waitpid' syscall.")));
  }
  if (!args[1]->IsInt32Array() || args[1].As<v8::Int32Array>()->Length() != 1) {
    // same as in `PosixSpawnSlow`
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'status' must be an IsInt32Array buffer when calling "
                 "'waitpid' syscall.")));
  }
  if (!args[2]->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'options' must be an number when calling "
                 "'waitpid' syscall.")));
  }

  Local<Number> pid_js = args[0].As<Number>();
  Local<Int32Array> status_js = args[1].As<Int32Array>();
  Local<Number> options_js = args[2].As<Number>();
  int pid = pid_js->Value();
  int options = options_js->Value();
  int statusPtr;

  int res = waitpid(pid, &statusPtr, options);

  status_js->Set(context, 0, Int32::New(isolate, statusPtr)).ToChecked();
  args.GetReturnValue().Set(res);
}

void PosixSpawnSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (!args[0]->IsInt32Array() || args[0].As<v8::Int32Array>()->Length() != 1) {
    // `posix_spawn` expects as first argument pointer to int that will be set
    // with pid of newly created process so here we are expecting ArrayBuffer
    // where child process pid will be set.
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'pid' must be an IsInt32Array buffer when calling "
                 "'posix_spawn' syscall.")));
  }
  if (!args[1]->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'path' must be an string when calling 'posix_spawn' syscall.")));
  }
  if (!args[2]->IsNull()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'file_actions' must be an null when calling 'posix_spawn' "
                 "syscall. Other "
                 "values are not supported for now.")));
  }
  if (!args[3]->IsNull()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'attrp' must be an null when calling 'posix_spawn' "
                 "syscall. Other "
                 "values are not supported for now.")));
  }
  if (!args[4]->IsArray()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'argv' must be an string when calling 'posix_spawn' syscall.")));
  }
  Local<Array> argv_js = args[4].As<Array>();
  for (u_int32_t i = 0; i < argv_js->Length(); i++) {
    if (!argv_js->Get(context, i).ToLocalChecked()->IsString()) {
      isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
          isolate,
          "'argv' all elements must be an string when calling 'posix_spawn' "
          "syscall.")));
    }
  }

  if (!args[5]->IsNull()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'envp' must be an null when calling 'posix_spawn' "
                 "syscall. Other "
                 "values are not supported for now.")));
  }

  Local<Int32Array> pid_js = args[0].As<Int32Array>();
  Local<String> path_js = args[1].As<String>();

  String::Utf8Value path_js_utf8(isolate, path_js);
  char *path = *path_js_utf8;

  char **child_argv = (char **)calloc(argv_js->Length(), sizeof(char *));
  for (u_int32_t i = 0; i < argv_js->Length(); i++) {
    Local<String> argv_js_current =
        argv_js->Get(context, i).ToLocalChecked().As<String>();
    String::Utf8Value argv_js_current_utf8(isolate, argv_js_current);
    // needs to allocate string with `strdup` because otherwise it is removed
    // from stack after this loop leave
    char *argv_current = strdup(*argv_js_current_utf8);
    child_argv[i] = argv_current;
  }

  char **environ = nullptr;
  pid_t child_pid = 0;

  int res = posix_spawn(&child_pid, path, NULL, NULL, child_argv, environ);

  // free allocated memory
  for (uint32_t i = 0; i < argv_js->Length(); i++) {
    char *strToDeallocate = child_argv[i];
    free(strToDeallocate);
  }
  free(child_argv);

  pid_js->Set(context, 0, Int32::New(isolate, child_pid)).ToChecked();
  args.GetReturnValue().Set(res);
}

void Get_glob_t(v8::Local<v8::Name> property,
                const v8::PropertyCallbackInfo<v8::Value> &info) {
  Isolate *isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Object> self = info.Holder();
  void *ptr = Local<External>::Cast(self->GetInternalField(0))->Value();

  glob_t *globbuf = static_cast<glob_t *>(ptr);

  if (property->Equals(context, String::NewFromUtf8Literal(isolate, "gl_pathc"))
          .FromJust()) {
    info.GetReturnValue().Set(globbuf->gl_pathc);
  }

  if (property->Equals(context, String::NewFromUtf8Literal(isolate, "gl_pathv"))
          .FromJust()) {
    Local<Array> gl_pathv_array_js = Array::New(isolate, globbuf->gl_pathc);
    for (ulong i = 0; i < globbuf->gl_pathc; i++) {
      Local<String> gl_pathv_js =
          String::NewFromUtf8(isolate, globbuf->gl_pathv[i]).ToLocalChecked();
      gl_pathv_array_js->Set(context, i, gl_pathv_js).ToChecked();
    }
    info.GetReturnValue().Set(gl_pathv_array_js);
  }
}

void GlobSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'pattern' must be an string when calling 'glob' syscall.")));
  }
  if (!args[1]->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'flags' must be an number when calling 'glob' syscall.")));
  }
  if (!args[2]->IsNull()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'errfunc' must be an null when calling 'glob' syscall. Other "
                 "values are not supported for now.")));
  }
  if (!args[3]->IsObject()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'pglob' must be an object when calling 'glob' syscall.")));
  }

  Local<String> pattern_js = args[0].As<String>();
  Local<Number> flags_js = args[1].As<Number>();
  Local<Object> pglob_js = args[3].As<Object>();

  String::Utf8Value pattern_js_utf8(isolate, pattern_js);
  char *pattern = *pattern_js_utf8;
  int flags = flags_js->ToInt32(context).ToLocalChecked()->Value();

  // TODO: delete it to remove memory leak
  glob_t *globbuf = new glob_t();

  glob(pattern, flags, NULL, globbuf);
  Local<ObjectTemplate> globbufPrototypeTmpl = ObjectTemplate::New(isolate);
  globbufPrototypeTmpl->SetInternalFieldCount(1);
  globbufPrototypeTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "gl_pathc"), Get_glob_t);
  globbufPrototypeTmpl->SetNativeDataProperty(
      String::NewFromUtf8Literal(isolate, "gl_pathv"), Get_glob_t);

  Local<Object> globbufPrototype_js =
      globbufPrototypeTmpl->NewInstance(context).ToLocalChecked();
  globbufPrototype_js->SetInternalField(0, External::New(isolate, globbuf));

  pglob_js->SetPrototype(context, globbufPrototype_js).ToChecked();
}

void CloseSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'socket_fd' must be an number when calling 'close' syscall.")));
  }

  int socketFd = args[0]->Int32Value(context).FromJust();
  int closeResCode = close(socketFd);
  if (closeResCode == -1) {
    const char *errmsg = gai_strerror(closeResCode);
    std::string msg = std::string("connect syscall failed with msg: ") + errmsg;

    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str()).ToLocalChecked()));
  }

  args.GetReturnValue().Set(closeResCode);
}

void SendSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> socketfdLocal = args[0].As<Value>();
  Local<ArrayBuffer> bufferLocalVal = args[1].As<ArrayBuffer>();
  Local<Value> bufferSizeLocal = args[2].As<Value>();
  Local<Value> flagsLocal = args[3].As<Value>();

  if (!socketfdLocal->IsNumber() || !bufferSizeLocal->IsNumber() ||
      !flagsLocal->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'socket_fd', 'buffer_size', 'flags' must be an number when calling "
        "'send' syscall")));
    return;
  }
  if (!bufferLocalVal->IsArrayBuffer()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'buffer' must be an ArrayBuffer when calling "
                 "'send' syscall")));
    return;
  }

  int socketfd = socketfdLocal->Int32Value(context).ToChecked();
  int bufferSizeLen = bufferSizeLocal->Int32Value(context).ToChecked();
  int flags = flagsLocal->Int32Value(context).ToChecked();
  void *bufferPtr = bufferLocalVal->Data();
  char *buffer = static_cast<char *>(bufferPtr);

  int bytes_send = send(socketfd, buffer, bufferSizeLen, flags);
  if (bytes_send == -1) {
    const char *errmsg = gai_strerror(bytes_send);
    std::string msg = std::string("send syscall failed with msg: ") + errmsg;

    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str()).ToLocalChecked()));
    return;
  }

  args.GetReturnValue().Set(bytes_send);
}

void RecvSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();

  Local<Value> socketfdLocal = args[0].As<Value>();
  Local<ArrayBuffer> bufferLocalVal = args[1].As<ArrayBuffer>();
  Local<Value> bufferSizeLocal = args[2].As<Value>();
  Local<Value> flagsLocal = args[3].As<Value>();

  if (!socketfdLocal->IsNumber() || !bufferSizeLocal->IsNumber() ||
      !flagsLocal->IsNumber()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'socket_fd', 'buffer_size', 'flags' must be an number when calling "
        "'recv' syscall")));
    return;
  }
  if (!bufferLocalVal->IsArrayBuffer()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'buffer' must be an ArrayBuffer when calling "
                 "'recv' syscall")));
    return;
  }

  int socketfd = socketfdLocal->Int32Value(context).ToChecked();
  int bufferSizeLen = bufferSizeLocal->Int32Value(context).ToChecked();
  int flags = flagsLocal->Int32Value(context).ToChecked();
  void *bufferPtr = bufferLocalVal->Data();
  char *buffer = static_cast<char *>(bufferPtr);

  int recv_result = recv(socketfd, buffer, bufferSizeLen, flags);
  if (recv_result == -1) {
    const char *errmsg = gai_strerror(recv_result);
    std::string msg = std::string("connect syscall failed with msg: ") + errmsg;

    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str()).ToLocalChecked()));
    return;
  }

  args.GetReturnValue().Set(recv_result);
}

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
  Local<Value> resultsArgs = args[3].As<Value>();

  if (!hostArgs->IsString() && !hostArgs->IsNull()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'host' must be an string or null when calling 'getaddrinfo' "
                 "syscall")));
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
  if (!resultsArgs->IsObject()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'results' must be an object when calling 'getaddrinfo' syscall")));
    return;
  }

  Local<String> hostLocal = hostArgs->ToString(context).ToLocalChecked();
  Local<String> portLocal = portArgs->ToString(context).ToLocalChecked();
  Local<Object> hintsLocal = hintsArgs->ToObject(context).ToLocalChecked();
  Local<Object> resultsLocal = resultsArgs->ToObject(context).ToLocalChecked();

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
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *result;
  int errCode;
  if (hostArgs->IsString()) {
    errCode = getaddrinfo(*hostUtf8, *portUtf8, &hints, &result);
  } else {
    errCode = getaddrinfo(NULL, *portUtf8, &hints, &result);
  }
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

  resultsLocal->SetPrototype(context, hostentLocal).ToChecked();
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> syscallObjTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "syscall"),
                     syscallObjTmpl);

  // NETWORK
  Local<FunctionTemplate> gethostbynameFnTmpl =
      FunctionTemplate::New(isolate, GetAddrInfoSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "getaddrinfo"),
                      gethostbynameFnTmpl);

  Local<FunctionTemplate> socketFnTmpl =
      FunctionTemplate::New(isolate, SocketSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "socket"),
                      socketFnTmpl);

  Local<FunctionTemplate> connectFnTmpl =
      FunctionTemplate::New(isolate, ConnectSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "connect"),
                      connectFnTmpl);

  Local<FunctionTemplate> recvFnTmpl = FunctionTemplate::New(isolate, RecvSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "recv"),
                      recvFnTmpl);

  Local<FunctionTemplate> sendFnTmpl = FunctionTemplate::New(isolate, SendSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "send"),
                      sendFnTmpl);

  Local<FunctionTemplate> closeFnTmpl =
      FunctionTemplate::New(isolate, CloseSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "close"),
                      closeFnTmpl);

  // FILE SYSTEM
  Local<FunctionTemplate> globFnTmpl = FunctionTemplate::New(isolate, GlobSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "glob"),
                      globFnTmpl);

  // PROCESS
  Local<FunctionTemplate> posix_spawnFnTmpl =
      FunctionTemplate::New(isolate, PosixSpawnSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "posix_spawn"),
                      posix_spawnFnTmpl);
  Local<FunctionTemplate> waitPidFnTmpl =
      FunctionTemplate::New(isolate, WaitPidSlow);
  syscallObjTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "waitpid"),
                      waitPidFnTmpl);
}

} // namespace done::syscall
