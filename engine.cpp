#include "pinhawiki.h"

namespace indexer {
  void LoadEngine() {
    ofstream ofs(utility::Path("log"), ios_base::app);
    ofs << "\n\n ======= STARTED LOADING ENGINE\n\n";
    ofs << "Loading titles.txt\n";
    LoadTitles();
    ofs << "Loaded " << N << " titles\n";
    ofs << "Loading original_titles.txt\n";
    LoadOriginalTitles();
    ofs << "Loading redirections.txt\n";
    LoadRedirections();
    ofs << "Loading terms.txt\n";
    LoadTerms();
    ofs << "Loaded " << M << " terms\n";
    ofs << "Loading norms.txt\n";
    LoadNorms();
    ofs << "Building title_to_id\n";
    BuildTitleToId();
    ofs << "Loading first_term_id_in_file\n";
    LoadFirstTermIdInFile();
    ofs << "Loading first_title_id_in_file\n";
    LoadFirstTitleIdInFile();
    ofs << "Loaded " << first_title_id_in_file.size() - 1 << " files\n";
    ofs << "\n ======= FINISHED LOADING ENGINE\n";
    ofs.close();
  }

  vector<IndexNode> InvertedIndex(int term_id) {
    vector<IndexNode> ans;

    auto ub = upper_bound(first_term_id_in_file.begin(), first_term_id_in_file.end(), term_id);
    const int file_id = ub - first_term_id_in_file.begin() - 1;

    ifstream ifs(utility::Path("index/" + utility::ToFileName(file_id)));
    string s;
    for (int i = 0; i <= term_id - first_term_id_in_file[file_id]; i++)
      getline(ifs, s);

    size_t sz;
    stringstream ss(s);
    ss >> sz;

    for (int i = 0; i < sz; i++) {
      ans.push_back(IndexNode());
      ss >> ans.back().j >> ans.back().w;
    }

    return ans;
  }

  string TextForSnippets(int title_id) {
    string s;

    auto ub = upper_bound(first_title_id_in_file.begin(), first_title_id_in_file.end(), title_id);
    const int file_id = ub - first_title_id_in_file.begin() - 1;

    ifstream ifs(utility::Path("text/" + utility::ToFileName(file_id)));

    if (ifs.good()) 
      for (int i = 0; i <= title_id - first_title_id_in_file[file_id]; i++)
        getline(ifs, s);

    return s;
  }

  string Snippet(int title_id, const string& text, const vector<string>& query_terms) {
    string ans = "... ";

    deque<string> cur, best;
    deque<bool> occ_q, best_occ_q;
    int occurrences = 0, best_occurrences = 0;
    int middle_score = 0, best_middle_score = 0;
    stringstream ss(text);
    string word;
    while (ss >> word) {
      if (word.find("&lt;") != string::npos || word.find("&#60;") != string::npos)
        continue;
      const string processed_word = 
        preprocess::StripWhitespaceSingleLine(
          preprocess::AlphaSingleLine(
            preprocess::LowerAsciiSingleLine(word)
          )
        );
      bool match = false;
      for (const string& term : query_terms) {
        if (term == processed_word) {
          match = true;
          break;
        }
      }
      cur.push_back(word);
      occ_q.push_back(match);
      if (match)
        ++occurrences;
      if (cur.size() > 35) {
        if (occ_q.front())
          --occurrences;
        occ_q.pop_front();
        cur.pop_front();
      }
      middle_score = 0;
      for (int i = 13; i < 22 && i < occ_q.size(); i++)
        middle_score += int{ occ_q[i] };
      if (
        best_occurrences < occurrences ||
        (best_occurrences == occurrences && cur.size() > best.size()) ||
        (best_occurrences == occurrences && cur.size() == 35 && middle_score > best_middle_score)
        ) {
        best_occurrences = occurrences;
        best = cur;
        best_occ_q = occ_q;
        best_middle_score = middle_score;
      }
    }

    if (best_occurrences == 0)
      return "-";

    for (int i = 0; i < best.size(); i++) {
      word = best_occ_q[i] ? "<b>" + best[i] + "</b>" : best[i];
      ans += word + " ";
    }

    return ans + "...";
  }

  string Query(string query, int page_number) {
    unordered_map<int, float> score; // Similarity between each document and the query
    unordered_map<int, float> numerators;

    auto comp = [&](int px, int qx) { // Heaviest first
      if (score[px] == score[qx])
        return px < qx;
      return score[px] > score[qx];
    };
    auto ranking = set<int, decltype(comp)>(comp); // Stores results sorted by score

    unordered_set<int> query_ids;
    vector<string> query_terms;

    stringstream ss(query);
    string term;

    // ↓ Build query_ids and filter query terms
    while (ss >> term) {
      if (!term_to_id.count(term)) // Term doesn't exist in collection
        continue;

      const int i = term_to_id[term];

      if (IDF[i] < 1) // Term is too frequent to matter
        continue;

      query_ids.insert(i);
      query_terms.push_back(term);
    }

    // ↓ Get info from relevant documents only (connected to query terms)
    for (int i : query_ids)
      for (const auto& node : InvertedIndex(i))
        numerators[node.j] += TFIDF(node.w, i);

    for (const auto& doc_info : numerators) {
      const int j = doc_info.first;
      const float numerator = doc_info.second;

      // ↓ Finishing formula from page 46, but dropping query norm and
      //   redefining wij to TFij and wiq to IDFi, as mentioned in the book
      score[j] = numerator / vector_norms[j];
      ranking.insert(j);
    }

    // ↓ First base case: perfect match
    if (title_to_id.count(query)) {
      const int j = title_to_id[query];
      ranking.erase(j);
      score[j] = 99.f;
      ranking.insert(j);
    }

    // ↓ Second base case: there exists a redirection for this exact query
    if (redirections.count(query)) {
      string redirected_title = redirections[query];
      while (redirections.count(redirected_title))
        redirected_title = redirections[redirected_title];
      redirected_title = preprocess::LowerAsciiSingleLine(redirected_title);
      if (title_to_id.count(redirected_title) &&
        redirected_title.find("(desamb") == string::npos) {
        const int j = title_to_id[redirected_title];
        ranking.erase(j);
        score[j] = 99.f;
        ranking.insert(j);
      }
    }

    // ↓ Third base case: no matches at all
    else if (ranking.empty())
      return utility::kEmptyResult;

    unordered_set<string> visited; // Try to make sure there are no duplicate results

    string ans;

    int results_to_skip = 10 * page_number;
    int results_added = 0;
    for (int j : ranking) {
      // ↓ Handle duplicates
      const string processed_title = preprocess::LowerAsciiSingleLine(original_titles[j]);
      if (visited.count(processed_title))
        continue;
      visited.insert(processed_title);

      if (--results_to_skip >= 0)
        continue;

      ++results_added;
      // ↓ Send the article names to the frontend
      ans += original_titles[j];
      ans.push_back(utility::kSeparator);

      // ↓ Send the snippet to the frontend
      ans += Snippet(j, TextForSnippets(j), query_terms);
      ans.push_back(utility::kSeparator);

      // ↓ Send the cosine to the frontend
      if (score[j] < 98) {
        stringstream converter;
        converter << fixed << setprecision(2) << score[j];
        string str_sim;
        converter >> str_sim;
        ans += str_sim;
      }
      else
        ans += "perfeita";
      ans.push_back(utility::kSeparator);

      if (results_added >= 10)
        break;
    }

    ans.pop_back();
    return ans;
  }
}