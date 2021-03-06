const cpp_addon = require('./build/Release/pinhawiki')
const express = require('express'), app = express()
const SEPARATOR = String.fromCharCode(30)
const over_limit_msg = `Error${SEPARATOR}Maximum query size exceeded${SEPARATOR}0.00`

function Exit(s) {
  console.log(s)
  process.exit(1)
}

const decode = s => s.split('a').map(x => String.fromCharCode(x)).join('')

let addon_running = false
let addon_ready = false

app.use(express.static('public'))

app.listen(process.env.PORT || 3000, () => {
  console.log('PinhaWiki stirs...')
})

function LoadAddon() {
  if (!addon_running) {
    addon_running = true
    try {
      const res = cpp_addon.HandleClientRequest('i')
      if (res == 'loaded') {
        console.log('Addon finished loading')
        addon_ready = true
      }
      else
        Exit(`Addon couldn't load`)
    }
    catch {
      Exit('Error loading addon')
    }
  }
}

app.post('/open', (req, res) => {
  LoadAddon()
  res.sendStatus(201)
})

app.get('/q?', (req, res) => {
  const page = req.query.page
  const query = req.query.query

  if (!addon_ready) {
    console.log(`Trying to access addon when it's not ready`)
    return
  }

  if (query.length > 500) {
    res.status(201).send(over_limit_msg)
    return
  }

  const decoded_data = decode(query)
  if (decoded_data.length > 50) {
    res.status(201).send(over_limit_msg)
    return
  }

  addon_ready = false

  try {
    const addon_answer = cpp_addon.HandleClientRequest(`${decoded_data}p${page}q`)
    res.status(201).send(addon_answer)
    addon_ready = true
  }
  catch {
    Exit(`Error getting addon answer for ${decoded_data}`)
  }
})