var DEBUG = false; // false for release

var separator = "\t"; // used for string=> multiple select list
// ====================================================== Submit Functions
function stdAction(fObj, act_str) {
	fObj.todo.value = act_str;
	dataToHidden(fObj);
	//submitDemo(fObj);
	fObj.submit();
}

//========================================================= Data Transfer Functions
function optionSelected(sel_obj) // return true or false
{
	return (sel_obj.selectedIndex > - 1 && sel_obj.selectedIndex < sel_obj.options.length) ? true: false;
}

function getSelected(sel_obj) // single select. Returns value. If value blank, return text
{
	var index = sel_obj.selectedIndex;
	if (index >= 0) return (sel_obj.options[index].value != "") ? sel_obj.options[index].value: sel_obj.options[index].text;
	else return "";
}

function getMultiSelected(sel_obj) // multi select. Always use text, not value
{
	var size = sel_obj.options.length;
	var i;
	var str = "";
	if (isNaN(size)) return str;
	if (size == 0) return str;
	str = separator;
	for (i = 0; i < size; i++)
	if (sel_obj.options[i].selected) str += sel_obj.options[i].text + separator;
	return str;
}

function setSelected(sel_obj, list) // list has multiple items from select obj
{
	var selSize = sel_obj.options.length;
	var startTextPos;
	var startValuePos;
	var textChar;
	var valueChar;
	for (var i = 0; i < selSize; i++) {
		startTextPos = - 1;
		startValuePos = - 1;
		sel_obj.options[i].selected = false;
		startTextPos = list.indexOf(separator + sel_obj.options[i].text + separator);
		if (sel_obj.options[i].value.length > 0) startValuePos = list.indexOf(separator + sel_obj.options[i].value + separator);
		if (startTextPos > - 1) sel_obj.options[i].selected = true;
		if (startValuePos > - 1) sel_obj.options[i].selected = true;
	}
}

function radioSelectedIndex(radio_object) // index of selected item, -1 if none
{
	if (!radio_object) return - 1;
	var size = radio_object.length;
	if (isNaN(size)) {
		if (radio_object.checked == true) return 0;
		else return - 1;
	}
	for (var i = 0; i < size; i++) {
		if (! (radio_object[i])) return (radio_object.checked) ? 0: - 1;
		if (radio_object[i].checked) return (i);
	}
	if (radio_object.checked == true) return 0;
	else return - 1;
}

function getRadioCheckedValue(radio_object) // value of selected item, "" if none
{
	var index = 0;
	if (!radio_object) return "";
	var size = radio_object.length;
	if (isNaN(size)) {
		if (radio_object.checked == true) return radio_object.value;
		else return "";
	}
	for (var i = 0; i < size; i++) {
		if (! (radio_object[i])) continue;
		if (radio_object[i].checked == true) return (radio_object[i].value);
	}
	if (radio_object.checked == true) return radio_object.value;
	else return "";
}

function getRadioIndex(radio_object, checked_value) // find index matching checkecd_value, 0 if no match
{
	if (!radio_object) return 0;
	if (radio_object.value == checked_value) return 0;
	var size = radio_object.length;
	if (isNaN(size)) return 0;
	for (var i = 0; i < size; i++) {
		if (! (radio_object[i])) continue;

		if (radio_object[i].value.toLowerCase() == checked_value.toLowerCase()) {
			return i;
		}

	}
	if (DEBUG) alert("DEBUG: " + radio_object.name + " (Radio button) has invalid value " + checked_value + "  Selecting 1st item instead");
	return 0; // if no match
}

function getvalue(field_obj) {
	var field_type = field_obj.type;
	if (field_type == "text" || field_type == "password" || field_type == "hidden" || field_type == "textarea") return field_obj.value;
	else if (field_type == "select-one") return getSelected(field_obj);
	else if (field_type == "select-multiple") return getMultiSelected(field_obj);
	else if (field_type == "checkbox") return (field_obj.checked) ? "enable": "disable";
	else if (field_type == "radio") return getRadioCheckedValue(field_obj);
	else if (field_obj.length > 0) // must be radio, but type shows as undefined
	return getRadioCheckedValue(field_obj);
	else return field_obj.value;
}

