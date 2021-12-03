const stopwatch_time = document.getElementById("stopwatch-time");
const newCheckPoint = document.getElementById("generate-checkpoint");
const result_score = document.getElementById("result-score");
const numberOfCheckpoint = 8;
const message = "C.02.12.45";
var totalScrore = 0;
var finishTime = "";

const watch = new Stopwatch(stopwatch_time);

// Window onload
window.onload = function () {
	//window.robotWindow = webots.window("scoring_window");

	// window.robotWindow.receive = function (msg) {
	// 	// Message handler
	// }
}

function HandleMessage(msg) {
	console.log("Handle message");
	let arr = message.split('.');
	let info = "checkpoint: " + arr[1] + "\tTime: " + arr[2] + ":" + arr[3];
	console.log(info);
	return arr;
}

// Auto generate check point
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
		cp_time.innerHTML = "--:--.---";

		checkpoint.appendChild(cp_name);
		checkpoint.appendChild(cp_time);
		newCheckPoint.appendChild(checkpoint);
	}
}
GenerateCheckpoint();
function Start() {
	console.log("START");
	watch.start();
	// window.robotWindow.send('1');
}
function Stop() {
	console.log("STOP");
	watch.stop();
	// window.robotWindow.send('2');
}
function Reset() {
	console.log("RESET");
	watch.reset();
	ResetCheckpoint();
	totalScrore = 0;
	result_score.innerHTML = totalScrore;
	// window.robotWindow.send('3');
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
	if (cp_index <= numberOfCheckpoint) {
		totalScrore++;
		SetCheckPointPassed(cp_index);
		if (cp_index == numberOfCheckpoint) {
			console.log("DONE");
			watch.stop();
		}
		result_score.innerHTML = totalScrore;
		cp_index++;
	}
}

function SetCheckPointPassed(index) {
	if (index > numberOfCheckpoint) return;
	// update status
	var id = "cp" + index;
	document.getElementById(id).setAttribute("class", "checkpoint pass");
	// update time
	var time = stopwatch_time.innerHTML;
	var time_id = "checkpoin-time" + index;
	document.getElementById(time_id).innerHTML = time;
}

function ResetCheckpoint() {
	for (var index = 1; index <= numberOfCheckpoint; index++) {
		// update status
		var id = "cp" + index;
		document.getElementById(id).setAttribute("class", "checkpoint not-pass");
		// update time
		var time = "--:--.---";
		var time_id = "checkpoin-time" + index;
		document.getElementById(time_id).innerHTML = time;
	}
}