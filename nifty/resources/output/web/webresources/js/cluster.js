$(document).ready(function() {
	curDateString = GetParameters()["date"];
	if (curDateString) {
		curDate = new Date(curDateString);
		$(".chart-link").each(function(){
		  this.href = this.href.split('?')[0] + "?date=" + curDateString;
		});
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
		var id_map = {};
    var clust_to_index = {};
		for (var i = 0; i < data.urls.length; ++i) {
			id_map[data.ids[i]] = data.quotes[i];
      clust_to_index[data.ids[i]] = i;
			var tr = document.createElement("tr");
            $(tr).append("<td>" + data.frequencies[i] + "</td>") // rank
            $(tr).append("<td><a href=\"" + data.urls[i] + "\">" + data.quotes[i] + "</a></td>") // quote
            $(table).append(tr);
        }
		
		$('#cluster-table').html(table);
    
    // filter parents
    /*real_parents = {}; // one or 0 parents per id
    has_parents = [];
    root_index = 0;
    for (var i = 0; i < data.parents.length; ++i) {
      //real_parents[i] = [];
      if (data.parents[i].length == 0) {
        root_index = i;
        real_parents[i] = -1;
        has_parents[0] = i; // lame.
      }
    }
    
    // brute force bwahahaha
    while (has_parents.length < data.parents.length) {
      for (var i = 0; i < data.parents.length; ++i) {
        if (!real_parents.hasOwnProperty(i)) {
          // check parents
        }
        for (var j = 0; j < data.parents[i].length; ++j) {
          // iterate across all parents
          if (
          graph_string += "\"" + id_map[data.parents[i][j]] + "\"";
          graph_string += " -> ";
          graph_string += "\"" + data.quotes[i] + "\";";
        }
      }
    }*/
    
    // this is horrible.
    // build edge matrix.
    edge_matrix = [];
    for (var i = 0; i < data.parents.length; ++i) {
      edge_matrix.push([]);
      for (var j = 0; j < data.parents.length; ++j) {
        edge_matrix[i].push(0);
      }
      for (var j = 0; j < data.parents[i].length; ++j) {
        edge_matrix[i][clust_to_index[data.parents[i][j]]] = 1;
      }
    }
    
    to_remove = [];
    for (var i = 0; i < data.parents.length; ++i) { // layer 0
      for (var j = 0; j < data.parents[i].length; ++j) { // layer 1
        j_ind = clust_to_index[data.parents[i][j]];
        for (var k = j + 1; k < data.parents[i].length; ++k) {
          k_ind = clust_to_index[data.parents[i][k]];
          if (edge_matrix[j_ind][k_ind] == 1) { // k is j's parent
            edge_matrix[i][k_ind] = 0;
          } else if (edge_matrix[k_ind][j_ind] == 1) { // j is k's parent
            edge_matrix[i][j_ind] = 0;
          }
        }
      }
    }
		
		// graph viz
    if (typeof data.parents !== 'undefined') {
      var graph_string = "digraph G { graph [ dpi = 66, width=8]; node [shape=box];"
      for (var i = 0; i < data.parents.length; ++i) {
        for (var j = 0; j < data.parents[i].length; ++j) {
          // only add if we didn't kill the edge
          if (edge_matrix[i][clust_to_index[data.parents[i][j]]] == 1) {
            graph_string += "\"" + id_map[data.parents[i][j]] + "\"";
            graph_string += " -> ";
            graph_string += "\"" + data.quotes[i] + "\";";
          } else {
            console.log(data.ids[i] + " - " + id_map[data.parents[i][j]] + " removed.");
          }
        }
        
      }
      graph_string += "}";
      var viz_output = Viz(graph_string, "svg");
      console.log(viz_output);
      $('#cluster-digraph').html(viz_output);
    }
		
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