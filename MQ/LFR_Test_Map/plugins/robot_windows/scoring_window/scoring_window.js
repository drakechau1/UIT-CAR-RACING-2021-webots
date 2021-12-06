var scoreDisplay = document.getElementById("score");
var modal = document.getElementById("myModal");
var modalMessage = document.getElementById("modalMessage");
var score = 0;

window.onload = function() {
  window.robotWindow = webots.window("scoring_window");

  // Receive messages from the controller and show them in the robot window
     // window.robotWindow.receive = (message) => {
        // document.getElementById('lbMessage').innerHTML = message;
      // };
	
	window.robotWindow.receive = function(msg) { 
		if (msg.charAt(0) == 'T') {
			msg = msg.substring(1);
			var cpElem = document.getElementById("lbTime");
			var newLabel = "Time: " + msg;
			cpElem.innerHTML = `
					<label id = "${cpElem.id}">${newLabel}</label>
					<br>
					`;
		}
		if (msg.charAt(0) == 'H') {
			var cpElem = document.getElementById("lbMessage");
			var newLabel = msg;
			cpElem.innerHTML = `
					<label id = "${cpElem.id}">${newLabel}</label>
					<br>
					`;
		}
	}
}

function messageSupervisor(msg) {
    window.robotWindow.send(msg);
}
