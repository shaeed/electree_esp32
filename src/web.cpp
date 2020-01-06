#include "web.h"

#if defined(ARDUINO_ARCH_ESP32)
    //WebServer httpServer(80);
	WebServer *httpServer;
#elif defined(ARDUINO_ARCH_ESP8266)
    //ESP8266WebServer httpServer (80);
	ESP8266WebServer *httpServer;
#endif

void handleNotFound() {
  String message = "Invalid request.\n\n";
  httpServer->send(404, "text/plain", message);
}

void handleRoot() {
    #if defined(ARDUINO_ARCH_ESP8266)
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
		<tr><td>Wifi 1 SSID:</td><td><input type='text' name='ssid1'></td></tr>\
		<tr><td>Wifi 1 Password:</td><td><input type='text' name='pass1'></td></tr>\
		<tr><td>Wifi 2 SSID:</td><td><input type='text' name='ssid2'></td></tr>\
		<tr><td>Wifi 2 Password:</td><td><input type='text' name='pass2'></td></tr>\
		<tr><td>Wifi 3 SSID:</td><td><input type='text' name='ssid3'></td></tr>\
		<tr><td>Wifi 3 Password:</td><td><input type='text' name='pass3'></td></tr>\
		<tr><td>Wifi 4 SSID:</td><td><input type='text' name='ssid4'></td></tr>\
		<tr><td>Wifi 4 Password:</td><td><input type='text' name='pass4'></td></tr>\
		<tr><td>Wifi 5 SSID:</td><td><input type='text' name='ssid5'></td></tr>\
		<tr><td>Wifi 5 Password:</td><td><input type='text' name='pass5'></td></tr>\
		<tr><td colspan='2'>-----</td></tr>\
		<tr><td>MQTT Server IP:</td><td><input type='text' name='mqttip'></td></tr>\
		<tr><td>MQTT User:</td><td><input type='text' name='mqttuser'></td></tr>\
		<tr><td>MQTT Password:</td><td><input type='text' name='mqttpass'></td></tr>\
		<tr><td>MQTT Port:</td><td><input type='text' name='mqttport'></td></tr>\
		<tr><td colspan='2'><input type='submit' value='Save'></td></tr>\
	</table>\
	</form>\
</center></body>\
</html>");

    #elif defined(ARDUINO_ARCH_ESP32)
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
    #endif
    httpServer->send(200, "text/html", temp);
}

void handleSave(){
    String ssid;
    String pass;

	for(byte i = 1; i <= WIFI_MAX_NETWORKS; i++){
		if(httpServer->hasArg("ssid" + String(i))){
    		ssid = httpServer->arg("ssid" + String(i));
    		pass = httpServer->arg("pass" + String(i));
			if(!ssid.equals(""))
    			saveWifiCredential(ssid, pass, i-1);
		}
	}

	if(httpServer->hasArg("mqttip")){
    	ssid = httpServer->arg("mqttuser");
    	pass = httpServer->arg("mqttpass");
		if(!httpServer->arg("mqttip").equals(""))
    		saveMqttConfig(httpServer->arg("mqttip"), httpServer->arg("mqttport"), ssid, pass);
	}

    httpServer->send(200, PSTR("text/html"), PSTR("Saved!!<br>Rebooting ..."));
	stopWebServer();
	delay(10);
    reset();
}

void webServerSetup(){
    if(wifiConnected())
        startWebServer();

    // Register loop
    espurnaRegisterLoop(webServerLoop);
}

void startWebServer(){
	if(httpServer != NULL)
		delete httpServer;
	
	#if defined(ARDUINO_ARCH_ESP32)
	httpServer = new WebServer(80);
	#elif defined(ARDUINO_ARCH_ESP8266)
	httpServer = new ESP8266WebServer(80);
	#endif

    httpServer->on("/", handleRoot);
    httpServer->on("/save", handleSave);
    httpServer->onNotFound(handleNotFound);
    httpServer->begin();

    DEBUG_MSG_P(PSTR("[WEB] HTTP Web server started\n"));
}

void stopWebServer(){
    httpServer->stop();
	delay(1);
	delete httpServer;
}

void webServerLoop(void) {
	if(httpServer != NULL)
    	httpServer->handleClient();
}