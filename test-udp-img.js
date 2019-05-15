const dgram = require('dgram');
const server = dgram.createSocket('udp4');

setInterval(() => {
    // let img = new Uint8Array(600);
    // for (let i = 0; i < img.length; i++)
    // {
    //     const bits = [];
    //     // for (let j = 0; j < 8; j++) bits.push(Math.random() < 0.75 ? 0 : 1);
    //     for (let j = 0; j < 8; j++) bits.push(i < 300 && i % 80 < 40 ? 1 : 0);
    //     img[i] = parseInt(bits.join(''), 2);
    // }

    let img = new Uint8Array(600);
    // let img = '';
    let i = 0;
    for (let y = 0; y < 60; y++) {
        for (let x = 0; x < 80; x += 8) {
            let thisByte = '';
            for (let j = 0; j < 8; j++) {
                if (y < 30) thisByte += '1';
                else thisByte += '0';
            }

            img[i++] = parseInt(thisByte, 2);
        }
    }
    console.log(i);

    server.send(img, 1234, '127.0.0.1', (err) => {
        if (err) console.log(err);
        console.log("Sent random image...");
    });
}, 50);