var DEBUG = false;

var HelpOptionsVar = "width=480,height=420,scrollbars,toolbar,resizable,dependent=yes";
var GlossOptionsVar = "width=420,height=180,scrollbars,toolbar,resizable,dependent=yes";
var bigsub = "width=560,height=440,scrollbars,menubar,resizable,status,dependent=yes";
var sitsub = "width=650,height=600,scrollbars,menubar,resizable,status,dependent=yes";
var rsub = "width=450,height=350,resizable,status,dependent=yes";
var Helpsub = "width=800,height=600,scrollbars,menubar,resizable,status,dependent=yes";

var smallsub = "width=500,height=360,scrollbars,resizable,dependent=yes";
var hugesub = "width=700,height=560,scrollbars,menubar,resizable,status,dependent=yes";
var prosub = "width=700,height=360,scrollbars,status,dependent=yes";
var logsub = "width=620,height=440,scrollbars,menubar,resizable,status,dependent=yes";
var logDsub = "width=510,height=500,dependent=yes";
var uamtimewin = "left=100,top=100,height=200,width=200,status=no,toolbar=no,menubar=no,location=no,resizable";
var helpWinVar = null;
var glossWinVar = null;
var datSubWinVar = null;
var ValidStr = 'abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789';
var hex_str = "ABCDEFabcdef0123456789";
var num_str = "0123456789";
var name_str = 'abcdefghijklmnopqrstuvwxyz-ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_ '
var invalidMSNameStr = "\"/\\[<]>.:;,|=+*? `";
var invalidDNSStr = "\"/\\[<]>:;,|=+*? ~!@#$%^&*()+=`{}";
document.write('<script language="javascript" type="text/javascript" src="ajax.js"></script>');
function dw(message) {
	document.write(message);
}

function showMsg() {
	if (top.frames.length > 0) top.frames[0].location.href = "setup.cgi?next_file=gheadmsg.htm";
	var msgVar = document.forms[0].message.value;
	if (msgVar.length > 1) alert(msgVar);
}

function closeWin(win_var) {
	if ((win_var != null) && (win_var.closed == false)) win_var.close();
}

function openHelpWin(file_name) {
	helpWinVar = window.open(file_name, 'help_win', Helpsub);
	if (helpWinVar.focus) setTimeout('helpWinVar.focus()', 200);
}

function openGlossWin() {
	glossWinVar = window.open('', 'gloss_win', GlossOptionsVar);
	if (glossWinVar.focus) setTimeout('glossWinVar.focus()', 200);
}

function openDataSubWin(filename, win_type) {
	if (datSubWinVar) {
		closeWin(datSubWinVar);
	}
	datSubWinVar = window.open(filename, 'datasub_win', win_type);
	if (datSubWinVar.focus) setTimeout('datSubWinVar.focus()', 200);
	return datSubWinVar;
}

function closeSubWins() {
	closeWin(helpWinVar);
	closeWin(glossWinVar);
	closeWin(datSubWinVar);
}

function checkBlank(fieldObj, fname) {
	var msg = "";
	if (fieldObj.value.length < 1) msg = addstr(msg_blank, fname);
	return msg;
}

function checkNoBlanks(fObj, fname) {
	var space = " ";
	if (fObj.value.indexOf(space) >= 0) return addstr(msg_space, fname);
	else return "";
}

function checkValid(text_input_field, field_name, Valid_Str, max_size, mustFill) {
	var error_msg = "";
	var size = text_input_field.value.length;
	var str = text_input_field.value;

	if ((mustFill) && (size == 0)) {
		error_msg = addstr(msg_blank_in, field_name);
		return error_msg;
	}
	if ((mustFill) && (size != max_size)) {
		error_msg = addstr(msg_mis_len, field_name, max_size);
		return error_msg;
	}
	for (var i = 0; i < size; i++) {
		if (! (Valid_Str.indexOf(str.charAt(i)) >= 0)) {
			error_msg = addstr(msg_invalid, field_name, Valid_Str);
			break;
		}
	}
	return error_msg;
}

function checkInvalidChars(inStr, InvalidStr) // check no chars of "InvalidStr" in "inStr"
{
	for (var i = 0; i < InvalidStr.length; i++)
	if (inStr.indexOf(InvalidStr.charAt(i)) >= 0) return false;
	return true;
}

function checkValidChars(inStr, validStr) // check all chars are "ValidStr" in "inStr"
{
	for (var i = 0; i < inStr.length; i++)
	if (validStr.indexOf(inStr.charAt(i)) < 0) return false;
	return true;
}

function checkAllNumChars(inStr) //check all chars are numeric
{
	for (var i = 0; i < inStr.length; i++)
	if (num_str.indexOf(inStr.charAt(i)) < 0) {
		return false;
	}
	return true;
}
function checkUrl(fObj, msg) {
	var url = fObj.value;

}
function checkInt(text_input_field, field_name, min_value, max_value, required)
// NOTE: Doesn't allow negative numbers, required is true/false
{
	var str = text_input_field.value;
	var error_msg = "";

	if (text_input_field.value.length == 0) // blank
	{
		if (required) error_msg = addstr(msg_blank, field_name);
	}
	else // not blank, check contents
	{
		for (var i = 0; i < str.length; i++) {
			if ((str.charAt(i) < '0') || (str.charAt(i) > '9')) error_msg = addstr(msg_check_invalid, field_name);
		}
		if (error_msg.length < 2) // don't parse if invalid
		{
			var int_value = parseInt(str, 10);
			if (int_value < min_value) error_msg = addstr(msg_greater, field_name, (min_value - 1));
			if (int_value > max_value) error_msg = addstr(msg_less, field_name, (max_value + 1));
		}
	}
	return (error_msg);
}

function blankIP(fn) // true if 0 or blank
{
	return ((fn.value == "") || (fn.value == "0"))
}
function checkStartIP(ip, msg) {
	var error_msg = "";

	if (ip.value == "127") {
		error_msg = addstr(msg_validIP1, msg);
		return error_msg;
	}

	if (! (isInt(ip.value, 1, 223))) error_msg = addstr(msg_validIP1, msg);

	return error_msg;
}

function checkIp(ip1, ip2, ip3, ip4, msg, rq_flag) {
	var errmsg = "";

	if ((rq_flag == false) && blankIP(ip1) && blankIP(ip2) && blankIP(ip3) && blankIP(ip4)) return "";

	errmsg = checkStartIP(ip1, msg);
	if (errmsg.length > 1) return errmsg;
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip2, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip3, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip4, msg, 1, 254, true);
	errmsg = (errmsg.length > 1) ? addstr(msg_validIP, msg) : "";
	return errmsg;
}

