const SEPARATOR = String.fromCharCode(30)
const WIKI_URL = 'https://pt.wikipedia.org/wiki/'
let last_query, virgin = true
const encode = s => s.split('').map(x => x.charCodeAt(0)).join('a')

$(document).ready(Load)

function Load() {
  $.post('/open', Main)
}

function Main() {
  $('#loading_gif').hide()
  $('#search_box').show()
  $('#search_box').focus()
  $('#search_box').on('keydown', Keydown)
}

function DisplayResponse(res) {
  $('#results_container').empty()
  const tokens = res.split(SEPARATOR)

  for (i = 0; i < tokens.length; i += 3) {
    const link = WIKI_URL + encodeURI(tokens[i])
    $('#results_container').append($(`<div class="result">
      <a class="result_link" href="${link}" target="_blank">${ tokens[i]}</a>
      <div class="snippet">
        <p class="similarity_label">
          Semelhança entre documento e consulta: ${tokens[i + 2]}
        </p>
        ${tokens[i + 1]}
      </div>
    </div>`))
  }
  $('#results_container').scrollTop(0)
}

function SendQuery() {
  const encoded_data = encode(last_query)
  $.get(`q/${encoded_data}`, DisplayResponse)
}

function Keydown(e) {
  if (e.keyCode == 13) {
    last_query = $('#search_box').val()
    $('#search_box').val('')
    if (!last_query)
      return
    $('#results_title').text(`Resultados da consulta "${last_query}":`)
    if (virgin) {
      virgin = false
      document.querySelector('style').textContent += results_styling
    }
    SendQuery()
  }
}