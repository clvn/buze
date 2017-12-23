function processAssetDir(path, targetPath) {
	processFilesInFolder(path, function(fi) {
		if (fi.name.indexOf(".") == 0) return ;
		createDirectory(targetPath);
		WFile.CopyFile(fi.path, targetPath + "/" + fi.name, true);
	});
	
	processSubfoldersInFolder(path, function(di) {
		if (di.name.indexOf(".") == 0) return ;
		processAssetDir(di.path, targetPath + "/" + di.name);
	});
}

function processTemplateDir(path, targetPath, depth) {
	processFilesInFolder(path, function(fi) {
		if (fi.name.indexOf(".") == 0) return ;
		processTemplateFile(fi.path, targetPath, depth);
	});
	
	processSubfoldersInFolder(path, function(di) {
		if (di.name.indexOf(".") == 0) return ;
		processTemplateDir(di.path, targetPath + "/" + di.name, depth + 1);
	});
}

function processTemplateFile(path, targetPath, depth) {
	var ld = path.lastIndexOf(".");
	var ext;
	if (ld != -1)
		ext = path.substr(ld + 1).toLowerCase(); 
	else
		ext = "";

	var fileName;
	var srcPath;
	var ls = Math.max(path.lastIndexOf("/"), path.lastIndexOf("\\"));
	if (ls != -1) {
		fileName = path.substr(ls + 1);
		srcPath = path.substr(0, ls + 1);
	} else {
		fileName = path;
		srcPath = "";
	}

	if (ext == "html") {
		transformFile(path, targetPath, fileName, depth, srcPath);
	} else {
		createDirectory(targetPath);
		WFile.CopyFile(path, targetPath + "\\" + fileName, true);
	}
}

function transformFile(path, targetPath, fileName, depth, srcPath) {


	createDirectory(targetPath);

	var rootPath = "";
	for (var i = 0; i < depth; i++) {
		rootPath += "../";
	}

	var xml = new ActiveXObject("MSXML2.DOMDocument");
	xml.async = false;
	xml.load(path);
	if (xml.parseError.errorCode != 0) {
		WScript.Echo(path + "\n" + xml.parseError.reason);
		WFile.CopyFile(path, targetPath + "\\" + fileName, true);
	} else {
		var xslproc = xsl.createProcessor();
		xslproc.addParameter("BaseName", fileName.substr(0, fileName.indexOf(".")));
		xslproc.addParameter("RootPath", rootPath);
		xslproc.addParameter("SourcePath", srcPath);
		xslproc.input = xml;
		xslproc.transform();

		writeFile(targetPath + "\\" + fileName, xslproc.output);

		//var result = new ActiveXObject("MSXML2.DOMDocument");
		//result.async=false;
		//result.loadXML(xslproc.output);
		//result.save(targetPath + "\\" + fileName);

		//var outf = WFile.CreateTextFile(targetPath + "\\" + fileName, true, true);
		//outf.write(xslproc.output);
		//outf.close();
	}
}

function writeFile(filename, data) {
	var stream = new ActiveXObject("ADODB.Stream");
	stream.Open();
	stream.CharSet = "UTF-8";
	stream.WriteText(data);
	stream.SaveToFile(filename, 2); // 2 == adSaveCreateOverWrite
	stream.Close();
}

function createTemplate(templateFile) {
	var template = new ActiveXObject("MSXML2.FreeThreadedDOMDocument");
	template.async = false;
	template.load(templateFile);
	if (template.parseError.errorCode != 0) {
		WScript.Echo("Cannot open template");
		WScript.Quit(0);
	}
	
	var xsl = new ActiveXObject("MSXML2.XSLTemplate");
	xsl.stylesheet = template;
	return xsl;
}

// enumerates files in a folder and calls a callback for each file
function processFilesInFolder(path, callback) {
    if (!WFile.FolderExists(path)) return ;
    var folder = WFile.GetFolder(path);
    var file = new Enumerator(folder.Files);
    var pluginsCopied = 0;
    var pluginsSkipped = 0;
    for (; !file.atEnd(); file.moveNext()) {
        var fileInfo = file.item();
        callback(fileInfo);
    }
}

function processSubfoldersInFolder(path, callback) {
    if (!WFile.FolderExists(path)) return ;
    var folder = WFile.GetFolder(path);
    var file = new Enumerator(folder.Subfolders);
    var pluginsCopied = 0;
    var pluginsSkipped = 0;
    for (; !file.atEnd(); file.moveNext()) {
        var fileInfo = file.item();
        callback(fileInfo);
    }
}

// creates a directory recursively
function createDirectory(path) {
    if (path.indexOf(":") != -1 && !WFile.FolderExists(WFile.GetDriveName(path)))
        return false;
    
    if (WFile.FolderExists(path))
        return true;

    if (!createDirectory(WFile.GetParentFolderName(path)))
        return false;
    
    WFile.CreateFolder(path);
    return true;
} 

