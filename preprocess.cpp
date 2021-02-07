#include "pinhawiki.h"

namespace preprocess {
  string StripWhitespaceSingleLine(string s) {
    while (!s.empty() && isspace(s.back()))
      s.pop_back();
    for (int i = 0; i < int(s.length()); i++)
      if (!isspace(s[i])) {
        if (i > 0)
          s = s.substr(i);
        break;
      }
    return s;
  }

  void StripWhitespace(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("nows"));
    string s;
    while (getline(ifs, s)) {
      while (!s.empty() && isspace(s.back()))
        s.pop_back();
      for (int i = 0; i < int(s.length()); i++)
        if (!isspace(s[i])) {
          if (i > 0)
            s = s.substr(i);
          break;
        }
      ofs << s << "\n";
    }
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  bool EveryCharEquals(const string& s, char cx) {
    for (char c : s)
      if (!isspace(c) && c != cx)
        return false;
    return true;
  }

  string KillCharStreaks(const string& s, char c) {
    if (s.empty())
      return "";

    string ans;

    for (int i = 0; i < int(s.length()); i++) {
      if (s[i] == c) {
        if (i > 0 && s[i - 1] == c)
          continue;
        if (i + 1 < int(s.length()) && s[i + 1] == c)
          continue;
      }
      ans.push_back(s[i]);
    }
    
    return ans;
  }

