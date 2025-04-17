#include "v8-array-buffer.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-template.h"
#include <memory>
#include <v8.h>

using v8::ArrayBuffer;
using v8::BackingStore;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

namespace done::textEncoder {

// based on nodejs implementation in
// https://github.com/arturgawlik/node/blob/4a4aa58fa4a3736744f605abc70eb6c167aadf9d/src/encoding_binding.cc#L118
void EncodeSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'input' must be an String when calling 'encode(string)' "
                 "function")));
  }

  Local<String> stringToEncode = args[0].As<String>();
  size_t length = stringToEncode->Utf8Length(isolate);

  std::unique_ptr<BackingStore> bs = ArrayBuffer::NewBackingStore(
      isolate, length, v8::BackingStoreInitializationMode::kUninitialized);
  stringToEncode->WriteUtf8(
      isolate, static_cast<char *>(bs->Data()),
      -1, // We are certain that `data` is sufficiently large
      nullptr, String::NO_NULL_TERMINATION | String::REPLACE_INVALID_UTF8);
  Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(bs));

  args.GetReturnValue().Set(buffer);
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> textEncoderTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(String::NewFromUtf8Literal(isolate, "textEncoder"),
                     textEncoderTmpl);

  Local<FunctionTemplate> encodeSlowFnTmpl =
      FunctionTemplate::New(isolate, EncodeSlow);
  textEncoderTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "encode"),
                       encodeSlowFnTmpl);
}

} // namespace done::textEncoder
