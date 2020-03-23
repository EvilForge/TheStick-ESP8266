const char PAGE_DefaultTop[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>The Stick</title><style>body{background-color:black;color:beige;font-family:Arial,Helvetica,sans-serif;}label{display:inline-block;width:100px;padding:3px 3px;margin:4px 4px;}input{display:inline-block;width:80px;padding:3px 3px;margin:4px 4px;}.d{display:inline;}
div{padding:5px;}</style><script type="text/javascript"> function copyLoc(e) { e.lat.value=document.getElementById("mylat").value; e.lon.value=document.getElementById("mylon").value; }
</script></head><body><h1>The Stick</h1><h2>Target Coordinates</h2><div><form action='/to'><fieldset><label for='lat'>Latitude</label><input type='text' id='lat' name='lat' value=')=====";

const char PAGE_DefaultMid[] PROGMEM = R"=====(
'><br><input type='submit' value='Update'><input type='button' value='Copy' onclick='copyLoc(this.form)'></fieldset></form></div><h2>Status</h2><div><fieldset><label>Mode</label><label>)=====";

const char PAGE_UpdateOK[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>The Stick</title><meta http-equiv="REFRESH" content="5;url=/"><style>body { background-color: black;color: white;font-family: Arial, Helvetica, sans-serif; }</style></head>
<body><p>Updates Accepted!</body></html>
)=====";

const char PAGE_Update400[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>The Stick</title><meta http-equiv="REFRESH" content="5;url=/"><style>body { background-color: black;color: white;font-family: Arial, Helvetica, sans-serif; }</style></head>
<body><p>
)=====";