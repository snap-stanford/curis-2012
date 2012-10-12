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
	history.pushState({date:dateString}, document.title, "monthly.html?date=" + dateString);
	date = new Date(dateString);
	dateString = GetMonthString(date); // make standard format
	$(".month").text(dateString);
	
	
	// set navigation links
	$('#prev-month').click(function(e) { 
		e.preventDefault();
		date.setMonth(date.getMonth() - 1);
		var dateString = GetMonthString(date);
		$(".month").text(dateString);
		GetData(dateString);
	});
	$('#next-month').click(function(e) { 
		e.preventDefault();
		date.setMonth(date.getMonth() + 1);
		var dateString = GetMonthString(date);
		$(".month").text(dateString);
		GetData(dateString);
	});
	
	GetData(dateString);
}

function GetParameters() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function GetMonthString(date) {
	return date.getUTCFullYear() + "-" + ("0" + (date.getUTCMonth() + 1)).slice(-2);
}

function GetData(dateString) {
	GetTableData(dateString);
	GetGraphData(dateString);
}

function GetTableData(dateString) {
	$.getJSON('json/monthly/clustertable-' + dateString + '.json', function(data) {
		// rewire data
		tableData = [];
		for (var i = 0; i < data.label.length; ++i) {
			tableData.push({
				label: data.label[i],
				frequency: data.frequency[i],
				numvariants: data.numvariants[i],
				quote: data.quote[i],
				rank: (i + 1)
			})
		}
		$("#table-error-log").hide();
		PrintTable(0, tableData.length);
		
	})
	.error(function() { 
		$('#table-error-log').html("Error: Table information either does not exist for given month or cannot be obtained at this time.");
		$('#table-error-log').show();
	});
}

function PrintData(start, end, allowClicking) {
	if (!graphData || !tableData) return;
	allClustersShown = allowClicking;
	curStart = start, curEnd = end;
	PrintTable(start, end);
	PrintGraph(start, end);
}

// inclusive, exclusive
function PrintTable(start, end) {
	var table = document.createElement("tbody");
	
	// Write the heading
	var headings = ["Rank", "Frequency", "#Variants", "Quote"];
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
	for (var i = start; i < tableData.length && i < end; ++i) {
		var tr = document.createElement("tr");
        $(tr).attr("id", tableData[i].label);
        $(tr).append("<td>" + tableData[i].rank + "</td>") // rank
        $(tr).append("<td>" + tableData[i].frequency + "</td>") // frequency
        $(tr).append("<td>" + tableData[i].numvariants + "</td>") // variants
        $(tr).append("<td><a href=\"cluster.html?id=" + tableData[i].label + "\">" + tableData[i].quote + "</a></td>") // quote
        $(table).append(tr);
    }
	
	$('#cluster-table').html(table);
}

function PrintGraph(start, end) {
	$("#tooltip").remove();
	if (!graphData) return;
	if (start != 0 || end != graphData.length) {
		var curGraph = [];
		for (var i = start; i < end; i++) {
			curGraph.push(graphData[i]);
		}
		$.plot(graphContainer, curGraph, options);
	} else {
		$.plot(graphContainer, graphData, options);
	}
	
}

function GetGraphData(dateString) {
	$.getJSON('json/monthly/clusterinfo-' + dateString + '.json', function(data) {
		allClustersShown = true;
		$('#graph-error-log').hide();
		
		// ### ITERATE THROUGH VALUES
		var dataPoints = [];
		for (var i = 0; i < data.label.length; i++) {
			dataPoints.push([]);
		}
		
		dates = [];		
		for (var i = 0; i < data.values.length; i++) {
			var curDate = new Date(data.values[i].label);
			var curDateTime = curDate.getTime();
			dates.push(curDateTime);
			var values = data.values[i].values;
			for (var j = 0; j < values.length; j++) {
				dataPoints[j].push([curDateTime, values[j]]);
			}
		}
		
		graphData = [];
		for (var i = 0; i < dataPoints.length; i++) {
			graphData.push( {
				label: data.quote[i],
				data: dataPoints[i],
				lines: { show: true, fill: 1.0, lineWidth: 0 }
			})
		}
		
		// ### GRAPH
		options = {
			// pink blue purple aqua 
			colors: ["#CAA4FC", "#92D7FC", "#FF3DB8", "#74E8E0", "#FFC88A", "#BE95FC", "#FFC3B5"], 
	        yaxis: { axisLabel: "Frequency of Cluster", axisLabelUseCanvas: true, min: 0 },
	        xaxis: { min: dates[0], max:dates[dates.length-1], axisLabelUseCanvas: true, mode: "time", timeformat: "%m/%d", tickSize: [2, "day"]},
	        legend: { show: false }, 
	        //stack: true,
	        series: {
				stack: true
			},
			grid: { clickable: true, hoverable: true, autoHighlight: false }
	    };
	    graphContainer = $("#cluster-graph");
	    PrintGraph(0, graphData.length);
	    curStart = 0, curEnd = graphData.length;
	    
	    // Mechanism for selecting one cluster on the plot
	    $("#cluster-graph").bind("plotclick", function (event, pos, item) {
	    	if (allClustersShown) {
	    		var info = FindCluster(pos.x, pos.y);
		        if (info) {
		        	PrintData(info.clusterIndex, info.clusterIndex + 1, false);
		        }
	    	}
	    });
	    // Mechanism for showing hovertext
	    $("#cluster-graph").bind("plothover", function (event, pos, item) {
	    	var info = FindCluster(pos.x, pos.y);
    		if (info) {
    			var numMentions = graphData[info.clusterIndex].data[info.dateIndex][1];
    			showTooltip(pos.pageX, pos.pageY,
                        "<b>" + numMentions + "</b> mentions of <b>\"" + graphData[info.clusterIndex].label + "\"</b>");
    		} else {
    			$("#tooltip").remove();
    		}
	    });
	})
	.error(function() { 
		$('#graph-error-log').html("Error: Graph information either does not exist for given month or cannot be obtained at this time.");
		$('#graph-error-log').show();
	});
}

function showTooltip(x, y, contents) {
	$("#tooltip").remove();
    $('<div id="tooltip">' + contents + '</div>').css( {
        position: 'absolute',
        width: '200px',
        'font-size': '12px',
        display: 'none',
        top: y + 5,
        left: x + 5,
        border: '1px solid #fdd',
        padding: '2px',
        'background-color': '#eee',
        opacity: 0.90
    }).appendTo("body").fadeIn(0);
}

function FindCluster(x, y) {
	// find x
	if (x < dates[0] || y < 0) return;
	var i = 0;
	for (; i < dates.length; i++) {
		if (x <= dates[i]) break;
	}
	if (i > 0) i--;
	
	if (i < dates.length) {
		var interpolation = ((x - dates[i]) / (dates[i + 1] - dates[i]));
		var one_minus_interpolation = 1 - interpolation;
		// find y
		var j = curStart, sum = 0;
		
		if (i + 1 < dates.length) {
			for (; j < curEnd; j++) {
				sum += one_minus_interpolation * graphData[j].data[i][1] + interpolation * graphData[j].data[i + 1][1];
				if (y <= sum) break;
			}
		} else {
			for (; j < curEnd; j++) {
				sum += graphData[j].data[i][1];
				if (y <= sum) break;
			}
		}
		
		if (j < curEnd) {
			return { clusterIndex: j, dateIndex: i };
		}
	}
}

function HeadingClicked(heading) {
	alert(heading);
}