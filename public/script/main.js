function ClearSearchbox() {
  $('#search_box').val('')
}

function DisplayResponse(res) {
  $('#results').empty()
  res.split(SEPARATOR).forEach(x => {
    const link = WIKI_URL + encodeURI(x)
    $('#results').append(
      $(`<a href="${link}" target="_blank">${x}</a>`)
    )
  }) 
}

function SendQuery(raw_data) {
  const encoded_data = encode(raw_data)
  $.get(`q/${encoded_data}`, DisplayResponse)
}

function Keydown(e) {
  if (e.keyCode == 13) {
    SendQuery($('#search_box').val())
    ClearSearchbox()
  }
}