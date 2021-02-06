const results_styling = `
#results_container {
  display: block;
}

#search_header {
  transform: translate(0, 0);
}

.wrapper {
  display: block;
  background: white;
}

#search_header {
  flex-direction: row;
  padding: 28px 0 28px 0;
  max-width: 900px;
}

  #search_header img {
    position: relative;
    width: 200px;
  }

#search_box {
  padding: 10px 20px;
  font-size: 18px;
  height: 40px;
  margin: 0 20% 0 4%;
}

#results_title {
  font-size: 17px;
  color: grey;
}

.result_link {
  margin: 20px 0px 10px 0px;
  font-size: 24px;
  color: #BDA22E;
  cursor: pointer;
}

  .result_link:hover {
    text-decoration: underline;
  }

.snippet {
  font-size: 19px;
  color: #575757;
  margin: 10px 0px;
}

#external_links_container, #general_info {
  display: none;
}

@media (max-width: 767px) {
  #search_header {
    flex-direction: column;
  }

    #search_header img {
      width: 170px;
      margin: 20px 0 0 0;
    }

  #search_box {
    margin: 20px 0px;
    height: 35px;
    font-size: 15px;
  }

  .snippet {
    font-size: 15px;
  }

  .result_link {
    font-size: 18px;
  }

  #results_title {
    font-size: 14px;
  }

  .similarity_label {
    font-size: 12px;
    color: grey;
    padding: 0 0 15 20;
  }
}`