var t_DiglogX, t_DiglogY, t_DiglogW, t_DiglogH;

var back_page;

function setHTML(windowObj, el, htmlStr) {
	if (document.all) {
		if (windowObj.document.all(el)) windowObj.document.all(el).innerHTML = htmlStr;
	}
	else if (document.getElementById) {
		if (windowObj.document.getElementById(el)) windowObj.document.getElementById(el).innerHTML = htmlStr;
	}
}

function setWidth(windowObj, el, newwidth) {
	if (document.all) {
		if (windowObj.document.all(el)) windowObj.document.all(el).style.width = newwidth;
	}
	else if (document.getElementById) {
		if (windowObj.document.getElementById(el)) windowObj.document.getElementById(el).style.width = newwidth;
	}
}

function gid(id) {
	return window.parent.document.getElementById ? window.parent.document.getElementById(id) : null;
}

function Browser() {
	var ua, s, i;
	this.isIE = false;
	this.isNS = false;
	this.isOP = false;
	this.isSF = false;
	ua = navigator.userAgent.toLowerCase();
	s = "opera";
	if ((i = ua.indexOf(s)) >= 0) {
		this.isOP = true;
		return;
	}
	s = "msie";
	if ((i = ua.indexOf(s)) >= 0) {
		this.isIE = true;
		return;
	}
	s = "netscape6/";
	if ((i = ua.indexOf(s)) >= 0) {
		this.isNS = true;
		return;
	}
	s = "gecko";
	if ((i = ua.indexOf(s)) >= 0) {
		this.isNS = true;
		return;
	}
	s = "safari";
	if ((i = ua.indexOf(s)) >= 0) {
		this.isSF = true;
		return;
	}
}

function ScreenConvert() {
	var browser = new Browser();
	var objScreen = gid("ScreenOver");
	if (!objScreen) var objScreen = window.parent.document.createElement("div");
	var oS = objScreen.style;
	objScreen.id = "ScreenOver";
	oS.display = "block";
	oS.top = oS.left = oS.margin = oS.padding = "0px";

	oS.width = window.parent.document.body.scrollWidth + "px";
	oS.height = window.parent.document.body.scrollHeight + "px";
	oS.position = "absolute";
	oS.zIndex = "3";
	if ((!browser.isSF) && (!browser.isOP)) {
		oS.background = "#d9e3e9"; //"#181818";
	}
	else {
		oS.background = "#d9e3e9";
	}
	oS.filter = "alpha(opacity=40)";
	oS.opacity = 40 / 100;
	oS.MozOpacity = 40 / 100;
	window.parent.document.body.appendChild(objScreen);
	//var allselect = gname("select");
	//for (var i=0; i<allselect.length; i++) allselect.style.visibility = "hidden";
}

function ScreenClean() {
	var objScreen = window.parent.document.getElementById("ScreenOver");
	if (objScreen) objScreen.style.display = "none";
	//var allselect = gname("select");
	//for (var i=0; i<allselect.length; i++) allselect.style.visibility = "visible";
}

