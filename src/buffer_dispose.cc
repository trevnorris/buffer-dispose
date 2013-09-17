#include <stdlib.h>
#include <v8.h>
#include <node.h>
#include <node_buffer.h>

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


void BufferDispose(const FunctionCallbackInfo<Value>& args) {
  // Safe since we've already done the IsObject check from JS.
  Local<Object> buf = args[0].As<Object>();
  char* data = static_cast<char*>(buf->GetIndexedPropertiesExternalArrayData());
  size_t length = buf->GetIndexedPropertiesExternalArrayDataLength();

  if (length > 0) {
    args.GetIsolate()->AdjustAmountOfExternalAllocatedMemory(-length);
    buf->SetIndexedPropertiesToExternalArrayData(NULL,
                                                 v8::kExternalUnsignedByteArray,
                                                 0);
  }

  if (data != NULL) {
    free(data);
  }
}


void BufferUnslice(const FunctionCallbackInfo<Value>& args) {
  Local<Object> buf = args[0].As<Object>();
  size_t length = buf->GetIndexedPropertiesExternalArrayDataLength();

  if (length > 0) {
    buf->SetIndexedPropertiesToExternalArrayData(NULL,
                                                 v8::kExternalUnsignedByteArray,
                                                 0);
  }
}


void Initialize(Handle<Object> target) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);

  Local<FunctionTemplate> t = FunctionTemplate::New(BufferDispose);
  Local<String> name = String::NewFromUtf8(isolate, "dispose");
  t->SetClassName(name);
  target->Set(name, t->GetFunction());

  t = FunctionTemplate::New(BufferUnslice);
  name = String::NewFromUtf8(isolate, "unslice");
  t->SetClassName(name);
  target->Set(name, t->GetFunction());
}


}  // namespace bdispose

NODE_MODULE(buffer_dispose, bdispose::Initialize)
