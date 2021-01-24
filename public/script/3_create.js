function Create() {
  this.physics.world.setBounds(0, vh(.015), vw(1), vh(1))

  //   ============================================
  //   =================   Logo   =================
  //   ============================================

  const logo_initial_x = mobile ? vw(.18) : vw(.08)
  const logo_initial_y = mobile ? vh(.85) : vh(.3)
  
  logo = this.physics.add.image(
    logo_initial_x, 
    logo_initial_y,
    'logo'
  )

  if (mobile) {
    logo.setScale(scalew(.15), scaleh(.15))
    logo.setDrag(0, vh(.4))
    logo.setGravity(0, -vh(1))
    logo.setVelocity(0, -vh(1))
    logo.setBounce(0, .2)
  }
  else {
    logo.setScale(scalew(.3), scaleh(.3))
    logo.setDrag(vw(.2), 0)
    logo.setVelocity(vw(1), 0)
    logo.setBounce(.8, 0)
  }
  
  logo.setCollideWorldBounds(true)
  logo.body.onWorldBounds = true
  logo.body.onOverlap = true
  logo.setDepth(2)

  //   =================================================
  //   =================   Logo Text   =================
  //   =================================================

  const logo_text_initial_x = mobile ? vw(.6) : vw(.55)
  const logo_text_initial_y = mobile ? vh(.07) : vh(.3)

  logo_text = this.physics.add.image(
    logo_text_initial_x,
    logo_text_initial_y,
    'logo_text'
  )
  logo_text.setVisible(false)

  if (mobile) {
    logo_text.setScale(scalew(.18), scaleh(.18))
  }
  else {
    logo_text.setScale(scalew(.42), scaleh(.42))
  }

  //logo.setVisible(false)

  //   ===================================================
  //   =================   Fire Emitter  =================
  //   ===================================================

  const particles = this.add.particles('fire')


  emitter = particles.createEmitter({
    scale: {
      start: mobile ? scalew(.2) : scalew(.5),
      end: mobile ? scalew(1.5) : scalew(2.5)
    },
    speed: 150,
    lifespan: 500,
    blendMode: 'ADD'
  })

  emitter.startFollow(logo)

  setTimeout(() => {
    emitter.stop()
    emitter.setScale({
      start: scalew(.4),
      end: scalew(.7)
    })
    emitter.setSpeed(100)
    emitter.setLifespan(mobile ? 500 : 600)
    emitter.start()
    logo_text.setVisible(true)
    Loaded()
  }, mobile ? 500 : 2000)

  //   ===========================================================
  //   =================   Landscape Collision   =================
  //   ===========================================================

  if (!mobile) {
    const zone = this.add.zone(vw(0.31), 100).setSize(vw(0.02), 600)
    this.physics.world.enable(zone)
    zone.body.setAllowGravity(false)
    zone.body.moves = false
    logo.body.onOverlap = true
    this.physics.add.overlap(logo, zone)

    this.physics.world.once('worldbounds', () => {
      this.physics.world.once('overlap', () => {
        logo.setDrag(vw(2), 0)
      })
    })
  }
}