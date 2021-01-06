#include "pinhawiki.h"

namespace Utility {
  const string path_prefix = "C:\\Users\\Administrator\\repos\\PinhaWikiC\\";

  unordered_map<int, int> compress_letter{
    { 128, 65 }, { 136, 69 }, { 140, 73 }, { 146, 79 }, { 153, 85 }, { 127, 65 },
    { 130, 65 }, { 148, 79 }, { 129, 65 }, { 137, 69 }, { 147, 79 }, { 134, 67 },
    { 160, 97 }, { 168, 101 }, { 172, 105 }, { 178, 111 }, { 185, 117 },
    { 159, 97 }, { 162, 97 }, { 180, 111 }, { 161, 97 }, { 169, 101 },
    { 179, 111 }, { 166, 99 }
  };

  vector<string> invalid_substrings{
    "Categoria:", "dia:", "Predefini", "Ficheiro:", "Portal:",
    "Anexo:", "pico:", "MediaWiki:", "dulo:", "Ajuda:", "Livro:"
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

  void Print_Elapsed_Time(double initial_time) {
    double seconds = (double(clock()) - initial_time) / 1000.;
    double minutes = (int)seconds / 60;
    seconds -= minutes * 60.;
    cout << "Time elapsed: " << minutes << " minutes, ";
    cout << fixed << setprecision(3) << seconds << " seconds.\n";
  }

  bool Valid_Title(const string& title) {
    for (const string& s : invalid_substrings)
      if (title.find(s) != string::npos)
        return 0;

    string s;
    stringstream ss(title);
    getline(ss, s, ' ');
    if (s.length() > 3 && s[0] == '(' && isdigit(s[1]) && s.back() == ')')
      return 0;

    return 1;
  }
}