function checkNetMask(ip1, ip2, ip3, ip4, msg) {
	var errmsg = checkInt(ip1, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip2, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip3, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip4, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? addstr(msg_validMask, msg) : "";
	return errmsg;
}

/* check broadcast ip or ip address */
function checkIPorBroadcastAddress(ip1, ip2, ip3, ip4, msg) {
	var errmsg = "";
	
	if (ip1.value.length == 0 || ip2.value.length == 0 || ip3.value.length == 0 || ip4.value.length == 0) {
		errmsg = addstr(msg_blank, msg);
		return errmsg;
	}	
	if(ip1.value == "127")
	{
		errmsg = msg;	
		return errmsg;	
	}	
	errmsg = checkInt(ip1, msg, 1, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip2, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip3, msg, 0, 255, true);
	errmsg = (errmsg.length > 1) ? errmsg: checkInt(ip4, msg, 1, 255, true);
	errmsg = (errmsg.length > 1) ? addstr(msg_validBroadcast, msg) : "";
	 
	return errmsg;
}

function checkIPAddressHightLow(ip1, ip2, msg) {
	var errmsg = "";
	var int_value_ip1 = parseInt(ip1.value);
	var int_value_ip2 = parseInt(ip2.value);

	if (int_value_ip1 < int_value_ip2) {
		errmsg = addstr(msg_trustedhostIP, msg);
		return errmsg;
	}
	return errmsg;
}

function checkIPAddress(ip1, ip2, ip3, ip4, msg) {
	var errmsg = "";
	/*	var ipaddr = new Array(parseInt(ip1.value,10), parseInt(ip2.value,10), parseInt(ip3.value,10),parseInt(ip4.value,10));
    
	if(!(ipaddr[0] && ipaddr[1] && ipaddr[2] && ipaddr[3]))
	{
	    errmsg =  addstr(msg_validIP, msg);
	    return errmsg;
	}
*/
	//	if (ipaddr[0] < 1 || ipaddr[0] > 223 || 127 == ipaddr[0])
	if (!isInt(ip1.value, 1, 223) || "127" == ip1.value) {
		errmsg = addstr(msg_validIP1, msg);
		return errmsg;
	}
	//	if (ipaddr[1] < 0 || ipaddr[1] > 255)
	if (!isInt(ip2.value, 0, 255)) {
		errmsg = addstr(msg_validIP2, msg);
		return errmsg;
	}
	//	if (ipaddr[2] < 0 || ipaddr[2] > 255)
	if (!isInt(ip3.value, 0, 255)) {
		errmsg = addstr(msg_validIP3, msg);
		return errmsg;
	}
	//	if (ipaddr[3] < 1 || ipaddr[3] > 254)
	if (!isInt(ip4.value, 1, 254)) {
		errmsg = addstr(msg_validIP4, msg);
		return errmsg;
	}
	return errmsg;

}
function checkIpMask(ip1, ip2, ip3, ip4, msg) {
	var errmsg = "";
	if (ip1.value != 255) {
		if (!_isNumeric(ip1)) {
			errmsg = addstr(msg_validMask1, msg);
			return errmsg;
		}
		if (ip2.value != 0 || ip3.value != 0 || ip4.value != 0) {
			errmsg = addstr(msg_validMask, msg);
			return errmsg;
		}
		switch (parseInt(ip1.value, 10)) {
		case 192:
		case 224:
		case 240:
		case 248:
		case 252:
		case 254:
			break;
		default:
			errmsg = addstr(msg_validMask1, msg);
			return errmsg;
		}
	}
	else if (ip2.value != 255) {
		if (!_isNumeric(ip2)) {
			errmsg = addstr(msg_validMask2, msg);
			return errmsg;
		}
		if (ip3.value != 0 || ip4.value != 0) {
			errmsg = addstr(msg_validMask, msg);
			return errmsg;
		}
		//0, 128, 192, 224, 240, 248, 252, 254
		switch (parseInt(ip2.value, 10)) {
		case 0:
		case 128:
		case 192:
		case 224:
		case 240:
		case 248:
		case 252:
		case 254:
			break;
		default:
			errmsg = addstr(msg_validMask2, msg);
			return errmsg;
		}
	}
	else if (ip3.value != 255) {
		if (!_isNumeric(ip3)) {
			errmsg = addstr(msg_validMask3, msg);
			return errmsg;
		}
		if (ip4.value != 0) {
			errmsg = addstr(msg_validMask, msg);
			return errmsg;
		}
		//0, 128, 192, 224, 240, 248, 252, 254
		switch (parseInt(ip3.value, 10)) {
		case 0:
		case 128:
		case 192:
		case 224:
		case 240:
		case 248:
		case 252:
		case 254:
			break;
		default:
			errmsg = addstr(msg_validMask3, msg);
			return errmsg;
		}
	}
	else {
		if (!_isNumeric(ip4)) {
			errmsg = addstr(msg_validMask4, msg);
			return errmsg;
		}
		//0, 128, 192, 224, 240, 248, 252
		switch (parseInt(ip4.value, 10)) {
		case 0:
		case 128:
		case 192:
		case 224:
		case 240:
		case 248:
		case 252:
			break;
		default:
			errmsg = addstr(msg_validMask4, msg);
			return errmsg;
		}
	}
	return errmsg;
}

function checkMAC(fieldObj, fmsg, flag) {
	var address = fieldObj.value;

	var myRE = /[0-9a-fA-F]{12}/;
	var newMAC = "";
	if (address.length > 11) {
		newMAC = address.replace(/[:-]/g, "");
		fieldObj.value = newMAC;
	}
	if (flag && (newMAC == "000000000000")) return addstr(msg_invalidMAC, fmsg);
	if ((newMAC.length != 12) || (parseInt(newMAC.charAt(1), 16) % 2 != 0) || (myRE.test(newMAC) != true)) return addstr(msg_invalidMAC, fmsg);

	return "";
}

function checkOUI(fieldObj, fmsg) {
	var address = fieldObj.value;
	var myRE = /[0-9a-fA-F]{6}/;
	var newOUI = "";
	if (address.length > 5) {
		newOUI = address.replace(/[:-]/g, "");
		fieldObj.value = newOUI;
	}
	if ((newOUI.length != 6) || (newOUI == "000000") || (myRE.test(newOUI) != true)) return addstr(msg_invalidOUI, fmsg);
	else return "";
}

function isBigger(str_a, str_b)
//  true if a bigger than b
{
	var int_value_a = parseInt(str_a);
	var int_value_b = parseInt(str_b);
	return (int_value_a > int_value_b);
}

function getSelIndex(sel_object, sel_text) {
	if (sel_text.length == 0) return - 1; //  Nothing may be valid. e.g. New SAP contain errors & returned.
	var size = sel_object.options.length;
	for (var i = 0; i < size; i++) {
		if ((sel_object.options[i].text == sel_text) || (sel_object.options[i].value == sel_text)) return (i);
	}
	if (DEBUG) alert("Selected value " + sel_text + " not found in " + sel_object.name);
	return 0; // if no match
}

var showit = "block";
var hideit = "none";

function show_hide(el, shownow) // IE & NS6; shownow = true, false
{
	//	alert("el = " + el);
	if (document.all) {
		if (document.all(el)) document.all(el).style.display = (shownow) ? showit: hideit;
	}
	else if (document.getElementById) {
		if (document.getElementById(el)) document.getElementById(el).style.display = (shownow) ? showit: hideit;
	}
}

function returnAC(avar) {
	var str = "";
	for (var i = 0; i < avar.length; i++)
	str += avar[i] + ",";
	return str;
}

function chkSelected(selObj, err_msg) {
	if (! (selObj.selectedIndex >= 0)) {
		//  alert(err_msg);
		return false;
	}
	return true;
}

function swapSel(selObj, updown_str) {
	var cf = document.forms[0];
	var imin = (updown_str == "up") ? 1: 0;
	var imax = (updown_str == "up") ? selObj.options.length - 1: selObj.options.length - 2;
	var index = selObj.selectedIndex;
	var swap_index;

	if ((index >= imin) && (index <= imax)) {
		swap_index = (updown_str == "up") ? index - 1: index + 1;
		var temp_text = selObj.options[index].text;
		var temp_v = selObj.options[index].value;
		selObj.options[index].text = selObj.options[swap_index].text;
		selObj.options[index].value = selObj.options[swap_index].value;
		selObj.options[swap_index].text = temp_text;
		selObj.options[swap_index].value = temp_v;
		selObj.selectedIndex = swap_index;
		return true;
	}
	else return false;
} // no msg if can't move
function getRadioCheckedValue(radio_object) {
	var size = radio_object.length;
	for (var i = 0; i < size; i++) {
		if (radio_object[i].checked == true) return (radio_object[i].value)
	}
	return (radio_object[0].value); // first value if nothing checked
}

function getRadioIndex(radio_object, checked_value) {
	var size = radio_object.length;
	for (var i = 0; i < size; i++) {
		if (radio_object[i].value == checked_value) return i;
	}

	return 0; // if no match
}

function addstr(input_msg) {
	var last_msg = "";
	var str_location;
	var temp_str_1 = "";
	var temp_str_2 = "";
	var str_num = 0;
	temp_str_1 = addstr.arguments[0];
	while (1) {
		str_location = temp_str_1.indexOf("%s");
		if (str_location >= 0) {
			str_num++;
			temp_str_2 = temp_str_1.substring(0, str_location);
			last_msg += temp_str_2 + addstr.arguments[str_num];
			temp_str_1 = temp_str_1.substring(str_location + 2, temp_str_1.length);
			continue;
		}
		if (str_location < 0) {
			last_msg += temp_str_1;
			break;
		}
	}
	return last_msg;
}

function isIPaddr(addr) {
	var i;
	var a;
	a = addr.split(".");
	if (a.length != 4) {
		return false;
	}
	for (i = 0; i < a.length; i++)
	if (! (isInt(a[i], (i == 0 || i == 3) ? 1: 0, (i == 0 || i == 3) ? 254: 255))) return false;
	return true;
}

function isInt(str, min_value, max_value) {
	if (str.length == 0) // blank
	return false
	for (var i = 0; i < str.length; i++)
	if ((str.charAt(i) < '0') || (str.charAt(i) > '9')) return false;
	var int_value = parseInt(str, 10);
	if ((int_value < min_value) || (int_value > max_value)) return false;
	return true;
}
/* MD@CPU_AP add for smtp at 20080114 */
function checkMail(fobj, fname) {
	var tmp_str = fobj.value;
	var msg = "";
	var i = 0;
	//matching Email address format(regular expression)
	i = /^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z0-9]+$/.test(tmp_str);
	if (i == 0) msg = addstr(msg_invalid_email, fname);
	return msg;
}
/* add end */

