#include "pinhawiki.h"

namespace command_line_interface {
  void PrintHelp() {
    cout << "Displaying help. Available commands:\n";
    cout << "help -- displays available commands\n";
    cout << "exit -- closes the program\n";
    cout << "head -- writes first million chars\n";
    cout << "head20k -- writes first 20k lines\n";
    cout << "nows -- strips leading/trailing whitespace\n";
    cout << "write_redirections -- writes map of alias to target title to file\n";
    cout << "notrash -- removes entries with trash titles, leaves only\n";
    cout << "  page tags, and inside them only title tags and text (content)\n";
    cout << "split -- writes titles to one file, articles to another\n";
    cout << "lower_ascii -- makes each char lowercase and ASCII,\n";
    cout << "  but will keep non-Portuguese non-ASCII chars\n";
    cout << "alpha -- prints only letters\n";
    cout << "nocommon -- deletes high-frequency words\n";
    cout << "nolong -- deletes single-letter words and words of 15+ letters\n";
    cout << "noextreme -- deletes terms with extreme freq (requires terms.txt)\n";
    cout << "preprocess -- does all preprocessing sequentially\n";
    cout << "write_terms -- writes all unique terms\n";
    cout << "  along with their total frequency in the collection\n";
    cout << "load_terms -- loads terms\n";
    cout << "load_titles -- loads document titles\n";
    cout << "build_index -- builds an index (requires load and terms.txt)\n";
    cout << "save_index -- saves the weights in index.txt as i j w lines\n";
    cout << "load_engine -- loads whole engine (titles, terms, index)\n";
    cout << "query -- queries the index (requires index)\n";
  }

  void HandleQuery() {
    string s;
    getline(cin, s);

    indexer::Query(s);
  }

  void HandleCommand(string command, string filename) {
    if (command == "help")
      PrintHelp();
    else if (command == "exit")
      exit(0);
    else if (command == "nows")
      preprocess::StripWhitespace(filename);
    else if (command == "write_redirections")
      preprocess::WriteRedirections(filename);
    else if (command == "notrash")
      preprocess::RemoveTrash(filename);
    else if (command == "head")
      preprocess::Head(filename);
    else if (command == "head20k")
      preprocess::Head20k(filename);
    else if (command == "split")
      preprocess::SplitTitlesAndArticles(filename);
    else if (command == "lower_ascii")
      preprocess::LowerAscii(filename);
    else if (command == "alpha")
      preprocess::Alpha(filename);
    else if (command == "nocommon")
      preprocess::DeleteCommon(filename);
    else if (command == "nolong")
      preprocess::DeleteLong(filename);
    else if (command == "write_terms")
      indexer::WriteTerms();
    else if (command == "load_terms")
      indexer::LoadTerms();
    else if (command == "noextreme")
      preprocess::DeleteExtremeFreq(filename);
    else if (command == "preprocess")
      preprocess::FullPreprocessing();
    else if (command == "load_titles")
      indexer::LoadTitles();
    else if (command == "build_index")
      indexer::BuildIndex();
    else if (command == "save_index")
      indexer::SaveIndex();
    else if (command == "load_engine")
      indexer::LoadEngine();
    else if (command == "query")
      HandleQuery();
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

      HandleCommand(command, utility::Path(filename));
    }
  }
}