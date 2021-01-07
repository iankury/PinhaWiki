#include "pinhawiki.h"

namespace Preprocess {
  void Strip_Whitespace(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("nows"));
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
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Remove_Trash(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("notrash"));
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
        if (!Utility::Valid_Title(s.substr(7, s.length() - 15))) {
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
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Head(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("head"));
    ifs >> noskipws;
    ofs << noskipws;
    char c;
    for (int i = 0; i < 1000000; i++) {
      ifs >> c;
      ofs << c;
    }
    ifs.close();
    ofs.close();
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Head20k(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("20k"));
    string s;
    for (int i = 0; i < 20000; i++) {
      getline(ifs, s);
      ofs << s << "\n";
    }
    ifs.close();
    ofs.close();
    Utility::Print_Elapsed_Time(initial_time);
  }

  void To_Pairs(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("articles"));
    ofstream ofst(Utility::Path("titles"));
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
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Lower_ASCII(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("lower"));
    ifs >> noskipws;
    ofs << noskipws;
    char c;
    int code;
    while (ifs >> c) {
      if (c == -61) {
        ifs >> c;
        code = ((int)c + 255) % 255;
        if (!Utility::compress_letter.count(code))
          ofs << char(-61) << c;
        else
          ofs << (char)tolower(Utility::compress_letter[code]);
      }
      else
        ofs << (char)tolower(c);
    }
    ifs.close();
    ofs.close();
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Alpha(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("alpha"));
    ifs >> noskipws;
    ofs << noskipws;
    char c, last = 0;
    while (ifs >> c) {
      if (isalpha(c) || c == '\n') {
        ofs << c;
        last = c;
      }
      else if (last != ' ') {
        ofs << ' ';
        last = ' ';
      }
    }
    ifs.close();
    ofs.close();
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Delete_Common(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("nocommon"));
    string s;
    while (getline(ifs, s)) {
      stringstream ss(s);
      while (getline(ss, s, ' '))
        if (!Utility::common_terms.count(s))
          ofs << s << " ";
      ofs << "\n";
    }
    ifs.close();
    ofs.close();
    Utility::Print_Elapsed_Time(initial_time);
  }

  void Delete_Long(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::Path("nolong"));
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
    Utility::Print_Elapsed_Time(initial_time);
  }

  void List_Terms(string filename) {
    double initial_time = clock();

    ifstream ifs(filename);
    string s;
    unordered_map<string, int> unique_terms;
    while (getline(ifs, s)) {
      stringstream ss(s);
      while (getline(ss, s, ' '))
        unique_terms[s]++;
    }
    ifs.close();

    ofstream ofs(Utility::Path("terms"));
    for (auto& term : unique_terms)
      ofs << term.first << " " << term.second << "\n";
    ofs.close();

    Utility::Print_Elapsed_Time(initial_time);
  }

  void Do_Everything(string filename) {
    cout << "Stripping whitespace from raw text.\n";
    Strip_Whitespace(filename);
    cout << "Removing all tags except page and title\n";
    cout << "  and all entries with invalid titles.\n";
    Remove_Trash(Utility::Path("nows"));
    remove((Utility::Path("nows")).c_str());
    cout << "Writing titles to one file, articles to another.\n";
    To_Pairs(Utility::Path("notrash"));
    remove((Utility::Path("notrash")).c_str());
    cout << "Making articles lowercase and ASCII.\n";
    Lower_ASCII(Utility::Path("articles"));
    remove((Utility::Path("articles")).c_str());
    cout << "Making articles letter-only.\n";
    Alpha(Utility::Path("lower"));
    remove((Utility::Path("lower")).c_str());
    cout << "Removing common words from articles.\n";
    Delete_Common(Utility::Path("alpha"));
    remove((Utility::Path("alpha")).c_str());
    cout << "Removing too long and too short words from articles.\n";
    Delete_Long(Utility::Path("nocommon"));
    remove((Utility::Path("nocommon")).c_str());
    rename(
      (Utility::Path("nolong")).c_str(),
      (Utility::Path("articles")).c_str()
    );
  }
}