#include <SPI.h>
#include <Ethernet.h>

// network configuration. dns server, gateway and subnet are optional.

// the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  

// the dns server ip
IPAddress dnServer(192, 168, 0, 1);

// the router's gateway address:
IPAddress gateway(192, 168, 0, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

//the IP address is dependent on your network
IPAddress ip(192, 168, 0, 3);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// Key to secure communication between client and server
String securekey="ciao";

void configureEthernet(byte *mac, IPAddress ip, IPAddress dnServer, IPAddress gateway, IPAddress subnet){
  
   // initialize the ethernet device
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());

}

void setup() {

  Serial.begin(9600);
  configureEthernet(mac, ip, dnServer, gateway, subnet);

    // initialize digital pin LED_BUILTIN as an output.
  pinMode(8, OUTPUT);
 
}

void loop() {

  int signal = 0;
  int line = 1;
  
  String message = "";
  String secretkey = "";
  String command = "";

  
  
  // listen for incoming clients
  EthernetClient client = server.available();
  
  if (client) {
    
    Serial.println("new client");
  
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
    
      if (client.available()) {

      
        char c = client.read();

        if (c != '\n' && c != '\r') {     
          message = message + c;
        }
                
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        
        if (c == '\n' && currentLineIsBlank) {

          client.println("SECRETKEY: " + secretkey);  
          client.println("COMMAND: " + command);
          client.println("SECUREKEY: "+ securekey);

          // Check securekey
          if(secretkey.equals(securekey)){
             
            client.println("KEY MATCH");

            
            if(command=="1"){
          
              digitalWrite(8, HIGH);   // turn the LED on (HIGH is the voltage level)
              client.println("\nLED ACCESO");
              delay(2000);
            }
              
            else{
              // wait for a second
              digitalWrite(8, LOW);    // turn the LED off by making the voltage LOW
              client.println("\nLED SPENTO");
              delay(2000); 
            }
          } else {
             client.println("KEY NOT MATCH");
          }
          
          client.println("EXIT");
      
          break;
        }
        
        if (c == '\n') {
          
          // you're starting a new line
          currentLineIsBlank = true;

          switch(line){
            case 1:
                    secretkey=message;
                    break;
            case 2:
                    command=message;
                    break;
            
          }
          
          message="";
          
          line++;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }



}
