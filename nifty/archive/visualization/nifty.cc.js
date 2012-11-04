var JSONFilePrefix = "graphdata/clusterinfo-";
var JSONTablePrefix = "tabledata/clustertable-";
var Daily = "daily";
var Weekly = "weekly";
var Monthly = "monthly";
var NumDaysInMonth = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
var Quotes = {};
var Colors = {};
var Ranking = [];
var Type;  // "daily", "weekly", or "monthly"; determined by $("main-desc")

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

var StartDate;
var CurrDate;
var a;

// Taken from http://stackoverflow.com/questions/3066586/get-string-in-yyyymmdd-format-from-js-date-object
Date.prototype.yyyymmdd = function() {
   var yyyy = this.getFullYear().toString();
   var mm = (this.getMonth() + 1).toString();  // month is zero-based
   var dd  = this.getDate().toString();
   return yyyy + "-" + (mm[1]?mm:"0"+mm[0]) + "-" + (dd[1]?dd:"0"+dd[0]); // padding
};

Date.prototype.mmddyyyy = function() {
   var yyyy = this.getFullYear().toString();
   var mm = (this.getMonth() + 1).toString();  // month is zero-based
   var dd  = this.getDate().toString();
   return  (mm[1]?mm:"0"+mm[0]) + "-" + (dd[1]?dd:"0"+dd[0]) + "-" + yyyy; // padding
};

// Taken from http://stackoverflow.com/questions/3646914/how-do-i-check-if-file-exists-in-jquery-or-javascript
function UrlExists(url)
{
    var http = new XMLHttpRequest();
    http.open('HEAD', url, false);
    http.send();
    return http.status!=404;
}

function IsLeapYear(Year) {
    if (Year % 4 != 0) return false;
    if (Year % 100 != 0 || Year % 400 == 0) return true;
    return false;
}

function NumDaysToChange(DateToChange, IsForward) {
    if (Type == "weekly") {
        return 7;
    } else if (Type == "monthly") {
        var Month = DateToChange.getMonth();
        if (!IsForward) {  // In this case, we want the number of the previous month
          Month -= 1;
          if (Month == -1) { Month = 11; }
        }

        if (Month == 1 && IsLeapYear(DateToChange.getFullYear())) {
            return 29;
        } else {
            return NumDaysInMonth[Month];
        }
    } else {
      return 1;
    }
}

function graphPrevDay() {
    var OrigNumDaysToChange = NumDaysToChange(CurrDate, false);
    CurrDate.setDate(CurrDate.getDate() - OrigNumDaysToChange);
      
    var CurrDateStr = CurrDate.yyyymmdd();
    var NewUrl = JSONFilePrefix + CurrDateStr + ".json";
    if (UrlExists(NewUrl)) {
      var c = $$(".stream-nav a");
      c.removeClass("selected");
      createGraph(CurrDate);
      fillInTable(CurrDate);
    } else {
      if (Type == Daily) {
        $("currdate-error").innerHTML = "Sorry, there is no quote frequency data available for " + CurrDate.mmddyyyy() + ".";
      } else {
        var EndGraphDate = new Date(CurrDate);
        EndGraphDate.setDate(CurrDate.getDate() + NumDaysToChange(CurrDate, true) - 1);
        $("currdate-error").innerHTML = "Sorry, there is no quote frequency data available for the range " + CurrDate.mmddyyyy() + " to " + EndGraphDate.mmddyyyy();
      }
      CurrDate.setDate(CurrDate.getDate() + OrigNumDaysToChange);  // restore the previous, working date
    }
}

