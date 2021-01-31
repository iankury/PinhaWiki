#include <bits/stdc++.h>
using namespace std;

namespace utility {
  extern const string kPathPrefix;
  extern const char kSeparator;
  extern unordered_map<int, int> compressed_chars;
  extern vector<string> invalid_substrings;
  extern unordered_set<string> common_terms;

  string Path(const string& filename);
  void PrintElapsedTime(const double& initial_time);
  bool ValidTitle(const string& title);
  bool AllAscii(const string& s);
  string NoParentheses(const string& s);
  size_t CountLines(const string& filename);
  string RemoveTrailingTrash(string s);
}

namespace preprocess {
  void StripWhitespace(const string& filename);
  void WriteRedirections(const string& filename);
  void RemoveTrash(const string& filename);
  void Head(const string& filename);
  void Head20k(const string& filename);
  void SplitTitlesAndArticles(const string& filename);
  string LowerAsciiSingleLine(const string& line);
  void LowerAscii(const string& filename);
  void Alpha(const string& filename);
  void DeleteCommon(const string& filename);
  void DeleteLong(const string& filename);
  void DeleteExtremeFreq(const string& filename);
  void Redirect(const string& filename);
  void SplitIndex();
  void FullPreprocessing();
}

namespace indexer {
  extern unordered_map<string, int> term_to_id;

  void LoadTitles();
  void LoadOriginalTitles();
  void LoadTerms();
  void SaveTerms();
  void LoadIndex();
  void SaveIndex();
  void LoadNorms();
  void SaveNorms();
  void BuildDisambiguation();
  void BuildIndex();

  // ↓ Engine
  void LoadEngine();
  string Query(string query);
}

namespace command_line_interface {
  void PrintHelp();
  void HandleCommand(string command, string filename);
  void Run();
}