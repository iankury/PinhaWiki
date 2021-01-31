$(document).ready(() => {
  pinhawiki = new Phaser.Game(config)
})

let initial_time

function Loaded() {
  $('#search_box').on('keydown', Keydown)
  $('#search_box').show()
  $('#search_box').focus()
  $.post('/open')
}

function ClearSearchbox() {
  $('#search_box').val('')
}

function DisplayResponse(res) {
  $('#results').empty()
  $('#results').append(
    $(`<p>Resultados da consulta "${query}":</p>`)
  )
  res.split(SEPARATOR).forEach((x, i) => {
    if (i % 2) { // Score
      $('#results').append(
        (x == '99000') ? $(`<p>Perfect match</p>`)
        : $(`<p>Vector space score was ${x}</p>`)
      )
    }
    else { // Article name
      const link = WIKI_URL + encodeURI(x)
      $('#results').append(
        $(`<a href="${link}" target="_blank">${x}</a>`)
      )
    }
  }) 
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