  void WriteRedirections(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    string s, alias;
    unordered_map<string, string> redirect;

    while (getline(ifs, s)) {
      if (s.find("<title>") != string::npos) 
        alias = s.substr(7, s.length() - 15);
      else {
        const vector<string> redir_strings{ ">#REDIRECIONAMENTO", ">#REDIRECT" };
        for (const string& redir_string : redir_strings) {
          auto it = std::search(
            s.begin(), s.end(),
            redir_string.begin(), redir_string.end(),
            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
          );
          if (it != s.end()) {
            const int idx = it - s.begin();
            int i = idx + 9, depth = 0;
            string target_title;
            while (i < int(s.length()) && (depth < 2 || s[i] != ']')) {
              if (depth == 2)
                target_title.push_back(s[i]);
              else if (s[i] == '[')
                depth++;
              i++;
            }
            redirect[alias] = target_title;
            break;
          }
        }
      }
    }

    ifs.close();

    ofstream ofs(utility::Path("redirections"));

    for (auto& x : redirect)
      ofs << x.first << "\n" << x.second << "\n";

    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  string KillDeepParts(const string& s) {
    string ans, str_buffer;
    ans.reserve(s.length());
    int depth = 0, tag_depth = 0;

    for (int i = 0; i < int(s.length()); i++) {
      const char c = s[i];
      if (c == '[' || c == '{') {
        ++depth;
        continue;
      }
      else if (c == ']' || c == '}') {
        --depth;
        continue;
      }

      if (c == '&' && (i + 3) < s.length()) {
        const string tag_check = s.substr(i, 4);
        if (tag_check == "&lt;" || tag_check == "&gt;") {
          if (tag_check[1] == 'l')
            ++tag_depth;
          else
            --tag_depth;
          ans += tag_check;
          i += 3;
          continue;
        }
      }
      
      if (tag_depth > 0)
        ans.push_back(c);
      else if (depth > 0)
        str_buffer.push_back(c);
      else {
        if (!str_buffer.empty()) {
          if (
            str_buffer.find_first_of(':') == string::npos &&
            str_buffer.find_first_of('=') == string::npos &&
            str_buffer.find("http") == string::npos &&
            str_buffer.find("col-") == string::npos) {
            auto ix = str_buffer.find_last_of('|');
            if (ix != string::npos)
              str_buffer = str_buffer.substr(ix + 1);
            ans += str_buffer;
          }
          str_buffer.clear();
        }
        ans.push_back(c);
      }
    }

    if (depth == 0 && !str_buffer.empty()) {
      if (
        str_buffer.find_first_of(':') == string::npos &&
        str_buffer.find_first_of('=') == string::npos &&
        str_buffer.find("http") == string::npos &&
        str_buffer.find("col-") == string::npos) {
        auto ix = str_buffer.find_last_of('|');
        if (ix != string::npos)
          str_buffer = str_buffer.substr(ix + 1);
        ans += str_buffer;
      }
    }

    return ans;
  }

  string KillTags(string s) {
    if (s.empty())
      return "";

    size_t last_length = s.length();
    while (!s.empty()) {
      auto j = s.find("&lt;/");
      if (j == string::npos)
        break;
      auto k = s.find("&gt;", j);
      string tag_name;
      for (auto i = j + 5; i < k && s[i] != '/' && !isspace(s[i]); i++)
        tag_name.push_back(s[i]);
      if (tag_name.empty())
        return "ErrorTagEmpty";
      auto i = s.find("&lt;" + tag_name);
      if (i == string::npos) 
        s = k == string::npos ? "" : s.substr(k + 4);
      else
        s = s.substr(0, i) + (k == string::npos ? "" : s.substr(k + 4));
      if (s.length() >= last_length)
        return "ErrorLengthNotDecreasing";
      last_length = s.length();
    }

    while (!s.empty()) {
      auto i = s.find("&lt;");
      if (i == string::npos)
        break;
      auto j = s.find("&gt;", i);
      if (j == string::npos)
        s = s.substr(0, i);
      else
        s = s.substr(0, i) + s.substr(j + 4);
      if (s.length() >= last_length)
        return "ErrorLengthNotDecreasing";
      last_length = s.length();
    }

    return s;
  }

  string HandleXml(string s) {
    s = KillTags(KillDeepParts(s));

    string ans, word;
    stringstream ss(s);
    while (ss >> word) {
      if (
        EveryCharEquals(word, '=') ||
        EveryCharEquals(word, '*') ||
        word.find("http") != string::npos)
        continue;
      ans += KillCharStreaks(KillCharStreaks(word, '='), '\'') + " ";
    }

    if (!ans.empty())
      ans.pop_back();

    return ans;
  }

  void RemoveTrash(const string& filename) {
    double initial_time = clock();

    cout << "Loading redirections.\n";
    indexer::LoadRedirections();
    cout << "Loaded " << indexer::redirections.size() << " redirections.\n";

    vector<int> title_stats(10);

    ifstream ifs(filename);
    ofstream ofs(utility::Path("notrash"));
    string s;
    vector<string> cur_page;
    bool skip = true;
    while (getline(ifs, s)) {
      if (s == "<page>") {
        cur_page.clear();
        cur_page.push_back(s);
      }
      else if (s == "</page>") {
        cur_page.push_back(s);
        for (const string& r : cur_page)
          ofs << r << "\n";
      }
      else if (s.find("<title>") != string::npos) {
        const int title_verdict = utility::ValidTitle(s.substr(7, s.length() - 15));
        ++title_stats[title_verdict];
        if (title_verdict != utility::GOOD_TITLE) {
          while (getline(ifs, s) && s != "</page>");
          continue;
        }
        cur_page.push_back(s);
      }
      else if (s.find("<text") != string::npos) {
        skip = false;
        auto i = s.find_first_of('>', s.find("<text"));
        if (i + 1 < s.length())
          cur_page.push_back(s.substr(i + 1));
      }
      else if (s.find("</text") != string::npos)
        skip = true;
      else if (!skip)
        cur_page.push_back(s);
    }

    ifs.close();
    ofs.close();

    cout << "Found " << title_stats[utility::GOOD_TITLE] << " good titles,\n";
    cout << "      " << title_stats[utility::PARENTHESIS_START] << " (-started titles,\n";
    cout << "      " << title_stats[utility::REDIRECTED_TITLE] << " redirected titles,\n";
    cout << "      " << title_stats[utility::CONTAINS_INVALID] << " invalid-substr titles,\n";
    cout << "  and " << title_stats[utility::DISAMBIGUATION_TITLE] << " disambiguations.\n";

    utility::PrintElapsedTime(initial_time);
  }

  void Head(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("head"));
    ifs >> noskipws;
    ofs << noskipws;
    char c;
    for (int i = 0; i < 1000000; i++) {
      ifs >> c;
      ofs << c;
    }
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void Head20k(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("20k"));
    string s;
    for (int i = 0; i < 20000; i++) {
      getline(ifs, s);
      ofs << s << "\n";
    }
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void SplitTitlesAndArticles(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("original_articles"));
    ofstream ofst(utility::Path("original_titles"));
    string s;
    while (getline(ifs, s)) {
      if (s == "<page>") {
        getline(ifs, s);
        const string title = s.substr(7, s.length() - 15);
        ofst << title << "\n";
      }
      else if (s == "</page>")
        ofs << "\n";
      else 
        ofs << s << " ";
    }
    ifs.close();
    ofs.close();
    ofst.close();

    utility::PrintElapsedTime(initial_time);
  }

