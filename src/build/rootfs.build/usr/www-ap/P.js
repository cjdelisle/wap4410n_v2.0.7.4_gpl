var _dragobj, _dragActive = true;

var _maxi = 0, _curri;

var ns = (navigator.product == ("Gecko"));

var ie = (document.all);

function mouseMove(e) {
	_dragActive = false;

	x = (ns) ? e.pageX: event.x;
	y = (ns) ? e.pageY: event.y;

	_dragobj.style.left = (_cx + x) + 'px';
	_dragobj.style.top = (_cy + y) + 'px';

}

function mouseUp(e) {
	_dragActive = true;
	_maxi++;

	document.onmousemove = noopMouse;
	document.onmousedown = noopMouse;
	document.onmouseup = noopMouse;

}

function mouseDown(e) {
	_dragobj.style.zIndex = _maxi;

	document.onmousemove = mouseMove;

	x = (ns) ? e.pageX: event.x;
	y = (ns) ? e.pageY: event.y;

	_cx = _dragobj.offsetLeft - x;
	_cy = _dragobj.offsetTop - y;

}

function noopMouse(e) {
	return false;
}

function dragInit() {
	var i, main, obj;

	obj = document.getElementById("CON");
	if (obj.style.zIndex == null || obj.style.zIndex == '') {
		obj.style.zIndex = i;
	}
	_maxi = Math.max(obj.style.zIndex, _maxi);

	_maxi++;
}

function startDrag(layerName) {

	if (_dragActive) {
		if (document.getElementById(layerName) != 'null') {
			_dragobj = document.getElementById(layerName);

			document.onmousedown = mouseDown;
			document.onmouseup = mouseUp;
		}
	} else {
		document.onmousemove = noopMouse;
		document.onmousedown = noopMouse;
		document.onmouseup = noopMouse;
	}
}