function checkSMTPserver(fobj, fname) {
	var tmp_str = fobj.value;
	var msg = "";

	if (isIPaddr(tmp_str)) {
		var i;
		var a;
		a = tmp_str.split(".");
		if (a.length != 4) {
			return msg_smtpServerInvalid;
		}
		if (!isInt(a[0], 1, 223) || 127 == a[0]) {

			return msg_smtpServerInvalid;
		}
		if (!isInt(a[1], 0, 255)) {

			return msg_smtpServerInvalid;
		}
		if (!isInt(a[2], 0, 255)) {

			return msg_smtpServerInvalid;
		}
		if (!isInt(a[3], 1, 254)) {

			return msg_smtpServerInvalid;
		}
		return "";
	}
	var pattern = /^([a-zA-Z0-9]+[_|\-|\.]?)*[a-zA-Z0-9]+\.[a-zA-Z]{2,3}$/gi;

	if (!pattern.test(tmp_str)) msg = addstr(msg_invalid_email, fname);

	return msg;
}
function chkIP_MSname_dns(fobj, msg) {
	var a;
	var urlStr = fobj.value.toLowerCase();
	if (urlStr.substr(0, 7) == "http://") urlStr = urlStr.substr(7);
	else if (urlStr.substr(0, 8) == "https://") urlStr = urlStr.substr(8);
	if (urlStr.toString() == "0.0.0.0") return (msg_redirect_invalid);
	if (urlStr.toString() == "127.0.0.1") return (msg_redirect_invalid);
	if (urlStr.toString() == "224.0.0.1") return (msg_redirect_invalid);
	if (urlStr.toString() == "255.255.255.255") return (msg_redirect_invalid);
	a = urlStr.split(".");
	if (a.length == 1) return (msg_redirect_invalid);
	if ((a.length == 4) && isIPaddr(urlStr)) return "";
	//if (urlStr.length <16  && checkInvalidChars(urlStr,invalidMSNameStr) && (!checkAllNumChars(urlStr)))
	if (urlStr.length < 16 && (!checkAllNumChars(urlStr))) return "";
	//if ((a.length >= 3) && (checkInvalidChars(urlStr,invalidDNSStr)))
	if (a.length >= 3) return "";

	if (checkAllNumChars(urlStr)) {
		return (msg + msg_not_number);
	}
	else return msg;
}

function chkIP_MSname(fobj, msg) // input could be ip or name
{
	var a = fobj.value.split(".");
	if (fobj.value.length == 0) return msg;
	if ((a.length == 4) && (!(isIPaddr(fobj.value)))) return msg;
	if (isIPaddr(fobj.value)) return "";
	if (checkInvalidChars(fobj.value, invalidMSNameStr) && (!checkAllNumChars(fobj.value))) return "";
	if (checkAllNumChars(fobj.value)) {
		return (msg + msg_not_number);
	}
	else return msg;
}

function chkIP_dns(fobj, msg) // input could be ip or dns
{
	var a = fobj.value.split(".");
	if (fobj.value.length == 0) return msg;
	if (a.length == 4) {
		if (isIPaddr(fobj.value)) return "";
		else {
			for (var i = 0; i < fobj.value.length; i++)
			if (((fobj.value.charAt(i) < '0') || (fobj.value.charAt(i) > '9')) && (fobj.value.charAt(i) != '.')) break;
			if (i == fobj.value.length) return msg;
		}
	}
	if ((a.length >= 3) && (checkInvalidChars(fobj.value, invalidDNSStr))) return "";
	else return msg;
}

function chkMSname(fobj, msg) // input could be name
{
	if (checkInvalidChars(fobj.value, invalidDNSStr) && (!checkAllNumChars(fobj.value))) return "";
	if (checkAllNumChars(fobj.value)) {
		return (msg + msg_not_number);
	}
	else return msg;
}

function chkHostName(fobj, msg) // input could be name
{
	if (!checkValidChars(fobj.value, ValidStr)) {
		return msg;
	} else if (checkAllNumChars(fobj.value)) {
		return (msg + msg_not_number);
	} else if (fobj.value.charAt(0) == '-' || fobj.value.charAt(fobj.value.length - 1) == '-') {
		return (msg + msg_hyphen);
	}
	else return "";
}

function chkDot1xName(fobj, msg) // input could be name
{
	if (fobj.value == "") return msg;
	if (checkInvalidChars(fobj.value, invalidMSNameStr)) return "";
	else return msg;
}

function isHex(str) {
	for (i = 0; i < str.length; i++) {
		if (isNaN(parseInt(str.charAt(i), 16))) return false;
	}
	return true;
}
function isInvalidChar(str) {
	for (i = 0; i < str.length; i++) {
		if (str.charAt(i) < ' ' || str.charAt(i) > '~') {
			return false;
		}
	}
	return true;
}
function checkWpaPass(pass, msg) {
	if (pass.length > 64 || pass.length < 8) {
		return (msg + msg_keysize_error);
	}
	if (pass.length < 65) {
		if (!isInvalidChar(pass)) {
			return (msg + msg_key_error);
		}
	}
	return "";
}
function checkRadiusKey(key, msg) {
	if (key.length > 64 || key.length < 1) {
		return (msg + msg_r_key_size);
	}
	if (key.length < 64) {
		if (!isInvalidChar(key)) {
			return (msg + msg_key_error);
		}
	}
	return "";
}
function setDisabled(dflag, objects) // objects can be individual, or an array of objects
{
	for (var i = 1; i < setDisabled.arguments.length; i++) {
		if (setDisabled.arguments[i].type == undefined && setDisabled.arguments[i].length) // array
		for (var j = 0; j < setDisabled.arguments[i].length; j++)
		setDisabled.arguments[i][j].disabled = dflag;
		else setDisabled.arguments[i].disabled = dflag;
	}
}

function setOptions(selObj, optionList) // rebuild option list from parameters
{
	var oldValue = selObj.options[selObj.selectedIndex].text;
	selObj.options.length = 0;
	for (var i = 1; i < setOptions.arguments.length; i++) {
		selObj.options[selObj.options.length] = new Option(arguments[i], arguments[i]);
	}
	selObj.selectedIndex = getSelIndex(selObj, oldValue)
}

function setOptionsWithValue(selObj, optionList) // rebuild option list from parameters
{
	var oldValue = selObj.options[selObj.selectedIndex].text;
	selObj.options.length = 0;
	for (var i = 1; i < setOptionsWithValue.arguments.length; i += 2) {
		selObj.options[selObj.options.length] = new Option(arguments[i], arguments[i + 1]);
	}
	selObj.selectedIndex = getSelIndex(selObj, oldValue)
}

function showHead(product_descrip, product_name, fw_version, menu) {
	var strHtml;

	strHtml = '<tr>' + '<td width="164"><img border="0" src="" width="164" height="57"></td>' + '<td width="646" colspan="2" valign="bottom" class="fwv">' + fv + ': ' + fw_version + '</td>' + '</tr>' + '<!-- header 2 -->' + '<tr>' + '<td colspan="3" height="11"><img border="0" src="UI_10.gif" width="810" height="11"></td>' + '</tr>' +

	'<!-- header 3 -->' + '<tr>' + '<td rowspan="4" width="164" align="center" class="bighead">' + menu + '</TD>' + '<td width="556" align="right" height="33" class="pname">' + product_descrip + '</TD>' + '<td width="90" height="12" align="center" class="mname">' + product_name + '</TD>' + '</TR>'

	' <!-- header 3 -->' + '<tr>' + ' <TD colspan="2" height="2" width="646" bgcolor="#000000"> </TD>' + ' </TR>';
	document.write(strHtml);
}
function showMenuItem(menu) {
	var strHtml;

	if (menu == vadmin) strHtml = '<td class="thistab" width="105">' + menu + '</td>';
	else strHtml = '<td class="thistab" width="83">' + menu + '</td>';
	document.write(strHtml);
}

