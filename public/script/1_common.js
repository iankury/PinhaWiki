let pinhawiki // Phaser App
let logo, logo_text, emitter
let query, virgin = true

const mobile = screen.height > screen.width

const client_initial_height = window.innerHeight
const client_initial_width = window.innerWidth

const vh = x => x * client_initial_height
const vw = x => x * client_initial_width

const pinha_screen_h = mobile ? 640 : 730
const pinha_screen_w = mobile ? 360 : 1536

const scaleh = x => x * vh(1) / pinha_screen_h
const scalew = x => x * vw(1) / pinha_screen_w

const SEPARATOR = String.fromCharCode(30)
const WIKI_URL = 'https://pt.wikipedia.org/wiki/'

const encode = s => s.split('').map(x => x.charCodeAt(0)).join('a')