function graphNextDay() {
    var OrigNumDaysToChange = NumDaysToChange(CurrDate, true);
    CurrDate.setDate(CurrDate.getDate() + OrigNumDaysToChange);
    var CurrDateStr = CurrDate.yyyymmdd();
    var NewUrl = JSONFilePrefix + CurrDateStr + ".json";
    if (UrlExists(NewUrl)) {
      var c = $$(".stream-nav a");
      c.removeClass("selected");
      createGraph(CurrDate);
      fillInTable(CurrDate);
    } else {
      if (Type == Daily) {
        $("currdate-error").innerHTML = "Sorry, there is no quote frequency data available for " + CurrDate.mmddyyyy() + ".";
      } else {
        var EndGraphDate = new Date(CurrDate);
        EndGraphDate.setDate(CurrDate.getDate() + NumDaysToChange(CurrDate, true) - 1);
        $("currdate-error").innerHTML = "Sorry, there is no quote frequency data available for the range " + CurrDate.mmddyyyy() + " to " + EndGraphDate.mmddyyyy();
      }
      CurrDate.setDate(CurrDate.getDate() - OrigNumDaysToChange);  // restore the previous, working date
    }
}

function fillInTable(TableDate) {
    var TableDateStr = TableDate.yyyymmdd();
    (new Request.JSON({
        url: JSONTablePrefix + TableDateStr + ".json",
        method: "get",
        onSuccess: function (b) {
            var quotes = b.quote;
            var labels = b.label;
            var frequencies = b.frequency;
            var numvariants = b.numvariants;

            var OldTable = $("cluster-table"),
                NewTable = OldTable.cloneNode(true);
            NewTable.innerHTML = "";

            var tbody = document.createElement("tbody");

            var heading_names = ["Rank", "Frequency", "# Variants", "Quote"];
            var tr_heading = document.createElement("tr");
            tr_heading.setAttribute("id", "heading");
            for (var i = 0; i < heading_names.length; ++i) {
              var td_heading = document.createElement("td");
              var bold = document.createElement("b");
              bold.appendChild(document.createTextNode(heading_names[i]));
              td_heading.appendChild(bold);
              tr_heading.appendChild(td_heading);
            }
            tbody.appendChild(tr_heading);
            

            for (var i = 0; i < labels.length; ++i) {
              var tr = document.createElement("tr");
              tr.setAttribute("id", labels[i]);
              var td_rank = document.createElement("td");
              td_rank.appendChild(document.createTextNode(i + 1));
              tr.appendChild(td_rank);

              var td_freq = document.createElement("td");
              td_freq.appendChild(document.createTextNode(frequencies[i]));
              tr.appendChild(td_freq);

              var td_variants = document.createElement("td");
              td_variants.appendChild(document.createTextNode(numvariants[i]));
              tr.appendChild(td_variants);

              var td_quote = document.createElement("td");
              td_quote.appendChild(document.createTextNode(quotes[i]));
              tr.appendChild(td_quote);

              tbody.appendChild(tr);
            }

            NewTable.appendChild(tbody);
            OldTable.parentNode.replaceChild(NewTable, OldTable);
            //OldTable.parentNode.appendChild(NewTable, OldTable);
        }
    })).send();
}

function filterTable(filteredQuotes) {
    var c = $$(".table tbody tr");
    c.addClass("hidden");

    var d = $$(".table tbody tr[id=heading]");
    d.removeClass("hidden");
    for (var i = 0; i < filteredQuotes.length; i++) {
        d = $$(".table tbody tr[id=" + filteredQuotes[i] + "]");
        d.removeClass("hidden");
    }
}