function showMenuLink(mlink, menu) {
	var strHtml;

	if (menu == vadmin) strHtml = '<td class="menucell" width="105"><a class="mainmenu" href="' + mlink + '">' + menu + '</a></td>';
	else strHtml = '<td class="menucell" width="83"><a class="mainmenu" href="' + mlink + '">' + menu + '</a></td>';
	document.write(strHtml);
}

function showSubmenu(submenu) {
	var strHtml = '<span class="current">' + submenu + '</span>';
	document.write(strHtml);
}
function showSubmenuLink(sublink, submenu) {
	var strHtml = '<a class="submenu" href="' + sublink + '">' + submenu + '</a>';
	document.write(strHtml);
}
function showMenu(menu, submenu) {
	var strHtml;
	var arrMenuItem = new Array(vsetup, vwireless, vap_mode, vadmin, vstatus);
	var arrMenuLink = new Array("Setup.htm", "Wireless.htm", "ApMode.htm", "Administration.htm", "Status.htm");
	var arrsubSetup = new Array(vsbasic, vstime, vsadv);
	var arrSubSetupLink = new Array("Setup.htm", "SetupTime.htm", "SetupAdvanced.htm");
	var arrSubWire = new Array(vwbasic, vwsecurity, vwconn, vwwps, vwvlan, vwadv);
	var arrSubWireLink = new Array("Wireless.htm", "WSecurity.htm&unit=0&vap=0", "WMACFilter.htm&unit=0&vap=0", "WpsSetup.htm", "VLAN.htm", "AdvancedWSettings.htm");
	var arrSubAPmode = new Array(vap_mode);
	var arrSubAPmodeLink = new Array("ApMode.htm");
	var arrSubAdmin = new Array(vamanag, valog, vadiag, vafacdef, vafirup, vareboot, vaconfig);
	var arrSubAdminLink = new Array("Administration.htm", "Log.htm", "Diagnostics.htm", "FactoryDefaults.htm", "FirmwareUpgrade.htm", "Reboot.htm", "ConfigManagement.htm", "CertManagement.htm");
	var arrSubStatus = new Array(vslan, vswire, vsper);
	var arrSubStatusLink = new Array("Status.htm", "StatusWireless.htm", "StatusSys.htm");

	var i;
	var subMenu;
	var subMenuLink;

	if (menu == vsetup) {
		subMenu = arrsubSetup;
		subMenuLink = arrSubSetupLink;
		strHtml = '<TR>' + '<TD height="42" colspan="2" width="646">' + '<table border="0" cellspacing="0" width="646" height="100%">' + '<tr>' + '<td height="10" background="UI_07.gif"> </td>' + '<td height="10" background="UI_06.gif" colspan="6"> </td>' + '</tr></tr>';

	}
	else if (menu == vwireless) {
		subMenu = arrSubWire;
		subMenuLink = arrSubWireLink;
		strHtml = '<TR>' + '<TD height="42" colspan="2" width="646">' + '<table border="0" cellspacing="0" width="646" height="100%">' + '<tr>' + '<td height="10" background="UI_06.gif"></td>' + '<td height="10" background="UI_07.gif"> </td>' + '<td height="10" background="UI_06.gif" colspan="5"> </td>' + '</tr></tr>';
	}
	else if (menu == vap_mode) {
		subMenu = arrSubAPmode;
		subMenuLink = arrSubAPmodeLink;
		strHtml = '<TR>' + '<TD height="42" colspan="2" width="646">' + '<table border="0" cellspacing="0" width="646" height="100%">' + '<tr>' + '<td height="10" colspan="2" background="UI_06.gif"></td>' + '<td height="10" background="UI_07.gif"></td>' + '<td height="10" colspan="4" background="UI_06.gif"></td>' + '</tr></tr>';
	}
	else if (menu == vadmin) {
		subMenu = arrSubAdmin;
		subMenuLink = arrSubAdminLink;
		strHtml = '<TR>' + '<TD height="42" colspan="2" width="646">' + '<table border="0" cellspacing="0" width="646" height="100%">' + '<tr>' + '<td height="10" colspan="3" background="UI_06.gif"></td>' + '<td height="10" background="UI_07.gif"></td>' + '<td height="10" colspan="3" background="UI_06.gif"></td>' + '</tr></tr>';
	}
	else {
		subMenu = arrSubStatus;
		subMenuLink = arrSubStatusLink;
		strHtml = '<TR>' + '<TD height="42" colspan="2" width="646">' + '<table border="0" cellspacing="0" width="646" height="100%">' + '<tr>' + '<td height="10" colspan="4" background="UI_06.gif"></td>' + '<td height="10" background="UI_07.gif"></td>' + '<td height="10" colspan="2" background="UI_06.gif">' + '</tr></tr>';
	}
	document.write(strHtml);

	for (i = 0; i < 5; i++) {
		if (menu == arrMenuItem[i]) {
			showMenuItem(arrMenuItem[i]);
		} else {
			showMenuLink(arrMenuLink[i], arrMenuItem[i]);
		}
	}
	strHtml = '<td class="menucell" width="83"></td>' + '</tr>' + '</table>' + ' </td>' + '</tr>';
	document.write(strHtml);

	//sub menu
	//     <!-- sub menu--> 
	//    <TR>
	//      <TD height="21" colspan="2" width="646"> &nbsp; &nbsp;
	//      <span class="current">Basic Setup</span><b class="separator">|</b>&nbsp;&nbsp;
	//      <a class="submenu" href="SetupTime.htm">Time</a><b class="separator">|</b>&nbsp;&nbsp;
	//      <a class="submenu" href="SetupAdvanced.htm">Advanced</a>
	//      </td>
	//    </tr>
	strHtml = '<TR>' + '<TD height="21" colspan="2" width="646">&nbsp;&nbsp;';
	document.write(strHtml);
	for (i = 0; i < subMenu.length; i++) {
		if (i != 0) document.write('<b class="separator">|</b>');
		if (submenu == subMenu[i]) showSubmenu(subMenu[i]);
		else showSubmenuLink(subMenuLink[i], subMenu[i]);
	}
	strHtml = '</td></tr>';
	document.write(strHtml);
}
function showSave() {
	dw('<input type="submit" name="save" value="');
	dw(save);
	dw('" class="stdbutton" onClick="return checkData();">');
}

function showCancel(curpage) {
	dw('<input type="RESET" name="cancel" value="');
	dw(cancel);
	dw('" class="stdbutton"  onClick="location.href=\'');
	dw(curpage);
	dw('\' ; "> &nbsp;  &nbsp; ');
}

function showRefresh() {
	dw('<input type="BUTTON" name="Refresh" value="');
	dw(btn_refresh);
	dw('" class="stdbutton" onClick="refresh()">');
}

function showButton(butname, butval, clickval) {
	var strHtml = '<input type="BUTTON" name="' + butname + '" value="' + butval + '"class="stdbutton" onClick="' + clickval + '">';
	document.write(strHtml);
}

function printPage() {
	window.print();
}

var isIE = navigator.userAgent.indexOf("MSIE") != - 1;

function CountObjWidth(obj) {
	var ObjLeft = 0;
	while (obj != null) {
		ObjLeft += obj.offsetLeft * 1;
		obj = obj.offsetParent;
	}
	return ObjLeft;
};
function CountObjHeight(obj) {
	var ObjHeight = 0;
	while (obj != null) {
		//	alert(obj.id+"=="+obj.offsetTop);
		ObjHeight += obj.offsetTop * 1;
		obj = obj.offsetParent;
	}

	return ObjHeight;
};

function CountNewHeight(id) {
	var space = 0;
	var obj = document.getElementById(id);
	if (obj == null) return;
	var spaceBelow = document.getElementById("MCopyright");
	if (spaceBelow != null) space += spaceBelow.offsetHeight;
	var newHeight = document.body.clientHeight - CountObjHeight(obj) - space;

	if (parseInt(newHeight) < 0) {
		newHeight = 0;
	}

	if (obj.style != null) obj.style.height = newHeight + "px";
	else obj.height = newHeight + "px";

}

