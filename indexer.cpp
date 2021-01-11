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
  struct RandomizedHash {
    // Hash source: Written in 2015 by Sebastiano Vigna: xorshift.di.unimi.it/splitmix64.c
    static uint64_t splitmix64(uint64_t x) {
      x += 0x9e3779b97f4a7c15;
      x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
      x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
      return x ^ (x >> 31);
    }

    size_t operator()(uint64_t x) const {
      static const uint64_t FIXED_RANDOM =
        chrono::steady_clock::now().time_since_epoch().count();
      return splitmix64(x + FIXED_RANDOM);
    }
  };

  struct PairHash { // Makes it possible to use a single unordered_map for weights w[{i,j}]
    size_t operator()(const pair<int, int>& p) const {
      return RandomizedHash{}(p.first) ^ RandomizedHash {}(p.second);
    }
  };

  int N = 0; // Number of documents in the collection (see LoadTitles)
  const int M = 3300000; // Number of terms in the collection (upper bound)

  vector<string> titles; // Maps each document number to its title
  unordered_map<string, int> encode; // Assigns an id to each term (0, 1, 2...)
  int TF[M]; // Total term frequency in collection (for local TF, see freq)
  float IDF[M]; // Inverse document frequency

  unordered_map<pair<int, int>, float, PairHash> w; // For term i, document j has weight w[{i,j}]

  void LoadTitles(const string& titles_path) {
    double initial_time = clock();

    ifstream ifs(titles_path);
    string s;
    while (getline(ifs, s)) 
      titles.push_back(s);
    ifs.close();

    N = titles.size();

    utility::PrintElapsedTime(initial_time);
  }

  void WriteTerms(const string& articles_path) {
    double initial_time = clock();

    ifstream ifs(articles_path);
    string s;
    unordered_map<string, int> unique_terms;
    while (getline(ifs, s)) {
      stringstream ss(s);
      while (getline(ss, s, ' '))
        unique_terms[s]++;
    }
    ifs.close();

    ofstream ofs(utility::Path("terms"));
    for (auto& term : unique_terms)
      ofs << term.first << " " << term.second << "\n";
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void LoadTerms() {
    double initial_time = clock();

    ifstream ifs(utility::Path("terms"));
    string s, fr;
    int id = -1;
    while (ifs >> s >> fr) {
      encode[s] = ++id;
      TF[id] = stoi(fr);
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
    LoadTitles(utility::Path("titles"));
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
      ofs << fixed << setprecision(4) << x.second << "\n";
    }
    ofs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void BuildIndex(const string& articles_path) {
    LoadTerms();

    cout << "Loaded " << encode.size() << " terms.\n";

    double initial_time = clock();

    ifstream ifs(articles_path);
    string s;

    for (int j = 0; getline(ifs, s); j++) { // For each document j
      stringstream ss(s), sst(titles[j]);
      unordered_map<string, int> freq; // Term i occurs freq[i] times in this document
      unordered_map<string, int> title_freq; // Term i occurs freq[i] times in this document's title
      vector<pair<float, int>> aux;
      int i;
      float weight, text_weight, title_weight;

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
          weight = .6f * title_weight + .4f * text_weight;
          if (weight > 0.03)
            aux.push_back({ weight, i });
        }
      }

      if (aux.size() < 201) {
        for (auto& p : aux)
          w[{ p.second, j }] = p.first;
      }
      else {
        sort(rbegin(aux), rend(aux));
        const int limit = max(100, (int)aux.size() / 4); // Top 25% heaviest, at least 100
        for (int k = 0; k < limit; k++) // Consider only the heaviest terms for document j
          w[{ aux[k].second, j }] = aux[k].first;
      }
    }
    ifs.close();

    utility::PrintElapsedTime(initial_time);
  }

  void Query(const string& query) {
    double initial_time = clock();
    
    stringstream ss(query);
    unordered_map<string, int> TFQ; // Term frequency in the query

    string term;
    while (getline(ss, term, ' ')) 
      TFQ[term]++;

    float numerator = 0, denominator = 0;
    float denominator_left_sum, denominator_right_sum;
    float wij, wiq;

    int i;

    vector<float> score(N); // Similarity between each document and the query
    vector<int> ranking;

    for (int j = 0; j < N; j++) {
      numerator = denominator_left_sum = denominator_right_sum = 0;
      for (auto& p : TFQ) {
        if (!encode.count(p.first))
          continue;

        i = encode[p.first];

        wij = w.count({ i, j }) ? w[{ i, j }] : 0;

        wiq = log2(1.f + p.second) * IDF[i];

        numerator += wij * wiq;
        denominator_left_sum += wij * wij;
        denominator_right_sum += wiq * wiq;
      }
      if (numerator > 0) {
        denominator = sqrt(denominator_left_sum) * sqrt(denominator_right_sum);
        if (titles[j] == query)
          score[j] = 9.99f;
        else
          score[j] = numerator / denominator;
        if (score[j] > 0.01)
          ranking.push_back(j);
      }
    }

    sort(begin(ranking), end(ranking), [&](int px, int qx) {
      return score[px] > score[qx];
      });

    const int threshold = min((int)ranking.size(), 50); // Get only top results
    for (i = 0; i < threshold; i++) {
      const int j = ranking[i];
      cout << right << setw(2) << (i + 1) << ") <";
      cout << fixed << setprecision(2) << score[j] << "> ";
      cout << titles[j] << "\n";
    }
    
    utility::PrintElapsedTime(initial_time);
  }
}