function net_mask_test(net_mask, lan_ipaddr, wan1, wan2, wan3, wan4) {
	var lan_all;
	var lan_len;
	var lan_tmp;
	var mask_all;
	var mask_len;
	var mask_tmp;

	lan_all = lan_ipaddr.value;
	mask_all = net_mask.value;

	//ip1
	mask_len = mask_all.length;
	mask_tmp = mask_all.indexOf(".");
	lan_len = lan_all.length;
	lan_tmp = lan_all.indexOf(".");
	if ((mask_all.substring(0, mask_tmp) & wan1.value) != (mask_all.substring(0, mask_tmp) & lan_all.substring(0, lan_tmp))) return 0;

	//ip2
	mask_all = mask_all.substring(mask_tmp + 1, mask_len);
	mask_len = mask_all.length;
	mask_tmp = mask_all.indexOf(".");
	lan_all = lan_all.substring(lan_tmp + 1, lan_len);
	lan_len = lan_all.length;
	lan_tmp = lan_all.indexOf(".");
	if ((mask_all.substring(0, mask_tmp) & wan2.value) != (mask_all.substring(0, mask_tmp) & lan_all.substring(0, lan_tmp))) return 0;

	//ip3
	mask_all = mask_all.substring(mask_tmp + 1, mask_len);
	mask_len = mask_all.length;
	mask_tmp = mask_all.indexOf(".");
	lan_all = lan_all.substring(lan_tmp + 1, lan_len);
	lan_len = lan_all.length;
	lan_tmp = lan_all.indexOf(".");
	if ((mask_all.substring(0, mask_tmp) & wan3.value) != (mask_all.substring(0, mask_tmp) & lan_all.substring(0, lan_tmp))) return 0;

	//ip4
	mask_all = mask_all.substring(mask_tmp + 1, mask_len);
	lan_all = lan_all.substring(lan_tmp + 1, lan_len);
	if ((mask_all & wan4.value) != (mask_all & lan_all)) return 0;
	else return 1;

}

function ip1to4(ipaddr, ip1, ip2, ip3, ip4) {
	//  alert("name: " + ipaddr.name);
	var len;
	var tmp;
	var all;
	all = ipaddr.value;

	//ip1
	len = all.length;
	tmp = all.indexOf(".");
	ip1.value = all.substring(0, tmp);

	//ip2
	all = all.substring(tmp + 1, len);
	len = all.length;
	tmp = all.indexOf(".");
	ip2.value = all.substring(0, tmp);

	//ip3
	all = all.substring(tmp + 1, len);
	len = all.length;
	tmp = all.indexOf(".");
	ip3.value = all.substring(0, tmp);

	//ip4
	all = all.substring(tmp + 1, len);
	ip4.value = all;
}

function ip4to1(ipaddr, ip1, ip2, ip3, ip4) {
	if (ip1.value.length > 0) ipaddr.value = ip1.value + "." + ip2.value + "." + ip3.value + "." + ip4.value;
	else ipaddr.value = "";
}

function mac1to6(macaddr, mac1, mac2, mac3, mac4, mac5, mac6) {

	var len;
	var tmp;
	var all;
	all = macaddr.value;

	//mac1
	len = all.length;
	tmp = all.indexOf(":");
	mac1.value = all.substring(0, tmp);

	//mac2
	all = all.substring(tmp + 1, len);
	len = all.length;
	tmp = all.indexOf(":");
	mac2.value = all.substring(0, tmp);

	//mac3
	all = all.substring(tmp + 1, len);
	len = all.length;
	tmp = all.indexOf(":");
	mac3.value = all.substring(0, tmp);

	//mac4
	all = all.substring(tmp + 1, len);
	len = all.length;
	tmp = all.indexOf(":");
	mac4.value = all.substring(0, tmp);

	//mac5
	all = all.substring(tmp + 1, len);
	len = all.length;
	tmp = all.indexOf(":");
	mac5.value = all.substring(0, tmp);

	//mac6
	all = all.substring(tmp + 1, len);
	mac6.value = all;
}

function mac6to1(macaddr, mac1, mac2, mac3, mac4, mac5, mac6) {
	if (mac1.value.length > 0) macaddr.value = mac1.value + ":" + mac2.value + ":" + mac3.value + ":" + mac4.value + ":" + mac5.value + ":" + mac6.value;
	else macaddr.value = "";
}

