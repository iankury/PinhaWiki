HTML and CSS by Maria Kury, C++ and Javascript by Ian Kury

PinhaWiki is an Information Retrieval project with the following purposes: testing/demonstrating my programming skills; practicing newly learned Information Retrieval concepts; serving as proof of concept of the Vector Space Model.

It is not a commercial project, and some key aspects of a real-world project are outside of its scope (e. g. system security).

The main source of Information Retrieval theory I've studied for this project (e. g. for weighting, modelling, indexing, scoring...) is the following ebook:

BAEZA-YATES, Ricardo; RIBEIRO-NETO, Berthier. Recuperação de Informação: Conceitos e tecnologia das máquinas de busca. 2. ed. Porto Alegre: Bookman, 2013. 590 p. ISBN 978-85-8260-049-8. E-book.

https://www.amazon.com/Recupera%C3%A7%C3%A3o-Informa%C3%A7%C3%A3o-Conceitos-Tecnologia-Portuguese-ebook/dp/B0176RV7H4

PinhaWiki is a search engine whose universe of action is strictly within the bounds of the Portuguese language version of Wikipedia (and the data used are not guaranteed to be the latest).

You can use the engine directly (making queries of up to 50 chars) here: https://pinhata.wiki.br


The project has a classic HTML-CSS-JS front-end, which communicates with a simple Node.js-Express.js backend, which in turn runs a C++ addon for the heavy computations. The heart of search is C++, which allows for efficient querying of the relatively large database, even within the constraints of the limited physical resources of a humble hosting plan.

If you want to build the whole project on your own computer:

1) Clone the repo

2) Download the raw data (XML) and save is as raw.txt

3) Preprocess the data

4) Run the server


Steps 3 and 4 consist of many parts, so I will go into a little more detail.

There is a #define for COMMAND_LINE_INTERFACE_MODE in main.cpp.
You should set this to true for all the preprocessing and for building/saving the index, and to false for compiling the addon that will be run inside the Node app.

When you compile in CLI mode, you can run the console app and use the "help" command for a description of the available commands.

When you run the command "preprocess", you are left with the following files:

1) redirections.txt - list of pairs of { title to be redirected, target tile }. It's a DAG.

2) articles.txt - heavily preprocessed text to be used for building the index.

3) original_titles.txt - non-preprocessed titles to be used for building the links

4) noxml.txt - lightly preprocessed text to be used for the snippets

You should then run the command "save_terms" in order to get terms.txt.

Then you may use "noextreme articles" in order to kill extreme (low and high) frequency terms (will make a new file; rename it to articles.txt to use it for the index).

Run "full_build" to get the index. 

Now we need to split the big files into a lot of small files (to be able to search without having everything in the RAM). You can preprocess noxml.txt a little further if you want (e. g. I just removed all words > 20 chars). The noxml file must be renamed as text.txt when you're done.
Run "split_text" to split text.txt into a lot of small files (will go into the "text" directory).
Run "split_index" to split index.txt into a lot of small files (will go into the "index" directory).

Now we're done with the preprocessing and can run the server.

You must have node (with npm) installed. 

Go to the project root and install express (npm install express) and node-gyp.

Set COMMAND_LINE_INTERFACE_MODE to false (in main.cpp).

Compile the project from the project root through node-gyp (node-gyp rebuild).

Now you can just run "node app" from the project root and open the local host in your browser to view the front-end.

Please let me know if you find something missing in my explanation: iankwen@gmail.com