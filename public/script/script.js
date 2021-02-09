const SEPARATOR = String.fromCharCode(30)
const WIKI_URL = 'https://pt.wikipedia.org/wiki/'
let virgin = true, current_query, current_page = 0
const encode = s => s.split('').map(x => x.charCodeAt(0)).join('a')

$('#logo_img').on('click', () => location.reload())

$(document).ready(() => $.post('/open', Main))

function Main() {
  $('#loading_gif').hide()
  $('#search_box').show()
  $('#search_box').focus()
  $(window).on('resize', Resize)
  $('#search_box').on('keydown', Keydown)
  $('#btn_load_more').on('click', LoadMore)
}

function Resize() {
  if (window.innerWidth < 768) {
    const ratio = window.innerHeight / window.innerWidth
    if (ratio < 1.618)
      $('#general_info').hide()
    else
      $('#general_info').show()
  }
}

function DisplayResponse(res) {
  const tokens = res.split(SEPARATOR)

  if (tokens.length < 3)
    return

  if (tokens.pop() != 'loadnomore')
    $('#load_more_container').css('display', 'flex')

  $('#results_title').text(`Results for query "${current_query}":`)

  for (i = 0; i < tokens.length; i += 3) {
    const link = WIKI_URL + encodeURI(tokens[i])
    $('#results_container').append($(`<div class="result">
      <a class="result_link" href="${link}" target="_blank">${ tokens[i]}</a>
      <div class="snippet">
        <p class="similarity_label">
          Vector space score: ${tokens[i + 2]}
        </p>
        ${tokens[i + 1]}
      </div>
    </div>`))
  }

  if (current_page > 0) 
    window.scrollBy(0, 150)
}

function SendQuery() {
  const encoded_data = encode(current_query)
  $.get(`q?page=${current_page}&query=${encoded_data}`, DisplayResponse)
}

function Keydown(e) {
  if (e.keyCode == 13) {
    current_query = $('#search_box').val()
    $('#search_box').val('')
    if (!current_query)
      return
    $('#results_container').empty()
    $('#results_title').text('')
    $('#load_more_container').css('display', 'none')
    current_page = 0
    if (virgin) {
      virgin = false
      document.querySelector('style').textContent += results_styling
      $(window).off('resize')
    }
    SendQuery()
  }
}

function LoadMore() {
  $('#load_more_container').css('display', 'none')
  ++current_page
  SendQuery()
}