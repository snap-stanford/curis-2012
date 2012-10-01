window.addEvent("domready", init);
function init() {
	var dateString = GetParameters()["date"];
	if (!dateString) dateString = "2012-01-01"; // TODO: read from defaults file
	var date = new Date(dateString);
}

function GetParameters() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function GetTable(dateString) {
	$.getJSON('../json/daily/' + dateString + '.json', function(data) {
		$('cluster-table').clear(); //TODO: is this an actual method?
		
		var table = document.createElement("tbody");
		
		// Write the heading
		var headings = ["Rank", "Previous", "Frequency", "# Variants", "Quote"];
		var tr_heading = document.createElement("tr");
		$(tr_heading).attr("id", "heading");
		for (var i = 0; i < headings.length; ++i) {
			var td_heading = document.createElement("td");
			$(tr_heading).attr("onclick", "HeadingClicked(" + headings[i] + ")");
			$(td_heading).text("<b>" + headings[i] + "</b>")
			$(tr_heading).append(td_heading);
		}
		$(table).append(tr_heading);
		
		// Write everything else
		for (var i = 0; i < data.label.length; ++i) {
			var tr = document.createElement("tr");
            $(tr).attr("id", data.labels[i]);
            $(tr).append("<td>" + (i + 1) + "</td>") // rank
            $(tr).append("<td>" + data.previous[i] + "</td>") // previous
            $(tr).append("<td>" + data.frequency[i] + "</td>") // frequency
            $(tr).append("<td>" + data.numvariants[i] + "</td>") // variants
            $(tr).append("<td><a href=\"cluster.html?id=" + data.quote[i] + "\">" + data.quote[i] + "</a></td>") // quote
            $(table).append(tr);
        }
		
		$('.cluster-table').replaceWith(table);
	});
}

function HeadingClicked(heading) {
	alert(heading);
}