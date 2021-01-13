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

function SendQuery(data_to_send) {
  $.get(`q/${data_to_send}`, DisplayResponse)
}

function Keydown(e) {
  if (e.keyCode == 13) {
    SendQuery($('#search_box').val())
    ClearSearchbox()
  }
}