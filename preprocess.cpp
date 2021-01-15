#include "pinhawiki.h"

namespace preprocess {
  void StripWhitespace(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("nows"));
    string s;
    while (getline(ifs, s)) {
      while (!s.empty() && isspace(s.back()))
        s.pop_back();
      for (int i = 0; i < s.length(); i++)
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

  void WriteRedirections(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    string s, alias;
    unordered_map<string, string> redirect;

    while (getline(ifs, s)) {
      if (s.find("<title>") != string::npos) 
        alias = s.substr(7, s.length() - 15);
      else {
        const int idx = s.find("#REDIRECIONAMENTO");
        if (idx != string::npos) {
          int i = idx + 16, depth = 0;
          string target_title;
          while (i < s.length() && (depth < 2 || s[i] != ']')) {
            if (depth == 2)
              target_title.push_back(s[i]);
            else if (s[i] == '[')
              depth++;
            i++;
          }
          redirect[alias] = target_title;
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

  void RemoveTrash(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("notrash"));
    string s;
    vector<string> cur_page;
    bool skip = 1;
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
        if (!utility::ValidTitle(s.substr(7, s.length() - 15))) {
          while (getline(ifs, s) && s != "</page>");
          continue;
        }
        cur_page.push_back(s);
      }
      else if (s.find("<text") != string::npos)
        skip = 0;
      else if (s.find("</text") != string::npos)
        skip = 1;
      else if (!skip)
        cur_page.push_back(s);
    }

    ifs.close();
    ofs.close();

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
    ofstream ofs(utility::Path("articles"));
    ofstream ofst(utility::Path("titles"));
    string s;
    while (getline(ifs, s)) {
      if (s == "<page>") {
        getline(ifs, s);
        ofst << s.substr(7, s.length() - 15) << "\n";
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

  string LowerAsciiSingleLine(const string& line) {
    string ans;
    for (int i = 0; i < line.length(); i++) {
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

  void Alpha(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("alpha"));
    string s;
    while (getline(ifs, s)) {
      string to_write;
      bool last_is_space = false;
      for (int i = 0; i < s.length(); i++) {
        if (isalpha(s[i]) || s[i] == '\n') {
          last_is_space = false;
          to_write.push_back(s[i]);
        }
        else if (!last_is_space) {
          to_write.push_back(' ');
          last_is_space = true;
        }
      }
      ofs << to_write << "\n";
    }
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
    while (getline(ss, s, ' ')) {
      if (!indexer::encode.count(s))
        continue;
      const int i = indexer::encode[s];
      const int freq = indexer::TF[i];
      if (freq > 2 && freq < 100000)
        ans += s + " ";
    }
    return ans;
  }

  void DeleteExtremeFreq(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("noextreme"));
    string s;
    while (getline(ifs, s))
      ofs << DeleteExtremeFreqSingleLine(s) << "\n";
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void FullPreprocessing() {
    double initial_time = clock();

    cout << "Stripping whitespace from raw text.\n";
    StripWhitespace(utility::Path("raw"));

    cout << "Writing redirections.\n";
    WriteRedirections(utility::Path("nows"));

    cout << "Removing all tags except page and title\n";
    cout << "  and all entries with invalid titles.\n";
    RemoveTrash(utility::Path("nows"));
    remove((utility::Path("nows")).c_str());

    cout << "Writing titles to one file, articles to another.\n";
    SplitTitlesAndArticles(utility::Path("notrash"));
    remove((utility::Path("notrash")).c_str());

    cout << "Making articles lowercase and ASCII.\n";
    LowerAscii(utility::Path("articles"));
    remove((utility::Path("articles")).c_str());

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

    cout << "Keeping file with original titles.\n";
    rename(
      (utility::Path("titles")).c_str(),
      (utility::Path("original_titles")).c_str()
    );

    cout << "Making separate lower ascii titles file.\n";
    LowerAscii(utility::Path("original_titles"));
    rename(
      (utility::Path("lower")).c_str(),
      (utility::Path("titles")).c_str()
    );

    cout << "Finished preprocessing chain.\n";
    utility::PrintElapsedTime(initial_time);
  }
}