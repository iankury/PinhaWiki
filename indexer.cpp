#include "pinhawiki.h"

struct pinha_hash {
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

struct hash_pair {
  size_t operator()(const pair<int, int>& p) const {
    auto hash1 = pinha_hash{}(p.first);
    auto hash2 = pinha_hash{}(p.second);
    return hash1 ^ hash2;
  }
};

namespace Indexer {
  int N = 0; // Number of documents in the collection
  const int M = 3300000; // Number of terms (upper bound)

  vector<string> titles; // Maps each document number to its title
  unordered_map<string, int> encode; // Assigns an id to each term (0, 1, 2...)
  int TF[M]; // Total term frequency in collection (for local TF, see freq)
  float IDF[M]; // Inverse document frequency

  unordered_map<pair<int, int>, float, hash_pair> w; // For term i, document j has weight w[{i,j}]

  void Load_Titles(string titles_path) {
    double initial_time = clock();

    ifstream ifs(titles_path);
    string s;
    while (getline(ifs, s))
      titles.push_back(s);
    ifs.close();

    N = titles.size();

    Utility::Print_Elapsed_Time(initial_time);
  }

  void Load_Terms() {
    double initial_time = clock();

    ifstream ifs(Utility::Path("terms"));
    string s, fr;
    int id = -1;
    while (ifs >> s >> fr) {
      encode[s] = ++id;
      TF[id] = stoi(fr);
    }
    ifs.close();

    for (int i = 0; i < encode.size(); i++)
      IDF[i] = log2((float)N / (float)TF[i]);

    Utility::Print_Elapsed_Time(initial_time);
  }

  void Load_Weights() {
    double initial_time = clock();

    ifstream ifs(Utility::Path("index"));
    int i, j;
    float wx;
    while (ifs >> i >> j >> wx)
      w[{ i, j }] = wx;

    ifs.close();

    Utility::Print_Elapsed_Time(initial_time);
  }

  void Load_Engine() {
    cout << "Loading titles.txt\n";
    Load_Titles(Utility::Path("titles"));
    cout << "Loading terms.txt\n";
    Load_Terms();
    cout << "Loading index.txt\n";
    Load_Weights();
  }

  void Build_Index(string articles_path) {
    Load_Terms();

    cout << "Loaded " << encode.size() << " terms.\n";

    double initial_time = clock();

    ifstream ifs(articles_path);
    string s;

    for (int j = 0; getline(ifs, s); j++) { // For each document j
      stringstream ss(s);
      unordered_map<string, int> freq; // Term i occurs freq[i] times in this document
      vector<pair<float, int>> aux;
      int i;
      float weight;

      while (ss >> s)
        freq[s]++;

      for (auto& p : freq) {
        i = encode[p.first];
        if (IDF[i] > 0) {
          weight = log2(1.f + p.second) * IDF[i];
          if (weight > 0.01)
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

    ofstream ofs(Utility::Path("index"));
    for (auto& x : w) {
      ofs << x.first.first << " " << x.first.second << " ";
      ofs << fixed << setprecision(4) << x.second << "\n";
    }
    ofs.close();

    Utility::Print_Elapsed_Time(initial_time);
  }

  void Query(string query) {
    double initial_time = clock();
    
    stringstream ss(query);
    unordered_map<string, int> TFQ; // Term frequency in the query

    string term;
    while (getline(ss, term, ' ')) 
      TFQ[term]++;

    vector<float> score(N); // Similarity between each document and the query

    float numerator = 0, denominator = 0;
    float denominator_left_sum, denominator_right_sum;
    float wij, wiq;

    int i;

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
        score[j] = numerator / denominator;
      }
      else 
        score[j] = 0;
    }

    vector<int> sorted(N);
    iota(begin(sorted), end(sorted), 0);
    sort(begin(sorted), end(sorted), [&](int p, int q) {
      return score[p] > score[q];
    });

    const int threshold = min(N, 50); // Get only top results
    for (int i = 0; i < threshold; i++) {
      const int j = sorted[i];
      if (score[j] < 0.01)
        break;
      cout << titles[j] << " ";
      cout << fixed << setprecision(4) << score[j] << "\n";
    }
    
    Utility::Print_Elapsed_Time(initial_time);
  }
}