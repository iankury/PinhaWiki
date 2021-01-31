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

  vector<vector<IndexNode>> inverted_index;

  vector<float> vector_norms;

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
    ifstream ifs(utility::Path("titles"));
    string s;
    while (getline(ifs, s)) 
      titles.push_back(utility::RemoveTrailingTrash(s));
    ifs.close();

    N = titles.size();
  }

  void LoadOriginalTitles() {
    ifstream ifs(utility::Path("original_titles"));
    string s;
    while (getline(ifs, s))
      original_titles.push_back(utility::RemoveTrailingTrash(s));
    ifs.close();
  }

  void LoadRedirections() {
    ifstream ifs(utility::Path("redirections"));
    string alias, target_title;
    while (getline(ifs, alias) && getline(ifs, target_title))
      redirections[utility::RemoveTrailingTrash(alias)] = utility::RemoveTrailingTrash(target_title);
    ifs.close();
  }

  void LoadTerms() {
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
  }

  void SaveTerms() {
    LoadTitles();
    cout << "Loaded " << N << " titles.\n";

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
  }

  void LoadIndex() {
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
  }

  void SaveIndex() {
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
  }

  void LoadNorms() {
    ifstream ifs(utility::Path("vector_norms"));

    vector_norms.resize(N);
    for (int i = 0; i < N; i++)
      ifs >> vector_norms[i];

    ifs.close();
  }

  void SaveNorms() {
    ofstream ofs(utility::Path("vector_norms"));
    for (int i = 0; i < N; i++)
      ofs << fixed << setprecision(3) << vector_norms[i] << "\n";
    ofs.close();
  }

  void BuildTitleToId() {
    for (int i = 0; i < N; i++) 
      title_to_id[titles[i]] = i;
  }

  void BuildIndex() {
    LoadTitles();
    cout << "Loaded " << N << " titles.\n";

    LoadTerms();
    cout << "Loaded " << M << " terms.\n";

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

        // ↓ Arbitrary decision of setting weight as 70% title weight plus 30% text weight
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


    // ↓ Build the index and the vector norms
    vector_norms.assign(N, 0);
    for (const auto& p : w) {
      const int i = p.first.first;
      const int j = p.first.second;
      const float weight = p.second;
      const int idx = inverted_index_current_position[i];

      inverted_index[i][idx] = IndexNode{ j, weight };

      ++inverted_index_current_position[i];

      vector_norms[j] += weight * weight;
    }
    for (float& norm : vector_norms)
      norm = sqrt(norm);
  }


  //  =============== ENGINE ===============

  void LoadEngine() {
    ofstream ofs(utility::Path("log"), ios_base::app);

    ofs << "Loading titles.txt\n";
    LoadTitles();
    ofs << "Loaded " << N << " titles.\n";
    ofs << "Loading original_titles.txt\n";
    LoadOriginalTitles();
    ofs << "Loading redirections.txt\n";
    LoadRedirections();
    ofs << "Loading terms.txt\n";
    LoadTerms();
    ofs << "Loaded " << M << " terms.\n";
    ofs << "Loading index.txt\n";
    LoadIndex();
    ofs << "Loading norms.txt\n";
    LoadNorms();
    ofs << "Building title_to_id\n";
    BuildTitleToId();

    ofs.close();
  }

  string Query(string query) {
    // ↓ First base case: there exists a redirection for this exact query
    if (title_to_id.count(query)) {
      const int j = title_to_id[query];
      if (redirections.count(original_titles[j])) {
        string redirected_title = original_titles[j];
        while (redirections.count(redirected_title))
          redirected_title = redirections[redirected_title];

        return Query(preprocess::LowerAsciiSingleLine(redirected_title));
      }
    }

    unordered_map<int, float> score; // Similarity between each document and the query
    unordered_map<int, float> numerators;

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

    // ↓ Get info from relevant documents only (connected to query terms)
    for (int i : query_ids) 
      for (const auto& node : inverted_index[i]) 
        numerators[node.j] += node.w;

    for (const auto& doc_info : numerators) {
      const int j = doc_info.first;
      const float numerator = doc_info.second;

      score[j] = numerator / vector_norms[j];
      ranking.insert(j);
    }

    // ↓ Second base case: perfect match
    if (title_to_id.count(query)) {
      const int j = title_to_id[query];
      ranking.erase(j);
      score[j] = 99.f; // Highest
      ranking.insert(j);
    }

    // ↓ Third base case: no matches at all
    else if (ranking.empty())
      return "Conjunto vazio" + string{ char{ 30 } } + "0";
    
    unordered_set<string> visited; // Try to make sure there are no duplicate results

    string ans;
    for (int j : ranking) {
      // ↓ Handle duplicates
      const string processed_title = preprocess::LowerAsciiSingleLine(original_titles[j]);
      if (visited.count(processed_title)) 
        continue;
      visited.insert(processed_title);

      // ↓ Send the article names to the frontend
      ans += original_titles[j];
      ans.push_back(30); // Separator
      
      // ↓ Send the scores to the frontend
      ans += to_string((int)( 1000.f * score[j] + .5f ));
      ans.push_back(30);

      if (visited.size() >= 100) // Keep only top results
        break;
    }

    ans.pop_back();
    return ans;
  }
}