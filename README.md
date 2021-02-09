PinhaWiki is an Information Retrieval project with the following purposes: testing/demonstrating my programming skills; practicing newly learned Information Retrieval concepts; serving as proof of concept of the Vector Space Model.

The main source of Information Retrieval theory I've studied for this project (e. g. for weighting, modelling, indexing, scoring...) is the following ebook:

BAEZA-YATES, Ricardo; RIBEIRO-NETO, Berthier. Recuperação de Informação: Conceitos e tecnologia das máquinas de busca. 2. ed. Porto Alegre: Bookman, 2013. 590 p. ISBN 978-85-8260-049-8. E-book.

https://www.amazon.com/Recupera%C3%A7%C3%A3o-Informa%C3%A7%C3%A3o-Conceitos-Tecnologia-Portuguese-ebook/dp/B0176RV7H4

PinhaWiki is a search engine whose universe of action is strictly within the bounds of the Portuguese language version of Wikipedia.

You can use the engine directly (making queries of up to 50 chars) here: https://pinhata.wiki.br


The project has a classic HTML/CSS/JS front-end, which communicates with a simple Node.js/Express.js back-end, which in turn runs a C++ addon for the heavy computations. The heart of search is C++, which allows for efficient querying of the relatively large database, even within the constraints of the limited physical resources of a humble hosting plan.

If you want to build the whole project on your own computer:

1) Download and install node (with npm), express and node-gyp

2) Clone the repo

3) Download the raw data (XML) and save it as raw.txt (in the root)

4) Use the CLI to preprocess the data (set COMMAND_LINE_INTERFACE_MODE to true in main.cpp, then compile C++ sources and run the command "preprocess")

5) Set COMMAND_LINE_INTERFACE_MODE to false in main.cpp, compile C++ sources using node-gyp (with command "node-gyp rebuild")

6) Run the js app with "node app" and go to localhost in the browser