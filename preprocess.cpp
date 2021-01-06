#include "pinhawiki.h"

namespace Preprocess {
  void Strip_Whitespace(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::path_prefix + "nows.txt");
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
    ofstream ofs(Utility::path_prefix + "notrash.txt");
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
    ofstream ofs(Utility::path_prefix + "head.txt");
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

  void To_Pairs(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::path_prefix + "articles.txt");
    ofstream ofst(Utility::path_prefix + "titles.txt");
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
    ofstream ofs(Utility::path_prefix + "lower.txt");
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

  void Alnum(string filename) {
    double initial_time = clock();
    ifstream ifs(filename);
    ofstream ofs(Utility::path_prefix + "alnum.txt");
    ifs >> noskipws;
    ofs << noskipws;
    char c, last = 0;
    while (ifs >> c) {
      if (isalnum(c) || c == '\n') {
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
    ofstream ofs(Utility::path_prefix + "nocommon.txt");
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
    ofstream ofs(Utility::path_prefix + "nolong.txt");
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

  void Do_Everything(string filename) {
    cout << "Stripping whitespace from raw text.\n";
    Strip_Whitespace(filename);
    cout << "Removing all tags except page and title\n";
    cout << "  and all entries with invalid titles.\n";
    Remove_Trash(Utility::path_prefix + "nows.txt");
    remove((Utility::path_prefix + "nows.txt").c_str());
    cout << "Writing titles to one file, articles to another.\n";
    To_Pairs(Utility::path_prefix + "notrash.txt");
    remove((Utility::path_prefix + "notrash.txt").c_str());
    cout << "Making articles lowercase and ASCII.\n";
    Lower_ASCII(Utility::path_prefix + "articles.txt");
    remove((Utility::path_prefix + "articles.txt").c_str());
    cout << "Making articles alphanumeric.\n";
    Alnum(Utility::path_prefix + "lower.txt");
    remove((Utility::path_prefix + "lower.txt").c_str());
    cout << "Removing common words from articles.\n";
    Delete_Common(Utility::path_prefix + "alnum.txt");
    remove((Utility::path_prefix + "alnum.txt").c_str());
    cout << "Removing too long and too short words from articles.\n";
    Delete_Long(Utility::path_prefix + "nocommon.txt");
    remove((Utility::path_prefix + "nocommon.txt").c_str());
    rename(
      (Utility::path_prefix + "nolong.txt").c_str(), 
      (Utility::path_prefix + "articles.txt").c_str()
    );
  }
}