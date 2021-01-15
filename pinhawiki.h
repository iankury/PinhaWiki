#include <bits/stdc++.h>
using namespace std;

namespace utility {
  extern const string kPathPrefix;
  string Path(const string& filename);
  extern unordered_map<int, int> compressed_chars;
  extern vector<string> invalid_substrings;
  extern unordered_set<string> common_terms;
  void PrintElapsedTime(const double& initial_time);
  bool ValidTitle(const string& title);
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
  void FullPreprocessing();
}

namespace indexer {
  extern unordered_map<string, int> encode;
  extern vector<int> TF;
  void LoadTitles();
  void LoadOriginalTitles();
  void WriteTerms();
  void LoadTerms();
  void LoadWeights();
  void LoadEngine();
  void SaveIndex();
  void BuildIndex();
  string Query(const string& query);
}

namespace command_line_interface {
  void PrintHelp();
  void HandleQuery();
  void HandleCommand(string command, string filename);
  void Run();
}