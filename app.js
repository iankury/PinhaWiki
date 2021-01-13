const cpp_addon = require('./build/Release/pinhawiki')
const express = require('express'), app = express()

app.use(express.static('public'))

app.listen(process.env.PORT || 3000, () => console.log('PinhaWiki stirs...'))

cpp_addon.HandleClientRequest('i')

app.get('/q/:data', (req, res) => res.status(201).send(
  cpp_addon.HandleClientRequest(`${req.params.data}q`)
))