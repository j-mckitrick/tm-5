// Simple validation for TM order table.

function getCB(f)
{
    var e = f.elements['orders[]'];

    if (e.length == null)
		e = [e];

    return e;
}

function doAll(which)
{
    var f = document.forms[0];
    var e = getCB(f);

    for (var i = 0; i < e.length; i++)
		e[i].checked = which;
}

function doSelectAll() { doAll(true); }
function doDeselectAll() { doAll(false); }

function doSubmit(mode)
{
    var perform = 'query';

    switch (mode)
    {
    case 0:
		perform = 'query';
		break;
    case 1:
		perform = 'update';
		break;
    case 2:
		perform = 'export';
		break;
    case 3:
		perform = 'clear';
		break;
    default:
		alert('oops');
		break;
    }

    var f = document.forms[0];
    f.perform.value = perform;
    f.submit();
}

function xbSetClass(el, className)
{
    el.setAttribute("class", className);
    el.setAttribute("className", className);
    return el;
}

function doHighlite(obj)
{
    if (obj.checked)
		xbSetClass(obj.parentNode.parentNode, 'hilite');
    else
		xbSetClass(obj.parentNode.parentNode, '');


}
