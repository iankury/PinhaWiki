#include "pinhawiki.h"

namespace utility {
  const string kPathPrefix = "C:\\Users\\Administrator\\repos\\PinhaWikiC\\";
  const char kSeparator{ 30 };
  const string kEmptyResult = (
    "Conjunto vazio" + string{ kSeparator } +
    "-" + string{ kSeparator } + "0" +
    string{ kSeparator } + "loadnomore"
  );
  const int kSnippetSize = 35;

  unordered_map<int, int> compressed_chars{
    { 128, 65 }, { 136, 69 }, { 140, 73 }, { 146, 79 }, { 153, 85 }, { 127, 65 },
    { 130, 65 }, { 148, 79 }, { 129, 65 }, { 137, 69 }, { 147, 79 }, { 134, 67 },
    { 160, 97 }, { 168, 101 }, { 172, 105 }, { 178, 111 }, { 185, 117 },
    { 159, 97 }, { 162, 97 }, { 180, 111 }, { 161, 97 }, { 169, 101 },
    { 179, 111 }, { 166, 99 }
  };

  vector<string> invalid_substrings{
    "Categoria:", "dia:", "Predefini", "Ficheiro:", "Portal:", "Anexo:", 
    "pico:", "MediaWiki:", "dulo:", "Ajuda:", "Livro:"
  };

  unordered_set<string> mega_common_terms{
    "de", "a", "o", "que", "e", "do", "da", "em", "um", "para",
    "com", "uma", "os", "no", "se", "na", "por", "as", "dos",
    "mas", "ao", "ele", "das", "tem", "seu", "sua", "ou"
  };

  unordered_set<string> common_terms{
    "de", "e", "a", "o", "do", "da", "em", "que", "lt", "quot", "gt", 
    "no", "para", "com", "um", "uma", "na", "por", "os", "se", "foi", 
    "the", "id", "como", "dos", "as", "ref", "of", "ns", "revision", 
    "contributor", "br", "sao", "ao", "titulo", "s", "nome", "imagem", 
    "mais", "sua", "data", "nao", "das", "futebol", "seu", "web", 
    "tambem", "ou", "align", "janeiro", "and", "pela", "brasil", "rio", 
    "ano", "partido", "nos", "pelo", "bgcolor", "ele", "in", "entre", 
    "center", "anos", "ser", "amp", "cidade", "acessodata", "estado", 
    "estados", "grande", "paulo", "m", "n", "mas", "title", "mpc", "era", 
    "marco", "mp", "primeiro", "sobre", "la", "campeonato", "jogos", 
    "julho", "redirect", "c", "agosto", "outubro", "area", "legenda", 
    "esta", "unidos", "lista", "dezembro", "d", "maio", "setembro", 
    "lingua", "tem", "ate", "i", "info", "sul", "seus", "left", "nova", 
    "pais", "nas", "to", "b", "obra", "onde", "apos", "p", "name", "ela", 
    "dia", "dois", "jose", "url", "club", "r", "sem", "aos", "suas", "mapa", 
    "for", "copa", "en", "x", "jpg", "tipo", "ja", "pode", "new", "t", 
    "novo", "on", "ver", "tres", "joao", "j", "este", "l", "gol", "vez", 
    "vida", "ii", "jogo", "alem", "duas", "ter", "casa", "km", "v", "so", 
    "cor", "john", "f", "fim", "sido", "tv", "eles", "site", "qual", "y", 
    "cada", "isbn", "teve", "bem", "mar", "brab", "g", "w", "agua", "hora", 
    "ouro", "vila", "ha", "pop", "peak", "u", "kitt", "real", "h", "isso", 
    "rd1", "from", "set", "at", "dias", "luis", "rei", "liga", "num", "nbsp", 
    "el", "with", "fez", "sob", "k", "al", "rock", "by", "top", "pai", "out", 
    "time", "eram", "yes", "york", "me", "del", "san", "fase", "ex", "los", 
    "logo", "cruz", "meio", "lado"
  };

  string Path(const string& filename) {
    return kPathPrefix + filename + ".txt";
  }

  void PrintElapsedTime(const double& initial_time) {
    double seconds = (double(clock()) - initial_time) / 1000.;
    int minutes = (int)seconds / 60;
    seconds -= minutes * 60.;

    cout << "==>" << right << setw(3) << minutes << " min";
    cout << right << setw(7) << fixed << setprecision(3) << seconds << " s\n";
  }

  int ValidTitle(const string& title) {
    const string processed_title = RemoveTrailingTrash(title);

    if (processed_title[0] == '(')
      return PARENTHESIS_START;

    if (indexer::redirections.count(processed_title))
      return REDIRECTED_TITLE;

    for (const string& s : invalid_substrings)
      if (processed_title.find(s) != string::npos)
        return CONTAINS_INVALID;

    if (processed_title.find("(desambig") != string::npos)
      return DISAMBIGUATION_TITLE;

    return GOOD_TITLE;
  }

  bool AllAscii(const string& s) {
    for (char c : s)
      if (c < 33 || c > 126)
        return false;
    return true;
  }

  string NoParentheses(const string& s) {
    string ans;

    for (char c : s)
      if (c != '(' && c != ')')
        ans.push_back(c);

    return ans;
  }

  size_t CountLines(const string& filename) {
    size_t ans = 0;
    ifstream ifs(Path(filename));
    string aux;
    while (getline(ifs, aux))
      ++ans;
    return ans;
  }

  string RemoveTrailingTrash(string s) {
    while (!s.empty() && isspace(s.back()))
      s.pop_back();
    return s;
  }

  string ToFileName(int x) {
    string s = to_string(x), padding{ "000" };
    const int len = s.length();
    for (int i = 1; i < len; i++)
      padding.pop_back();
    return padding + s;
  }
}