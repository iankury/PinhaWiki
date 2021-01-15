function Deploy() {
  pinhawiki = new Phaser.Game(config)
}

$(document).ready(Deploy)

function Loaded() {
  $('#search_box').show()
  $('#search_box').focus()
}