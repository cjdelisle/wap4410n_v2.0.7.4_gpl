/** tbd: consider adding to prototype property of String class. */

/**
 * determine whether given text contains only valid characters determined by pattern.
 * empty string pass, nulls do not.
 */
function isPattern( text, pattern ) {

	if (text == null) return false;
	if (text == "") return true;

	var reg = new RegExp(pattern, "gi");
	var s = text.replace(reg, "");
	if( s.length == 0 ) {
		return true;
	}
	return false;
}

/**
 * determine whether given text contains only alpha-numberic characters.
 * empty string pass, nulls do not.
 */
function isAlphaNumeric( text ) {
	if (text == null) return false;
	if (text == "") return true;

	var reg = new RegExp("\\W+", "gi");
	var s = text.replace(reg, "");
	if( s.length != text.length ) {
		return false;
	}
	return true;
}

/**
 * determine whether given text contains only numeric characters.
 * empty string pass, nulls do not.
 */
function isNumeric( text ) {
	if (text == null) return false;
	if (text == "") return true;

	var reg = new RegExp("\\d+", "gi");
	var s = text.replace(reg, "");
	if( s.length > 0 ) {
		return false;
	}
	return true;
}

/**
 * determine whether given text contains only alphabetic characters.
 * empty string pass, nulls do not.
 */
function isAlphabetic( text ) {
	if (text == null) return false;
	if (text == "") return true;

	var reg = new RegExp("[a-z]+", "gi");
	var s = text.replace(reg, "");
	if( s.length == 0 ) {
		return true;
	}
	return false;
}

/**
 * determine whether given text contains only valid username characters.
 * empty string pass, nulls do not.
 */
function isUserName( text ) {

	if (text == null) return false;
	if (text == "") return true;

	var reg = new RegExp("[^ \t`~!#%^&*=?\"$[+<>();:|,']+", "gi");
	var s = text.replace(reg, "");
	if( s.length == 0 ) {
		return true;
	}
	return false;
}

/**
 * determine whether given text contains only valid username characters.
 * empty string pass, nulls do not.
 */
function isIP( text ) {

	if (text == null) return false;
	if (text == "") return true;

	var reg = new RegExp("[0-9.]+", "gi");
	var s = text.replace(reg, "");
	if( s.length == 0 ) {
		return true;
	}
	return false;
}

/**
 * consecutively replaces all instances of "%s" contained in @format string with
 * objects contained in @params as strings.
 * multiline strings are supported.
 */
function formatResourceString( format, params ) {
	var local = format;
	if( params.length && params.length > 0 ) {
		var reg = new RegExp( "%s", "m" );
		for( var i = 0; i < params.length; i++ ) {
			local = local.replace( reg, params[i] );
		}
	}
	return local;
}

/**
 * consecutively replaces all instances of "%N" (where N is greater
 * than 1) contained in @format string with
 * objects contained in the @params array as strings.
 * multiline strings are supported.
 */
function formatOrderedResourceString( format, params ) {
	var local = format;
	if( params.length && params.length > 0 ) {
		var reg = null;
		for( var i = params.length - 1; i >= 0; i-- ) {
			reg = new RegExp( "%" + (i+1), "m" );
			local = local.replace( reg, params[i] );
		}
	}
	return local;
}



