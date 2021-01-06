#include "pinhawiki.h"

namespace Indexer {
  float N = 0; // Number of documents in the collection

  struct TermInfo {
    int freq; // Total term frequency in collection
    float IDF() const { return log2(N / freq); } // Inverse Document Frequency
    unordered_map<int, int> TF; // Document [first] has frequency [second] for this term
    unordered_map<int, float> w; // Document [first] has weight [second] for this term
    TermInfo() : freq(0) {}
    void Compute_Weights() {
      const float idf = IDF();
      if (idf > 0)
        for (auto& p : TF) 
          w[p.first] = (log2((float)p.second) + 1) * idf;
      TF.clear(); // Get rid of TF after computing w in order to save memory
    }
    float Weight_In_Query(int frequency) const {
      if (freq == 0)
        return 0;
      return (log2((float)frequency) + 1) * IDF();
    }
  };

  unordered_map<string, TermInfo> index; // Maps each term to its data (across documents)

  vector<string> titles; // Maps each document number to its title

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

  void Build_Index(string articles_path) {
    double initial_time = clock();

    ifstream ifs(articles_path);
    string s;
    int j = 0; // Document number
    while (getline(ifs, s)) {
      stringstream ss(s);
      while (ss >> s) {
        index[s].freq++;
        index[s].TF[j]++;
      }
      j++;
    }
    ifs.close();

    for (auto& entry : index)
      entry.second.Compute_Weights();

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

    float numerator, denominator;
    float denominator_left_sum, denominator_right_sum;
    float wij, wiq;

    for (int j = 0; j < N; j++) {
      numerator = denominator_left_sum = denominator_right_sum = 0;
      for (auto& p : TFQ) {
        TermInfo& term_info = index[p.first];
        wij = term_info.w[j];
        wiq = term_info.Weight_In_Query(p.second);
        numerator += wij * wiq;
        denominator_left_sum += wij * wij;
        denominator_right_sum += wiq * wiq;
      }
      if (numerator > 0) {
        denominator = sqrt(denominator_left_sum) * sqrt(denominator_right_sum);
        score[j] = numerator / denominator;
      }
      else {
        score[j] = 0;
      }
    }

    vector<int> sorted(N);
    iota(begin(sorted), end(sorted), 0);
    sort(begin(sorted), end(sorted), [&](int p, int q) {
      return score[p] > score[q];
    });

    // Get only top results
    const int threshold = min(int(N + .5), 20);
    for (int i = 0; i < threshold; i++) {
      const int j = sorted[i];
      cout << "The score of document " << titles[j] << " is ";
      cout << fixed << setprecision(4) << score[j] << "\n";
    }

    Utility::Print_Elapsed_Time(initial_time);
  }
}