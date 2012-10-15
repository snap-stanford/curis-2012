///////////////////////////////////////////////////////////////////////////////////////
/// GENERAL CODE
///////////////////////////////////////////////////////////////////////////////////////

function init(setupFn) {
	parameterDateString = GetParameters()["date"];
	if (!parameterDateString) {
		$.ajax({
            url: 'currentdate.txt',
            type: 'GET',
            dataType: 'text',
            success: function(data) {
            	setupFn(data);
            },
            error: function(xhr, textStatus, errorThrown) {
            	setupFn("2012-01-01");
            }
        });
	} else {
		setupFn(parameterDateString)
	}
}

function GetParameters() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function GetMonthString(curDate) {
	return curDate.getUTCFullYear() + "-" + ("0" + (curDate.getUTCMonth() + 1)).slice(-2);
}

function GetDateString(curDate) {
	return curDate.getUTCFullYear() + "-" + ("0" + (curDate.getUTCMonth() + 1)).slice(-2) + "-" + ("0" + curDate.getUTCDate()).slice(-2);
}

function HeadingClicked(heading) {
	alert(heading);
}

function PrintData(start, end, allowClicking) {
	if (!graphData || !tableData) return;
	allClustersShown = allowClicking;
	curStart = start, curEnd = end;
	PrintTable(start, end);
	PrintGraph(start, end);
}

///////////////////////////////////////////////////////////////////////////////////////
/// TABLE CODE
///////////////////////////////////////////////////////////////////////////////////////

var headings = ["rank", "prev", "frequency", "numvariants", "quote"];
var headingStrings = { rank:"Rank", prev:"Previous", frequency: "Frequency", numvariants: "#Variants", quote: "Quote" };
var sortTableBy = "rank";
var tableAscending = true;

function GetTableData(fileName) {
	$.getJSON(fileName, function(data) {
		curStart = 0, curEnd = data.label.length;
		// rewire data
		tableData = [];
		for (var i = 0; i < data.label.length; ++i) {
			var info = {
					label: data.label[i],
					frequency: data.frequency[i],
					numvariants: data.numvariants[i],
					quote: data.quote[i],
					rank: (i + 1)
				};
			if (data.prev) {
				info.prev = data.prev[i];
			}
			tableData.push(info);
		}
		$("#table-error-log").hide();
		PrintTable(0, tableData.length);
		
	})
	.error(function() { 
		$('#table-error-log').html("Error: Table information either does not exist for given month or cannot be obtained at this time.");
		$('#table-error-log').show();
		$('#cluster-table').hide();
	});
}

function PrintTable(start, end) {
	SortTable();
	//PrintGivenTable(start, end, tableData);
}

//inclusive, exclusive
function PrintGivenTable(start, end, curTable) {
	if (curTable.length == 0) return;
	var table = document.createElement("tbody");
	
	// Write the heading
	var tr_heading = document.createElement("tr");
	$(tr_heading).attr("id", "heading");
	for (var i = 0; i < headings.length; ++i) {
		if (tableData[0][headings[i]]) {
			var td_heading = document.createElement("td");
			$(td_heading).click(function() { HeadingClicked(this); });
			$(td_heading).attr("id", headings[i]);
			if (headings[i] == sortTableBy) {
				$(td_heading).addClass("selected");
			}
			$(td_heading).html("<b>" + headingStrings[headings[i]] + "</b>")
			$(tr_heading).append(td_heading);
		}
	}
	$(table).append(tr_heading);
	
	// Write everything else
	for (var i = start; i < curTable.length && i < end; ++i) {
		var tr = document.createElement("tr");
        $(tr).attr("id", curTable[i].label);
        $(tr).append("<td>" + curTable[i].rank + "</td>") // rank
        if (tableData[i].prev) {
        	$(tr).append("<td>" + curTable[i].prev + "</td>") // previous
		}
        $(tr).append("<td>" + curTable[i].frequency + "</td>") // frequency
        $(tr).append("<td>" + curTable[i].numvariants + "</td>") // variants
        $(tr).append("<td><a href=\"cluster.html?date=" + parameterDateString + "&id=" + curTable[i].label + "\">" + curTable[i].quote + "</a></td>") // quote
        $(table).append(tr);
    }
	
	$('#cluster-table').html(table);
	$('#cluster-table').show();
}

function HeadingClicked(header) {
	var curHeading = $(header).attr("id");
	if (curHeading == sortTableBy) {
		tableAscending = !tableAscending;
	} else {
		$("#" + sortTableBy).removeClass("selected");
		sortTableBy = curHeading;
		$("#" + sortTableBy).addClass("selected");
		tableAscending = true;
	}
	SortTable();
}

