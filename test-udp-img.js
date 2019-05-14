const dgram = require('dgram');
const server = dgram.createSocket('udp4');

setInterval(() => {
    let img = new Uint8Array(600);
    for (let i = 0; i < img.length; i++)
    {
        const bits = [];
        for (let j = 0; j < 8; j++) bits.push(Math.floor(Math.random() * 2));
        img[i] = parseInt(bits.join(''), 2);
    }

    server.send(img, 1234, '127.0.0.1', (err) => {
        if (err) console.log(err);
    });
}, 50);