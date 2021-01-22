#include "pinhawiki.h"

namespace indexer {
  // ↓ Enables us to save space by using a single unordered_map for weights w[{ i, j }]
  struct PairHash {
    size_t operator()(const pair<int, int>& p) const {
      return hash<int>{}(p.first) ^ hash<int>{}(p.second);
    }
  };

  struct IndexNode {
    int j;
    float w;
  };

  struct DocumentNode {
    float numerator, denominator_left_sum;
    DocumentNode() : numerator(0), denominator_left_sum(0) {}
  };

  // ↓ Number of documents in the collection (assigned in LoadTitles)
  size_t N; 

  // ↓ Number of terms in the collection (assigned in LoadTerms)
  size_t M;

  vector<string> titles; // Maps document id to title
  unordered_map<string, int> title_to_id; // Maps title to document id (for checking exact matches)

  // ↓ Unprocessed titles for returning results that can be used directly in links
  vector<string> original_titles; 

  // ↓ Send only the final destination when returning results
  unordered_map<string, string> redirections; // Maps alias to target title

  unordered_map<string, int> term_to_id; // Maps term to id (0, 1, 2, ...)

  unordered_map<string, int> disambiguation; // Maps disambiguation title to title id

  vector<vector<IndexNode>> inverted_index;

  // ↓ Resized and assigned in LoadTerms
  vector<int> TF; // Term Frequency of term i in collection (not the local TF)

  // ↓ Resized in LoadTerms, assigned in BuildIDF
  vector<float> IDF; // Inverse Document Frequency of term i

  inline float ComputeIDF(int i) {
    return log2(float(N) / TF[i]); // In the book: page 38, formula 2.6
  }

  // ↓ Uses "local" TF, i. e. term frequency in the current document, not in the collection
  inline float TFIDF(int local_TF, int i) { // Returns the TF-IDF weight
    return log2(1.f + local_TF) * IDF[i]; // In the book: page 42, table 2.6, row 3
  }

  void BuildIDF() {
    for (int i = 0; i < M; i++)
      IDF[i] = ComputeIDF(i);
  }

