
#include "WiFiEsp.h"


#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif
#define relay 13
char ssid[] = "my ssid";
char pass[] = "my password";
int status = WL_IDLE_STATUS;



WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
  pinMode(relay, OUTPUT);
  Serial.begin(9600);
  Serial1.begin(9600);
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void loop()
{
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer
        if (buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
          break;
        }

        // Check to see if the client request was "GET /H"
        if (buf.endsWith("GET /H")) {
          Serial.println("Water Pump ON..");
          digitalWrite(relay, HIGH);   // turn the relay on for 20sec.
          delay(20000);
          Serial.println("Water Pump OFF..");
          digitalWrite(relay, LOW);
        }
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}


void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  client.print("The water level is ");
  client.print(analogRead(A0));
  client.println("<br>");
  client.println("<br>");

  // if the water level is below 63, user can turn on the relay which is connected to the water pump.
  if(analogRead(A0)<63){
  client.println("Click <a href=\"/H\">here</a> turn the pump on<br>");
  client.println("<a href=\"/\">Refresh</a><br>");
    }
  //if the water level is above 63, user cannot turn on the relay.
  else{
    client.println("Enough water..<br>");
    client.println("<a href=\"/\">Refresh</a><br>");
    }
  
  // The HTTP response ends with another blank line:
  client.println();
}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