function SortTable() {
	if (sortTableBy == "rank" && tableAscending) {
		PrintGivenTable(curStart, curEnd, tableData);
	}
	var tableCopy = tableData.slice(curStart, curEnd);
	if (sortTableBy != "prev") {
		//alert(curStart + "-" + curEnd + " sort by: " + sortTableBy);
		if (sortTableBy != "quote") {
			tableCopy.sort(function(a,b) { 
				if (tableAscending) {
					return parseInt(a[sortTableBy]) - parseInt(b[sortTableBy]);
				} else {
					return parseInt(b[sortTableBy]) - parseInt(a[sortTableBy]);
				}
			});
		} else {
			tableCopy.sort(function(a, b) {
				if (tableAscending) {
					return strcmp(a[sortTableBy], b[sortTableBy]);
					//return a[sortTableBy] > b[sortTableBy];
				} else {
					return strcmp(b[sortTableBy], a[sortTableBy]);
					//return a[sortTableBy] < b[sortTableBy];
				}
			});
		}
	} else {
		tableCopy.sort(function(a, b) {
			var aText = $(a[sortTableBy]).text(), bText = $(b[sortTableBy]).text();
			//alert(aText + " vs. " + bText);
			// new checking
			if (aText == 'new!') {
				return (tableAscending) ? -1 : 1;
			} else if (bText == 'new!') {
				return (tableAscending) ? 1 : -1;
			}
			if (tableAscending) {
				return parseInt(aText) - parseInt(bText);
			} else {
				return parseInt(bText) - parseInt(aText);
			}
		});
	}
	PrintGivenTable(0, tableCopy.length, tableCopy);
}

function strcmp(str1, str2) {
	return (str1 < str2) ? -1 : (str1 > str2 ? 1 : 0 );
}
///////////////////////////////////////////////////////////////////////////////////////
/// GRAPHING CODE
///////////////////////////////////////////////////////////////////////////////////////

function PrintGraph(start, end) {
	$('.graph').show();
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

function GetGraphData(fileName, tickSize, offset) {
	$.getJSON(fileName, function(data) {
		allClustersShown = true;
		$('#graph-error-log').hide();
		
		// ### ITERATE THROUGH VALUES
		var dataPoints = [];
		for (var i = 0; i < data.label.length; i++) {
			dataPoints.push([]);
		}
		
		dates = [];		
		for (var i = 0; i < data.values.length; i++) {
			var curDate = new Date(data.values[i].label.replace(" ", "T"));
			//alert(data.values[i].label);
			var curDateTime = curDate.getTime();
			dates.push(curDateTime);
			var values = data.values[i].values;
			for (var j = 0; j < values.length; j++) {
				dataPoints[j].push([curDateTime, values[j]]);
			}
		}
		var maxDate = new Date(dates[dates.length - 1]);
		maxDate.setHours(maxDate.getHours() + offset);
		
		graphData = [];
		for (var i = 0; i < dataPoints.length; i++) {
			graphData.push( {
				label: data.quote[i],
				data: dataPoints[i],
				lines: { show: true, fill: 1.0, lineWidth: 0 }
			})
		}
		
		// Colors are from kuler.adobe.com; "Hey Mr. Grey Rainbow" by stephenmweathers
		//var SetColors = ["#CC5C54", "#F69162", "#FFFFCD", "#85A562", "#7AB5DB"];
		// "Fairgrounds"
		//var SetColors = ["#FFF6C9", "#C8E8C7", "#A4DEAB", "#85CC9F", "#499E8D"];
		//var StartDate = new Date(2012, 6, 1, 0, 0, 0, 0); // Note that month is zero-based

		// Colors are from www.colourlovers.com/business/trends/branding/7880/Papeterie_Haute-Ville_Logo
		//var SetColors = ["#113F8C", "#01A4A4", "#00A1CB", "#61AE24", "#D0D102", "#32742C", "#D70060", "#E54028", "#F18D05", "#616161"];

		// Colors from www.colourlovers.com/business/trends/branding/7783/HUEMINCE
		//var SetColors = ["#B3D334", "#FFC40A", "#00ABF0", "#EF1C25", "#87318C", "#0160A0", "#F59EE5", "#C3D6E4"];

		// Colors from www.colourlovers.com/palette/2366249/1
		//var SetColors = ["#84F57B", "#1FE6E6", "#9A1FE6", "#E61FB0", "#F7AF48"];

		// Original color selection
		var SetColors = ["#0099FF", "#33CC00", "#FF3366", "#9933FF", "#FFFF00", "#FF6600", "#3399FF", "#00CC00", "#CC3366", "#9900FF", "#FFFF33", "#FF9900", "#3399CC", "#00FF00", "#FF6699", "#6633CC", "#FFFF66", "#FF9933", "#6699CC", "#99FF66", "#FF3399", "#9933CC", "#FFFF99", "#FFCC33"];
		
		// Caroline's color selection
		//var SetColors = ["#CAA4FC", "#92D7FC", "#FF3DB8", "#74E8E0", "#FFC88A", "#BE95FC", "#FFC3B5"];
		
		// ### GRAPH
		options = {
			colors: SetColors,
	        yaxis: { axisLabel: "Frequency of Cluster", axisLabelUseCanvas: true, min: 0 },
	        xaxis: { min: dates[0], max:maxDate, axisLabelUseCanvas: true, mode: "time", timeformat: "%m/%d", tickSize: tickSize},
	        legend: { show: false }, 
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
		$('.graph').hide();
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