var WIFI_NAME = "Filibo";
var WIFI_OPTIONS = { password : ".C121989t." };
var MQTT_HOST = "m10.cloudmqtt.com";
var PATH = "espruino-1";
/*var MQTTOPTION = { // all optional - the defaults are below
  client_id : "random", // the client ID sent to MQTT - it's a good idea to define your own static one based on `getSerial()`
  keep_alive: 60, // keep alive time in seconds
  port: 1883, // port number
  clean_session: true,
  username: "username", // default is undefined
  password: "password",  // default is undefined
  protocol_name: "MQTT", // or MQIsdp, etc..
  protocol_level: 4, // protocol level
}*/
var mqtt;
var wifi;

function mqttMessage(pub) {
  console.log(
     "MQTT=> ",pub.topic,pub.message);
  if (pub.topic==PATH+"1/set") {
    var v = pub.message!=0;
    digitalWrite(B3, !v);
    mqtt.publish(PATH+"1/status", v?1:0);
  }
  if (pub.topic==PATH+"2/set") {
    var v = pub.message!=0;
    digitalWrite(B4, !v);
    mqtt.publish(PATH+"2/status", v?1:0);
  }
}

function mqttConnect() {
  console.log('mqttconnet');
  mqtt = require("MQTT").connect({
    host: MQTT_HOST,
    username: "	bzilbybp", // default is undefined
    password: "IA_nXwT1xiyV",  // default is undefined
    protocol_name: "MQTT",
    port: 	12039 // or MQIsdp, etc..

  });
  mqtt.on('connected', function() {
    console.log("MQTT connected");
    // subscribe to wildcard for our name
    //mqtt.subscribe(PATH+"#");
    // subscribe to wildcard for our name
    mqtt.subscribe(PATH);
  });
  mqtt.on('publish', mqttMessage);
  mqtt.on('disconnected', function() {
    console.log("MQTT disconnected... reconnecting.");
    setTimeout(function() {
      mqtt.connect();
    }, 1000);
  });
}

setInterval(function() {
  if (!mqtt) return;
  mqtt.publish(
    PATH+"cputemp",
    E.getTemperature());
}, 2*60*1000);

setWatch(function() {
  if (!mqtt) return;
  mqtt.publish(
    PATH+"buttonpress",
    1);
}, BTN, {edge:"rising",repeat:true,debounce:50});


function onInit() {
  console.log("Connecting to WiFi");
  wifi = require("Wifi");
  wifi.connect(WIFI_NAME, WIFI_OPTIONS,
               function(e) {
    if (e) {
      console.log("Connection Error: "+e);
      return;
    }
    console.log("WiFi Connected");
    wifi.getIP(function(f,ip) {
      console.log("IP: ",ip);
         console.log(wifi.getStatus());
       console.log(wifi.getIP());
      mqttConnect();
    });
  });
}