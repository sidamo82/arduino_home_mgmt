// Include librerie per comunicazione seriale
#include <SPI.h>
// Include librerie per controller ethernet
#include <Ethernet.h>
// Include librerie per sensore temperatura
#include <dht_nonblocking.h>

// Definisco il tipo di sensore di temperatura
#define DHT_SENSOR_TYPE DHT_TYPE_11

// Definisco il PIN dove è presente il sensore di temperatura
static const int DHT_SENSOR_PIN = 6;

// Definisco i 3 PIN utilizzati dal convertitore Seriale->Parallelo
static const int S74HC595_LATCH_PIN = 3;
static const int S74HC595_CLOCK_PIN = 2;
static const int S74HC595_DATA_PIN = 4; 

// Definisco il PIN dove collegare il Relè per l'apertura del cancellino elettrico
static const int GATE_OUTPUT_PIN = 5; 

// Se variabile è impostata a true stampo le stringe di DEBUG sul monitor seriale
static const bool DEBUG = true;

// Password per comunicare con Alexa
#define SECURE_KEY "secure_key" 

// Porta TCP sulla quale è in ascolto la scheda di rete
#define TCP_PORT 10000

// NETWORK CONFIGURATION

// network configuration. dns server, gateway and subnet are optional.
// the media access control (ethernet hardware) address for the shield:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  

// the dns server ip
IPAddress dnServer(172, 16, 0, 2);

// the router's gateway address:
IPAddress gateway(172, 16, 0, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

//the IP address is dependent on your network
IPAddress ip(172, 16, 0, 4);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
EthernetServer server(TCP_PORT);

/* Variabile che contiene lo stato delle uscite del convertitore seriale->parallelo 74HC595 */
byte leds = 0;

/* Inizializzo la libreria per la gestione del sensore di temperatura */
DHT_nonblocking dht_sensor( DHT_SENSOR_PIN, DHT_SENSOR_TYPE );

/* Funzione per la rilevazione della temperatura e dell'umidità */
static bool measure_environment( float *temperature, float *humidity )
{
  if( dht_sensor.measure( temperature, humidity ) == true )
  {
      return( true );
  }
  
  return( false );
}

/* Funzione che aggiorna i segnali di uscita dal convertitore seriale->parallelo */
void aggiornaStatoUsciteConvertitoreSerialeParallelo(byte leds_pattern)
{
   
   digitalWrite(S74HC595_LATCH_PIN, LOW);
   shiftOut(S74HC595_DATA_PIN, S74HC595_CLOCK_PIN, LSBFIRST, leds_pattern);
   digitalWrite(S74HC595_LATCH_PIN, HIGH);
}


/* Configurazione della scheda ethernet */
void configureEthernet(byte *mac, IPAddress ip, IPAddress dnServer, IPAddress gateway, IPAddress subnet){
  
   // initialize the ethernet device
  Ethernet.begin(mac, ip, dnServer, gateway, subnet);
  //print out the IP address
  Serial.print("IP = ");
  Serial.println(Ethernet.localIP());

}

/* Funzione che accende la luce */
void sendLightONCommand(int lightID){

  /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
  char buffer[50];
  
  bitSet(leds, lightID);
            
  sprintf(buffer, "LIGHT ON: %d", lightID);
  Serial.println(buffer);
  
  sprintf(buffer, "LEDS: %d", leds);
  Serial.println(buffer);
  
  aggiornaStatoUsciteConvertitoreSerialeParallelo(leds);

       
}

/* Funzione che spegne la luce */
void sendLightOFFCommand(int lightID){

  /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
  char buffer[50];
  
  bitClear(leds, lightID);
            
  sprintf(buffer, "LIGHT OFF: %d", lightID);
  Serial.println(buffer);
  
  sprintf(buffer, "LEDS: %d", leds);
  Serial.println(buffer);
  
  aggiornaStatoUsciteConvertitoreSerialeParallelo(leds);
     
}

/* Funzione che apre in cancellino */
void sendOpenGateCommand(int led){

  /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
  char buffer[50];
  
  digitalWrite(led, HIGH);
  sprintf(buffer, "GATE OPEN");
  Serial.println(buffer);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);          

  digitalWrite(led, HIGH);
  delay(300);
  digitalWrite(led, LOW);
  delay(300);          

  digitalWrite(led, HIGH);
  delay(300);
  digitalWrite(led, LOW);
     
}

void sendLightCommand(int iLightID, String sAction){

  if(sAction.equals("0")==true){
    sendLightOFFCommand(iLightID);
    
  } else if(sAction.equals("1")==true){
    
    sendLightONCommand(iLightID);
    
  }
  
}

/* Restituisce la temperatura e umidità rilevate */
void getTemperature(){

  float temperature;
  float humidity;
  
  /* Misuro la temperatura e l'umidità */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
  } 
}

