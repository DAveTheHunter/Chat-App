const ws = new WebSocket("ws://localhost:8001");
const messageDiv = document.getElementById("messages");
const input = document.getElementById("input");
const button = document.getElementById("button");

ws.onopen = () => {
  appendMessage(`Connected to chat server👏`);
};

ws.onmessage = () => {
  appendMessage(`server : ${event.data}`);
};

ws.onerror = (error) => {
  appendMessage(`Error: ${error.message}`);
};

ws.onclose = () => {
  appendMessage("Disconnected");
};

function send(){
const message = input.value.trim();
  if (message) {
    if (message.toLowerCase() === "\q") {
      ws.close();
      return;
    }
    ws.send(message);
    input.value = "";
  }
}

button.addEventListener("click", () => send);

const appendMessage = (text) => {
  const p = document.createElement("p");
  p.textContent = text;
  messageDiv.appendChild(p);
  messageDiv.scrollTop = messageDiv.scrollHeight;
};
//send message to enter key
input.addEventListener("keypress", (e) => {
  if(e.key === 'Enter'){
    send();
  }
});
