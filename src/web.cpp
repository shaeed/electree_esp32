#include "web.h"

WebServer httpServer(80);

void handleNotFound() {
  String message = "Invalid request.\n\n";
  httpServer.send(404, "text/plain", message);
}

void handleRoot() {
    String temp = PSTR("<html>\
<head>\
	<title>ELECTREE-Wifi Conf page</title>\
	<style>\
		body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
	</style>\
</head>\
<body><center>\
	<h1>Wifi and MQTT Configuration</h1>\
	<form action='/save' method='post'>\
	<table>\
		<tr><td>Wifi 1 SSID:</td><td><input type='text' name='ssid1' value='" + getSetting("ssid", 0, "") + "'></td></tr>\
		<tr><td>Wifi 1 Password:</td><td><input type='text' name='pass1' value='" + getSetting("pass", 0, "") + "'></td></tr>\
		<tr><td>Wifi 2 SSID:</td><td><input type='text' name='ssid2' value='" + getSetting("ssid", 1, "") + "'></td></tr>\
		<tr><td>Wifi 2 Password:</td><td><input type='text' name='pass2' value='" + getSetting("pass", 1, "") + "'></td></tr>\
		<tr><td>Wifi 3 SSID:</td><td><input type='text' name='ssid3' value='" + getSetting("ssid", 2, "") + "'></td></tr>\
		<tr><td>Wifi 3 Password:</td><td><input type='text' name='pass3' value='" + getSetting("pass", 2, "") + "'></td></tr>\
		<tr><td>Wifi 4 SSID:</td><td><input type='text' name='ssid4' value='" + getSetting("ssid", 3, "") + "'></td></tr>\
		<tr><td>Wifi 4 Password:</td><td><input type='text' name='pass4' value='" + getSetting("pass", 3, "") + "'></td></tr>\
		<tr><td>Wifi 5 SSID:</td><td><input type='text' name='ssid5' value='" + getSetting("ssid", 4, "") + "'></td></tr>\
		<tr><td>Wifi 5 Password:</td><td><input type='text' name='pass5' value='" + getSetting("pass", 4, "") + "'></td></tr>\
		<tr><td colspan='2'>-----</td></tr>\
		<tr><td>MQTT Server IP:</td><td><input type='text' name='mqttip' value='" + getSetting(K_MQTT_SERVER, MQTT_SERVER) + "'></td></tr>\
		<tr><td>MQTT User:</td><td><input type='text' name='mqttuser' value='" + getSetting(K_MQTT_USER, MQTT_USER) + "'></td></tr>\
		<tr><td>MQTT Password:</td><td><input type='text' name='mqttpass' value='" + getSetting(K_MQTT_PASS, MQTT_PASS) + "'></td></tr>\
		<tr><td>MQTT Port:</td><td><input type='text' name='mqttport' value='" + getSetting(K_MQTT_PORT, MQTT_PORT) + "'></td></tr>\
		<tr><td colspan='2'><input type='submit' value='Save'></td></tr>\
	</table>\
	</form>\
</center></body>\
</html>");
    httpServer.send(200, "text/html", temp);
}

void handleSave(){
    String ssid = httpServer.arg("ssid1");
    String pass = httpServer.arg("pass1");
    saveWifiCredential(ssid, pass, 0);
    ssid = httpServer.arg("ssid2");
    pass = httpServer.arg("pass2");
    saveWifiCredential(ssid, pass, 1);
    ssid = httpServer.arg("ssid3");
    pass = httpServer.arg("pass3");
    saveWifiCredential(ssid, pass, 2);
    ssid = httpServer.arg("ssid4");
    pass = httpServer.arg("pass4");
    saveWifiCredential(ssid, pass, 3);
    ssid = httpServer.arg("ssid5");
    pass = httpServer.arg("pass5");
    saveWifiCredential(ssid, pass, 4);

    String mqttip = httpServer.arg("mqttip");
    ssid = httpServer.arg("mqttuser");
    pass = httpServer.arg("mqttpass");
    String port = httpServer.arg("mqttport");
    saveMqttConfig(mqttip, port, ssid, pass);

    httpServer.send(200, "text/html", "Saved!!\nRebooting ...");
    reset();
}

void webServerSetup(){
    if(wifiConnected())
        startWebServer();

    // Register loop
    espurnaRegisterLoop(webServerLoop);
}

void startWebServer(){
    httpServer.on("/", handleRoot);
    httpServer.on("/save", handleSave);
    httpServer.onNotFound(handleNotFound);
    httpServer.begin();

    DEBUG_MSG_P(PSTR("[WEB] HTTP Web server started\n"));
}

void stopWebServer(){
    httpServer.stop();
}

void webServerLoop(void) {
    httpServer.handleClient();
}