function isIPaddr(addr) {
	var i;
	var a;
	if (addr.split) {
		a = addr.split(".");
	} else {
		a = cdisplit(addr, ".");
	}
	if (a.length != 4) {
		return false;
	}
	for (i = 0; i < a.length; i++) {
		var x = a[i];
		if (x == null || x == "" || ! _isNumeric(x) || x < 0 || x > 255) {
			return false;
		}
	}
	return true;
}

function _isNumeric(str) {
	var i;
	for (i = 0; i < str.length; i++) {
		var c = str.substring(i, i + 1);
		if ("0" <= c && c <= "9") {
			continue;
		}
		return false;
	}
	return true;
}

function isHex(str) {
	var i;
	for (i = 0; i < str.length; i++) {
		var c = str.substring(i, i + 1);
		if (("0" <= c && c <= "9") || ("a" <= c && c <= "f") || ("A" <= c && c <= "F")) {
			continue;
		}
		return false;
	}
	return true;
}

function checkMacStr(mac) {
	if ((mac.value.indexOf(':') != - 1) || (mac.value.indexOf('-') != - 1)) {
		mac.value = mac.value.replace(/:/g, "");
		mac.value = mac.value.replace(/-/g, "");
	}
	var temp = mac.value;
	if (mac.value.length != 12) {
		mac.focus();
		return true;
	}
	for (i = 0; i < mac.value.length; i++) {
		var c = mac.value.substring(i, i + 1)
		if (("0" <= c && c <= "9") || ("a" <= c && c <= "f") || ("A" <= c && c <= "F")) {
			continue;
		}
		mac.focus();
		return true;
	}
	mac.value = temp.toUpperCase();
	return false;
}
/* Check Mac Address Format*/
function checkMacMain(mac) {
	if (mac.value.length == 0) {
		mac.focus();
		return true;
	}
	for (i = 0; i < mac.value.length; i++) {
		var c = mac.value.substring(i, i + 1)
		if (("0" <= c && c <= "9") || ("a" <= c && c <= "f") || ("A" <= c && c <= "F")) {
			continue;
		}
		mac.focus();
		return true;
	}
	if (mac.value.length == 1) {
		mac.value = "0" + mac.value;
	}
	mac.value = mac.value.toUpperCase();
	return false;
}
function checkMacAddress(mac1, mac2, mac3, mac4, mac5, mac6) {
	if (checkMacMain(mac1)) return true;
	if (checkMacMain(mac2)) return true;
	if (checkMacMain(mac3)) return true;
	if (checkMacMain(mac4)) return true;
	if (checkMacMain(mac5)) return true;
	if (checkMacMain(mac6)) return true;
	return false;
}

/* Check IP Address Format*/
/*
function checkIPMain(ip,max) 
{
    if( false == isNumeric(ip, max) ) 
    {
        ip.focus();
        return true;
    }
    
    return false;
}

function checkIP(ip1, ip2, ip3, ip4, max) {
    if(checkIPMain(ip1,255)) return true; 
    if(checkIPMain(ip2,255)) return true;
    if(checkIPMain(ip3,255)) return true;
    if(checkIPMain(ip4,max)) return true;
    if((parseInt(ip1.value)==0)||(parseInt(ip1.value)==0)&&(parseInt(ip2.value)==0)&&(parseInt(ip3.value)==0)&&(parseInt(ip4.value)==0))
    	return true;
    return false;
}
*/
/* Check Numeric*/
function isNumeric(str, max) {
	if (str.value.length <= 3) {
		str.value = str.value.replace(/^000/g, "0");
		str.value = str.value.replace(/^00/g, "0");
		if (str.value.length > 1) str.value = str.value.replace(/^0/g, "");
	}

	if (str.value.length == 0 || str.value == null || str.value == "") {
		str.focus();
		return false;
	}

	var i = parseInt(str.value);
	if (i > max) {
		str.focus();
		return false;
	}
	for (i = 0; i < str.value.length; i++) {
		var c = str.value.substring(i, i + 1);
		if ("0" <= c && c <= "9") {
			continue;
		}
		str.focus();
		return false;
	}
	return true;
}

/* Check Blank*/
function isBlank(str) {
	if (str.value == "") {
		str.focus();
		return true;
	} else return false;
}

/* Check Phone Number*/
function isPhonenum(str) {
	var i;
	if (str.value.length == 0) {
		str.focus();
		return true;
	}
	for (i = 0; i < str.value.length; i++) {
		var c = str.value.substring(i, i + 1);
		if (c >= "0" && c <= "9") continue;
		if (c == '-' && i != 0 && i != (str.value.length - 1)) continue;
		if (c == ',') continue;
		if (c == ' ') continue;
		if (c >= 'A' && c <= 'Z') continue;
		if (c >= 'a' && c <= 'z') continue;
		str.focus();
		return true;
	}
	return false;
}

/* 0:close 1:open*/
function openHelpWindow(filename) {
	helpWindow = window.open(filename, "thewindow", "width=300,height=400,scrollbars=yes,resizable=yes,menubar=no");
}

function checkSave() {
	answer = confirm(msg_confirm_savepage);
	if (answer != 0) {
		return true;
	} else return false;
}

function alertPassword(formObj) {
	alert(msg_confirm_password);
	formObj.focus();
}
function isEqual(cp1, cp2) {
	if (parseInt(cp1.value) == parseInt(cp2.value)) {
		cp2.focus();
		return true;
	}
	else return false;
}
function setDisabled(OnOffFlag, formFields) {
	for (var i = 1; i < setDisabled.arguments.length; i++)
	setDisabled.arguments[i].disabled = OnOffFlag;
}

function cp_ip(from1, from2, from3, from4, to1, to2, to3, to4)
//true invalid from and to ip;  false valid from and to ip;
{
	var total1 = 0;
	var total2 = 0;

	total1 += parseInt(from4.value);
	total1 += parseInt(from3.value) * 256;
	total1 += parseInt(from2.value) * 256 * 256;
	total1 += parseInt(from1.value) * 256 * 256 * 256;

	total2 += parseInt(to4.value);
	total2 += parseInt(to3.value) * 256;
	total2 += parseInt(to2.value) * 256 * 256;
	total2 += parseInt(to1.value) * 256 * 256 * 256;
	if (total1 > total2) return true;
	return false;
}
function pi(val) {
	return parseInt(val, 10);
}
function alertR(str) {
	alert(str);
	return false;
}

