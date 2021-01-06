// https://javascript-minifier.com/  ???
// https://jsminify.org/

function ajax_request_type(method, url, body, callback) {
    var xhr;
    if (typeof XMLHttpRequest !== "undefined") xhr = new XMLHttpRequest();
    else {
        var versions = ["MSXML2.XmlHttp.5.0", "MSXML2.XmlHttp.4.0", "MSXML2.XmlHttp.3.0", "MSXML2.XmlHttp.2.0", "Microsoft.XmlHttp"];
        for (var i = 0, len = versions.length; i < len; i++) {
            try {
                xhr = new ActiveXObject(versions[i]);
                break;
            } catch (e) {}
        }
    }
    xhr.onreadystatechange = ensureReadiness;

    function ensureReadiness() {
        if (xhr.readyState < 4) {
            return;
        }
        if (xhr.status !== 200) {
            return;
        }
        if (xhr.readyState === 4) {
            callback(xhr);
        }
    }
    xhr.open(method, url, true);
    if ( method == "POST" )
        xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xhr.send(body);
}

function ajax_request(url, callback) {
    ajax_request_type("GET", url, "", callback)
}

function ajax_post(url, body, callback) {
    ajax_request_type("POST", url, body, callback)
}

function on(element, event, callback) {
    if (element.addEventListener) {
        element.addEventListener(event, function() {
            callback(element, event);
        }, false);
    } else {
        if (element.attachEvent) {
            element.attachEvent(event, function() {
                callback(element, event);
            });
        }
    }
}

var inputs = document.getElementsByTagName("input");
for (var i = 0; i < inputs.length; i++) {
    if (inputs[i].type.toLowerCase() == "range") {
        ["change", "input"].forEach(function(ev) {
            on(inputs[i], ev, function(obj, ev) {
                if (ev == "change") {

                }
                if (ev == "input") {
                    document.getElementById(obj.name).innerHTML = obj.value;
                }
            });
        });
    } else if ( inputs[i].type.toLowerCase() == "checkbox" ) {
        /*
		if ( inputs[i].getAttribute("rel") == "relay" ) {
			["change", "input"].forEach(function(ev) {
				on(inputs[i], ev, function(obj, ev) {
					if (ev == "change") {
						var vnew = 1 - parseInt(obj.getAttribute("value"));
						ajax_request("/gpio?pin=" + obj.name.substr(5) + "&st=" + vnew, function() {
							obj.setAttribute("value", vnew);			
							obj.checked = vnew;			
							document.getElementById(obj.name).innerHTML = vnew ? "ON" : "OFF";
						});
					}
					if (ev == "input") {
						document.getElementById(obj.name).innerHTML = obj.checked ? "ON" : "OFF";
					}
				});
			});			
        }
        */
	}
}

/*
var a = document.getElementsByTagName("a");
for (var i = 0; i < a.length; i++) {
    if (a[i].getAttribute("rel") == "relay") {
        on(a[i], "click", function(obj, ev) {
            var vnew = 1 - parseInt(obj.getAttribute("data-val"));
            ajax_request("/gpio?st=" + vnew + "&pin=" + obj.getAttribute("data-id"), function() {
                obj.setAttribute("data-val", vnew);
                obj.innerHTML = "<button class='relay " + (vnew ? "on" : "off") + "' >" + obj.getAttribute("data-title") + "</button>";
            });
        });
    }
}
*/


function reboot() {
	ajax_request("/reboot?st=1", setTimeout( function() {window.location.replace("/");}, 3000));
	document.getElementById("rbt").style.display = "block";	
}

function i2cscan() {
	ajax_request("/i2cscan?st=1", function(res) {
		document.getElementById("i2cres").innerHTML = "<h4>Результаты сканирования:</h4><p>" + res.responseText + "</p>";	
	});
	
}

// id: id элемента, который нажали
// id2: id элемента, который надо изменять
// v:  подстановка результата запроса в innerHTML, 0 - не подставлять, 1 - добавлять к data-text, 2 - вставлять во внутрь data-text (перемемнная {0} )
// st: менять состояние кнопки, 1 - менять, 0 - не менять
function btnclick(id, id2, v, st) {
	var btn = document.getElementById( id );
	var uri = btn.getAttribute("data-uri");
	var value = btn.getAttribute("data-val");
	
    //console.log('params: id = %s, id2 = %s, v = %d, st = %d', id, id2, v, st);
    var new_uri = uri;
	if ( value > -1 ) new_uri = uri + value;

	ajax_request(new_uri, function(res) {
		
		if ( st == 1 ) {
			var vnew = 0;
			var resp = res.responseText
			if ( resp == "OFF" ) vnew = 0;
			else if ( resp == 0 ) vnew = 0;
			else if ( resp == "ON" ) vnew = 1;
			else if ( resp == 1 ) vnew = 1;
			else if ( resp == "ERROR" ) vnew = value;
			else if ( resp == "OK" ) vnew = !value;
			else vnew = 0;			
			var cls = btn.getAttribute("data-class");
			var snew = vnew > 0 ? " on" : " off";
			btn.setAttribute("class", cls + snew);
			btn.setAttribute("data-val", 1 - vnew); 
			
			//console.log('resp = %s, vnew = %d, !vnew = %d', resp, vnew, 1 - vnew);
		} 
		var elem = document.getElementById( id2 )
		if ( v == 1 ) {
			elem.innerHTML = btn.getAttribute("data-text") + res.responseText;	
		} else if ( v == 2 ) {
			var rs = btn.getAttribute("data-text");
			elem.innerHTML = rs.replace("{0}", res.responseText );	
		} else {
			elem.innerHTML = btn.getAttribute("data-text");	
		}
		
	});
}


function slider(val, name, uri) 
{
    change_color(val, name);
	ajax_request(uri + val, function(res) {
		var resp = res.responseText;	
		var duty = document.getElementById( name );
		duty.innerHTML = resp; 		
	});

}

function otaGet(fsize) {
    var x = document.getElementById("selectedFile");
    var file = x.files[0];

    var str = "<h4>File: " + file.name + "<br>" + "Size: " + file.size + " bytes</h4>";
    if ( file.size > fsize ) str = str + "<h4 style='color: red;'>Files size is bigger than partition size!!!</h4>";
    document.getElementById("file_info").innerHTML = str;
}

function updFw() {
    // Form Data
    //var formData = new FormData();

    var fileSelect = document.getElementById("selectedFile");
    
    if (fileSelect.files && fileSelect.files.length == 1) {
        var file = fileSelect.files[0];
        //formData.set("file", file, file.name);
        document.getElementById("ota_process").innerHTML = "Uploading " + file.name + " , Please Wait...";

        // Http Request
        //var request = new XMLHttpRequest();

        //request.upload.addEventListener("progress", updateProgress);

        //request.open('POST', "/update");
        //request.responseType = "blob
		
        //request.send(formData);


    } else {
        window.alert('Select A File First')
    }
}