  void MakeArticlesNoXml(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("noxml_articles"));
    string s;
    while (getline(ifs, s))
      ofs << HandleXml(s) << "\n";
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  string LowerAsciiSingleLine(const string& line) {
    string ans;
    for (int i = 0; i < int(line.length()); i++) {
      if (line[i] == -61) {
        ++i;
        const int code = ((int)line[i] + 255) % 255;
        if (!utility::compressed_chars.count(code)) {
          ans.push_back(char{ -61 });
          ans.push_back(line[i]);
        }
        else
          ans.push_back(tolower(utility::compressed_chars[code]));
      }
      else
        ans.push_back(tolower(line[i]));
    }
    return ans;
  }

  void LowerAscii(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("lower"));
    string s;
    while (getline(ifs, s)) 
      ofs << LowerAsciiSingleLine(s) << "\n";
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  string AlphaSingleLine(const string& s) {
    string ans;
    bool last_is_space = false;
    for (int i = 0; i < int(s.length()); i++) {
      if (isalpha(s[i]) || s[i] == '\n') {
        last_is_space = false;
        ans.push_back(s[i]);
      }
      else if (!last_is_space) {
        ans.push_back(' ');
        last_is_space = true;
      }
    }
    return ans;
  }

  void Alpha(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("alpha"));
    string s;
    while (getline(ifs, s)) 
      ofs << AlphaSingleLine(s) << "\n";
    
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  string DeleteCommonSingleLine(const string& line) {
    string s, ans;
    stringstream ss(line);
    while (getline(ss, s, ' '))
      if (!utility::common_terms.count(s))
        ans += s + " ";
    return ans;
  }

  void DeleteCommon(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("nocommon"));
    string s;
    while (getline(ifs, s))
      ofs << DeleteCommonSingleLine(s) << "\n";
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void DeleteLong(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("nolong"));
    string s;
    while (getline(ifs, s)) {
      stringstream ss(s);
      while (getline(ss, s, ' ')) 
        if (s.length() > 1 && s.length() < 15) 
          ofs << s << " ";
      ofs << "\n";
    }
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  string DeleteExtremeFreqSingleLine(const string& line) {
    string s, ans;
    stringstream ss(line);
    while (getline(ss, s, ' ')) 
      if (indexer::term_to_id.count(s))
        ans += s + " ";
    return ans;
  }

