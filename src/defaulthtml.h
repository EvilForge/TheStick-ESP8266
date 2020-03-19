const char PAGE_DefaultTop[] PROGMEM = R"=====(
 <!DOCTYPE html>
<html><head><title>The Stick</title><style>body { background-color: black;color: white;font-family: Arial, Helvetica, sans-serif; }</style></head>
<body><p>Target Coordinates:</p><form action='/to'>
)=====";

const char PAGE_DefaultBot[] PROGMEM = R"=====(
</body></html>
)=====";

const char PAGE_UpdateOK[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><title>The Stick</title><meta http-equiv="REFRESH" content="5;url=/"><style>body { background-color: black;color: white;font-family: Arial, Helvetica, sans-serif; }</style></head>
<body><p>Updates Accepted!</body></html>
)=====";