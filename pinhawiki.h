#include <bits/stdc++.h>
using namespace std;

namespace Utility {
  extern const string path_prefix;
  extern unordered_map<int, int> compress_letter;
  extern vector<string> invalid_substrings;
  extern unordered_set<string> common_terms;
  void Print_Elapsed_Time(double initial_time);
  bool Valid_Title(const string& title);
}

namespace Preprocess {
  void Strip_Whitespace(string filename);
  void Remove_Trash(string filename);
  void Head(string filename);
  void To_Pairs(string filename);
  void Lower_ASCII(string filename);
  void Alnum(string filename);
  void Delete_Common(string filename);
  void Delete_Long(string filename); 
  void Do_Everything(string filename);
}

namespace Indexer {
  extern float N;
  struct Occurrence;
  struct TermInfo;
  void Load_Titles(string titles_path);
  void Build_Index(string articles_path);
  void Query(string query);
}

namespace Interface {
  void Print_Help();
  void Handle_Query();
  void Handle_Command(string command, string filename);
  void Run();
}