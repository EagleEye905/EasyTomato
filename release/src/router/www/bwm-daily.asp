<!DOCTYPE html>
<!--
	Tomato GUI
	Copyright (C) 2006-2010 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
-->
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv='content-type' content='text/html;charset=utf-8'>
<meta name='robots' content='noindex,nofollow'>
<title>[<% ident(); %>] Bandwidth: Daily</title>
<% include("common-header.html"); %>

<script type='text/javascript' src='tomato.js'></script>

<!-- / / / -->

<script type='text/javascript' src='debug.js'></script>
<script type='text/javascript' src='bwm-hist.js'></script>

<script type='text/javascript'>

//	<% nvram("wan_ifname,lan_ifname,rstats_enable"); %>
try {
//	<% bandwidth("daily"); %>
}
catch (ex) {
	daily_history = [];
}
rstats_busy = 0;
if (typeof(daily_history) == 'undefined') {
	daily_history = [];
	rstats_busy = 1;
}

function save()
{
	cookie.set('daily', scale, 31);
}

function genData()
{
	var w, i, h, t;

	w = window.open('', 'tomato_data_d');
	w.document.writeln('<pre>');
	for (i = 0; i < daily_history.length; ++i) {
		h = daily_history[i];
		t = getYMD(h[0]);
		w.document.writeln([t[0], t[1] + 1, t[2], h[1], h[2]].join(','));
	}
	w.document.writeln('</pre>');
	w.document.close();
}

function getYMD(n)
{
	// [y,m,d]
	return [(((n >> 16) & 0xFF) + 1900), ((n >>> 8) & 0xFF), (n & 0xFF)];
}

function redraw()
{
	var h;
	var grid;
	var rows;
	var ymd;
	var d;
	var lastt;
	var lastu, lastd;

	if (daily_history.length > 0) {
		ymd = getYMD(daily_history[0][0]);
		d = new Date((new Date(ymd[0], ymd[1], ymd[2], 12, 0, 0, 0)).getTime() - ((30 - 1) * 86400000));
		E('last-dates').innerHTML = '(' + ymdText(ymd[0], ymd[1], ymd[2]) + ' to ' + ymdText(d.getFullYear(), d.getMonth(), d.getDate()) + ')';

		lastt = ((d.getFullYear() - 1900) << 16) | (d.getMonth() << 8) | d.getDate();
	}

	lastd = 0;
	lastu = 0;
	rows = 0;
	block = '';
	gn = 0;

	grid = '<table class="table table-striped table-bordered">';
	grid += '<thead><tr><th>Date</th><th>Download</th><th>Upload</th><th>Total</th></tr></thead>';


	for (i = 0; i < daily_history.length; ++i) {
		h = daily_history[i];
		ymd = getYMD(h[0]);
		grid += makeRow(((rows & 1) ? 'odd' : 'even'), ymdText(ymd[0], ymd[1], ymd[2]), rescale(h[1]), rescale(h[2]), rescale(h[1] + h[2]));
		++rows;

		if (h[0] >= lastt) {
			lastd += h[1];
			lastu += h[2];
		}
	}

	E('bwm-daily-grid').innerHTML = grid + '</table>';

	E('last-dn').innerHTML = rescale(lastd);
	E('last-up').innerHTML = rescale(lastu);
	E('last-total').innerHTML = rescale(lastu + lastd);
}

function init()
{
	var s;

	if (nvram.rstats_enable != '1') return;

	if ((s = cookie.get('daily')) != null) {
		if (s.match(/^([0-2])$/)) {
			E('scale').value = scale = RegExp.$1 * 1;
		}
	}

	initDate('ymd');
	daily_history.sort(cmpHist);
	redraw();
}
</script>

</head>
<body onload='init()'>

    <% include(header.html); %>

<!-- / / / -->

<h3>WAN Bandwidth - Daily</h3>
<div class="row">
<div id='bwm-daily-grid' class="span5"></div>
<div class="span4">
<table class='table table-striped table-bordered table-condensed'>
	<thead>
<tr><th colspan=2 style='text-align:center'>Last 30 Days<br><span style='font-weight:normal' id='last-dates'></span></th></tr>
</thead>
<tbody>
<tr><td>Down</td><td id='last-dn'>-</td></tr>
<tr><td>Up</td><td id='last-up'>-</td></tr>
<tr><td>Total</td><td id='last-total'>-</td></tr>
</tbody>
</table>

<br/>
<hr/>
<br/>

<b>Date</b> <select onchange='changeDate(this, "ymd")' id='dafm'><option value=0>yyyy-mm-dd</option><option value=1>mm-dd-yyyy</option><option value=2>mmm dd, yyyy</option><option value=3>dd.mm.yyyy</option></select><br>
<b>Scale</b> <select onchange='changeScale(this)' id='scale'><option value=0>KB</option><option value=1>MB</option><option value=2 selected>GB</option></select><br>

<a href="javascript:genData()" class="btn">Data</a> 
<a href="admin-bwm.asp" class="btn">Configure</a>
</div>

<!-- / / / -->

 </div><!--/row-->

 <div id='footer' class="row-fluid">
 <div class="form-actions">
<input class="btn" type='button' value='Refresh' onclick='reloadPage()'>
</div>

</div>
        </div><!--/span-->
      </div><!--/row-->
      <hr>
      <footer>
        <p>&copy; Tomato 2012</p>
      </footer>
    </div><!--/.fluid-container-->
</body>
</html>
