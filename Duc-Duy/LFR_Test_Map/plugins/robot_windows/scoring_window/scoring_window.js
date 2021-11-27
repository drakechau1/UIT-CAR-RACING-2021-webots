const stopwatch_time = document.getElementById("stopwatch-time");
const newCheckPoint = document.getElementById("generate-checkpoint");
const numberOfCheckpoint = 5;


window.onload = function () {
	window.robotWindow = webots.window("scoring_window");

	window.robotWindow.receive = function (msg) {
		if (msg.charAt(0) == 'T') {

		}
		if (msg.charAt(0) == 'H') {

		}
	}
}

function messageSupervisor(msg) {
	window.robotWindow.send(msg);
}

function Start() {
	console.log("START");
}

function Stop() {
	console.log("STOP");
}

function Reset() {
	console.log("RESET");
}

function LoadPreviousTeam() {
	console.log("Previous teams");
}

function LoadNextTeam() {
	console.log("Next teams");
}

function SaveResult() {
	console.log("Save result");
}

var cp_index = 1;
function PassedDemo() {
	SetCheckPointPassed(cp_index);
	cp_index += 1;
}

function SetCheckPointPassed(index) {
	if (index > numberOfCheckpoint) return;
	// update status
	var id = "cp" + index;
	document.getElementById(id).setAttribute("class", "checkpoint pass");
	// update time
	var today = new Date();
	var time = today.getMinutes() + ":" + today.getSeconds() + ":" + today.getMilliseconds();
	var time_id = "checkpoin-time" + index;
	document.getElementById(time_id).innerHTML = time;
}

function GenerateCheckpoint() {
	for (var i = 1; i <= numberOfCheckpoint; i++) {
		var checkpoint = document.createElement("div");
		checkpoint.setAttribute("class", "checkpoint not-pass");
		var cp_id = "cp" + i;
		checkpoint.setAttribute("id", cp_id)

		var cp_name = document.createElement("h2");
		cp_name.innerHTML = "CHECK POINT " + i;

		var cp_time = document.createElement("h2");
		var time_id = "checkpoin-time" + i;
		cp_time.setAttribute("id", time_id);
		cp_time.innerHTML = "--:--:--";

		checkpoint.appendChild(cp_name);
		checkpoint.appendChild(cp_time);
		newCheckPoint.appendChild(checkpoint);
	}
}

GenerateCheckpoint();