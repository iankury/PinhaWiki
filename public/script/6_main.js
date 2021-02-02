$(document).ready(() => {
  setTimeout(() => {
    $.post('/open', res => {
      $('h2').hide()
      pinhawiki = new Phaser.Game(config)
    })
  }, 500)
})

let initial_time

function Loaded() {
  $('#search_box').on('keydown', Keydown)
  $('#search_box').show()
  $('#search_box').focus()
}

function ClearSearchbox() {
  $('#search_box').val('')
}

function DisplayResponse(res) {
  $('#results').empty()
  $('#results').append(
    $(`<p>Resultados da consulta "${query}":</p>`)
  )
  const tokens = res.split(SEPARATOR)

  for (i = 0; i < tokens.length; i += 3) {
    const link = WIKI_URL + encodeURI(tokens[i])
    $('#results').append(
      $(`<a href="${link}" target="_blank">${tokens[i]}</a>`)
    )
    $('#results').append($(`<p style="color:#1b5196">Similarity between document and query: ${tokens[i + 2]}</p>`))
    $('#results').append($(`<p>${tokens[i + 1]}</p>`))
  }
  $('#results').scrollTop(0)

  delta_time = new Date() - initial_time

  console.log(`Query time: ${delta_time}`)
}

function SendQuery(raw_data) {
  const encoded_data = encode(raw_data)
  $.get(`q/${encoded_data}`, DisplayResponse)
}

function FirstQueryRestyle() {
  virgin = false
  $('#results').css('top', mobile ? '12%' : '21%')
  if (mobile)
    return
  $('#search_box').css({
    top: '7%',
    left: '70%'
  })
  logo.setScale(scalew(.22), scaleh(.22))
  logo_text.setScale(scalew(.3), scaleh(.3))
  logo.setPosition(vw(.2), vh(.1))
  logo_text.setPosition(vw(.37), vh(.1))
  emitter.setScale({
    start: scalew(.6),
    end: scalew(.1)
  })
  emitter.setLifespan(700)
}

function Keydown(e) {
  if (e.keyCode == 13) { // Pressed enter
    initial_time = new Date()
    query = $('#search_box').val()
    if (query.length > 0 && query.length <= 50) {
      if (virgin)
        FirstQueryRestyle()
      SendQuery(query)
    }
    ClearSearchbox()
  }
}

$('#search_box').on('click', () => {
  if (mobile) {
    pinhawiki.destroy()
    $('canvas').remove()
    $('#search_box').css('top', '4%')
  }
})