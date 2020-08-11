#include <ESP8266WiFi.h>
#include <DHT.h>
 
#define DHTPIN 1                               // Sensor pin
#define relPin = 0;                            // Relay pin (if using relay module for esp-01 it's always 0) 
#define DHTTYPE DHT22                          // Sensor used: DHT 22 (AM2302), AM2321
DHT dht(DHTPIN, DHTTYPE);                      // Initialazing
 
 
const char* ssid = "SSID";
const char* password = "Password";
 
int  state = LOW;

WiFiServer server(80);

String RedirectHtml()
{
  String html =
     String("HTTP/1.1 200 OK\r\n") +     // HTTP response header
            "\r\n" +
            "<!DOCTYPE HTML>" +
            "<script>window.location.href = \"/\";</script>" +            
            "\r\n";
  return html;
}

String HtmlPage(String string, float t, float h)
{
   char temp[5];
   char hum[5];
   dtostrf(t,2,1,temp);
   dtostrf(h,2,0,hum);
   
   String html =
     String("HTTP/1.1 200 OK\r\n") +
            "Content-Type: text/html\r\n" +
            "Connection: close\r\n" +             // The connection will be closed after completion of the response
            "Refresh: 10\r\n" +                   // Refresh the page automatically every 10 sec
            "\r\n" +
            "<!DOCTYPE HTML>" +
            "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css\">" +
            "<style>" +
            "{" +
              "box-sizing: border-box;" +
              "font-family: Arial, Helvetica, sans-serif;" +
            "}" +

            "body {" +
              "margin: 0;" +
              "font-family: Arial, Helvetica, sans-serif;" +
              "font-size: 22px;" +
              "align: center;" +
             "}" +

             ".button {" +
              "background-color: #4CAF50;" +
              "border: none;" +
              "color: white;" +
              "padding: 16px 32px;" +
              "text-align: center;" +
              "text-decoration: none;" +
              "display: inline-block;" +
              "font-size: 16px;" +
              "margin: 4px 20px;" +
              "-webkit-transition-duration: 0.4s;" +
              "transition-duration: 0.4s;" +
              "cursor: pointer;" +
             "}" +
             
             ".on {" +
              "background-color: #4CAF50;" +
              "color: white;" +              
              "border: 2px solid #4CAF50;" +
             "}" +
             ".on:hover {" +
              "background-color: white;" +
              "color: black; " +
             "}" +
            ".off {" +
              "background-color: #555555;" +
              "color: white;" +
              "border: 2px solid #555555;" +
            "}" +
            ".off:hover {" +
              "background-color: white;" +
              "color: black;" +
            "}" +
            "</style>" +
            "<html>" +
            "<HTML>" +
            "<TITLE>Light switch</TITLE>" +
            "<META NAME='viewport' CONTENT='width=device-width, initial-scale=1'>" +
            "</HEAD>" +
            "<BODY>" +
            "<H2 style=\"padding: 3px 25px\">Ceiling light</H2>" +
            string  +
            "<p style=\"padding: 0px 20px\"><i class=\"fa fa-thermometer-2\" aria-hidden=\"true\"></i> Temperature: " + temp + "C <a href=\"/light\"><i class=\"fa fa-refresh\" style=\"color:black\" aria-hidden=\"true\"></i></a></p>" +
            "<p style=\"padding: 0px 20px\"><i class=\"fa fa-tint\" aria-hidden=\"true\"></i> Humidity: " + hum + "%</p>"+
            "</br></br></br></br></br></br>" +
            "<p style=\"padding: 0px 20px\">Ver 2.1</p>" +
            "</HTML>"   
            "\r\n";

   return html;
}
 
void setup() {
  
  pinMode(relPin, OUTPUT);
  digitalWrite(relPin, LOW); 
 
  Serial.begin(115200);
  delay(3000);
 
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //Serial.println(WiFi.status());
  }
  
  Serial.println("100%");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/"); 

   dht.begin();
}

void loop()
{
  bool switching = false;
  
  WiFiClient client = server.available();
  // wait for a client (web browser) to connect
  if (client)
  {
    Serial.println("\n[Client connected]");
    float h = dht.readHumidity();                
    float t = dht.readTemperature();     
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String request = client.readStringUntil('\r');
        Serial.print(request);
        
        if (request.indexOf("GET /light/toggle") != -1)  
        {
          switching = true;
          digitalWrite(relPin, state);
          state = !state;
        }

        
        // wait for end of client's request, that is marked with an empty line
        if (request.length() == 1 && request[0] == '\n')
        {
          if(!switching)
          {
            if(state == LOW)
            {
              client.println(HtmlPage("<a href=\"/light/toggle\"><button class = \"button off\"><i class=\"fa fa-power-off\" aria-hidden=\"true\"></i> Off</button></a>&nbsp;</p>", t, h));
              break;
            }
            else
            {
              client.println(HtmlPage("<a href=\"/light/toggle\"><button class = \"button on\"><i class=\"fa fa-power-off\" aria-hidden=\"true\"></i> On</button></a>&nbsp;</p>", t, h));
              break;
            }
          }
          else 
          {
            client.println(RedirectHtml());       
            break;
          }
        }
      }
    }
    delay(5); // give the web browser time to receive the data

    // close the connection:
  }
  delay(1);
}