function btn_mouse_over(object) {
	if (object.className == "btn_normal") object.className = "btn_over";
}
function td_mouse_over(object) {
	if (object.className == "TableTd") object.className = "TdMouseOver";

}
function btn_mouse_out(object) {
	if (object.className == "btn_over") object.className = "btn_normal";
}
function td_mouse_out(object) {
	if (object.className == "TdMouseOver") object.className = "TableTd";
}
var DivName = ["btn_div0","btn_div1", "btn_div2", "btn_div3", "btn_div4", "btn_div5"];
var DisDiv = ["","clk_div1", "clk_div2", "clk_div3", "clk_div4", "clk_div5"];
var LinkDiv = ["0-0","1-1", "2-1", "3-1", "4-1", "5-1"];
var PicName = ["Toggler0","Toggler1", "Toggler2", "Toggler3", "Toggler4", "Toggler5"];
var RightID = ["0-0","1-1", "1-2", "1-3", "2-1", "2-2", "2-3", "2-4", "2-5", "2-6", "3-1", "4-1", "4-2", "4-3", "4-4", "4-5", "4-6", "4-7", "4-8", "5-1", "5-2", "5-3"];
var RightPage = ["Wizard.htm", "Setup.htm", "SetupTime.htm", "SetupAdvanced.htm", "Wireless.htm", "WSecurity.htm&unit=0&vap=0", "WMACFilter.htm&unit=0&vap=0", "WpsSetup.htm", "VLAN.htm", "AdvancedWSettings.htm", "ApMode.htm", "Administration.htm", "Log.htm", "Diagnostics.htm", "FactoryDefaults.htm", "FirmwareUpgrade.htm", "Reboot.htm", "ConfigManagement.htm", "CertManagement.htm", "Status.htm", "StatusWireless.htm", "StatusSys.htm"];
function SetDisDiv(id1, id2)
{
	if(id1.length==0)
		return;	
	document.getElementById(id1).style.display = "block";
	//document.getElementById(id2).focus();
	if (isIE) document.getElementById(id2).click();
	else {
		menu_clk(document.getElementById(id2));
	}
	return;
}
function SetNoneDiv(id) {
	if(id.length==0)
		return;
	document.getElementById(id).style.display = "none";
	return;

}
function SetPic(id, type) {
	if(id=="Toggler0")
		return;
	if (type == 1) {
		document.getElementById(id).src = "./toggleD.gif";
		document.getElementById(id).title = home_collapse;
	}
	else {
		document.getElementById(id).src = "./toggleR.gif";
		document.getElementById(id).title = home_expand;
	}
	return;
}
function SetClk(object) {
	var id = object.id;
	var j = 0;
	
	object.className = "btn_clk";
	for (i = 0; i < 6; i++) {
		if (id != DivName[i]) {
			//	if(document.getElementById(DivName[i]).className!="btn_normal"){
			document.getElementById(DivName[i]).className = "btn_normal";
			SetPic(PicName[i], 0);
			SetNoneDiv(DisDiv[i]);
			//	}
		}
		else {
			SetPic(PicName[i], 1);
			j = i;
			SetDisDiv(DisDiv[i], LinkDiv[i]);
			i = j;
		}

	}
	return;

}

function Toggleclick(object) {
	for (i = 0; i < 5; i++) {
		if (object.id == DivName[i]) break;
	}
	if (i >= 5) return;

	if (document.getElementById(PicName[i]).title == "Expand") {
		SetClk(object);
	}
	else {
		object.className = "btn_normal";
		document.getElementById(DisDiv[i]).style.display = "none";
		SetPic(PicName[i], 0);
	}
	return;
}

function btn_clk(evt, object) {

	var eventObjid = (isIE) ? evt.srcElement.id: evt.target.id;
	if (eventObjid.indexOf("Toggler") != - 1) {
		Toggleclick(object);
		return;
	}
	if (object.className == "btn_clk") return;
	SetClk(object);

}

function  btn_clk1(object)
{
	if(object.className=="btn_clk"){
		return;
	}
	SetClk(object);
}

function menu_clk(obj) {
	if (obj.className != "LmenuSel") obj.className = "LmenuSel";

	for (i = 0; i < 22; i++) {
		if (obj.id == RightID[i]) {
			if(obj.id=="0-0"){
				window.parent.top.showWizard ();
			}
			else{
				document.getElementById("rightframe").src = "./" + RightPage[i];
				changehelp("init", RightPage[i]);
			}
		}
		else {
			if(RightID[i].length==0)
				continue;
			var objname = document.getElementById(RightID[i]).className;
			if (objname == "LmenuSel") document.getElementById(RightID[i]).className = "Lmenu";
		}
	}
	return;

}
function getPagebyName(nm) {
	for (i = 0; i < 21; i++) {
		if (nm == RightPage[i]) {
			menu_clk(document.getElementById(RightID[i]));
			return 0;
		}
	}
	return 1;
}
var WidthofScroller = isIE ? 19: 16;
var high = 0;
var wid = 0;
function IsHScrollofContent(id) {
	try {
		if (id == null) id = "RightContentArea";
		var obj = document.getElementById(id);
		return (obj.scrollHeight > obj.offsetHeight);
	}
	catch(e) {}
	return false;
};
function UpdateContentArea(id) {
	try {
		var space = 10;
		var obj = document.getElementById(id);
		if (obj == null) return;
		high = (document.body.clientHeight - CountObjHeight(obj));

		var fact = isIE ? 1: 2;
		var parentWidth = Math.max(obj.parentNode.clientWidth, obj.parentNode.offsetWidth);
		var ContentAreaNewW = (document.body.clientWidth - CountObjWidth(obj));
		wid = Math.min((parentWidth - fact * space), ContentAreaNewW);
		if (obj.style != null) {
			obj.style.height = high + "px";
			obj.style.width = wid + "px";
			//   alert(obj.style.width);
			obj.style.marginLeft = space + "px";
			obj.style.paddingRight = space + "px";
		}
		else {
			obj.height = high + "px";
			obj.width = wid + "px";
		}
		/*
        if (obj.clientWidth != obj.offsetWidth)
            WidthofScroller = Math.max(19, Math.min(25, obj.offsetWidth -
                obj.clientWidth));
        else if (obj.clientHeight != obj.offsetHeight)
            WidthofScroller = Math.max(16, Math.min(25, obj.offsetWidth -
                obj.clientWidth));
                */
	}
	catch(e) {}
};

function ResizeContent() {

	UpdateContentArea("RightContentArea");
	/* try
    {
    	
        if (parent.window != null )
        {
          // var myIframe = parent.window.document.getElementsByTagName("iframe")[0];
         //   var t =  - 1 * CountObjHeight(myIframe);
         //   var l =  - 1 * CountObjWidth(myIframe);
           // alert(t+","+l);
         //document.body.style.backgroundPosition = l + " " + t;
            
        }
    }
   catch(e){}
   */

};

var CntOnresizeTimer = null;
function CntOnresizeHandler() {

	if (CntOnresizeTimer != null) clearTimeout(CntOnresizeTimer);
	CntOnresizeTimer = setTimeout("ResizeContent()", 150);
};

function MouseOverGripper(evt, obj) {
	obj.style.cursor = "col-resize";
	obj.parentNode.style.backgroundColor = "#0088c2";
}
function MouseOutGripper(evt, obj) {
	obj.style.cursor = "default";
	obj.parentNode.style.backgroundColor = "";
}
function FindObjByTagname(obj, tagName) {
	try {
		tagName = tagName.toLowerCase();
		while (obj.tagName.toLowerCase() != tagName && obj != null)
		obj = obj.parentNode;
	}
	catch(e) {
		obj = null;
	}
	return obj;
};
var ResizeBox = null;
var ResizeLayer = null;
var Resizing = false;

function GetMoveArea() {
	var MoveArea = null;
	try {
		var AllArea = document.getElementById("MLayout");
		var numofcell = AllArea.rows[0].cells.length;
		for (var i = 0; i < numofcell; i++) {
			if (AllArea.rows[0].cells[i].id == "MMoveArea") {
				MoveArea = AllArea.rows[0].cells[i - 1];
				break;
			}
		}
	}
	catch(e) {}
	return MoveArea;
}
function EventBubbleStop(evt) {
	try {
		if (evt == null) evt = window.event;
		if (evt.stopPropagation) evt.stopPropagation();
		evt.cancelBubble = true;
		evt.returnValue = false;
	}
	catch(e) {}

};

