$(document).ready(function() {
	init(SetupDate);
});

function SetupDate(dateString) {
	date = new Date(dateString);

	// set navigation links
	$('#prev-month').click(function(e) { 
		e.preventDefault();
		date.setMonth(date.getMonth() - 3);
		UpdateDate(date);
	});
	$('#next-month').click(function(e) {
		e.preventDefault();
		date.setMonth(date.getMonth() + 3);
		UpdateDate(date);
	});
	
	UpdateDate(date);
}

function UpdateDate(date) {
	var dayDateString = GetDateString(date);
	if (parameterDateString != dayDateString) {
		parameterDateString = dayDateString;
		history.pushState({date:dayDateString}, document.title, "3month.html?date=" + dayDateString);
	}
	$(".chart-link").each(function(){
	  this.href = this.href.split('?')[0] + "?date=" + dayDateString;
	});
	
	var Month3Date = new Date(date);
	Month3Date.setMonth(Month3Date.getMonth() - Month3Date.getMonth() % 3);
	var Month3End = new Date(Month3Date);
	Month3End.setMonth(Month3End.getMonth() + 2);
	
	var monthDateString = GetMonthString(Month3Date);
	var ThreeMonthString = monthDateString + " to " + GetMonthString(Month3End);
	$(".3month").text(ThreeMonthString);
	GetData(monthDateString);
}

function GetData(dateString) {
	GetTableData('json/3month/clustertable-' + dateString + '.json');
	GetGraphData('json/3month/clusterinfo-' + dateString + '.json', [7, "day"], 268);
}