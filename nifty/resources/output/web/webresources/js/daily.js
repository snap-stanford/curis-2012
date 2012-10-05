$(document).ready(function() {
	var dateString = GetParameters()["date"];
	if (!dateString) {
		$.ajax({
            url: 'currentdate.txt',
            type: 'GET',
            dataType: 'text',
            success: function(data) {
            	SetupDate(data);
            },
            error: function(xhr, textStatus, errorThrown) {
                SetupDate("2012-01-01");
            }
        });
	} else {
		SetupDate(dateString)
	}
});

function PostError(message) {
	$('#error-log').html("Error: " + message);
	$('#error-log').show();
}

function SetupDate(dateString) {
	$(".today").text(dateString);
	date = new Date(dateString);
	
	// set navigation links
	$('#prev-day').click(function(e) { 
		e.preventDefault();
		date.setDate(date.getDate() - 1);
		var dateString = GetDateString(date);
		$(".today").text(dateString);
		GetTable(dateString);
	});
	$('#next-day').click(function(e) { 
		e.preventDefault();
		date.setDate(date.getDate() + 1);
		var dateString = GetDateString(date);
		$(".today").text(dateString);
		GetTable(dateString);
	});
	
	GetTable(dateString);
}

function GetParameters() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function GetDateString(date) {
	return date.getUTCFullYear() + "-" + ("0" + (date.getUTCMonth() + 1)).slice(-2) + "-" + ("0" + date.getUTCDate()).slice(-2);
}

function GetTable(dateString) {
	history.pushState({date:dateString}, document.title, "daily.html?date=" + dateString);
	$.getJSON('json/daily/' + dateString + '.json', function(data) {
		$("#error-log").hide();
		var table = document.createElement("tbody");
		
		// Write the heading
		var headings = ["Rank", "Previous", "Frequency", "#Variants", "Quote"];
		var tr_heading = document.createElement("tr");
		$(tr_heading).attr("id", "heading");
		for (var i = 0; i < headings.length; ++i) {
			var td_heading = document.createElement("td");
			$(tr_heading).click(function() { HeadingClicked(headings[i]); });
			$(td_heading).html("<b>" + headings[i] + "</b>")
			$(tr_heading).append(td_heading);
		}
		$(table).append(tr_heading);
		
		// Write everything else
		for (var i = 0; i < data.label.length; ++i) {
			var tr = document.createElement("tr");
            $(tr).attr("id", data.label[i]);
            $(tr).append("<td>" + (i + 1) + "</td>") // rank
            $(tr).append("<td>" + data.prev[i] + "</td>") // previous
            $(tr).append("<td>" + data.frequency[i] + "</td>") // frequency
            $(tr).append("<td>" + data.numvariants[i] + "</td>") // variants
            $(tr).append("<td><a href=\"cluster.html?date=" + dateString + "&id=" + data.label[i] + "\">" + data.quote[i] + "</a></td>") // quote
            $(table).append(tr);
        }
		
		$('#cluster-table').html(table);
	})
	.error(function() { 
		PostError("Table information either does not exist for given date or cannot be obtained at this time.");
	});
}

function HeadingClicked(heading) {
	alert(heading);
}