  void LoadTitles() {
    double initial_time = clock();

    ifstream ifs(utility::Path("titles"));
    string s;
    while (getline(ifs, s)) {
      title_to_id[s] = titles.size();
      titles.push_back(s);
    }
    ifs.close();

    N = titles.size();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadOriginalTitles() {
    double initial_time = clock();

    ifstream ifs(utility::Path("original_titles"));
    string s;
    while (getline(ifs, s))
      original_titles.push_back(s);
    ifs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadRedirections() {
    double initial_time = clock();

    ifstream ifs(utility::Path("redirections"));
    string alias, target_title;
    while (getline(ifs, alias) && getline(ifs, target_title)) 
      redirections[alias] = target_title;
    ifs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadTerms() {
    double initial_time = clock();

    M = utility::CountLines("terms");
    TF.resize(M);
    IDF.resize(M);
    inverted_index.resize(M);

    ifstream ifs(utility::Path("terms"));
    string term;
    int id = -1;
    while (++id < M) {
      if (!(ifs >> term >> TF[id]))
        break;
      term_to_id[term] = id;
    }
    ifs.close();

    BuildIDF();

    utility::PrintElapsedTime(initial_time);
  }

  void SaveTerms() {
    LoadTitles();
    cout << "Loaded " << N << " titles.\n";

    double initial_time = clock();

    ifstream ifs(utility::Path("articles"));
    string article, term;
    unordered_map<string, int> unique_terms;
    while (getline(ifs, article)) {
      stringstream ss(article);
      while (ss >> term)
        unique_terms[term]++;
    }
    ifs.close();

    // ↓ For article title, keep all all-ascii terms
    for (const string& title : titles) {
      stringstream ss(title);
      while (ss >> term)
        if (utility::AllAscii(term)) {
          const string processed_term = utility::NoParentheses(term);
          if (processed_term.length() > 1)
            unique_terms[processed_term]++;
        }
    }

    // ↓ Get rid of extreme terms
    for (auto& term : unique_terms)
      if (term.second < 4 || term.second > 99999)
        term.second = 0;

    ofstream ofs(utility::Path("terms"));
    for (auto& term : unique_terms)
      if (term.second > 0)
        ofs << term.first << " " << term.second << "\n";
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadIndex() {
    double initial_time = clock();

    ifstream ifs(utility::Path("index"));

    string line;
    int sz, j;
    float w;
    for (int i = 0; i < M; i++) {
      getline(ifs, line);
      stringstream ss(line);
      ss >> sz;
      inverted_index[i].resize(sz);

      for (int k = 0; k < sz; k++) {
        ss >> j >> w;
        inverted_index[i][k] = IndexNode{ j, w };
      }
    }

    ifs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void SaveIndex() {
    double initial_time = clock();

    ofstream ofs(utility::Path("index"));
    for (int i = 0; i < M; i++) {
      const size_t sz = inverted_index[i].size();
      ofs << sz;
      for (size_t k = 0; k < sz; k++) {
        ofs << " " << inverted_index[i][k].j << " ";
        ofs << fixed << setprecision(3) << inverted_index[i][k].w;
      }
      ofs << "\n";
    }
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void BuildDisambiguation() {
    double initial_time = clock();

    for (int i = 0; i < N; i++) {
      const string& title = titles[i];
      const size_t idx = title.find("(desambiguacao)");
      if (idx != string::npos) {
        string prefix = title.substr(0, idx);
        while (!prefix.empty() && (prefix.back() == '(' || prefix.back() == ' '))
          prefix.pop_back();
        if (!prefix.empty())
          disambiguation[prefix] = i;
      }
    }

    utility::PrintElapsedTime(initial_time);
  }

  void BuildIndex() {
    LoadTitles();
    cout << "Loaded " << N << " titles.\n";

    LoadTerms();
    cout << "Loaded " << M << " terms.\n";

    double initial_time = clock();

    // ↓ For term i, document j has weight w[{ i, j }]
    unordered_map<pair<int, int>, float, PairHash> w;

    ifstream ifs(utility::Path("articles"));

    string line, term;

    for (int j = 0; j < N && getline(ifs, line); j++) { // For each document j
      stringstream ss(line), sst(titles[j]);

      // ↓ Term Frequency for each term i in this document
      unordered_map<int, int> document_TF; 

      // ↓ Term Frequency for each term i in this document's title
      unordered_map<int, int> title_TF;

      // ↓ Temporary container for pairs of weights and ids (for sorting)
      vector<pair<float, int>> w_and_id;

      // ↓ Build title Term Frequencies
      while (sst >> term) 
        if (utility::AllAscii(term)) {
          term = utility::NoParentheses(term);
          if (term.length() < 2)
            continue;

          if (term_to_id.count(term)) {
            const int i = term_to_id[term];
            title_TF[i]++;
            document_TF[i] = 0;
          }
        }

      // ↓ Build text Term Frequencies
      while (ss >> term) 
        if (term_to_id.count(term)) {
          const int i = term_to_id[term];
          document_TF[i]++;
        }

      // ↓ Build weights for all terms for document j
      for (const pair<int, int>& p : document_TF) {
        const int i = p.first;
        
        if (IDF[i] < 1) // Term is too frequent to matter
          continue;

        // ↓ Occurrences of term i in the text of document j
        const int text_ct = p.second; 
        // ↓ Occurrences of term i in the title of document j (save memory by not adding extra zeroes)
        const int title_ct = title_TF.count(i) ? title_TF[i] : 0; 

        const float text_weight = TFIDF(text_ct, i);
        const float title_weight = TFIDF(title_ct, i);

        // ↓ Arbitrary decision of setting weight as 60% title weight plus 40% text weight
        const float weight = .7f * title_weight + .3f * text_weight;

        if (weight > .02f) // Discard tiny weights
          w_and_id.push_back({ weight, i });
      }

      // ↓ Consider only the top 50% heaviest terms for document j
      sort(begin(w_and_id), end(w_and_id), greater<pair<float, int>>());
      for (size_t k = 0; k < w_and_id.size() / 2; k++) {
        const float weight = w_and_id[k].first;
        const int i = w_and_id[k].second;
        w[{ i, j }] = weight;
      }
    }
    
    ifs.close();

    // ↓ Set inverted index sizes
    vector<int> inverted_index_sizes(M);
    for (const auto& p : w) 
      inverted_index_sizes[p.first.first]++;
    for (int i = 0; i < M; i++)
      inverted_index[i].resize(inverted_index_sizes[i]);

    vector<int> inverted_index_current_position(M);

    // ↓ Build the index
    for (const auto& p : w) {
      const int i = p.first.first;
      const int j = p.first.second;
      const float weight = p.second;
      const int idx = inverted_index_current_position[i];

      inverted_index[i][idx] = IndexNode{ j, weight };

      ++inverted_index_current_position[i];
    }

    utility::PrintElapsedTime(initial_time);
  }


  //  =============== ENGINE ===============

  void LoadEngine() {
    cout << "Loading titles.txt\n";
    LoadTitles();
    cout << "Loading original_titles.txt\n";
    LoadOriginalTitles();
    cout << "Loading terms.txt\n";
    LoadTerms();
    cout << "Loading index.txt\n";
    LoadIndex();
    cout << "Building disambiguation\n";
    BuildDisambiguation();
  }

  string Query(const string& query) {
    vector<float> score(N); // Similarity between each document and the query

    unordered_map<int, DocumentNode> doc_info;

    auto comp = [&](int px, int qx) { // Heaviest first
      if (score[px] == score[qx])
        return px < qx;
      return score[px] > score[qx];
    };
    auto ranking = set<int, decltype(comp)>(comp); // Stores results sorted by score

    unordered_set<int> query_ids;

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
    }

    float denominator_right_sum = 0;

    // ↓ Precompute denominator_right_sum
    for (int i : query_ids) {
      const float wiq = IDF[i];
      denominator_right_sum += wiq * wiq;
    }
    denominator_right_sum = sqrt(denominator_right_sum);

    // ↓ Get relevant document info (only for documents connected to query terms)
    for (int i : query_ids) {
      for (const auto& node : inverted_index[i]) {
        // ↓ Part of formula 2.10 on page 46 of the book
        doc_info[node.j].numerator += node.w * IDF[i]; // Assume wiq = IDF[i]
        doc_info[node.j].denominator_left_sum += node.w * node.w;
      }
    }

    for (const auto& doc : doc_info) {
      const int j = doc.first;
      const float numerator = doc.second.numerator;
      const float denominator_left_sum = sqrt(doc.second.denominator_left_sum);

      if (numerator > 0) {
        score[j] = numerator / (denominator_left_sum * denominator_right_sum);
        ranking.insert(j);
      }
    }

    // ↓ First base case: perfect match
    if (title_to_id.count(query)) {
      const int j = title_to_id[query];
      ranking.erase(j);
      score[j] = 2.f; // Highest after disambiguation
      ranking.insert(j);
    }

    // ↓ Second base case: found disambiguation for this exact query
    if (disambiguation.count(query)) {
      const int disambiguation_id = disambiguation[query];
      ranking.erase(disambiguation_id);
      score[disambiguation_id] = 3.f; // Highest
      ranking.insert(disambiguation_id);
    }

    // ↓ Third base case: no matches at all
    if (ranking.empty())
      return "Conjunto vazio";

    string ans;
    unordered_set<string> visited; // Try to make sure there are no duplicate results

    for (int j : ranking) {
      string cur_title = original_titles[j];

      // ↓ Redirect until final destination
      while (redirections.count(cur_title))
        cur_title = redirections[cur_title];

      // ↓ Handle duplicates
      const string processed_title = preprocess::LowerAsciiSingleLine(cur_title);
      if (visited.count(processed_title))
        continue;
      visited.insert(processed_title);

      ans += cur_title;
      ans.push_back(30); // Separator

      if (visited.size() >= 50) // Keep only top results
        break;
    }

    return ans;
  }
}