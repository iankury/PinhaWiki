#include "pinhawiki.h"

/*
Source of the main Information Retrieval ideas applied and
theoretical groundwork for the model chosen (Vector Space Model),
particularly the formulas for computing TF-IDF weights
and measuring the similarity between queries and documents:

BAEZA-YATES, Ricardo; RIBEIRO-NETO, Berthier. 
Recuperação de Informação: Conceitos e tecnologia das máquinas de busca. 
2. ed. Porto Alegre: Bookman, 2013. 590 p. ISBN 978-85-8260-049-8. E-book.
*/

namespace indexer {
  struct PairHash { // For using a single unordered_map for weights w[{i,j}]
    size_t operator()(const pair<int, int>& p) const {
      return hash<int>{}(p.first) ^ hash<int>{}(p.second);
    }
  };

  int N = 0; // Number of documents in the collection (see LoadTitles)
  const int M = 1763000; // Number of terms in the collection (upper bound)

  vector<string> titles; // Maps each document number to its title
  vector<string> original_titles; // Same as above but with raw titles (for making links)
  unordered_map<string, string> redirections;
  unordered_map<string, int> encode; // Assigns an id to each term (0, 1, 2...)
  vector<int> TF(M); // Total term frequency in collection (for local TF, see freq)
  vector<float> IDF(M); // Inverse document frequency

  unordered_map<pair<int, int>, float, PairHash> w; // For term i, document j has weight w[{i,j}]

  void LoadTitles() {
    double initial_time = clock();

    ifstream ifs(utility::Path("titles"));
    string s;
    while (getline(ifs, s))
      titles.push_back(s);
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

  void WriteTerms() {
    double initial_time = clock();

    ifstream ifs(utility::Path("articles"));
    string s;
    unordered_map<string, int> unique_terms;
    while (getline(ifs, s)) {
      stringstream ss(s);
      while (getline(ss, s, ' '))
        unique_terms[s]++;
    }
    ifs.close();

    for (const string& x : titles) {
      stringstream ss(x);
      while (getline(ss, s, ' '))
        unique_terms[s]++;
    }

    ofstream ofs(utility::Path("terms"));
    for (auto& term : unique_terms)
      ofs << term.first << " " << term.second << "\n";
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadTerms() {
    double initial_time = clock();

    ifstream ifs(utility::Path("terms"));
    string s;
    int id = 0;
    while (ifs >> s >> TF[id]) {
      encode[s] = id;
      ++id;
    }
    ifs.close();

    for (int i = 0; i < encode.size(); i++) 
      IDF[i] = log2((float)N / (float)TF[i]);

    utility::PrintElapsedTime(initial_time);
  }

  void LoadIndex() {
    double initial_time = clock();

    ifstream ifs(utility::Path("index"));
    int i, j;
    float wx;
    while (ifs >> i >> j >> wx)
      w[{ i, j }] = wx;

    ifs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadEngine() {
    cout << "Loading titles.txt\n";
    LoadTitles();
    cout << "Loading original_titles.txt\n";
    LoadOriginalTitles();
    cout << "Loading terms.txt\n";
    LoadTerms();
    cout << "Loading index.txt\n";
    LoadIndex();
  }

  void SaveIndex() {
    double initial_time = clock();

    ofstream ofs(utility::Path("index"));
    for (auto& x : w) {
      ofs << x.first.first << " " << x.first.second << " ";
      ofs << fixed << setprecision(3) << x.second << "\n";
    }
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void BuildIndex() {
    LoadTerms();

    cout << "Loaded " << encode.size() << " terms.\n";

    double initial_time = clock();

    ifstream ifs(utility::Path("articles"));
    string s;

    for (int j = 0; getline(ifs, s); j++) { // For each document j
      stringstream ss(s), sst(titles[j]);
      unordered_map<string, int> freq; // Term i occurs freq[i] times in this document
      unordered_map<string, int> title_freq; // Term i occurs freq[i] times in this document's title
      vector<pair<float, int>> aux;
      int i;
      float text_weight, title_weight, weight;

      while (sst >> s) {
        title_freq[s]++;
        freq[s] = 0;
      }

      while (ss >> s)
        freq[s]++;

      for (auto& p : freq) {
        i = encode[p.first];
        if (IDF[i] > 0) {
          text_weight = log2(1.f + p.second) * IDF[i];
          title_weight = title_freq.count(p.first) ? log2(1.f + title_freq[p.first]) * IDF[i] : 0;
          weight = .7f * title_weight + .3f * text_weight;
          if (weight > .03f)
            aux.push_back({ weight, i });
        }
      }

      const int at_least = 100;
      if (aux.size() <= at_least) {
        for (auto& p : aux)
          w[{ p.second, j }] = p.first;
      }
      else {
        sort(rbegin(aux), rend(aux));
        const int limit = max(at_least, (int)aux.size() / 4);
        for (int k = 0; k < limit; k++) // Consider only the heaviest terms for document j
          w[{ aux[k].second, j }] = aux[k].first;
      }
    }
    ifs.close();

    utility::PrintElapsedTime(initial_time);
  }

  string Query(const string& query) {
    double initial_time = clock();
    
    stringstream ss(query);
    unordered_map<string, int> TFQ; // Term frequency in the query

    string term;
    while (getline(ss, term, ' ')) 
      TFQ[term]++;

    float numerator = 0, denominator = 0;
    float denominator_left_sum, denominator_right_sum;
    float wij, wiq;

    vector<float> score(N); // Similarity between each document and the query
    auto comp = [&](int px, int qx) { // Heaviest first
      if (score[px] == score[qx])
        return px < qx;
      return score[px] > score[qx]; 
    }; 
    auto ranking = set<int, decltype(comp)>(comp);

    for (int j = 0; j < N; j++) {
      numerator = denominator_left_sum = denominator_right_sum = 0;
      for (auto& p : TFQ) {
        if (!encode.count(p.first))
          continue;

        const int i = encode[p.first];

        wij = w.count({ i, j }) ? w[{ i, j }] : .0f;

        wiq = log2(1.f + p.second) * IDF[i];

        numerator += wij * wiq;
        denominator_left_sum += wij * wij;
        denominator_right_sum += wiq * wiq;
      }
      if (numerator > 0) {
        denominator = sqrt(denominator_left_sum) * sqrt(denominator_right_sum);
        if (titles[j] == query)
          score[j] = 998.99f;
        else if (titles[j] == query + " (desambiguacao)")
          score[j] = 999.99f;
        else
          score[j] = numerator / denominator;
        if (score[j] > 0.01)
          ranking.insert(j);
      }
    }

    string ans;
    unordered_set<string> visited;

    for (int j : ranking) {
      auto it = redirections.find(original_titles[j]);
      const string cur_title = it == redirections.end() ? original_titles[j] : (*it).second;
      if (visited.count(cur_title))
        continue;
      visited.insert(cur_title);

      ans += cur_title;
      ans.push_back(30); // Separator

      if (visited.size() >= 50) // Keep only top results
        break;
    }
    
    utility::PrintElapsedTime(initial_time);

    return ans;
  }
}