function dataToVisible(form_obj) // both hidden & visible fields in same form
{
	var form_size = form_obj.elements.length;
	var sourceField;
	var last_name;
	var radioIndex;
	var baseRef;

	for (var i = 0; i < form_size; i++) {
		if (form_obj.elements[i].name.substr(0, 3) == "c4_") {
			baseRef = "form_obj." + form_obj.elements[i].name.substr(3);
			ip1to4(form_obj.elements[i], eval(baseRef + "1"), eval(baseRef + "2"), eval(baseRef + "3"), eval(baseRef + "4"));
		}

		if (form_obj.elements[i].name.substr(0, 3) == "c6_") {
			baseRef = "form_obj." + form_obj.elements[i].name.substr(3);
			mac1to6(form_obj.elements[i], eval(baseRef + "1"), eval(baseRef + "2"), eval(baseRef + "3"), eval(baseRef + "4"), eval(baseRef + "5"), eval(baseRef + "6"));
		}
		sourceField = eval("form_obj.h_" + form_obj.elements[i].name);
		if (! (sourceField)) continue;
		if (sourceField.value == "") continue;
		if (form_obj.elements[i].type == "select-one") form_obj.elements[i].selectedIndex = getSelIndex(form_obj.elements[i], sourceField.value);
		if (form_obj.elements[i].type == "select-multiple") setSelected(form_obj.elements[i], sourceField.value);
		if (form_obj.elements[i].type == "checkbox") form_obj.elements[i].checked = (sourceField.value == "enable");
		if (form_obj.elements[i].type == "radio") {
			if (last_name == form_obj.elements[i].name) continue; // already done this one
			last_name = form_obj.elements[i].name;
			radioIndex = getRadioIndex(form_obj.elements[form_obj.elements[i].name], sourceField.value);
			if (form_obj.elements[form_obj.elements[i].name][radioIndex]) form_obj.elements[form_obj.elements[i].name][radioIndex].checked = true;
			else form_obj.elements[form_obj.elements[i].name].checked = true;
		}
	}
}

function dataToHidden(form_obj) // both hidden & visible fields in same form
{
	var form_size = form_obj.elements.length;
	var destField;
	var last_name;
	var radioIndex;
	var baseRef;

	for (var i = 0; i < form_size; i++) {
		if (form_obj.elements[i].name.substr(0, 3) == "c4_") {
			baseRef = "form_obj." + form_obj.elements[i].name.substr(3);
			ip4to1(form_obj.elements[i], eval(baseRef + "1"), eval(baseRef + "2"), eval(baseRef + "3"), eval(baseRef + "4"));
		}

		if (form_obj.elements[i].name.substr(0, 3) == "c6_") {
			baseRef = "form_obj." + form_obj.elements[i].name.substr(3);
			mac6to1(form_obj.elements[i], eval(baseRef + "1"), eval(baseRef + "2"), eval(baseRef + "3"), eval(baseRef + "4"), eval(baseRef + "5"), eval(baseRef + "6"));
		}
		destField = eval("form_obj.h_" + form_obj.elements[i].name);
		if (! (destField)) continue;
		if (form_obj.elements[i].type == "select-one") destField.value = getSelected(form_obj.elements[i]);
		if (form_obj.elements[i].type == "select-multiple") destField.value = getMultiSelected(form_obj.elements[i]);
		if (form_obj.elements[i].type == "checkbox") destField.value = (form_obj.elements[i].checked) ? "enable": "disable";
		if (form_obj.elements[i].type == "radio") {
			if (last_name == form_obj.elements[i].name) continue; // already done this one
			last_name = form_obj.elements[i].name;
			destField.value = getRadioCheckedValue(form_obj.elements[form_obj.elements[i].name]);

		}

	}
}

function ipsAppdataToHidden(form_obj) // both hidden & visible fields in same form
{
	var form_size = form_obj.elements.length;
	var index;

	for (var i = 0; i < form_size; i++) {
		if (form_obj.elements[i].name.substr(0, 7) == "ipsano_" && form_obj.elements[i].checked) {
			form_obj.ipsano.value += form_obj.elements[i].name.substr(7);
			form_obj.ipsano.value += ",";
			form_obj.ipsano.value += form_obj.elements[i].value == "enable" ? "1;": "0;";

		}

		if (form_obj.elements[i].name.substr(0, 6) == "ipsim_" && form_obj.elements[i].checked) {
			form_obj.ipsim.value += form_obj.elements[i].name.substr(6);
			form_obj.ipsim.value += ",";
			form_obj.ipsim.value += form_obj.elements[i].value == "enable" ? "1;": "0;";
		}

		if (form_obj.elements[i].name.substr(0, 7) == "ipsp2p_" && form_obj.elements[i].checked) {
			form_obj.ipsp2p.value += form_obj.elements[i].name.substr(7);
			form_obj.ipsp2p.value += ",";
			form_obj.ipsp2p.value += form_obj.elements[i].value == "enable" ? "1;": "0;";
		}
	}
}

