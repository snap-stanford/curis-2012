$(document).ready(function() {
	init(SetupDate);
	SetInterpolationWindow(4);
});

function SetupDate(dateString) {
	daysPerMonth = [30, 27, 30, 29, 30, 29, 30, 30, 29, 30, 29, 30, 30, 27, 30, 29, 30, 29, 30, 30, 29, 30, 29, 30, 30]; // extra for january
	date = new Date(dateString);

	// set navigation links
	$('#prev-month').click(function(e) { 
		e.preventDefault();
		if (daysPerMonth[date.getMonth() + 11] <= date.getDate())
			date.setDate(daysPerMonth[date.getMonth() + 11]);
		date.setMonth(date.getMonth() - 1);
		UpdateDate(date);
	});
	$('#next-month').click(function(e) { 
		e.preventDefault();
		if (daysPerMonth[date.getMonth() + 13] <= date.getDate())
			date.setDate(daysPerMonth[date.getMonth() + 13]);
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