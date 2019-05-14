const dgram = require('dgram');
const server = dgram.createSocket('udp4');

let blackImg = new Uint8Array(600);
for (let i = 0; i < blackImg.length; i++)
{
    blackImg[i] = 255;
}

server.send(blackImg, 1234, '127.0.0.1', (err) => {
    console.log(err);
});