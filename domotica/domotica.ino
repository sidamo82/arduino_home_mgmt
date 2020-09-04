// Include librerie per comunicazione seriale
#include <SPI.h>
// Include librerie per controller ethernet
#include <Ethernet.h>
// Include librerie per sensore temperatura
#include <dht_nonblocking.h>

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
IPAddress gateway(192, 168, 0, 1);

// the subnet:
IPAddress subnet(255, 255, 255, 0);

//the IP address is dependent on your network
IPAddress ip(192, 168, 0, 4);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
EthernetServer server(TCP_PORT);

/* Variabile che contiene lo stato delle uscite del convertitore seriale->parallelo 74HC595 */
byte leds = 0;

/* Definizione dei sensori di temperatura */
/* Inizializzo la libreria per la gestione del sensore di temperatura */

// Definisco il tipo di sensore di temperatura
#define DHT_SENSOR_TYPE_01 DHT_TYPE_11

// Definisco il PIN dove è presente il sensore di temperatura
static const int DHT_SENSOR_PIN_01 = 6;

DHT_nonblocking dht_sensor_1( DHT_SENSOR_PIN_01, DHT_SENSOR_TYPE_01 );
  
/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment( float *temperature, float *humidity )
{
  static unsigned long measurement_timestamp = millis( );

  /* Measure once every four seconds. */
  if( millis( ) - measurement_timestamp > 3000ul )
  {
    if( dht_sensor_1.measure( temperature, humidity ) == true )
    {
      measurement_timestamp = millis( );
      return( true );
    }
  }

  return( false );
}

