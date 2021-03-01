///////////////////////////////////////////////
///////////////////////////////////////////////

void handlePostForm(){
  //webPage = htmlPage;
  //Notice = "update at ";
  //Notice += timemeasure.c_str();
  //Serial.println("Text received. Contents: ");
  //Serial.println(notice);
}
///////////////////////////////////////////////
///////////////////////////////////////////////

/* Web Page
*/

void handleRootInicial() {
  // SW to create the html
  String html = "<html><head><title> Programa Geladeira Suzuki</title>";
  html += "<style> body ( background-color:#cccccc; ";
  html += "font-family: Arial, Helvetica, Sans-Serif; ";
  html += "Color: #000088; ) </style> ";
  html += "</head><body> ";
  html += "<h1>Controle Geladeira</h1> ";
  html += "Current Time:";
  html += timemeasure.c_str();
  html += "<br>";
  html += "Conectado no Wireless (0-desconectado, 1-conectado): ";
  html += (WiFi.status() == WL_CONNECTED);
  html += "<br>";
  html += "Conectado ao MQTT (0-desconectado, 1-conectado): ";
  html += (client.connected()and mqttButton);
  html += "<br>";
  html += "<p>----------------------------------------------------------</p>";
  html += " <a href=\"htmlControl\"><button>Control</button></a>";
  //html += "<p>----------------------------------------------------------</p>";
  html += " <a href=\"countTemp\"><button>Temperature Sensor Counter</button></a>";
  html += " Numero de sensores: ";
  html += String(tempDeviceCount).c_str();
  html += "<p>----------------------------------------------------------</p>";
  html += "<p>Temperature Table:</p>";
  html += "<table style=\"width:50%\">";
  html += "<form method=\"POST\" action=\"/tempPost\">";
  //first line
  html += "<tr>";
  html += "<th></th>";
  html += "<th>Select Temp "; //column
  html += "<input type=\"submit\" value=\"Submit Data\"></th>";
  html += "<th>Current Selection</th>"; //column
  html += "<th>Read temp</th>"; //column
  html += "<th>Command</th>"; //column
  html += "</tr>";
  //second line
  html += "<tr>";
  html += "<th>Select Sensor</th>";
  html += "<th><input type=\"number\" name=\"setSensor\" min=\"1\" max\"10\" value=\"\" step=\"1\"></th>";
  html += "<th>";
  html += String(selectSensor).c_str();
  html += "</th>";
  html += "<th></th>";
  html += "<th></th>";
  html += "</tr>";
  //third line
  html += "<tr>";
  html += "<th>Positiv limit</th>";
  html += "<th><input type=\"number\" name=\"setTempPos\" min=\"-5\" max\"5\" value=\"\" step=\"0.01\"></th>";
  html += "<th>";
  html += String(selectTempPos).c_str();
  html += "(";
  html += String(selectDeltaPos).c_str();
  html += ")";
  html += "</th>";
  html += "<th></th>";
  html += "<th></th>";
  html += "</tr>";
  //fourth line
  html += "<tr>";
  html += "<th>Temperature</th>";
  html += "<th><input type=\"number\" name=\"setTemp\" min=\"-30\" max\"30\" value=\"\" step=\"0.01\"></th>";
  html += "<th>";
  html += String(selectTemp).c_str();
  html += "</th>";
  html += "<th>";
  html += String(selSensorTemp).c_str();
  html += "</th>";
  html += "<th>";
  html += String(relaystatus).c_str();
  html += "</th>";
  html += "</tr>";
  //fifth line
  html += "<tr>";
  html += "<th>Negative Limit</th>";
  html += "<th><input type=\"number\" name=\"setTempNeg\" min=\"-5\" max\"5\" value=\"\" step=\"0.01\"></th>";
  html += "<th>";
  html += String(selectTempNeg).c_str();
  html += "(";
  html += String(selectDeltaNeg).c_str();
  html += ")";
  html += "</th>";
  html += "<th></th>";
  html += "<th></th>";
  html += "</tr>";
  html += "</table>";
  //table of sensors
  html += "<table>";
  //line
  html += "<tr>";
  //column
  html += "<th>#</th>";
  html += "<th> Temp </th>";
  html += "<th> Address </th>";
  html += "</tr>";
  //line
  html += "<tr>";
  //column
  html += "<th>1</th>";
  html += "<th>";
  html += String(temp1).c_str();
  html += "</th>";
  html += "<th>";
  html += temp1Add;
  html += "</th>";
  html += "</tr>";
  //line
  html += "<tr>";
  //column
  html += "<th>2</th>";
  html += "<th>";
  html += String(temp2).c_str();
  html += "</th>";
  html += "<th>";
  html += temp2Add;
  html += "</th>";
  html += "</tr>";
  //line
  html += "<tr>";
  //column
  html += "<th>3</th>";
  html += "<th>";
  html += String(temp3).c_str();
  html += "</th>";
  html += "<th>";
  html += temp3Add;
  html += "</th>";
  html += "</tr>";
  //line
  html += "<tr>";
  //column
  html += "<th>4</th>";
  html += "<th>";
  html += String(temp4).c_str();
  html += "</th>";
  html += "<th>";
  html += temp4Add;
  html += "</th>";
  html += "</tr>";
  //line
  html += "<tr>";
  //column
  html += "<th>5</th>";
  html += "<th>";
  html += String(temp5).c_str();
  html += "</th>";
  html += "<th>";
  html += temp5Add;
  html += "</th>";
  html += "</tr>";
  //line
  html += "<tr>";
  //column
  html += "<th>6</th>";
  html += "<th>";
  html += String(temp6).c_str();
  html += "</th>";
  html += "<th>";
  html += temp6Add;
  html += "</th>";
  html += "</tr>";
  html += "</table>";
  html += "</body></html>";

  servidorWeb.send(200, "text/html", html);

}
///////////////////////////////////////////////
void handleControl() {
  // SW to create the html
  String html = "<html><head><title> Programa Cerveja Suzuki</title>";
  html += "<style> body ( background-color:#cccccc; ";
  html += "font-family: Arial, Helvetica, Sans-Serif; ";
  html += "Color: #000088; ) </style> ";
  html += "</head><body> ";
  html += "<h1>Controle Geladeira ESP_B3A978</h1> ";

  html += "<p>----------------------------------------------------------</p>";
  html += " <a href=\"htmlRootInicial\"><button>Pagina Principal</button></a>";
  html += "<p>----------------------------------------------------------</p>";
  html += "<p>----------------------------------------------------------</p>";
  html += "<p><a href=\"/update\">Update by OTA</a></p>";
  html += "<p>----------------------------------------------------------</p>";
  
  html += "<table style=\"width:50%\">";
  html += "<tr>";
  html += "<th>Module:</th>";
  html += "<th>";
  html += Placa;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>Controller name:</th>";
  html += "<th>";
  html += ControlName;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>MAC:</th>";
  html += "<th>";
  html += MAC_Address;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>MQTT Server:</th>";
  html += "<th>";
  html += servidor_mqtt;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>MQTT Port:</th>";
  html += "<th>";
  html += servidor_mqtt_porta;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>MQTT User:</th>";
  html += "<th>";
  html += servidor_mqtt_usuario;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>MQTT Password:</th>";
  html += "<th>";
  html += servidor_mqtt_senha;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>Notice text:</th>";
  html += "<th>";
  html += Notice;
  html += "</th>";
  html += "</tr>";
  html += "</table>";

  html += "<br> Time pass:";  
  html += timemeasure.c_str();
  //html += "</p>";
  html += "<br> Conectado no Wireless (0-desconectado, 1-conectado): ";
  html += (WiFi.status() == WL_CONNECTED);
  //html += "</p>";
  html += "<br> WiFi.status: ";
  html += (WiFi.status());
  //html += "</p>";
  html += "<br>Possible WiFi Status: ";
  html += "<ul>";
  html += "<li>WL_NO_SHIELD=255</li>";
  html += "<li>WL_IDLE_STATUS=0</li>";
  html += "<li>WL_NO_SSID_AVAIL=1</li>";
  html += "<li>WL_SCAN_COMPLETED=2</li>";
  html += "<li>WL_CONNECTED=3</li>";
  html += "<li>WL_CONNECT_FAILED=4</li>";
  html += "<li>WL_CONNECTION_LOST=5</li>";
  html += "<li>WL_DISCONNECTED = 6</li>";
  html += "</ul>";
  /*
    html += "<p> WiFi Reconnect count : ";
    html += (countWire);
    html += "</p>";
    html += "<p>marca o tempo se for desconectado: ";
    html += tempoReconnect;
    html += "</p>";
  */
  /*
    html += "Conectado ao MQTT (0-desconectado, 1-conectado): ";
    html += client.connected();
    html += " <a href=\"MQTTButtonWeb\"><button>MQTTButton</button></a>";
    html += " Button status (1-on/0-off): ";
    html += mqttButton;
    html += "";
    html += " MQTT Reconnect count : ";
    //html += (countMQTT);
    html += "";
    html += "";
  */

  html += "<table style=\"width:50%\">";
  html += "<tr>";
  html += "<th>Conectado ao MQTT (0-desconectado, 1-conectado):</th>";
  html += "<th>";
  html += (client.connected() and mqttButton);
  html += "</th>";
  html += "<th>" "</th>";
  html += "<th>";
  html += "<a href=\"MQTTButtonWeb\"><button>MQTTButton</button></a>";
  html += "</th>";
  html += "<th>Button status (1-on/0-off): </th>";
  html += "<th>";
  html += mqttButton;
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th>Delay to start</th>";
  html += "<form method=\"POST\" action=\"/delayPwrUpForm\">";
  html += "<th><input type=\"number\" name=\"str_delayPwrUp\" min=\"0\" max\"100\" value=\"\" step=\"1\"></th>";
  html += "<th>";
  html += String(delayPowerUp).c_str();
  html += "seg.</th>";
  html += "<th>";
  html += "<input type=\"submit\" value=\"Submit Power Up Delay\">";
  html += "</form>";
  html += "</th>";
  html += "</tr>";
  html += "<tr>";
  html += "<th> print serial:" "</th><th>";
  html += serialPrint;
  html += "</th></tr>";
  html += "<tr><th> mqtt set parameter:" "</th><th>";
  html += mqttSetParameter;
  html += "</th></tr>";
  html += "<tr><th> print serial temp:" "</th><th>";
  html += serialPrintTemp;
  html += "</th></tr>";
  html += "<tr><th> print serial memory:" "</th><th>";
  html += serialPrintMem;
  html += "</th></tr>";
  html += "<tr><th> record memory:" "</th><th>";
  html += recordMemory;
  html += "<tr><th> print thermo address:" "</th><th>";
  html += serialPrintThermo;
  html += "</th></tr>";
  
  html += "</tr>";
  html += "</table>";

  
  html += "<p><a href=\"ESPReset\"><button>Reset</button></a></p>";
  html += "<p>----------------------------------------------------------</p>";
  html += "<br>";
  html += "mqtt server, mqtt port, mqtt user, mqtt password";
  html += "<form method=\"POST\" action=\"/postForm\">";
  html += "mqtt server: ";
  html += "<input type=\"text\" name=\"mqttServer\" value=\"\">";
  html += "<br> mqtt port: ";
  html += "<input type=\"text\" name=\"mqttPort\" value=\"\">";
  html += "<br> mqtt user: ";
  html += "<input type=\"text\" name=\"mqttUser\" value=\"\">";
  html += "<br> mqtt passwd :";
  html += "<input type=\"password\" name=\"mqttPassword\" value=\"\">";
  html += "<br> notice :";
  html += "<input type=\"text\" name=\"nOtice\" value=\"\"><br>";
  html += "<input type=\"submit\" value=\"Submit Data\">";
  html += "</form>";
  html += "<p>----------------------------------------------------------</p>";
  html += "<ul>";
  html += "<li>rev 0.1 - Geladeira program 1</li>";
  html += "<li>req-001: power on the refrigerator if temp read > temp set + deltapos;</li>";
  html += "<li>req-002: power off the refrigerator if temp read < temp set - deltaneg;</li>";
  html += "<li>req-003: the command to power on/off the refrigerator is at 1 second;</li>";
  html += "<li>req-004: read the DS18B20 every 1 second;</li>";
  html += "<li>req-005: read the temp and set the temperature through the html site</li>";
  html += "<li>req-007: update the software through the OTA;</li>";
  html += "<li>req-008.1: mqtt: send the temp read, set temperature and relay status</li>";
  html += "<li>req-008.2:  mqtt:  set the temperatures through the mqtt, fix the node-red to retain the publish</li>";
  html += "<li>req-009.0: EEPROM save the set temp in the memory in case of power cycle (https://randomnerdtutorials.com/esp32-flash-memory/ or https://www.filipeflop.com/blog/esp8266-gravando-dados-memoria-flash/);</li>";
  html += "<li>req-009.0: the delta negative and delta positive is always 1 (one) for this application, only the set temperature is save</li>";
  html += "<li>req-010: receive set temperature through mqtt and save in the memory - ready but mqtt has priority, once that it returns, it will use the mqtt stored, better to disable the mqtt only save in the html</li>";
  html += "<li>req-009.1: figure out how to record negative set condition, as -10, the set goes from -30 to 30</li>";
  html += "<li>req-012: add a delay power up of the refrigerator during power on, > 30 seconds;</li>";
  html += "<li>req-014.0: to monitor more than one temperature sensor and select where to use, it was used eppron, it was select according the Number of the sensor, it should be according the address</li>";
  html += "</ul>";
  html += "<br>";
  html += "To Do";
  html += "<ul>";
  html += "<li>req-009.1; FS Record the save thing of req-009.0 + temp sensor address, control parameters, mqtt server, etc</li>";
  html += "<li>req-014.1: select the temperature sensor and record and select by the address</li>";
  html += "<li>req-011: control the fan command according the refrigerator power on;</li>";
  html += "<li>req-013: it should be designe to work in the sonoff dual, sonoff normal and nodeMCU;</li>";
  html += "</ul>";
  /*
    html += "<input type=\"text\" class=\"form-control\" placeholder=\"Mqtt Server\" id=\"mqttServer\">";
    html += "<input type=\"text\" class=\"form-control\" placeholder=\"Mqtt user\" id=\"mqttUser\">";
    html += "<input type=\"password\" class=\"form-control\" placeholder=\"Mqtt password\" id=\"mqttPass\">";

    html += "<button type=\"button\" onclick=\"sendValues()\">Send values</button>";
    html += "<script>"
    html += "</script>"
  */
  html += "</body></html>";


  servidorWeb.send(200, "text/html", html);

}

///////////////////////////////////////////////
void handleControl2() {
  // SW to create the html
  String html = "<html><head><title> Programa Cerveja Suzuki</title>";
  html += "</body></html>";
  html += "<p>----------------------------------------------------------</p>";
  html += "<p><a href=\"/update\">Update by OTA</a></p>";
  html += "<p>----------------------------------------------------------</p>";
  html += "<p><a href=\"ESPReset\"><button>Reset</button></a></p>";
  html += "<p>----------------------------------------------------------</p>";
  servidorWeb.send(200, "text/html", html);

}
///////////////////////////////////////////////
/* Web Page RESET
*/
void WebReset() {
  // SW to create the html
  String html = "<html><head><title> Programa Cerveja Suzuki</title>";
  html += "<style> body ( background-color:#cccccc; ";
  html += "font-family: Arial, Helvetica, Sans-Serif; ";
  html += "Color: #000088; ) </style> ";
  html += "</head><body> ";
  html += "<h1>suzuki_teste cerveja basic ESP_D6C81D</h1> ";
  html += "<h1>RESET</h1> ";
  html += "</body></html>";

  servidorWeb.send(200, "text/html", html);

}
///////////////////////////////////////////////
