const dgram = require('dgram');
const server = dgram.createSocket('udp4');

function sendData(address)
{
    const packet =
`epabcdabcdeepefghefghi`;

    console.log(packet);
    server.send(packet, 1235, address, (err) => {
        if (err) console.error(err);
        else console.log('ebig pagket sent');
    });
}

module.exports = sendData;