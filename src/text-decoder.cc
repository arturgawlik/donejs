#include "v8-array-buffer.h"
#include "v8-primitive.h"
#include "v8-template.h"
#include <v8.h>

using v8::ArrayBuffer;
using v8::DataView;
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
void DecodeSlow(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();

  if (!args[0]->IsArrayBuffer() && !args[0]->IsDataView()) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8Literal(
        isolate, "'input' must be an ArrayBuffer or DataView when calling "
                 "'encode(input, encoding)' "
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
    Local<ArrayBuffer> arrayBuffer;
    bool isArrayBuffer = false;
    Local<DataView> dataView;
    if (args[0]->IsArrayBuffer()) {
      isArrayBuffer = true;
      arrayBuffer = args[0].As<ArrayBuffer>();
    } else if (args[0]->IsDataView()) {
      dataView = args[0].As<DataView>();
    }

    const char *data;
    int bufferLen;
    if (isArrayBuffer) {
      data = (char *)arrayBuffer->Data();
      bufferLen = arrayBuffer->ByteLength();
    } else {
      data = (char *)dataView->Buffer()->Data();
      bufferLen = dataView->ByteLength();
    }

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

  Local<FunctionTemplate> decodeSlowFnTmpl =
      FunctionTemplate::New(isolate, DecodeSlow);
  textDecoderTmpl->Set(v8::String::NewFromUtf8Literal(isolate, "decode"),
                       decodeSlowFnTmpl);
}

} // namespace done::textDecoder