function MLayerMouseUp(evt, obj) {
	try {
		if (!Resizing) {
			return MResizeAborted(evt);
		}
		DelMoveListeners();
		if (evt == null) evt = window.event;
		ResizeLayer.style.display = "none";
		var moveArea = GetMoveArea();
		if (moveArea != null) {
			var space = 0;
			var wid;
			var spaceLeft = document.getElementById("MLeftSpace");
			if (spaceLeft != null) space = spaceLeft.offsetWidth;
			if ((evt.clientX - space < 0) || (evt.clientX > (document.body.offsetWidth - space))) {
				MResizeAborted(evt);
				return;
			}
			else wid = (evt.clientX - space) + "px";

			moveArea.width = wid;
			var divs = moveArea.getElementsByTagName("div");
			var indexid = - 1;
			for (var j = 0; j < divs.length; j++) {
				if (divs[j].className.indexOf("MLeftContainer") == 0) {
					indexid = j;
					divs[j].style.width = wid;
				}
				else if (divs[j].className.indexOf("MLeftLayer") == 0) divs[j].style.width = wid;
			}

		}
		//  EventBubbleStop(evt);
		//  document.selection.empty();
		return false;
	} catch(e) {}
	return false;
}
function DelMoveListeners() {
	if (document.removeEventListener) {
		document.removeEventListener('mousemove', MLayerMouseMove, false);
		document.removeEventListener('mouseup', MLayerMouseUp, false);
		//  document.removeEventListener('mouseout', MLayerMouseOut, false); 
	}
	else if (document.detachEvent) {
		document.detachEvent('onmousemove', MLayerMouseMove);
		document.detachEvent('onmouseup', MLayerMouseUp);
		// document.detachEvent('onmouseout', MLayerMouseOut);
	}
}
function MResizeAborted(evt, obj) {
	try {
		Resizing = false;
		ResizeLayer.style.display = "none";
		DelMoveListeners();
		//  EventBubbleStop(evt);
		//  document.selection.empty();
		return false;
	} catch(e) {}
	return false;
}
function MLayerMouseMove(evt) {
	try {
		if (evt == null) evt = window.event;

		if (!Resizing || evt.clientX < 0 || evt.clientX > document.body.offsetWidth) {
			// alert(evt.clientX+","+document.body.offsetWidth);
			MResizeAborted(evt);
		}
		// needed for IE which sends events WAY too often
		if (ResizeBox.style.left == evt.clientX + "px") return false;
		ResizeBox.style.left = evt.clientX;
		//  EventBubbleStop(evt);
		//  document.selection.empty();
		return false;
	} catch(e) {}
	return false;
}
function MLayerMouseOut(evt, obj) {
	try {
		if (evt == null) evt = window.event;
		if (!Resizing || evt.clientX < 0 || evt.clientX > document.body.offsetWidth) {
			return MResizeAborted(evt);
		}
		// EventBubbleStop(evt);
		// document.selection.empty();
		return false;
	} catch(e) {}
	return false;

}
function AddMoveListeners() {
	if (document.addEventListener) {
		document.addEventListener('mousemove', MLayerMouseMove, false);
		document.addEventListener('mouseup', MLayerMouseUp, false);
		//   document.addEventListener('mouseout', MLayerMouseOut, false); 
	}
	else if (document.attachEvent) {
		document.attachEvent('onmousemove', MLayerMouseMove);
		document.attachEvent('onmouseup', MLayerMouseUp);
		// document.attachEvent('onmouseout', MLayerMouseOut);
	}
}

function ResizeLayerMouseDown(evt, obj) {
	try {
		if (evt == null) evt = window.event;
		var obj = (isIE) ? evt.srcElement: evt.target;
		if (obj.tagName.toLowerCase() == "img") {
			obj = FindObjByTagname(obj, "td");
			obj.style.backgroundColor = "";
		}
		if (obj.id != "MMoveArea") return true;
		var MoveArea = GetMoveArea();
		if (MoveArea != null && MoveArea.style.display == "none") {
			//    EventBubbleStop(evt);
			//  document.selection.empty();
			return false;
		}
		if (ResizeLayer == null) ResizeLayer = document.getElementById("MResizeLayer");
		ResizeLayer.style.display = "block";
		ResizeLayer.style.cursor = "col-resize";
		// force to be last element in page.  zIndex not enough for Firefox
		document.body.appendChild(ResizeLayer);
		if (ResizeBox == null) ResizeBox = document.getElementById("MResizeBox");
		//  alert(CountObjHeight(obj));
		ResizeBox.style.top = 63;
		ResizeBox.style.left = CountObjWidth(obj);
		ResizeBox.style.width = obj.offsetWidth;
		ResizeBox.style.height = obj.offsetHeight;
		Resizing = true;
		AddMoveListeners();
		//   EventBubbleStop(evt);
		//  document.selection.empty();
		return false;
	} catch(e) {}
	return false;
}

function AboutOpen(url) {
	var width = 480;
	var Resize = "yes";
	var AboutWindow = window.open(url, "AboutWindow", "status=no,scrollbars=no,resizable=" + Resize + ",height=345px,width=" + width + "px");

}

function ResizeContentToWindow() {
	try {
		var obj = null;
		for (var i = document.body.childNodes.length - 1; i >= 0; i--)
		if (document.body.childNodes[i].tagName != null && document.body.childNodes[i].tagName.toLowerCase() != "script") {
			obj = document.body.childNodes[i];
			if (obj.offsetHeight > 0) break;
		}
		var high = CountObjHeight(obj);
		var neededhigh = high + (obj.offsetHeight * 1);
		var ResizeHeight = neededhigh - window.document.body.offsetHeight;

		window.resizeBy(0, ResizeHeight);
		window.moveBy(0, - 1 * ResizeHeight / 2);

	}
	catch(e) {}
}

var helppage;

function changehelp(obj, Hpage) {

	if (obj == 'init') {
		helppage = Hpage;
	}
	else if (obj == 'change') {
		var p = helppage.indexOf(".");
		var str = help_dir + "/h_" + helppage.substring(0, p) + ".htm";
		if ((str == help_dir + "/h_StatusSys.htm") || (str == help_dir + "/h_StatusWireless.htm")) str = help_dir + "/h_Status.htm";
		openHelpWin(str);

	}
}

function PNGFix(id) {
	//  if (!isIE || navigator.userAgent.indexOf("MSIE 7") !=  - 1)
	//     return ;
	if (!isIE) return;
	var Limage = document.getElementById(id);
	if (!Limage) return;

	if (!Limage.Height) Limage.Height = 59;
	if (!Limage.width) Limage.width = 93;
	Limage.style.cssText = "width:" + Limage.width + "px;height:" + Limage.Height + "px;filter:progid:DXImageTransform.Microsoft.AlphaImageLoader(src='" + Limage.src + "', sizingMethod=scale);";
	Limage.src = "./spacer.gif";

};

/* Alert Box */
var ABTRF = "AB_FRAME";
var ABBtnSubmit = "<div id=\"AB_OK_S\" class=\"Norm2\" onMouseOver=\"this.className='Over2'\" onMouseOut=\"this.className='Norm2'\" onClick=\"OK_SUBMIT('" + ABFile + "');\">" + btn_ok + "</div>";
var wizard_ABBtnSubmit = "<div id=\"AB_OK_S\" class=\"Norm2\" onMouseOver=\"this.className='Over2'\" onMouseOut=\"this.className='Norm2'\" onClick=\"wizard_OK_SUBMIT('" + ABFile + "');\">" + btn_ok + "</div>";

var ABBtnOK = "<div id=\"AB_OK\" class=\"Norm2\" onMouseOver=\"this.className='Over2'\" onMouseOut=\"this.className='Norm2'\" onClick=\"OK_AB('" + ABFile + "');\">" + btn_ok + "</div>";
var wizard_ABBtnOK = "<div id=\"AB_OK\" class=\"Norm2\" onMouseOver=\"this.className='Over2'\" onMouseOut=\"this.className='Norm2'\" onClick=\"wizard_OK_AB('" + ABFile + "');\">" + btn_ok + "</div>";
var ABBtnCANCEL = "<div id=\"AB_CANCEL\" class=\"Norm5\" onMouseOver=\"this.className='Over5'\" onMouseOut=\"this.className='Norm5'\" onClick=\"CANCEL_AB('" + ABFile + "');\">" + cancel + "</div>";
var wizard_ABBtnCANCEL = "<div id=\"AB_CANCEL\" class=\"Norm5\" onMouseOver=\"this.className='Over5'\" onMouseOut=\"this.className='Norm5'\" onClick=\"wizard_CANCEL_AB('" + ABFile + "');\">" + cancel + "</div>";

var ABBtnYES = "<div id=\"AB_YES\" class=\"Norm3\" onMouseOver=\"this.className='Over3'\" onMouseOut=\"this.className='Norm3'\" onClick=\"YES_AB('" + ABFile + "');\">" + ar_yes + "</div>";
var ABBtnNO = "<div id=\"AB_NO\" class=\"Norm2\" onMouseOver=\"this.className='Over2'\" onMouseOut=\"this.className='Norm2'\" onClick=\"enableAllFields();\">" + ar_no + "</div>";

