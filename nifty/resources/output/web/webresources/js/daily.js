$(document).ready(function() {
	init(SetupDate);
});

function SetupDate(dateString) {
	date = new Date(dateString);

	// set navigation links
	$('#prev-day').click(function(e) { 
		e.preventDefault();
		date.setDate(date.getDate() - 1);
		UpdateDate(date);
	});
	$('#next-day').click(function(e) { 
		e.preventDefault();
		date.setDate(date.getDate() + 1);
		UpdateDate(date);
	});
	
	UpdateDate(date);
}

function UpdateDate(date) {
	var dayDateString = GetDateString(date);
	if (parameterDateString != dayDateString) {
		parameterDateString = dayDateString;
		history.pushState({date:dayDateString}, document.title, "daily.html?date=" + dayDateString);
	}
	$(".chart-link").each(function(){
	  this.href = this.href.split('?')[0] + "?date=" + dayDateString;
	});
	$(".today").text(dayDateString);
	GetTableData('json/daily/' + dayDateString + '.json');
	GetData(dayDateString);
}

function HeadingClicked(heading) {
	alert(heading);
}