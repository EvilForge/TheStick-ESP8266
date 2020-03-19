const char PAGE_DefaultTop[] PROGMEM = R"=====(
 <!DOCTYPE html>
<html><head><title>The Stick</title>
<style>input{width:120px;}label{width:20%;}body {background-color: black;color: white;font-family: Arial, Helvetica, sans-serif; }</style>
<script type="text/javascript">
function copyLoc(e) { e.lat.value=document.getElementById("mylat").value;
e.lon.value=document.getElementById("mylon").value;
}</script></head>
<body><p>Target Coordinates:</p><form action='/to'><label>Lat: </label><input type='text' id='lat' name='lat' value='
)=====";

const char PAGE_DefaultBot[] PROGMEM = R"=====(
</body></html>
)=====";

const char PAGE_UpdateOK[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>The Stick</title><meta http-equiv="REFRESH" content="5;url=/"><style>body { background-color: black;color: white;font-family: Arial, Helvetica, sans-serif; }</style></head>
<body><p>Updates Accepted!</body></html>
)=====";