var ABSubBtnOK = "<div id=\"AB_Sub_OK\" class=\"Norm2\" onMouseOver=\"this.className='Over2'\" onMouseOut=\"this.className='Norm2'\" onClick=\"OK_Sub_AB('" + ABFile + "');\">" + btn_ok + "</div>";

//element
var ABTitle;
var ABFile; // current file with
var ABType; // Crit, Info, Warn
var ABMsg; // Message
var ABNOM; // No/Yes Message
var ABBT1; // Button 1
var ABBT2; // Button 2
var ABNum; // Message Number / Numbers of Message
var BOX;

function setABox(windowObj, el, ABTitle, ABFile, ABType, ABMsg, ABNOM, ABBT1, ABBT2) {
	BOX = '<table border="0" cellspacing="0" cellpadding="0" class="AB_TABLE"><tr><td colspan="3" class="AB_TD_top"><table border="0" cellspacing="0" cellpadding="0" class="top_TABLE"><tr><td class="top_td_l"></td><td id="AB_Title" class="top_td_ct">' + ABTitle + "</td><td class=\"top_td_cc\"><img src=\"img_alert/Close.gif\" class=\"td_cc_img\"  onClick=\"CANCEL_AB('" + ABFile + "');\"></td><td class=\"top_td_r\"></td></tr></table></td></tr><tr><td class=\"AB_TD_Ml\"></td><td class=\"AB_TD_Mc\"><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" class=\"Help_TABLE\"><tr><td></td></tr></table><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" class=\"MSG_TABLE\"><tr><td id=\"AB_Type\" class=\"MSG_type\"><img id=\"AB_Tye\" src=\"img_alert/ICON-" + ABType + '.gif"></td><td id="AB_Msg" class="MSG_cont">' + ABMsg.replace(/\n/g, "<br>") + '</td></tr></table><table border="0" cellspacing="0" cellpadding="0" class="MSG_TABLE"><tr><td class="MSG_lc"></td><td id="AB_NoMsg" class="MSG_no">' + ABNOM + '</td></tr></table><table border="0" cellspacing="0" cellpadding="0" class="MSG_TABLE"><tr><td class="MSG_BTN_t"></td></tr><tr><td><table border="0" cellspacing="0" cellpadding="0" align="center"><tr><td id="AB_Btn1">' + ABBT1 + '</td><td id="AB_Btn2">' + ABBT2 + '</td></tr></table></td></tr></table><input type="hidden" name="h_NoMsg" value="@h_NoMsg#"><table border="0" cellspacing="0" cellpadding="0" class="MSG_TABLE"></table></td><td class="AB_TD_Mr"></td></tr><tr><td colspan="3" class="AB_TD_btm"><table border="0" cellspacing="0" cellpadding="0" class="btm_TABLE"><tr><td class="btm_td_l"></td><td class="btm_td_cc"></td><td class="btm_td_r"></td></tr></table></td></tr></table>';

	if (document.all) {
		if (windowObj.document.all(el)) {
			windowObj.document.all(el).innerHTML = BOX;
		}
	}
	else if (document.getElementById) {
		if (windowObj.document.getElementById(el)) {
			windowObj.document.getElementById(el).innerHTML = BOX;
		}
	}
	disableAllFields();
	if (ABNOM != "") {
		var af = document.ABox.NoMsg;
		af.disabled = false;
	}
	else {
		ABNOM = "&nbsp;";
	}
}

function wizard_setABox(windowObj, el, ABTitle, ABFile, ABType, ABMsg, ABNOM, ABBT1, ABBT2) {
	BOX = '<table border="0" cellspacing="0" cellpadding="0" class="AB_TABLE"><tr><td colspan="3" class="AB_TD_top"><table border="0" cellspacing="0" cellpadding="0" class="top_TABLE"><tr><td class="top_td_l"></td><td id="AB_Title" class="top_td_ct">' + ABTitle + "</td><td class=\"top_td_cc\"><img src=\"img_alert/Close.gif\" class=\"td_cc_img\"  onClick=\"CANCEL_AB('" + ABFile + "');\"></td><td class=\"top_td_r\"></td></tr></table></td></tr><tr><td class=\"AB_TD_Ml\"></td><td class=\"AB_TD_Mc\"><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" class=\"Help_TABLE\"><tr><td></td></tr></table><table border=\"0\" cellspacing=\"0\" cellpadding=\"0\" class=\"MSG_TABLE\"><tr><td id=\"AB_Type\" class=\"MSG_type\"><img id=\"AB_Tye\" src=\"img_alert/ICON-" + ABType + '.gif"></td><td id="AB_Msg" class="MSG_cont">' + ABMsg.replace(/\n/g, "<br>") + '</td></tr></table><table border="0" cellspacing="0" cellpadding="0" class="MSG_TABLE"><tr><td class="MSG_lc"></td><td id="AB_NoMsg" class="MSG_no">' + ABNOM + '</td></tr></table><table border="0" cellspacing="0" cellpadding="0" class="MSG_TABLE"><tr><td class="MSG_BTN_t"></td></tr><tr><td><table border="0" cellspacing="0" cellpadding="0" align="center"><tr><td id="AB_Btn1">' + ABBT1 + '</td><td id="AB_Btn2">' + ABBT2 + '</td></tr></table></td></tr></table><input type="hidden" name="h_NoMsg" value="@h_NoMsg#"><table border="0" cellspacing="0" cellpadding="0" class="MSG_TABLE"></table></td><td class="AB_TD_Mr"></td></tr><tr><td colspan="3" class="AB_TD_btm"><table border="0" cellspacing="0" cellpadding="0" class="btm_TABLE"><tr><td class="btm_td_l"></td><td class="btm_td_cc"></td><td class="btm_td_r"></td></tr></table></td></tr></table>';

	if (document.all) {
		if (windowObj.document.all(el)) {
			windowObj.document.all(el).innerHTML = BOX;
		}
	}
	else if (document.getElementById) {
		if (windowObj.document.getElementById(el)) {
			windowObj.document.getElementById(el).innerHTML = BOX;
		}
	}
	wizard_disableAllFields();
	parent.document.getElementById('abox').style.display = 'block';
	parent.document.getElementById('CON').style.display = 'block';
	
	if (ABNOM != "") {
		var af = document.ABox.NoMsg;
		af.disabled = false;
	}
	else {
		ABNOM = "&nbsp;";
	}
}
function ALERT(windowObj, el, ABTitle, ABFile, ABType, ABMsg, ABNOM, ABBT1, ABBT2) {
	if (ABMsg != "") {
		setABox(windowObj, el, ABTitle, ABFile, ABType, ABMsg, ABNOM, ABBT1, ABBT2);
		return false;
	}
	else {
		document.forms[0].submit();
	}
}

function wizard_ALERT(windowObj, el, ABTitle, ABFile, ABType, ABMsg, ABNOM, ABBT1, ABBT2) {
	if (ABMsg != "") {
		wizard_setABox(windowObj, el, ABTitle, ABFile, ABType, ABMsg, ABNOM, ABBT1, ABBT2);
		return false;
	}
	else {
		document.forms[0].submit();
	}
}

function disableAllFields() {
	var e = parent.document.getElementsByTagName('a');
	for (var i = 0; i < e.length; i++) {
		disableAnchor(e[i], true);
	}
	var f = document.getElementsByTagName('input');
	for (var i = 0; i < f.length; i++) {
		f[i].setAttribute('disabled', true)
	}
	var g = document.getElementsByTagName('select');
	for (var i = 0; i < g.length; i++) {
		g[i].setAttribute('disabled', true)
	}
	var A = parent.document.getElementsByTagName('a');
	for (var i = 0; i < A.length; i++) {
		disableAnchor(A[i], true);
	}
	parent.document.getElementById('abox').style.display = 'block';
	parent.document.getElementById('CON').style.display = 'block';
}

function wizard_disableAllFields() {
	var f = parent.document.getElementsByTagName('input');
	for (var i = 0; i < f.length; i++) {
		f[i].setAttribute('disabled', true)
	}
	var g = parent.document.getElementsByTagName('select');
	for (var i = 0; i < g.length; i++) {
		g[i].setAttribute('disabled', true)
	}
	var A = parent.document.getElementsByTagName('a');
	for (var i = 0; i < A.length; i++) {
		disableAnchor(A[i], true);
	}
	parent.document.getElementById('abox').style.display = 'block';
	parent.document.getElementById('CON').style.display = 'block';

}

