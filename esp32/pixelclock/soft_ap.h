#ifndef _SOFT_AP_H_
#define _SOFT_AP_H_

/*
  copied from the example WiFiAccessPoint.ino;
  creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point SSID 
  2. Point your web browser to http://192.168.4.1

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

class SoftAP {
  private:
    // Set these to your desired credentials.
    const char *ssid = "fraggle clock";
    const char *password = "";

    WiFiServer *server;

    void handleFormData(String formData) {
      Serial.print("got form data:");
      Serial.println(formData);
    }

  public:

    void setup() {
      Serial.println();
      Serial.println("Configuring access point...");

      server = new WiFiServer(80);
      
      // You can remove the password parameter if you want the AP to be open.
      // a valid password must have more than 7 characters
      if (!WiFi.softAP(ssid, password)) {
        log_e("Soft AP creation failed.");
        while(1);
      }
      IPAddress myIP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(myIP);
      server->begin();

      Serial.println("Server started");
    }

    void loop_forever() {
      #ifdef RGB_BUILTIN
        neopixelWrite(RGB_BUILTIN,0,RGB_BRIGHTNESS,0); // Green
      #endif
      while(true) {
        loop();
      }
    }

    void loop() {
      WiFiClient client = server->available();   // listen for incoming clients

      if (client) {                             // if you get a client,
        Serial.println("New Client.");           // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
          if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            Serial.write(c);                    // print it out the serial monitor
            if (c == '\n') {                    // if the byte is a newline character

              // if the current line is blank, you got two newline characters in a row.
              // that's the end of the client HTTP request, so send a response:
              if (currentLine.length() == 0) {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println();

                const char form[]  = 
                  "<form action='/submit' method='get'>\n"
                  "SSID: <input type='text' id='ssid' name='ssid' required><br>\n"
                  "Passcode: <input type='text' id='passcode' name='passcode' required><br>\n"
                  "Zip Code: <input type='text' id='zip' name='zip' required><br>\n"
                  "<button type='submit'>Submit</button>\n"
                  "</form>";
                // the content of the HTTP response follows the header:
                // client.print("hello, world");
                client.print(form);

                // The HTTP response ends with another blank line:
                client.println();
                // break out of the while loop:
                break;
              } else {    // if you got a newline, then clear currentLine:
                currentLine = "";
              }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
              currentLine += c;      // add it to the end of the currentLine
            }

            /*
            if (currentLine.indexOf("&zip=") != -1) {
              Serial.print("got form data:");
              Serial.println(currentLine);
            }
            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.endsWith("GET /H")) {
              Serial.println("got H");
              digitalWrite(LED_BUILTIN, HIGH);               // GET /H turns the LED on
              neopixelWrite(RGB_BUILTIN,0,0,RGB_BRIGHTNESS); // Blue
            }
            if (currentLine.endsWith("GET /L")) {
              Serial.println("got L");
              digitalWrite(LED_BUILTIN, LOW);                // GET /L turns the LED off
              neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black
            }
            */
          }
          if (currentLine.indexOf("&zip=") != -1 &&
              currentLine.endsWith("HTTP/1.1")) {
              handleFormData(currentLine);
              break;
          }
        }
        // close the connection:
        client.stop();
        Serial.println("Client Disconnected.");
      }
    }
};

#endif