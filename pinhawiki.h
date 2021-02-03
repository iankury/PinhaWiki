#include <bits/stdc++.h>
using namespace std;

namespace utility {
  enum {
    GOOD_TITLE, EMPTY_TITLE, PARENTHESIS_START, REDIRECTED_TITLE,
    CONTAINS_INVALID, DISAMBIGUATION_TITLE
  };

  extern const string kPathPrefix;
  extern const char kSeparator;
  extern const string kEmptyResult;
  extern unordered_map<int, int> compressed_chars;
  extern vector<string> invalid_substrings;
  extern unordered_set<string> common_terms;

  string Path(const string& filename);
  void PrintElapsedTime(const double& initial_time);
  int ValidTitle(const string& title);
  bool AllAscii(const string& s);
  string NoParentheses(const string& s);
  size_t CountLines(const string& filename);
  string RemoveTrailingTrash(string s);
  string ToFileName(int x);
}

namespace preprocess {
  string StripWhitespaceSingleLine(string s);
  void StripWhitespace(const string& filename);
  void WriteRedirections(const string& filename);
  void RemoveTrash(const string& filename);
  void Head(const string& filename);
  void Head20k(const string& filename);
  void SplitTitlesAndArticles(const string& filename);
  string LowerAsciiSingleLine(const string& line);
  void LowerAscii(const string& filename);
  string AlphaSingleLine(const string& s);
  void Alpha(const string& filename);
  void DeleteCommon(const string& filename);
  void DeleteLong(const string& filename);
  void DeleteExtremeFreq(const string& filename);
  void NoXml(const string& filename);
  void SplitIndex();
  void SplitText();
  void FullPreprocessing();
}

namespace indexer {
  struct IndexNode { int j, w; };
  extern size_t N, M;
  extern unordered_map<string, int> title_to_id;
  extern vector<string> original_titles;
  extern unordered_map<string, string> redirections;
  extern unordered_map<string, int> term_to_id;
  extern vector<int> first_term_id_in_file, first_title_id_in_file;
  extern vector<float> IDF, vector_norms;

  // ↓ Uses "local" TF, i. e. term frequency in the current document, not in the collection
  inline float TFIDF(int local_TF, int i) { // Returns the TF-IDF weight
    return log2(1.f + local_TF) * IDF[i]; // In the book: page 42, table 2.6, row 3
  }

  void LoadTitles();
  void LoadOriginalTitles(); 
  void LoadRedirections();
  void LoadTerms();
  void SaveTerms();
  void LoadIndex();
  void SaveIndex();
  void LoadNorms();
  void SaveNorms();
  void LoadFirstTermIdInFile();
  void LoadFirstTitleIdInFile();
  void BuildTitleToId();
  void BuildIndex();
  void FullBuild();

  // ↓ Engine
  void LoadEngine();
  string Query(string query);
}

namespace command_line_interface {
  void PrintHelp();
  void HandleCommand(string command, string filename);
  void Run();
}