$(document).ready(function() {
	curDateString = GetParameters()["date"];
	if (curDateString) {
		curDate = new Date(curDateString);
	}
	clusterID = GetParameters()["id"];
	if (!clusterID) {
		PostError("Please specify a cluster ID.");
	} else {
		SetupCluster(clusterID);
	}
});

function GetParameters() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function PostError(message) {
	$('#error-log').html("<br />Error: " + message);
	$('#contents').hide();
}

function SetupCluster(clusterID) {
	var firstIndex = ~~(clusterID / 10000);
	var secondIndex = ~~(firstIndex / 1000);
	$.getJSON('json/clusters/' + secondIndex + '/' + firstIndex + '/' + clusterID + '.json', function(data) {
		// #### QUOTE
		$('.quote-container').text(data.quote[0]);
		// #### TABLE
		var table = document.createElement("tbody");
		
		// Write the heading
		var headings = ["Frequency", "Variant"];
		var tr_heading = document.createElement("tr");
		$(tr_heading).attr("id", "heading");
		for (var i = 0; i < headings.length; ++i) {
			var td_heading = document.createElement("td");
			$(td_heading).html("<b>" + headings[i] + "</b>")
			$(tr_heading).append(td_heading);
		}
		$(table).append(tr_heading);
		
		// Write everything else
		for (var i = 0; i < data.urls.length; ++i) {
			var tr = document.createElement("tr");
            $(tr).append("<td>" + data.frequencies[i] + "</td>") // rank
            $(tr).append("<td><a href=\"" + data.urls[i] + "\">" + data.quotes[i] + "</a></td>") // quote
            $(table).append(tr);
        }
		
		$('#cluster-table').html(table);
		
		// ### SETUP GRAPH TIME
		var originalDate = new Date(data.modified);
		if (curDateString && curDate > originalDate) {
			$('.status-message').text("This cluster has ended its lifecycle and is no longer being updated!")
		}
		originalDate.setDate(originalDate.getDate() + 1);
		for (var i = 0; i < data.peak.length; i++) {
			var newDate = new Date(originalDate);
			newDate.setHours(newDate.getHours() + data.peak[i][0]);
			data.peak[i][0] = newDate.getTime();
		}
		for (var i = 0; i < data.plot.length; i++) {
			var newDate = new Date(originalDate);
			newDate.setHours(newDate.getHours() + data.plot[i][0]);
			data.plot[i][0] = newDate.getTime();
		}
		
		
		// ### GRAPH
		var options = {
	        yaxis: { axisLabel: "Frequency of Cluster", axisLabelUseCanvas: true, min: 0 },
	        xaxis: { axisLabelUseCanvas: true, mode: "time", timeformat: "%m/%d", tickSize: [1, "day"]},
	        legend: { position: "nw" }
	    };
		var graphData = [{
		    label: "plot", 
		    data: data.plot, 
		    lines: { show: true, fill: true },
			color: "rgb(202, 164, 252)"
		}, { 
			label: "peaks", 
			data: data.peak, 
			points: {show: true},
			color: "rgb(255, 61, 184)"
		}];
	    var graphContainer = $("#cluster-graph");
	    $.plot(graphContainer, graphData, options);
	})
	.error(function() { 
		PostError("Cluster information either does not exist or cannot be obtained at this time.");
	});
}