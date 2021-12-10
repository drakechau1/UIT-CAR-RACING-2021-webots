var tabs = document.querySelectorAll(".lboard_tabs .tabs .button");
var sinhvien = document.querySelector(".sinhvien");
var thpt = document.querySelector(".thpt");
var items = document.querySelectorAll(".lboard_item");

tabs.forEach(function(tab){
	tab.addEventListener("click", function(){
		var currenttab = tab.getAttribute("data-div");
		
		tabs.forEach(function(tab){
			tab.classList.remove("active");
		})

		tab.classList.add("active");

		items.forEach(function(item){
			item.style.display = "none";
		})

		if(currenttab == "sinhvien"){
			sinhvien.style.display = "block";
		}
		else if(currenttab == "thpt"){
			thpt.style.display = "block";
		}

	})
})
