// https://javascript-minifier.com/  ???
// https://jsminify.org/

function effects() {
	var eff = document.getElementById("effects");
	var effid = eff.options[eff.selectedIndex].value;
	var effname = eff.options[eff.selectedIndex].text;
	ajax_request("/colors?type=effect&id=" + effid, function() {
				eff.options[effid].selected="true";
				// var efft = document.getElementById("color");
                // efft.innerHTML = effname + "(" + effid + ")";
            });
}

function change_color(val, name)
{
	if ( name.indexOf('ledc10') >= 0) {
		var st = document.getElementById( 'colors' ).getAttribute('style' );
        st = st.substr(15, st.length);
        st = st.substr(0, st.length-1);
        var rgb = st.split(',');
        if ( name == "ledc100") rgb[0] = val;
        else if ( name == "ledc101") rgb[1] = val;
        else if ( name == "ledc102") rgb[2] = val;
        st = rgb.join();
        st = 'background:rgb(' + st + ')';
        document.getElementById( 'colors' ).setAttribute('style', st );
	}
}