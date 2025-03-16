#include "v8-array-buffer.h"
#include "v8-primitive.h"
#include "v8-template.h"
#include <v8.h>

using v8::ArrayBuffer;
using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::ObjectTemplate;
using v8::String;
using v8::Value;

namespace done::textDecoder {

// implemented based on
// https://github.com/cjihrig/node/blob/f69726f70d34af9550c922afa39ab0f933272b64/src/encoding_binding.cc#L145
void EncodeSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();

  if (!args[0]->IsArrayBuffer()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'input' must be an ArrayBuffer when calling 'encode(input, encoding)' "
        "function")));
  }

  if (!args[1]->IsString()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate,
        "'encoding' must be an string when calling 'encode(input, encoding)' "
        "function")));
  }
  Local<String> encoding = args[1].As<String>();

  if (encoding->StringEquals(String::NewFromUtf8Literal(isolate, "utf-8")) ||
      encoding->StringEquals(String::NewFromUtf8Literal(isolate, "utf8"))) {
    Local<ArrayBuffer> buffer = args[0].As<ArrayBuffer>();
    int bufferLen = buffer->ByteLength();

    const char *data = (char *)buffer->Data();
    Local<String> encodedVal =
        String::NewFromUtf8(isolate, data, v8::NewStringType::kNormal,
                            bufferLen)
            .ToLocalChecked();

    args.GetReturnValue().Set(encodedVal);
  }
}

void Initialize(Local<ObjectTemplate> globalObjTmpl) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<ObjectTemplate> textDecoderTmpl = ObjectTemplate::New(isolate);
  globalObjTmpl->Set(String::NewFromUtf8Literal(isolate, "textDecoder"),
                     textDecoderTmpl);

  Local<FunctionTemplate> encodeSlowFnTmpl =
      FunctionTemplate::New(isolate, EncodeSlow);
  textDecoderTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "encode"),
                       encodeSlowFnTmpl);
}

} // namespace done::textDecoder