// =================================== Development ========================
function show_data(form_obj)
// shows form information - used only for debugging
{
	var form_size = form_obj.elements.length;
	var debug_win = window.open("", "debug", "width=540,height=360,menubar=yes,scrollbars=yes,resizable=yes");
	with(debug_win.document) {
		open();
		writeln('<html><head><title>Debugging Window</title></head>');
		writeln('<body><h2>Form being submitted</h2>');
		writeln('<p>Form Name: ' + form_obj.name);
		writeln('<br>Form Action: ' + form_obj.action);
		writeln('<br>Form Target: ' + form_obj.target);
		writeln('</p><h3>Form Data</h3><p>Following table shows ALL fields, even if not submitted.</p>');
		writeln('<p><table border=1><tr bgcolor="#cccccc"><th nowrap>Field Name</th><th>Type</th><th>Value</th></tr>');
		for (var i = 0; i < form_size; i++) {
			writeln('<tr><td nowrap>' + form_obj.elements[i].name + '</td>');
			writeln('<td nowrap>' + form_obj.elements[i].type + '</td>');
			writeln('<td nowrap>');
			if ((form_obj.elements[i].type == "select-one") || (form_obj.elements[i].type == "select-multiple")) writeln('Selected item: ' + form_obj.elements[i].options[form_obj.elements[i].selectedIndex].text);
			else writeln(form_obj.elements[i].value);
			if ((form_obj.elements[i].type == "radio") && (form_obj.elements[i].checked)) write(' (Selected)');
			if ((form_obj.elements[i].type == "checkbox") && (form_obj.elements[i].checked)) writeln(' (Checked)');
			writeln('</td></tr>');
		}
		writeln('</table></body></html>');
		close();
	}
}

function submitDemo(form_obj) {
	show_data(form_obj);
}

function writeForm(form_obj)
// shows form information - used for developement
{
	var form_size = form_obj.elements.length;
	var hfieldStr = "";
	var hdemoStr = "";
	var last_name = "";
	var i;
	var j;

	var debug_win = window.open("", "form_def", "width=680,height=360,menubar=yes,scrollbars=yes,resizable=yes");
	with(debug_win.document) {
		open();
		writeln('<html><head><title>Form Definition Window</title></head>');
		writeln('<p>Form Name: ' + form_obj.name);
		writeln('<br>Form Action: ' + form_obj.action);
		writeln('<br>Form Target: ' + form_obj.target);
		writeln('<p>Following table shows checkboxes, radio buttons, selects fields.</p>\n<pre>');
		for (var i = 0; i < form_size; i++) {
			if (form_obj.elements[i].type == "select-one") {
				write(form_obj.elements[i].type + " : " + form_obj.elements[i].name + " &nbsp; ");
				for (j = 0; j < form_obj.elements[i].options.length; j++)
				write(" [" + j + "]" + form_obj.elements[i].options[j].text);
				writeln();
				hfieldStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="@' + form_obj.elements[i].name + '#">';
				hdemoStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="' + form_obj.elements[i].options[1].text + '">';
			}

			else if (form_obj.elements[i].type == "select-multiple") {
				write(form_obj.elements[i].type + " : " + form_obj.elements[i].name + " &nbsp; ");
				for (j = 0; j < form_obj.elements[i].options.length; j++)
				write(" [" + j + "]" + form_obj.elements[i].options[j].text);
				writeln();
				hfieldStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="@' + form_obj.elements[i].name + '#">';
				hdemoStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="' + form_obj.elements[i].options[1].value + '">';
			}

			else if (form_obj.elements[i].type == "radio") {
				if (last_name == form_obj.elements[i].name) continue; // already done this one
				else {
					last_name = form_obj.elements[i].name;
					write(form_obj.elements[i].type + " : " + form_obj.elements[i].name + " &nbsp; ");
					for (var j = 0; j < form_obj.elements[form_obj.elements[i].name].length; j++)
					write(" [" + j + "]" + form_obj.elements[form_obj.elements[i].name][j].value);
					writeln();
					hfieldStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="@' + form_obj.elements[i].name + '#">';
					hdemoStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="' + form_obj.elements[form_obj.elements[i].name][1].value + '">';
				}
			}

			else if (form_obj.elements[i].type == "checkbox") {
				writeln(form_obj.elements[i].type + " : " + form_obj.elements[i].name);
				hfieldStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="@' + form_obj.elements[i].name + '#">';
				hdemoStr += '\n<input type="hidden" name="h_' + form_obj.elements[i].name + '" value="enable">';
			}
			else; // no action
		}
		writeln('\n<!-- DemoStart -->' + hdemoStr + '\n<!-- DemoEnd -->');
		writeln('\n<!-- ServerStart\n' + hfieldStr + '\nServerEnd -->');
		writeln('</pre></body></html>');
		close();
	}
}

