let pinhawiki

const vh = x => x * window.innerHeight
const vw = x => x * window.innerWidth

const scaleh = x => x * vh(1) / 730
const scalew = x => x * vw(1) / 1536

const SEPARATOR = String.fromCharCode(30)
const WIKI_URL = 'https://pt.wikipedia.org/wiki/'

const encode = s => s.split('').map(x => x.charCodeAt(0)).join('a')