function createGraph(GraphDate) {
        // Clear StreamGraph, in case it is not already; and reset the global variables
        var tmpDiv = new Element('div', {html:'<div id="stream-viz"></div>'});
        tmpDiv.getFirst().replaces($('stream-viz'));
        Quotes = {};
        Colors = {};
        Ranking = [];

        var GraphDateStr = GraphDate.yyyymmdd();
        var GraphDateArr = GraphDateStr.split("-");
        var GraphDateMDY = GraphDateArr[1] + "-" + GraphDateArr[2] + "-" + GraphDateArr[0];
        if (Type == Daily) {
          $("quote-graph-currdate").innerHTML = "Current Date: " + GraphDate.mmddyyyy();
          $("top-20-quotes").className = "selected";
        } else {
          var EndGraphDate = new Date(GraphDate);
          EndGraphDate.setDate(GraphDate.getDate() + NumDaysToChange(GraphDate, true) - 1);
          $("quote-graph-currdate").innerHTML = "Date Range: " + GraphDate.mmddyyyy() + " to " + EndGraphDate.mmddyyyy();
          $("all-quotes").className = "selected";
        }
        $("currdate-error").innerHTML = "";

        (new Request.JSON({
            //url: "data/match-summary.json",
            url: JSONFilePrefix + GraphDateStr + ".json",
            method: "get",
            onSuccess: function (b) {
                var quotes = b.quote;
                var labels = b.label;
                for (var i = 0; i < labels.length; ++i) {
                  Quotes[labels[i]] = quotes[i];
                  Ranking.push(labels[i]);
                }
                b.color = b.label.map(function (a) {
                    // Hack, since all the colors must be different in order for JIT to determine which
                    // quote graph is being hovered over
                    var colorStr = SetColors[Ranking.indexOf(a) % SetColors.length];
                    colorStr = colorStr.slice(1, colorStr.length);
                    var color = parseInt(colorStr, 16);
                    var offset = Math.floor(Ranking.indexOf(a) / SetColors.length);
                    var newColorStr = (color + offset).toString(16).toUpperCase();
                    Colors[a] = '#000000'.slice(0, -newColorStr.length) + newColorStr;

                    return Colors[a];
                });
                //a = new $jit.StreamChart({
                a = new $jit.AreaChart({
                    injectInto: "stream-viz",
                    //type: window.G_vmlCanvasManager ? "stacked" : "smooth",
                    type: "stacked",
                    animate: true,
                    Margin: {
                        top: 5,
                        left: 0,
                        right: 0,
                        bottom: 5
                    },
                    labelOffset: 0,
                    //showAggregates: true,
                    showLabels: false,
                    Label: {
                      type: 'HTML',
                      size: 13,
                      family: 'Arial',
                      color: 'black'
                    },
                    Tips: {
                        enable: true,
                        onShow: function (a, b, c) {
                            var c = c.name.slice(0, 10).split("-");
                            //var c = c.name;
                            var d = +b.value;
                            1E6 <= d ? d = Math.round(d / 1E5) / 10 + "M" : 1E3 <= d && (d = Math.round(d / 100) / 10 + "K");
                            a.innerHTML = "<b>" + d + "</b> mentions of <b>\"" + Quotes[b.name.trim()] + "\"</b> on " + [c[1], c[2], c[0]].join("/");
                            //a.innerHTML = "<b>" + d + "</b> mentions of <b>\"" + Quotes[b.name] + "\"</b> on " + c;
                        }
                    },
                    Events: {
                        enable: true,
                        onClick: function (q, x, y) {
                            if (!q) { return; }
                            c = $$(".stream-nav a");
                            c.removeClass("selected");
                            filteredQuotes = Ranking.filter(function (a) {
                                return a == q.name.trim();
                            });
                            a.filter(filteredQuotes, {
                                onComplete: function () {
                                  filterTable(filteredQuotes);
                                  //a.setupLabels();
                                }
                            });
                            //a && r(a);
                            //a && (a = $(a.name.trim()).getParent(), a.addClass("selected"), h.toElement(a));
                        }
                    },
                    filterOnClick: false,
                    restoreOnRightClick: false,
                    selectOnHover: false
                });
                a.loadJSON(b);
                a.canvas.getPos = function () {
                    for (var a = this.getElement(), b = 0, c = 0; a && !/^(?:html)$/i.test(a.tagName);) b += a.offsetLeft, c += a.offsetTop, a = a.offsetParent;
                    return this.pos = {
                        x: b,
                        y: c
                    };
                };
                a.config.animate = !0;
                /*var filteredQuotes = Ranking.filter(function (a) {
                    return Ranking.indexOf(a) > -1 && Ranking.indexOf(a) < 20
                });*/
                /*var filteredQuotes = Ranking;
                a.filter(filteredQuotes, {
                    onComplete: function () {
                        a.setupLabels()
                    }
                });*/
                var c = $("background-stream-viz");
                if (!c) {
                    if (window.G_vmlCanvasManager) return;
                    c = document.createElement("canvas");
                    c.id = "background-stream-viz";
                    $(c).inject("stream-viz", "top")
                }
                c.width = 900;
                c.height = 480;
                var f = c.getContext("2d"),
                    b = b.values,
                    d = 900 / (b.length - 1);
                f.lineWidth = 0.2;
                f.strokeStyle = "#ddd";
                f.fillStyle = "#bbb";
                b.each(function (a, b) {
                    f.beginPath();
                    f.moveTo(d * b, 0);
                    f.lineTo(d * b, 460);
                    f.closePath();
                    f.stroke();
                    var c = a.label.split(" ")[0].split("-"),
                        c = c[1] + "/" + c[2];
                    // TODO: Remove dates on x axis from AreaChart; put here
                    f.fillText(c, d * b + 5, 460)
                })
            }
        })).send()
}


