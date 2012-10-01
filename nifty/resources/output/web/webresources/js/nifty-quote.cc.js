var JSONClustersPrefix = "clusters/data";

var SetColors = ["#0099FF"];
//var SetColors = ["#0099FF", "#33CC00", "#FF3366", "#9933FF", "#FFFF00", "#FF6600", "#3399FF", "#00CC00", "#CC3366", "#9900FF", "#FFFF33", "#FF9900", "#3399CC", "#00FF00", "#FF6699", "#6633CC", "#FFFF66", "#FF9933", "#6699CC", "#99FF66", "#FF3399", "#9933CC", "#FFFF99", "#FFCC33"];

var a;
var Quote;
var Frequencies;
var Variants;
var Urls;
var ClusterId;

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

function fillInTableAndGraph(ClusterId) {
    var Index = Math.floor(ClusterId / 10000);
    (new Request.JSON({
        url: JSONClustersPrefix + Index + "/" + ClusterId + ".json",
        method: "get",
        onSuccess: function (b) {
            Quote = b.quote;
            Frequencies = b.frequency;
            Variants = b.variants;
            Urls = b.varianturls;

            $("quote-desc").innerHTML = "Quote: " + Quote;

            var OldTable = $("cluster-table"),
                NewTable = OldTable.cloneNode(true);
            NewTable.innerHTML = "";

            var tbody = document.createElement("tbody");

            var heading_names = ["Frequency", "Variant"];
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
            
            for (var i = 0; i < Variants.length; ++i) {
              var tr = document.createElement("tr");
              tr.setAttribute("id", i);

              var td_freq = document.createElement("td");
              td_freq.appendChild(document.createTextNode(Frequencies[i]));
              tr.appendChild(td_freq);

              var td_variant = document.createElement("td");
              var variant_link = document.createElement("a");
              variant_link.setAttribute("href", Urls[i]);
              variant_link.appendChild(document.createTextNode(Variants[i]));
              td_variant.appendChild(variant_link);
              tr.appendChild(td_variant);

              tbody.appendChild(tr);
            }

            NewTable.appendChild(tbody);
            OldTable.parentNode.replaceChild(NewTable, OldTable);
            //OldTable.parentNode.appendChild(NewTable, OldTable);

            b.color = SetColors[0];

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
                showAggregates: true,
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
                        a.innerHTML = "<b>" + d + "</b> mentions of <b>\"" + Quote + "\"</b> on " + [c[1], c[2], c[0]].join("/");
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
            var counter = 0;
            b.each(function (a, b) {
                if (counter % 2 != 0) {
                    f.beginPath();
                    f.moveTo(d * b, 0);
                    f.lineTo(d * b, 460);
                    f.closePath();
                    f.stroke();
                    var c = a.label.split(" ")[0].split("-"),
                        c = c[1] + "/" + c[2];
                    f.fillText(c, d * b + 5, 460);
                }
                counter += 1;
            })
        }
    })).send()
}

function getUrlVars() {
    var vars = {};
    var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {
        vars[key] = value;
    });
    return vars;
}

function init() {
    (function () {
        ClusterId = getUrlVars()["id"];
        fillInTableAndGraph(ClusterId);
    })()
}
window.addEvent("domready", init);