/* Restituisce la temperatura e umidità rilevate */
String getTemperature(int iID){

  float temperature;
  float humidity;
  String sOutput;
  
  /* Measure temperature and humidity.  If the functions returns
     true, then a measurement is available. */
  if( measure_environment( &temperature, &humidity ) == true )
  {
    Serial.print( "T = " );
    Serial.print( temperature, 1 );
    Serial.print( " deg. C, H = " );
    Serial.print( humidity, 1 );
    Serial.println( "%" );
    
    sOutput = String (temperature);
  } 

    return sOutput;
 
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
void sendLightONCommand(int iLightID){
  
  /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
  char buffer[50];

   
  bitSet(leds, iLightID-1);
            
  sprintf(buffer, "LIGHT ON: %d", iLightID);
  Serial.println(buffer);
  
  sprintf(buffer, "LEDS: %d", leds);
  Serial.println(buffer);
  
  aggiornaStatoUsciteConvertitoreSerialeParallelo(leds);

       
}

/* Funzione che spegne la luce */
void sendLightOFFCommand(int iLightID){

  /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
  char buffer[50];
    
  bitClear(leds, iLightID-1);
            
  sprintf(buffer, "LIGHT OFF: %d", iLightID);
  Serial.println(buffer);
  
  sprintf(buffer, "LEDS: %d", leds);
  Serial.println(buffer);
  
  aggiornaStatoUsciteConvertitoreSerialeParallelo(leds);
     
}

/* Funzione che apre in cancellino */
void sendOpenGateCommand(int iEndpointID){

  /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
  char buffer[50];

  /* Numero del PIN della scheda arduino che comanda il cancellino */
  int iArduinoOutputPin = 0;

  /* Identifico il PIN sulla scheda arduino dove è agganciato il relè che comanda il cancellino */
  switch(iEndpointID){

    case 1: 
      iArduinoOutputPin = GATE_OUTPUT_PIN;
      break;
  }
  

  digitalWrite(iArduinoOutputPin, HIGH);
  sprintf(buffer, "GATE OPEN");
  Serial.println(buffer);
  delay(500);
  digitalWrite(iArduinoOutputPin, LOW);
  delay(500);          

  digitalWrite(iArduinoOutputPin, HIGH);
  delay(300);
  digitalWrite(iArduinoOutputPin, LOW);
  delay(300);          

  digitalWrite(iArduinoOutputPin, HIGH);
  delay(300);
  digitalWrite(iArduinoOutputPin, LOW);
     
}

void sendLightCommand(int iLightID, String sAction){
  
  if(sAction.equals("0")==true){
    sendLightOFFCommand(iLightID);
    
  } else if(sAction.equals("1")==true){
    
    sendLightONCommand(iLightID);
    
  }
  
}

String parseCommand(String command, int iNumItem){
    
    int iIndexOfStart=0;
    int iIndexOfEnd=0;
    
    String sOutput;
    
    for(int iCont=0; iCont<iNumItem; iCont++){
           
      if(iCont==0){
        iIndexOfStart=iIndexOfEnd;      
      }
      else if(iIndexOfEnd != -1){
        iIndexOfStart = iIndexOfEnd + 1;
      }
      
      iIndexOfEnd = command.indexOf('-', iIndexOfStart);
   
    }
     
    if(iIndexOfEnd != -1){
      sOutput = command.substring(iIndexOfStart, iIndexOfEnd);
    } else{
      sOutput = command.substring(iIndexOfStart);
    }
   
    return sOutput;

}

String getDeviceType(String command){
  
  Serial.println("getDeviceType()");
  return parseCommand(command, 1);
}

String getEndpointID(String command){

  Serial.println("getEndpointID()");
  return parseCommand(command, 2);
}

String getCommandType(String command){

  Serial.println("getCommandType()");
  return parseCommand(command, 3);

}

/* Funzione che riceve in ingresso il comando ricevuto da Alexa e scatena l'evento sulla scheda Arduino */
String runHomeCommand(String command) {

     /* Buffer di 50 caratteri per stampare stringa di debug su monitor seriale */
     char buffer[50];
     
     String deviceType="";
     String endpointID="";
     String commandType="";

     /* Stringa per inviare l'output con l'esito del comando  */
     String sOutput="";
     
     deviceType = getDeviceType(command);
     endpointID = getEndpointID(command);
     commandType = getCommandType(command);
    
     if(DEBUG == true){
       
       Serial.println("BEGIN runHomeCommand()");
       Serial.println("COMMAND: " + command);
       Serial.println("DEVICE TYPE: " + deviceType);
       Serial.println("DEVICE ENDPOINTID: " + endpointID);
       Serial.println("COMMAND TYPE: " + commandType);
       Serial.println("DOCOMMAND");
       
     }
     
     /* Gestore dei sensori di temperatura */
     if(deviceType.equals("temperaturesensor")){
      
       if(DEBUG == true){
          Serial.println("SENSORE TEMPERATURE COMMAND TRIGGERED");
          
        }   

        int iTemperatureSensorID = 0;
        /* Converto l'ID endpoint in ingresso da formato stringa a numero intero */
        iTemperatureSensorID = endpointID.toInt();
        
        sOutput = getTemperature(iTemperatureSensorID);
        return sOutput;
     } 
     
     /* Gestore delle luci */
     else if(deviceType.equals("light")){
      
      if(DEBUG == true){
        Serial.println("LIGHT COMMAND TRIGGERED");
      }
      
      int iLightID = 0;

      /* Converto l'ID endpoint in ingresso da formato stringa a numero intero */
      iLightID = endpointID.toInt();
            
      if(DEBUG == true){
        sprintf(buffer, "ID Luce: %d", iLightID);
        Serial.println(buffer);
      }
      
      if(commandType.equals("1")){

        if(DEBUG == true){
          sprintf(buffer, "ACCENDI LUCE", iLightID);
            Serial.println(buffer);
        }
        
        sendLightONCommand(iLightID);
      }
      else if(commandType.equals("0")){

        if(DEBUG == true){
          sprintf(buffer, "SPEGNI LUCE", iLightID);
          Serial.println(buffer);
        }
        
        sendLightOFFCommand(iLightID);
      }

      return sOutput;
      
     }
     
     /* Gestore del cancellino */
     else if(deviceType.equals("gate")){

      if(DEBUG == true){
        Serial.println("GATE COMMAND TRIGGERED");
      }
      
      int iGateID = 0;

      /* Converto l'ID endpoint in ingresso da formato stringa a numero intero */
      iGateID = endpointID.toInt();

      if(DEBUG == true){
        sprintf(buffer, "ID Cancellino: %d", iGateID);
        Serial.println(buffer);
      }
      
      if(commandType.equals("1")){

        if(DEBUG == true){
          sprintf(buffer, "APRI CANCELLINO", iGateID);
          Serial.println(buffer);
        }

        /* Invio il comando di apertura del cancellino */
        sendOpenGateCommand(iGateID);
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

/* Inizio programma */
void loop() {

  //int signal = 0;
  int line = 1;


  /* Chiave di sicurezza impostata nel programma */
  String local_key = SECURE_KEY;
  /* Chiave di sicurezza inviata da Alexa ad ogni chiamata - deve coincidere con quella impostata nel programma */
  String remote_key = "";

  /* Stringa temporanea dove salvare i messaggi in arriva dalla scheda di rete */
  String message = "";
  String command = "";

  String sOutput = "";

  
  //bool doCommand = false;

  /* Stringa di 50 caratteri usata come memoria temporanea per le varie funzioni del programma */
  char buffer[50];

  /* Attivo la scheda di rete in ascolto per i dati in ingresso */
  EthernetClient client = server.available();
  
  /* Se la scheda di rete è pronta, procedo con il programma */
  if (client) {

    if(DEBUG == true){
      Serial.println("new client");
    }
  
    /* La richiesta in ingresso viene presa in carico dal programma quando termina con una riga di testo vuota */
    boolean currentLineIsBlank = true;

    /* Istruzioni eseguite quando Alexa invia la chiamata alla scheda ethernet di Arduino */
    while (client.connected()) {
  
      if (client.available()) {
      
        /* Leggo il carattere in ingresso sulla scheda di rete */
        char c = client.read();

        /* Se non è un carattere di "fine riga" concateno il carattere alla stringa "message" */
        if (c != '\n' && c != '\r') {   
          message = message + c;
        }
                
        /* Se ricevo il carattere di fine riga e currentLineIsBlank=true, significa che Alexa ha finito di inviarmi il comando, procedo dunque ad eseguirlo */
        if (c == '\n' && currentLineIsBlank) {

          if(DEBUG == true){
            Serial.println("REMOTE_KEY: " + remote_key);  
            Serial.println("LOCAL_KEY: " + local_key);
            Serial.println("COMMAND: " + command);
          }
          
          /* Controllo di sicurezza - controllo che la password inviata sia uguale a quella impostata nel programma */
          if(remote_key.equals(local_key)){

            if(DEBUG == true){ 
              Serial.println("KEY MATCH");
            }

            /* Invio il comando di "OK" ad Alexa - Significa che ho ricevuto correttamente il comando da eseguire */
            client.println("OK");
                  
            
            /* Invio il comando alla scheda Arduino per l'esecuzione */
            sOutput = runHomeCommand(command);

            if(DEBUG == true){ 
              Serial.println("CLIENT OUTPUT: " + sOutput);
            }
            
            client.println(sOutput);

            if(DEBUG == true){ 
              Serial.println("CLIENT DISCONNECTED");
            }
        
            /* Chiudo la connessione con Alexa */
            client.stop();

          } 
          /* Controllo di sicurezza fallito - non faccio nulla e chiudo la connessione con Alexa */
          else {

             /* Invio il comando di "KO" ad Alexa - Significa che c'è stato un problema */     
             client.println("KO");
             
             /* Chiudo la connessione con Alexa */
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

        /* Se da Alexa ricevo il carattere di fine riga, salvo la stringa inviata nelle variabili locali */
        if (c == '\n') {
          
          // you're starting a new line
          currentLineIsBlank = true;

          /* In base al numero di linea di comando ricevuta, salvo la stringa nella variabile corretta */
          switch(line){
          
           /* LINEA 1: Ricevo la password impostata su Alexa per la comunicazione con "Arduino" - la salvo nella variabile "remote_key" */
           case 1:
                    remote_key=message;
                    break;
                    
           /* LINEA 2: Ricevo il comando da eseguire - la password impostata su Alexa per la comunicazione con "Arduino" - lo salvo nella variabile "command" */
           case 2:
                    command=message;
                    break;
          }

          /* Svuoto la variabile "message" - in modo che sia pronta a ricevere la prossima riga da Alexa */
          message="";

          /* Incremento il numero di righe */
          line++;
        }
        /* Se il carattere non è un fine riga proseguo con la prossima */ 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
   
    }

   }
   
  //getTemperature(1);
    
}
