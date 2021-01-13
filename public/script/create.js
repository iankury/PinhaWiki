function Create() {
  this.physics.world.setBounds(0, 0, vw(1), vh(1))

  const logo = this.physics.add.image(500, 300, 'logo')

  logo.setOrigin(0, 0)
  logo.setPosition(vw(0.01), vh(0.01))
  logo.setScale(scalew(0.3), scaleh(0.3))
  logo.setDrag(vw(0.2), 0)  
  logo.setVelocity(vw(0.8), 0)
  logo.setBounce(0.8, 0)
  logo.setCollideWorldBounds(true)
  logo.body.onWorldBounds = true
  logo.body.onOverlap = true
  logo.setDepth(2)

  const particles = this.add.particles('fire')

  let emitter = particles.createEmitter({
    scale: { start: 0, end: scalew(2) },
    speed: 100,
    lifespan: 400,
    blendMode: 'ADD'
  })

  emitter.startFollow(logo)
  emitter.followOffset.set(0.025 * logo.width, 0.15 * logo.height)

  setTimeout(() => {
    emitter.stop()
  }, 1650)

  setTimeout(() => {
    emitter.setScale({ start: scalew(0.7), end: scalew(0.2) })
    emitter.setSpeed(60)
    emitter.setLifespan(900)
    emitter.start()
  }, 2000)

  const zone = this.add.zone(vw(0.5) - 0.15 * logo.width, 100).setSize(vw(0.02), 200)
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