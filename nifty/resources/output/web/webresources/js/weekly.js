$(document).ready(function() {
	init(SetupDate);
});

function SetupDate(dateString) {
	date = new Date(dateString);

	// set navigation links
	$('#prev-week').click(function(e) { 
		e.preventDefault();
		date.setDate(date.getDate() - 7);
		UpdateDate(date);
	});
	$('#next-week').click(function(e) { 
		e.preventDefault();
		date.setDate(date.getDate() + 7);
		UpdateDate(date);
	});
	
	UpdateDate(date);
}

function UpdateDate(date) {
	var dayDateString = GetDateString(date);
	if (parameterDateString != dayDateString) {
		parameterDateString = dayDateString;
		history.pushState({date:dayDateString}, document.title, "weekly.html?date=" + dayDateString);
	}
	$(".chart-link").each(function(){
	  this.href = this.href.split('?')[0] + "?date=" + dayDateString;
	});
	
	var targetDate = 2; // CHANGE TO WHATEVER IT ENDS UP BEING!
	var dateBegin = new Date(date);
	var difference = (dateBegin.getUTCDay() + 7 - targetDate) % 7;
	dateBegin.setDate(dateBegin.getDate() - difference);
	var dateEnd = new Date(dateBegin);
	dateEnd.setDate(dateBegin.getDate() + 6);
	var weekDateString = GetDateString(dateBegin) + " to " + GetDateString(dateEnd);
	$(".week").text(weekDateString);
	GetData(GetDateString(dateBegin));
}

function GetData(dateString) {
	GetTableData('json/week/clustertable-' + dateString + '.json');
	GetGraphData('json/week/clusterinfo-' + dateString + '.json', [1, "day"], 24);
}