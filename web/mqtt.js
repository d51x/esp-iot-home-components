// https://javascript-minifier.com/  ???
function mqttsubs(id,act){
	console.log("base"+id)
	console.log("endpt"+id)
	var base_topic = document.getElementsByName("base"+id)[0].value;
	var endpoints = document.getElementsByName("endpt"+id)[0].value;
    var params = "id="+id+"&base="+base_topic+"&endpt="+endpoints+"&act="+act;
    ajax_post("/mqttsub", params, function(res){
        console.log(res.responseText);
        document.location.reload();
    })
}

function mqttsuba(id, base_cnt, endp_cnt){
    var base = document.getElementById(id);
    var div = document.createElement('div');
    var str = "<div id='basetop'>";
    str += "<hr>";
    str += "<label class='lf'>Base topic " + (base_cnt+1) + ": </label><input size='20' name='base" + base_cnt + "' class='edit rh' />" ;
    str += "<label class='lf'>endpoints: </label><input size='20' name='endpt" + base_cnt + "' class='edit rh' />" ;
    str += "<p class='lf2'>";
    str += "<button class='button norm rht' onclick='mqttsubs(" + base_cnt + ",0)'>Set</button>";
    str += "<button class='button norm rht' onclick='mqttsubs(" + base_cnt + ",1)'>Del</button>";
    str += "</p>";

    str += "</div>";
    base.innerHTML = base.innerHTML + str;
    //base.append(div);
}