function move_up() {
	document.getElementById('RightContentArea').scrollTop = 0;
}

function disableAnchor(obj, disable) {
	if (disable) {
		var href = obj.getAttribute("href");
		if (href && href != "" && href != null) {
			obj.setAttribute('href_bak', href);
		}
		obj.removeAttribute('href');
	}
	else {
		obj.setAttribute('href', obj.attributes['href_bak'].nodeValue);
	}
}

function enableAllFields() {
	parent.document.getElementById('CON').style.display = 'none';
	parent.document.getElementById('abox').style.display = 'none';
	var e = parent.document.getElementsByTagName('a');
	for (var i = 0; i < e.length; i++) {
		disableAnchor(e[i], false);
	}
	var A = parent.document.getElementsByTagName('a');
	for (var i = 0; i < A.length; i++) {
		disableAnchor(A[i], false);
	}
	if (rightframe) {
		var f = rightframe.document.getElementsByTagName('input');
		for (var i = 0; i < f.length; i++) {
			f[i].disabled = false;
		}
		var g = rightframe.document.getElementsByTagName('select');
		for (var i = 0; i < g.length; i++) {
			g[i].disabled = false;
		}
	}
}

function wizard_enableAllFields() {
	parent.document.getElementById('CON').style.display = 'none';
	parent.document.getElementById('abox').style.display = 'none';

	var A = parent.document.getElementsByTagName('a');
	for (var i = 0; i < A.length; i++) {
		disableAnchor(A[i], false);
	}
	var f = parent.document.getElementsByTagName('input');
	for (var i = 0; i < f.length; i++) {
		f[i].disabled = false;
	}
	var g = parent.document.getElementsByTagName('select');
	for (var i = 0; i < g.length; i++) {
		g[i].disabled = false;
	}
}

function OK_Sub_AB() {
	parent.document.getElementById('CON').style.display = 'none';
	parent.document.getElementById('abox').style.display = 'none';
	var e = parent.document.getElementsByTagName('a');
	for (var i = 0; i < e.length; i++) {
		disableAnchor(e[i], false);
	}

	var f = parent.document.getElementsByTagName('input');
	for (var i = 0; i < f.length; i++) {
		f[i].disabled = false;
	}
	var g = parent.document.getElementsByTagName('select');
	for (var i = 0; i < g.length; i++) {
		g[i].disabled = false;
	}
	return;
}

function OK_AB() {

	var cf = rightframe.document.forms[0];
	enableAllFields();
	if (cf.wl_ssid0) {
		if (cf.wl_ssid0.value == "") {
			cf.wl_ssid0.focus();
		}
	}

	if (cf.wl_ssid1) {
		if (cf.wl_ssid1.value == "") {
			cf.wl_hide_ssid1.disabled = true;
		}
	}
	if (cf.wl_ssid2) {
		if (cf.wl_ssid2.value == "") {
			cf.wl_hide_ssid2.disabled = true;
		}
	}
	if (cf.wl_ssid3) {
		if (cf.wl_ssid3.value == "") {
			cf.wl_hide_ssid3.disabled = true;
		}
	}
	if (cf.Vlanenable) {
		var ssidArr = cf.r_sdn.value.split(" ");
		for (i = 0; i < 4; i++)
		setDisabled(true, eval("cf.wl_vid_ssid" + i), eval("cf.wl_vlan_priority" + i), eval("cf.wl_wmm_" + i));

		setDisabled(false, cf.wl_vlan_id, cf.wl_vlan_tag, cf.wl_vlan_birdge, cf.wl_wds_tag);
		for (i = 0; i < ssidArr.length; i++)
		setDisabled(false, eval("cf.wl_vid_ssid" + ssidArr[i]), eval("cf.wl_vlan_priority" + ssidArr[i]), eval("cf.wl_wmm_" + ssidArr[i]));
		/*
	if(cf.wds_vlan_list.options.length==4)
		cf.add_wds_vlan.disabled=true;
	else
		cf.add_wds_vlan.disabled=false;
	if(cf.wds_vlan_list.options.length==0)
		cf.del_wds_vlan.disabled=true;
	else
		cf.del_wds_vlan.disabled=false;	
*/

	}

}


function wizard_OK_AB() {

	var cf = rightframe.document.forms[0];
	
	wizard_enableAllFields();
	parent.document.getElementById('abox').style.display = 'none';
	parent.document.getElementById('CON').style.display = 'none';

}

function OK_SUBMIT(ABFile) {

	var cf = rightframe.document.forms[0];
	enableAllFields();
	if (rightframe.document.getElementById('warning')) {
		rightframe.document.getElementById('warning').style.display = "none";
	}
	if (rightframe.document.getElementById('uploading')) {
		rightframe.document.getElementById('uploading').style.display = "block";
	}

	if (cf.name == "upgrade") {

		ScreenConvert();
	}
	if (cf.name == "backupcfg") {
		cf = rightframe.document.forms[1];
		//alert(cf.name);
	}

	cf.submit();
}

function wizard_OK_SUBMIT(ABFile) {

	var cf = parent.document.forms[0];
	enableAllFields();
	if (parent.document.getElementById('warning')) {
		parent.document.getElementById('warning').style.display = "none";
	}
	if (parent.document.getElementById('uploading')) {
		parent.document.getElementById('uploading').style.display = "block";
	}

	if (cf.name == "upgrade") {

		ScreenConvert();
	}
	if (cf.name == "backupcfg") {
		cf = rightframe.document.forms[1];
	}

	cf.submit();
}
function CANCEL_AB() {
	if (!parent.window.frames['rightframe']) {
		parent.document.getElementById('CON').style.display = 'none';
		parent.document.getElementById('abox').style.display = 'none';
		var e = parent.document.getElementsByTagName('a');
		for (var i = 0; i < e.length; i++) {
			disableAnchor(e[i], false);
		}

		var f = parent.document.getElementsByTagName('input');
		for (var i = 0; i < f.length; i++) {
			f[i].disabled = false;
		}
		var g = parent.document.getElementsByTagName('select');
		for (var i = 0; i < g.length; i++) {
			g[i].disabled = false;
		}
		return;
	}
	var cf = rightframe.document.forms[0];
	enableAllFields();
	if (cf.Vlanenable) {
		var ssidArr = cf.r_sdn.value.split(" ");
		for (i = 0; i < 4; i++)
		setDisabled(true, eval("cf.wl_vid_ssid" + i), eval("cf.wl_vlan_priority" + i), eval("cf.wl_wmm_" + i));

		setDisabled(true, cf.wl_vlan_id, cf.wl_vlan_tag, cf.wl_vlan_birdge, cf.wl_wds_tag);
		for (i = 0; i < ssidArr.length; i++)
		setDisabled(true, eval("cf.wl_vid_ssid" + ssidArr[i]), eval("cf.wl_vlan_priority" + ssidArr[i]), eval("cf.wl_wmm_" + ssidArr[i]));
		/*	if(cf.wds_vlan_list.options.length==4)
			cf.add_wds_vlan.disabled=true;
		else
			cf.add_wds_vlan.disabled=false;
		if(cf.wds_vlan_list.options.length==0)
			cf.del_wds_vlan.disabled=true;
		else
			cf.del_wds_vlan.disabled=false;	
		cf.add_wds_vlan.disabled=true;
		cf.del_wds_vlan.disabled=true;
		cf.new_wds_vlan.disabled=true;
		*/
		cf.Vlanenable[1].checked = true;
	}

	if (cf.wl_ssid0) {
		if (cf.wl_ssid0.value == "") {
			cf.wl_ssid0.focus();
		}
	}
}

function wizard_CANCEL_AB() {
		parent.document.getElementById('CON').style.display = 'none';
		parent.document.getElementById('abox').style.display = 'none';
		var e = parent.document.getElementsByTagName('a');
		for (var i = 0; i < e.length; i++) {
			disableAnchor(e[i], false);
		}

		var f = parent.document.getElementsByTagName('input');
		for (var i = 0; i < f.length; i++) {
			f[i].disabled = false;
		}
		var g = parent.document.getElementsByTagName('select');
		for (var i = 0; i < g.length; i++) {
			g[i].disabled = false;
		}
		return;
	}

function YES_AB() {
	var Af = document.ABox;
	Af.submit();
	var cf = rightframe.document.forms[0];
	cf.todo.value = "proceed";
	cf.submit();
	enableAllFields()
}

