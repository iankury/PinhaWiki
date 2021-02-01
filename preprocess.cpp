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
      else if (s.find("</text") != string::npos || s.find("{{Refer") != string::npos)
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

  string AlphaSingleLine(const string& s) {
    string ans;
    bool last_is_space = false;
    for (int i = 0; i < s.length(); i++) {
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

    ifstream ifs(filename);
    ofstream ofs(utility::Path("noextreme"));
    string s;
    while (getline(ifs, s))
      ofs << DeleteExtremeFreqSingleLine(s) << "\n";
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void Redirect(const string& filename) {
    double initial_time = clock();

    unordered_map<string, string> redirections;

    ifstream ifs_redir(utility::Path("redirections"));
    string alias, target_title;
    while (getline(ifs_redir, alias) && getline(ifs_redir, target_title))
      if (alias != target_title)
        redirections[alias] = target_title;
    ifs_redir.close();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("redirected_titles"));
    string s;
    while (getline(ifs, s)) {
      int sanity = 123456;
      while (redirections.count(s) && --sanity) 
        s = redirections[s];
      if (sanity < 1) {
        cout << "Eternal redirection detected.\n";
        cout << s << " to " << redirections[s] << "\n";
        exit(0);
      }
      ofs << s << "\n";
    }
    ifs.close();
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void NoXml(const string& filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    ofstream ofs(utility::Path("noxml"));
    ifs >> noskipws;
    ofs << noskipws;
    char c;
    while (ifs >> c) {
      if (c == '|')
        ofs << " ";
      else if (c != '\'' && c != '[' && c != ']' && c != '{' && c != '}' && c != '"')
        ofs << c;
    }

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
        ofstream ofs(utility::Path("index/" + to_string(file_id)));
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
      ofstream ofs(utility::Path("index/" + to_string(file_id)));
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
        ofstream ofs(utility::Path("text/" + to_string(file_id)));
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
      ofstream ofs(utility::Path("text/" + to_string(file_id)));
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

    cout << "Removing all tags except page and title\n";
    cout << "  and refs and all entries with invalid titles.\n";
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

    cout << "Keeping original titles.\n";

    cout << "Finished preprocessing chain.\n";
    utility::PrintElapsedTime(initial_time);
  }
}