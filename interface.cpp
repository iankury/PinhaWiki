#include "pinhawiki.h"

namespace command_line_interface {
  void PrintHelp() {
    cout << "Displaying help. Available commands:\n";
    cout << "\n--- Utility commands ---\n";
    cout << "help -- displays available commands\n";
    cout << "exit -- closes the program\n";
    cout << "head <path> -- writes first million chars\n";
    cout << "head20k <path> -- writes first 20k lines\n";
    cout << "\n--- Preprocessing commands ---\n";
    cout << "preprocess -- executes preprocessing chain (requires raw.txt)\n";
    cout << "nows <path> -- strips leading/trailing whitespace\n";
    cout << "notrash <path> -- removes entries with trash titles, leaves only\n";
    cout << "  page tags, and inside them only title tags and text (content)\n";
    cout << "  Requires redirections.txt\n";
    cout << "split <path> -- writes titles to one file, articles to another\n";
    cout << "lower_ascii <path> -- makes each char lowercase and ASCII,\n";
    cout << "  but will keep non-Portuguese non-ASCII chars\n";
    cout << "alpha <path> -- leaves only letters\n";
    cout << "nocommon <path> -- deletes high-frequency words\n";
    cout << "nolong <path> -- deletes single-letter words and words of 15+ letters\n";
    cout << "noextreme <path> -- deletes terms with extreme freq (requires terms.txt)\n";
    cout << "split_index -- writes index to a lot of files,\n";
    cout << "  first term id for each file (requires index.txt)\n";
    cout << "split_text -- same as above but with text (requires text.txt)\n";
    cout << "\n--- Indexer commands ---\n";
    cout << "save_terms -- writes all unique terms along with their total freq\n";
    cout << "  in the collection (requires articles.txt, titles.txt)\n";
    cout << "load_terms -- loads terms\n";
    cout << "load_titles -- loads document titles\n";
    cout << "build_index -- builds index (requires articles.txt, terms.txt, titles.txt)\n";
    cout << "save_index -- saves the weights to index.txt as i j w lines\n";
    cout << "save_norms -- saves vector norms for all documents\n";
    cout << "full_build -- builds index, saves index, saves vector norms\n";
    cout << "  (requires articles.txt, terms.txt, titles.txt)\n";
    cout << "\n";
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
    else if (command == "save_terms")
      indexer::SaveTerms();
    else if (command == "load_terms")
      indexer::LoadTerms();
    else if (command == "noextreme")
      preprocess::DeleteExtremeFreq(filename);
    else if (command == "split_index")
      preprocess::SplitIndex();
    else if (command == "split_text")
      preprocess::SplitText();
    else if (command == "preprocess")
      preprocess::FullPreprocessing();
    else if (command == "load_titles")
      indexer::LoadTitles();
    else if (command == "build_index")
      indexer::BuildIndex();
    else if (command == "save_index")
      indexer::SaveIndex();
    else if (command == "save_norms")
      indexer::SaveNorms();
    else if (command == "full_build")
      indexer::FullBuild();
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
      cout << "Finished command " << command << "\n\n";
    }
  }
}