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
	String style = PSTR("<style>\
				body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
				</style>");

    String head = PSTR("<html><head><title>ELECTREE-Wifi Conf page</title>");
	String bodystart = PSTR("</head><body><center>\
	<h1>Wifi and MQTT Configuration</h1>\
	<form action='/save' method='post'>\
	<table>");

	String wifiTable = "";
	for(byte i = 0; i < WIFI_MAX_NETWORKS; i++){
		wifiTable += PSTR("<tr><td>Wifi 1 SSID:</td><td><input type='text' name='ssid")
					 + String(i) + "' value='" + getSetting("ssid", i, "") + "'></td></tr>";

		wifiTable += PSTR("<tr><td>Wifi 1 Password:</td><td><input type='text' name='pass")
					 + String(i) + "' value='" + getSetting("pass", i, "") + "'></td></tr>";
	}

	wifiTable += PSTR("<tr><td colspan='2'>-----</td></tr>");

	//Mqtt
	wifiTable += PSTR("<tr><td>MQTT Server IP:</td><td><input type='text' name='mqttip' value='") 
				 + getSetting(K_MQTT_SERVER, MQTT_SERVER) + "'></td></tr>";
	wifiTable += PSTR("<tr><td>MQTT User:</td><td><input type='text' name='mqttuser' value='")
				 + getSetting(K_MQTT_USER, MQTT_USER) + "'></td></tr>";
	wifiTable += PSTR("<tr><td>MQTT Password:</td><td><input type='text' name='mqttpass' value='")
				 + getSetting(K_MQTT_PASS, MQTT_PASS) + "'></td></tr>";
	wifiTable += PSTR("<tr><td>MQTT Port:</td><td><input type='text' name='mqttport' value='")
				 + getSetting(K_MQTT_PORT, MQTT_PORT) + "'></td></tr>";

	wifiTable += PSTR("<tr><td colspan='2'><input type='submit' value='Save'></td></tr>");
	wifiTable += PSTR("</table></form></center></body></html>");
	
	String page = head + style + bodystart + wifiTable;
    httpServer->send(200, "text/html", page);
}

void handleSave(){
    String ssid;
    String pass;

	for(byte i = 0; i < WIFI_MAX_NETWORKS; i++){
		if(httpServer->hasArg("ssid" + String(i))){
    		ssid = httpServer->arg("ssid" + String(i));
    		pass = httpServer->arg("pass" + String(i));
			if(!ssid.equals("") || i > 0)
    			saveWifiCredential(ssid, pass, i);
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