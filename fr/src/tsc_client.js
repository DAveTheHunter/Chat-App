const net = require('net');
const webSocket = require('ws');

const ws_port = 8001;
const host = '127.0.0.1';
const tcp_port = 8000;

const wss = new webSocket.Server({ port : ws_port });

wss.on('connection', (ws) => {
  console.log(`Web socket connected`);
  const tcp_client = new net.Socket();
  tcp_client.setEncoding('utf-8');

  tcp_client.connect(tcp_port, host, () => {
    console.log('connected to tcp Server');
  })
  ws.on('message', (message) => {
     const msg = message.toString().trim();
    if (msg) {
       tcp_client.write(`${msg}\n`);
    }
  })
  ws.on('close', () => {
    console.log('Disconnected');
    tcp_client.end();
  })
   ws.on('error', (err) => {
    console.log(`Error : ${err.message}`);
    tcp_client.end();
  })
  tcp_client.on('data', (data) => {
    ws.send(data.trim());
  })
  tcp_client.on('error', (err) => {
    console.log(`TCP : ${err.message}`);
    ws.close();
  })
  tcp_client.on('close', () => {
     console.log('disconnected from tcp');
    ws.close();
  })
})

console.log(`Web socket listening on ${ws_port}`);
