#include "pinhawiki.h"

#define COMMAND_LINE_INTERFACE_MODE 1

#if COMMAND_LINE_INTERFACE_MODE == 1

int main() {

  command_line_interface::Run();

  return 0;
}

// Code below is for running the c++ addon for Node.JS
#else

#include <node.h>

string HandleClientRequest(string req) {
  if (req.empty())
    return "";

  char req_type = req.back();
  req.pop_back();

  if (req_type == 'i') {
    indexer::LoadEngine();
    return "";
  }
  else if (req_type == 'q') 
    return indexer::Query(req);

  cout << "Error: unexpected req_type\n";
  return "Error";
}

// Code below handles communication with Node.JS server (app.js)
void Boilerplate(const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::String::Utf8Value input(isolate, args[0]);
  string output = HandleClientRequest(*input);
  args.GetReturnValue().Set(
    v8::String::NewFromUtf8(isolate, output.c_str()).ToLocalChecked()
  );
}

void Initialize(v8::Local<v8::Object> exports) { 
  NODE_SET_METHOD(exports, "HandleClientRequest", Boilerplate); 
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)

#endif