function init() {
    (function () {
        // Set the Type variable
        var MainDesc = $("main-desc").innerHTML;
        if (MainDesc.indexOf(Weekly) != -1) {
          Type = Weekly;
          StartDate = new Date(2012, 5, 1, 0, 0, 0, 0);
          SetColors = ["#D91A2A", "#A6056D", "#A9D943", "#F2E527", "#F25D07"];
        } else if (MainDesc.indexOf(Monthly) != -1) {
          Type = Monthly;
          StartDate = new Date(2012, 1, 1, 0, 0, 0, 0);
          //SetColors = ["#8A2BE2", "#DC143C", "#70A35E", "#FFD700", "#FF69B4"];
        } else {
          Type = Daily;
          StartDate = new Date(2012, 5, 30, 0, 0, 0, 0); // Note that month is zero-based
        }

        CurrDate = StartDate;
        var c = $$(".stream-nav a");

        var Top20Q = $("top-20-quotes");
        if (Top20Q) {
            Top20Q.addEvent("click",

            function (b) {
                a.busy || (b.stop(), c.removeClass("selected"), this.addClass("selected"), filteredQuotes = Ranking.filter(function (q) {
                    return Ranking.indexOf(q) > -1 && Ranking.indexOf(q) < 20
                }), a.filter(filteredQuotes, {
                    onComplete: function () {
                        filterTable(filteredQuotes);
                        //a.setupLabels()
                    }
                }))
            });
        }

        var Top10Q = $("top-10-quotes");
        if (Top10Q) {
            Top10Q.addEvent("click", function (b) {
                a.busy || (b.stop(), c.removeClass("selected"), this.addClass("selected"), filteredQuotes = Ranking.filter(function (q) {
                    return Ranking.indexOf(q) > -1 && Ranking.indexOf(q) < 10
                }), a.filter(filteredQuotes, {
                    onComplete: function () {
                        filterTable(filteredQuotes);
                        //a.setupLabels()
                    }
                }))
            });
        }

        var AllQ = $("all-quotes");
        if (AllQ) {
            AllQ.addEvent("click", function (b) {
                b.stop();
                a.busy || (c.removeClass("selected"), this.addClass("selected"), a.filter(Ranking, {
                    onComplete: function () {
                        filterTable(Ranking);
                        //a.setupLabels()
                    }
                }))
            });
        }

        var h = new Fx.Scroll(window, {
            onComplete: function () {
                $$(".row-wrapper.selected").removeClass("selected")
            }
        });
        $$(".row-wrapper > a").addEvent("click", function (a) {
            a.stop();
            h.toElement($("container"))
        });

        createGraph(StartDate);
        fillInTable(StartDate);
    })()
}
window.addEvent("domready", init);