/* Funzione che riceve in ingresso il comando ricevuto da Alexa e scatena l'evento sulla scheda arduino */
void runHomeCommand(String command) {

     /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
     char buffer[50];
     char eventID[4];
     char commandType[2];
     int charIndex=0;

     charIndex=command.indexOf('-');
    
     sprintf(eventID, "%c%c%c", command[0],command[1], command[2]);
     sprintf(commandType, "%c", command[4]);

     if(DEBUG == true){
       Serial.println("--------");
       sprintf(buffer, "Parse command: eventid: %s - commandtype: %s", eventID, commandType);
       Serial.println(buffer);
       Serial.println("DOCOMMAND");
     }
     

     // Conversione da array di char[] a oggetto String
     String eventIDStr(eventID);  

     if(eventIDStr.equals("001"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 01");
      }
      
      sendLightCommand(0, commandType);
      
     } 
     else if(eventIDStr.equals("002"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 02");
      }
      
      sendLightCommand(1, commandType);
      
     } 
     else if(eventIDStr.equals("003"))
     {
      
     if(DEBUG == true){
        Serial.println("LUCE 03");
      }
      
      sendLightCommand(2, commandType);     
     }
      else if(eventIDStr.equals("004"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 04");
      }
      
      sendLightCommand(3, commandType);    
     }
      else if(eventIDStr.equals("005"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 05");
      }
      
      sendLightCommand(4, commandType);    
     }
      else if(eventIDStr.equals("006"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 06");
      }
      
      sendLightCommand(5, commandType);    
     }
      else if(eventIDStr.equals("007"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 07");
      }
      
      sendLightCommand(6, commandType);    
     }
      else if(eventIDStr.equals("008"))
     {
      
      if(DEBUG == true){
        Serial.println("LUCE 08");
      }
      
      sendLightCommand(7, commandType);    
     }
     else if(eventIDStr.equals("009"))
     {
      
      if(DEBUG == true){
        Serial.println("Cancellino");
      }
      
      sendOpenGateCommand(GATE_OUTPUT_PIN);
      
     } 
     else{
      if(DEBUG == true){
        Serial.println("ERRORE COMANDO");
      }
     }
 }

/* Funzione che spegne tutte le luci contemporaneamente */
void spegniTutteLuci(){
  leds=0;
  aggiornaStatoUsciteConvertitoreSerialeParallelo(leds);
}

/* Funzione che accende tutte le luci contemporaneamente */
void accendiTutteLuci(){
  leds=255;
  aggiornaStatoUsciteConvertitoreSerialeParallelo(leds);
}

void setup() {

  Serial.begin(9600);

  configureEthernet(mac, ip, dnServer, gateway, subnet);
  
  /* Inizialize SerialToParallel Chip */
  pinMode(S74HC595_LATCH_PIN, OUTPUT);
  pinMode(S74HC595_DATA_PIN, OUTPUT);  
  pinMode(S74HC595_CLOCK_PIN, OUTPUT);
  pinMode(GATE_OUTPUT_PIN, OUTPUT);

  /* All'avvio della scheda spengo tutte le luci */
 spegniTutteLuci();
  
}

void loop() {

  int signal = 0;
  int line = 1;
  
  String local_key = SECURE_KEY;
  String remote_key = "";
  
  String message = "";
  String command = "";
  bool doCommand = false;
  
  char buffer[50];

  // listen for incoming clients
  EthernetClient client = server.available();
  
  /* New TCP connection triggered */
  if (client) {

    if(DEBUG == true){
      Serial.println("new client");
    }
  
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
  
      //Serial.println("CONNECTION ESTABLISHED");
      
      if (client.available()) {
      
        char c = client.read();
  
        if (c != '\n' && c != '\r') {   
          message = message + c;
        }
                
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {

          if(DEBUG == true){
            Serial.println("REMOTE_KEY: " + remote_key);  
            Serial.println("LOCAL_KEY: " + local_key);
            Serial.println("COMMAND: " + command);
          }
          // Check securekey
          if(remote_key.equals(local_key)){

            if(DEBUG == true){ 
              Serial.println("KEY MATCH");
            }
            
            client.println("OK");

            // close the connection:
            client.stop();
            
            if(DEBUG == true){ 
              Serial.println("client disconnected");
            }
            /* Send Command to Arduino Output */
            runHomeCommand(command);

          } else {
            
            
             client.println("KO");
             // close the connection:
             client.stop();

             if(DEBUG == true){ 
              Serial.println("KEY NOT MATCH");
              Serial.println("client disconnected");
             }
          }
          
          if(DEBUG == true){ 
            Serial.println("EXIT");
          }
          
          break;
        }
        
        if (c == '\n') {
          
          // you're starting a new line
          currentLineIsBlank = true;

          switch(line){
           case 1:
                    remote_key=message;
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
    
   }

   //getTemperature();
   
}
