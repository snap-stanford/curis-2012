$(document).ready(function() {
	init(SetupDate);
});

function SetupDate(dateString) {
	date = new Date(dateString);

	// set navigation links
	$('#prev-month').click(function(e) { 
		e.preventDefault();
		date.setMonth(date.getMonth() - 1);
		UpdateDate(date);
	});
	$('#next-month').click(function(e) { 
		e.preventDefault();
		date.setMonth(date.getMonth() + 1);
		UpdateDate(date);
	});
	
	UpdateDate(date);
}

function UpdateDate(date) {
	var dayDateString = GetDateString(date);
	if (parameterDateString != dayDateString) {
		parameterDateString = dayDateString;
		history.pushState({date:dayDateString}, document.title, "monthly.html?date=" + dayDateString);
	}
	$(".chart-link").each(function(){
	  this.href = this.href.split('?')[0] + "?date=" + dayDateString;
	});
	
	var monthDateString = GetMonthString(date);
	$(".month").text(monthDateString);
	GetData(monthDateString);
}

function GetData(dateString) {
	GetTableData('json/month/clustertable-' + dateString + '.json');
	GetGraphData('json/month/clusterinfo-' + dateString + '.json', [2, "day"], 84);
}