#include "pinhawiki.h"

namespace indexer {
  // ↓ Enables us to save space by using a single unordered_map for weights w[{ i, j }]
  struct PairHash {
    size_t operator()(const pair<int, int>& p) const {
      return hash<int>{}(p.first) ^ hash<int>{}(p.second);
    }
  };

  const int kRatioTitleToText = 30;

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
    for (int i = 0; i < int(M); i++)
      IDF[i] = ComputeIDF(i);
  }

  void LoadTitles() {
    ifstream ifs(utility::Path("titles"));
    string s;
    titles.clear();
    while (getline(ifs, s)) 
      titles.push_back(utility::RemoveTrailingTrash(s));
    ifs.close();

    N = titles.size();
  }

  void LoadOriginalTitles() {
    ifstream ifs(utility::Path("original_titles"));
    string s;
    original_titles.clear();
    while (getline(ifs, s))
      original_titles.push_back(utility::RemoveTrailingTrash(s));
    ifs.close();
  }

  void LoadRedirections() {
    ifstream ifs(utility::Path("redirections"));
    string alias, target_title;
    redirections.clear();
    while (getline(ifs, alias) && getline(ifs, target_title))
      redirections[utility::RemoveTrailingTrash(alias)] = utility::RemoveTrailingTrash(target_title);

    unordered_map<string, string> to_add;
    for (const auto& p : redirections) {
      const string temp = preprocess::LowerAsciiSingleLine(p.first);
      if (temp != p.second && !redirections.count(temp))
        to_add[temp] = p.second;
    }
    for (const auto& p : to_add)
      redirections.insert(p);

    ifs.close();
  }

  void LoadTerms() {
    M = utility::CountLines("terms");
    TF.resize(M);
    IDF.resize(M);

    term_to_id.clear();

    ifstream ifs(utility::Path("terms"));
    string term;
    int id = -1;
    while (++id < int(M)) {
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

    inverted_index.clear();
    inverted_index.resize(M);
    string line;
    int sz, j, w;
    for (int i = 0; i < int(M); i++) {
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
    for (int i = 0; i < int(M); i++) {
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
    for (int i = 0; i < int(N); i++)
      ifs >> vector_norms[i];

    ifs.close();
  }

  void SaveNorms() {
    ofstream ofs(utility::Path("vector_norms"));
    for (int i = 0; i < int(N); i++)
      ofs << fixed << setprecision(2) << vector_norms[i] << "\n";
    ofs.close();
  }

  void LoadFirstTermIdInFile() {
    ifstream ifs(utility::Path("first_term_id_in_file"));

    first_term_id_in_file.clear();
    int x;
    while (ifs >> x)
      first_term_id_in_file.push_back(x);
    first_term_id_in_file.push_back(INT32_MAX);

    ifs.close();
  }

  void LoadFirstTitleIdInFile() {
    ifstream ifs(utility::Path("first_title_id_in_file"));

    first_title_id_in_file.clear();
    int x;
    while (ifs >> x)
      first_title_id_in_file.push_back(x);
    first_title_id_in_file.push_back(INT32_MAX);

    ifs.close();
  }

  void BuildTitleToId() {
    title_to_id.clear();
    for (int i = 0; i < int(N); i++) 
      title_to_id[titles[i]] = i;
  }

  inline double PunishTinyDocs(int doc_word_count, double vector_norm) {
    if (doc_word_count > 1500)
      return vector_norm;
    const double c = .6 - .002 * min(250, max(0, 1000 - doc_word_count));
    const double m = (1. - c) / 1500.;
    return pow(vector_norm, 1. / (m * doc_word_count + c));
  }

  void BuildIndex() {
    LoadTitles();
    cout << "Loaded " << N << " titles.\n";

    LoadTerms();
    cout << "Loaded " << M << " terms.\n";

    // ↓ For term i, document j has weight w[{ i, j }]
    unordered_map<pair<int, int>, int, PairHash> w;

    vector<int> term_count_in_doc(N);

    ifstream ifs(utility::Path("articles"));

    string line, term;

    for (int j = 0; j < int(N) && getline(ifs, line); j++) { // For each document j
      stringstream ss(line), sst(titles[j]);

      // ↓ Term Frequency for each term i in this document
      unordered_map<int, int> document_TF; 

      // ↓ Whether each term i occurs is title
      unordered_set<int> occurs_in_title;

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
            occurs_in_title.insert(i);
            document_TF[i] = 0;
          }
        }

      // ↓ Build text Term Frequencies
      while (ss >> term) 
        if (term_to_id.count(term)) {
          const int i = term_to_id[term];
          ++document_TF[i];
          ++term_count_in_doc[j];
        }

      // ↓ Build weights for all terms for document j
      for (const pair<int, int>& p : document_TF) {
        const int i = p.first;
        
        if (IDF[i] < 1) // Term is too frequent to matter
          continue;

        // ↓ Occurrences of term i in the text of document j
        const int text_ct = p.second; 
        // ↓ Occurrences of term i in the title of document j (save memory by not adding extra zeroes)

        const int weight = kRatioTitleToText * int(occurs_in_title.count(i)) + text_ct;

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
    for (int i = 0; i < int(M); i++)
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
    for (int j = 0; j < int(vector_norms.size()); j++)
      vector_norms[j] = sqrt(PunishTinyDocs(term_count_in_doc[j], vector_norms[j]));
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