/*
Source of the main Information Retrieval techniques applied here and
theoretical groundwork for the model chosen (Vector Space Model),
particularly the formulas for computing TF-IDF weights and
document-query similarity scores:

BAEZA-YATES, Ricardo; RIBEIRO-NETO, Berthier.
Recupera��o de Informa��o: Conceitos e tecnologia das m�quinas de busca.
2. ed. Porto Alegre: Bookman, 2013. 590 p. ISBN 978-85-8260-049-8. E-book.
*/

#include "pinhawiki.h"

#define COMMAND_LINE_INTERFACE_MODE 0

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
  else if (req_type == 'q') {
    const string processed_query = preprocess::LowerAsciiSingleLine(req);
    ofstream ofs("log.txt", ios_base::app);
    ofs << req << " -> " << processed_query << "\n";
    ofs.close();

    return indexer::Query(processed_query);
  }

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