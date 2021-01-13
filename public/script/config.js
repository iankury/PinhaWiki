const config = {
  type: Phaser.AUTO,
  render: { transparent: true },
  width: vw(1),
  height: vh(1),
  scale: {
    mode: Phaser.Scale.RESIZE,
    autoCenter: Phaser.Scale.CENTER_BOTH
  },
  physics: {
    default: 'arcade',
    arcade: {
      debug: false
    }
  },
  scene: {
    preload: Preload,
    create: Create,
    update: Update
  }
};