  void DeleteExtremeFreq(const string& filename) {
    double initial_time = clock();

    indexer::LoadTerms();
    cout << "Loaded " << indexer::term_to_id.size() << " terms.\n";

    ifstream ifs(filename);
    ofstream ofs(utility::Path("noextreme"));
    string s;
    while (getline(ifs, s))
      ofs << DeleteExtremeFreqSingleLine(s) << "\n";
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void SplitIndex() {
    double initial_time = clock();

    ifstream ifs(utility::Path("index"));
    
    vector<int> first_term_id_in_file{ 0 };
    int current_size = 0, file_id = 0, term_id = 0;
    vector<string> lines;
    string line;
    while (getline(ifs, line)) {
      current_size += line.length();
      lines.push_back(line);

      if (current_size > 1000000) {
        current_size = 0;
        ofstream ofs(utility::Path("index/" + utility::ToFileName(file_id)));
        for (const string& s : lines)
          ofs << s << "\n";
        ofs.close();
        lines.clear();

        ++file_id;
        first_term_id_in_file.push_back(term_id + 1);
      }

      ++term_id;
    }

    ifs.close();

    if (!lines.empty()) {
      ofstream ofs(utility::Path("index/" + utility::ToFileName(file_id)));
      for (const string& s : lines)
        ofs << s << "\n";
      ofs.close();
    }

    ofstream ofs(utility::Path("first_term_id_in_file"));
    for (int x : first_term_id_in_file)
      ofs << x << "\n";
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void SplitText() {
    double initial_time = clock();

    ifstream ifs(utility::Path("text"));

    vector<int> first_title_id_in_file{ 0 };
    int current_size = 0, file_id = 0, term_id = 0;
    vector<string> lines;
    string line;
    while (getline(ifs, line)) {
      current_size += line.length();
      lines.push_back(line);

      if (current_size > 1000000) {
        current_size = 0;
        ofstream ofs(utility::Path("text/" + utility::ToFileName(file_id)));
        for (const string& s : lines)
          ofs << s << "\n";
        ofs.close();
        lines.clear();

        ++file_id;
        first_title_id_in_file.push_back(term_id + 1);
      }

      ++term_id;
    }

    ifs.close();

    if (!lines.empty()) {
      ofstream ofs(utility::Path("text/" + utility::ToFileName(file_id)));
      for (const string& s : lines)
        ofs << s << "\n";
      ofs.close();
    }

    ofstream ofs(utility::Path("first_title_id_in_file"));
    for (int x : first_title_id_in_file)
      ofs << x << "\n";
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void FullPreprocessing() {
    double initial_time = clock();

    cout << "Stripping whitespace from raw text.\n";
    StripWhitespace(utility::Path("raw"));

    cout << "Writing redirections.\n";
    WriteRedirections(utility::Path("nows"));

    cout << "Removing all tags except page, title and content of text,\n";
    cout << "  removing all entries with invalid titles.\n";
    RemoveTrash(utility::Path("nows"));
    remove((utility::Path("nows")).c_str());

    cout << "Writing titles to one file, articles to another.\n";
    SplitTitlesAndArticles(utility::Path("notrash"));
    remove((utility::Path("notrash")).c_str());

    cout << "Getting rid of XML in articles.\n";
    MakeArticlesNoXml(utility::Path("original_articles"));
    remove((utility::Path("original_articles")).c_str());

    cout << "Making articles lowercase and ASCII\n";
    cout << "But keeping noxml copy for texts.\n";
    LowerAscii(utility::Path("noxml_articles"));

    cout << "Making articles letter-only.\n";
    Alpha(utility::Path("lower"));
    remove((utility::Path("lower")).c_str());

    cout << "Removing common words from articles.\n";
    DeleteCommon(utility::Path("alpha"));
    remove((utility::Path("alpha")).c_str());

    cout << "Removing too long and too short words from articles.\n";
    DeleteLong(utility::Path("nocommon"));
    remove((utility::Path("nocommon")).c_str());
    rename(
      (utility::Path("nolong")).c_str(),
      (utility::Path("articles")).c_str()
    );

    cout << "Making LowerASCII version of titles.\n";
    LowerAscii(utility::Path("original_titles"));
    rename(
      (utility::Path("lower")).c_str(),
      (utility::Path("titles")).c_str()
    );

    rename(
      (utility::Path("noxml_articles")).c_str(),
      (utility::Path("text")).c_str()
    );

    cout << "Writing terms.\n";
    indexer::SaveTerms();
    cout << "Finished saving terms.\n";

    cout << "Making noextreme version of articles.\n";
    DeleteExtremeFreq(utility::Path("articles"));
    remove((utility::Path("articles")).c_str());
    rename(
      (utility::Path("noextreme")).c_str(),
      (utility::Path("articles")).c_str()
    );
    cout << "Finished making noextreme version of articles.\n";

    indexer::FullBuild();
    cout << "Finished building and saving index and norms.\n";

    cout << "Splitting text.\n";
    SplitText();
    cout << "Finished splitting text.\n";

    cout << "Splitting index.\n";
    SplitIndex();
    cout << "Finished splitting index.\n";

    cout << "\nFinished preprocessing chain.\n";
    utility::PrintElapsedTime(initial_time);
  }
}