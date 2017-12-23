
var APIDOC = null;

function loadxml(url) {
	var xmlhttp = new XMLHttpRequest();
	xmlhttp.open("GET", url, false);
	xmlhttp.send('');
	
	var domDoc = (new DOMParser()).parseFromString(xmlhttp.responseText, "text/xml");
	return domDoc;
	//alert(xmlhttp.responseText + ", " + xmlhttp.responseXML);
	//return xmlhttp.responseXML;
}

function renderNamespace(sourcexml) {
	var processor = new XSLTProcessor();
	var namespacexsl = loadxml("namespace.xsl");
	processor.importStylesheet(namespacexsl);

	APIDOC = loadxml(sourcexml);
	//APIDOC = loadxml("armstrong.xml");

	var newFragment = processor.transformToFragment(APIDOC, document);
	$("#namespacecontainer").append(newFragment);
}

function renderClass(className) {
	var processor = new XSLTProcessor();

	var xsl = loadxml("class.xsl");
	processor.importStylesheet(xsl);
	processor.setParameter(null, "className", className);

	var newFragment = processor.transformToFragment(APIDOC, document);
	$("#classcontainer").html("");
	$("#classcontainer").append(newFragment);
}

function renderMethod(methodName) {
	var processor = new XSLTProcessor();

	var xsl = loadxml("method.xsl");
	processor.importStylesheet(xsl);
	processor.setParameter(null, "methodName", methodName);

	var newFragment = processor.transformToFragment(APIDOC, document);
	$("#classcontainer").html("");
	$("#classcontainer").append(newFragment);
}
