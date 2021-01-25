#include "pinhawiki.h"

namespace command_line_interface {
  void PrintHelp() {
    cout << "Displaying help. Available commands:\n";
    cout << "help -- displays available commands\n";
    cout << "exit -- closes the program\n";
    cout << "head <path> -- writes first million chars\n";
    cout << "head20k <path> -- writes first 20k lines\n";
    cout << "nows <path> -- strips leading/trailing whitespace\n";
    cout << "write_redirections <path> -- writes map of alias to target title to file\n";
    cout << "notrash <path> -- removes entries with trash titles, leaves only\n";
    cout << "  page tags, and inside them only title tags and text (content), no refs\n";
    cout << "split <path> -- writes titles to one file, articles to another\n";
    cout << "lower_ascii <path> -- makes each char lowercase and ASCII,\n";
    cout << "  but will keep non-Portuguese non-ASCII chars\n";
    cout << "alpha <path> -- prints only letters\n";
    cout << "nocommon <path> -- deletes high-frequency words\n";
    cout << "nolong <path> -- deletes single-letter words and words of 15+ letters\n";
    cout << "noextreme <path> -- deletes terms with extreme frequencies";
    cout << "  from articles (requires terms.txt)\n";
    cout << "redirect <path> -- writes final targets for each original title\n";
    cout << "preprocess -- does all preprocessing sequentially\n";
    cout << "save_terms -- writes all unique terms (requires titles.txt)\n";
    cout << "  along with their total frequency in the collection\n";
    cout << "load_terms -- loads terms\n";
    cout << "load_titles -- loads document titles\n";
    cout << "build_index -- builds an index (requires titles.txt and terms.txt)\n";
    cout << "save_index -- saves the weights to index.txt as i j w lines\n";
    cout << "save_norms -- saves vector norms for all documents\n";
    cout << "load_engine -- loads whole engine (titles, terms, index)\n";
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
    else if (command == "redirect")
      preprocess::Redirect(filename);
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
    else if (command == "load_engine")
      indexer::LoadEngine();
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
      cout << "Finished command " << command << "\n";
    }
  }
}