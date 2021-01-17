const cpp_addon = require('./build/Release/pinhawiki')
const express = require('express'), app = express()

const decode = s => s.split('a').map(x => String.fromCharCode(x)).join('')

app.use(express.static('public'))

app.listen(process.env.PORT || 3000, () => console.log('PinhaWiki stirs...'))

cpp_addon.HandleClientRequest('i')

app.get('/q/:data', (req, res) => {
  const decoded_data = decode(req.params.data)
  res.status(201).send(cpp_addon.HandleClientRequest(`${decoded_data}q`))
})