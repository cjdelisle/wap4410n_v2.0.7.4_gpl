/** tbd: consider adding to prototype property of String class. */

var StringUtils = new Object();

/**
 * Determine whether given text contains only valid characters determined by pattern.
 * empty string pass, nulls do not.
 */
StringUtils.isPattern = function( text, pattern ) {

	if( text == null ) return false;
	if( text == "" && pattern != "" ) return false;

	var reg = new RegExp( pattern  );
	var s = text.replace( reg, "" );
	if( s.length == 0 ) {
		return true;
	}
	return false;
}

/**
 * determine whether given text contains only alpha-numberic characters.
 * empty string pass, nulls do not.
 */
StringUtils.isAlphaNumeric = function( text ) {
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
 StringUtils.isNumeric = function( text ) {
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
StringUtils.isAlphabetic = function( text ) {
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
StringUtils.isUserName = function( text ) {

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
StringUtils.isIP = function( text ) {

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
 * Consecutively replaces all instances of "%N" (where N is greater
 * than 0) contained in @format string with
 * objects contained in the @params array as strings.
 * multiline strings are supported.
 */
StringUtils.formatOrderedResourceString = function( format, params ) {
	if( !DomUtils.isArray( params ) ) { Debug.logMessage( "StringUtils.formatOrderedResourceString: Second argument must be an array" ); return; }
	
	var local = format;
	if( params.length && params.length > 0 ) {
		var reg = null;
		for( var i = params.length - 1; i >= 0; i-- ) {
			reg = new RegExp( "%" + (i+1), "m" );
			//reg = new RegExp( "\{$" + (i+1) + "\}", "m" ); // TBD: use {$N}
			local = local.replace( reg, params[i] );
		}
	}
	return local;
}
