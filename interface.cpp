#include "pinhawiki.h"

namespace Interface {
  void Print_Help() {
    cout << "Displaying help. Available commands:\n";
    cout << "help -- displays available commands\n";
    cout << "exit -- closes the program\n";
    cout << "head -- writes first million chars\n";
    cout << "head20k -- writes first 20k lines\n";
    cout << "nows -- strips leading/trailing whitespace\n";
    cout << "notrash -- removes entries with trash titles, leaves only title & page\n";
    cout << "pairs -- writes titles to one file, articles to another\n";
    cout << "lower -- makes each char lowercase and ASCII\n";
    cout << "alpha -- prints only letters\n";
    cout << "nocommon -- deletes high-frequency words\n";
    cout << "nolong -- deletes words with 1 letter or more than 14\n";
    cout << "terms -- writes all unique terms\n";
    cout << "load -- loads document titles\n";
    cout << "index -- builds an index (requires load and terms.txt)\n";
    cout << "engine -- loads whole engine (titles, terms, index)\n";
    cout << "query -- queries the index (requires index)\n";
  }

  void Handle_Query() {
    string s;
    getline(cin, s);

    // TO DO: preprocess query

    Indexer::Query(s);
  }

  void Handle_Command(string command, string filename) {
    if (command == "help")
      Print_Help();
    else if (command == "exit")
      exit(0);
    else if (command == "nows")
      Preprocess::Strip_Whitespace(filename);
    else if (command == "notrash")
      Preprocess::Remove_Trash(filename);
    else if (command == "head")
      Preprocess::Head(filename);
    else if (command == "head20k")
      Preprocess::Head20k(filename);
    else if (command == "pairs")
      Preprocess::To_Pairs(filename);
    else if (command == "lower")
      Preprocess::Lower_ASCII(filename);
    else if (command == "alpha")
      Preprocess::Alpha(filename);
    else if (command == "nocommon")
      Preprocess::Delete_Common(filename);
    else if (command == "nolong")
      Preprocess::Delete_Long(filename);
    else if (command == "terms")
      Preprocess::List_Terms(filename);
    else if (command == "everything")
      Preprocess::Do_Everything(filename);
    else if (command == "load")
      Indexer::Load_Titles(filename);
    else if (command == "index")
      Indexer::Build_Index(filename);
    else if (command == "engine")
      Indexer::Load_Engine();
    else if (command == "query")
      Handle_Query();
    else {
      cout << command << " is not a valid command. ";
      cout << "Type help for a list of commands.\n";
    }
  }

  void Run() {
    string line;

    while (getline(cin, line)) {
      stringstream ss(line);
      
      string command, filename;

      getline(ss, command, ' ');
      getline(ss, filename);

      Handle_Command(command, Utility::Path(filename));
    }
  }
}