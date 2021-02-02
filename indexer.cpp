#include "pinhawiki.h"

namespace indexer {
  // ↓ Enables us to save space by using a single unordered_map for weights w[{ i, j }]
  struct PairHash {
    size_t operator()(const pair<int, int>& p) const {
      return hash<int>{}(p.first) ^ hash<int>{}(p.second);
    }
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

  vector<int> first_term_id_in_file, first_title_id_in_file;

  // ↓ Resized and assigned in LoadTerms
  vector<int> TF; // Term Frequency of term i in collection (not the local TF)

  // ↓ Resized in LoadTerms, assigned in BuildIDF
  vector<float> IDF; // Inverse Document Frequency of term i

  vector<float> vector_norms;

  inline float ComputeIDF(int i) {
    return log2(float(N) / TF[i]); // In the book: page 38, formula 2.6
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
      if (term.second < 4 || term.second > 499999)
        term.second = 0;

    ofstream ofs(utility::Path("terms"));
    for (auto& term : unique_terms)
      if (term.second > 0)
        ofs << term.first << " " << term.second << "\n";
    ofs.close();
  }

  void LoadIndex() {
    ifstream ifs(utility::Path("index"));

    inverted_index.resize(M);
    string line;
    int sz, j, w;
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
        ofs << " " << inverted_index[i][k].j << " " << inverted_index[i][k].w;
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
      ofs << fixed << setprecision(2) << vector_norms[i] << "\n";
    ofs.close();
  }

  void LoadFirstTermIdInFile() {
    ifstream ifs(utility::Path("first_term_id_in_file"));

    int x;
    while (ifs >> x)
      first_term_id_in_file.push_back(x);
    first_term_id_in_file.push_back(INT32_MAX);

    ifs.close();
  }

  void LoadFirstTitleIdInFile() {
    ifstream ifs(utility::Path("first_title_id_in_file"));

    int x;
    while (ifs >> x)
      first_title_id_in_file.push_back(x);
    first_title_id_in_file.push_back(INT32_MAX);

    ifs.close();
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
    unordered_map<pair<int, int>, int, PairHash> w;

    ifstream ifs(utility::Path("articles"));

    string line, term;

    for (int j = 0; j < N && getline(ifs, line); j++) { // For each document j
      stringstream ss(line), sst(titles[j]);

      // ↓ Term Frequency for each term i in this document
      unordered_map<int, int> document_TF; 

      // ↓ Term Frequency for each term i in this document's title
      unordered_map<int, int> title_TF;

      // ↓ Temporary container for pairs of weights and ids (for sorting)
      vector<pair<int, int>> w_and_id;

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

        // ↓ Arbitrary decision of setting setting title : text weight ratio to 10 : 1
        //   Also, we reduce w[i][j] to only TF, as described in page 47 of the book
        const int weight = 10 * title_ct + text_ct;

        w_and_id.push_back({ weight, i });
      }

      // ↓ Consider only the top 80% heaviest terms for document j
      const size_t threshold = .8 * w_and_id.size();
      sort(begin(w_and_id), end(w_and_id), greater<pair<int, int>>());
      for (size_t k = 0; k < threshold; k++) {
        const int weight = w_and_id[k].first;
        const int i = w_and_id[k].second;
        w[{ i, j }] = weight;
      }
    }
    
    ifs.close();

    inverted_index.resize(M);

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
      const int weight = p.second;
      const int idx = inverted_index_current_position[i];

      inverted_index[i][idx] = IndexNode{ j, weight };

      ++inverted_index_current_position[i];

      vector_norms[j] += (float)weight * weight;
    }
    for (float& norm : vector_norms)
      norm = sqrtf(norm);
  }

  void FullBuild() {
    cout << "Building index.\n";
    BuildIndex();
    cout << "Saving index.\n";
    SaveIndex();
    cout << "Saving norms.\n";
    SaveNorms();
  }
}