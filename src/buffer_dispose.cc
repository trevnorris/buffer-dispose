#include <stdlib.h>

#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include "njsutil.h"

namespace bdispose {

using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Handle;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;


inline void ClearBuffer(Local<Object> buf) {
  if (node::Buffer::Length(buf) > 0)
    buf->SetIndexedPropertiesToExternalArrayData(NULL,
                                                 v8::kExternalUnsignedByteArray,
                                                 0);
}


void BufferDispose(const FunctionCallbackInfo<Value>& args) {
  NJ_SCOPE_SETUP();

  Local<Object> buf = args[0].As<Object>();
  char* data = node::Buffer::Data(buf);
  size_t length = node::Buffer::Length(buf);

  if (length > 0)
    isolate->AdjustAmountOfExternalAllocatedMemory(-length);

  ClearBuffer(buf);

  if (data != NULL)
    free(data);
}


void BufferUnslice(const FunctionCallbackInfo<Value>& args) {
  NJ_SCOPE_SETUP();

  Local<Object> buf = args[0].As<Object>();
  ClearBuffer(buf);
}


void Initialize(Handle<Object> target) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<FunctionTemplate> t = FunctionTemplate::New(BufferDispose);
  t->SetClassName(String::NewFromUtf8(isolate, "dispose"));
  target->Set(String::New("dispose"), t->GetFunction());
}


}  // namespace bdispose

NODE_MODULE(buffer_dispose, bdispose::Initialize)
