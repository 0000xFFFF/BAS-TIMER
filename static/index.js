var socket = io.connect("http://" + document.domain + ":" + location.port);

// Handle fetched data updates
socket.on("vars", function(data) {
    document.getElementById("currents").innerText = JSON.stringify(data, null, 2);
});
