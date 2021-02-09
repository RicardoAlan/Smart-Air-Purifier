#define BLYNK_PRINT Serial
#define USE_AVG

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "J7izQfzQO9cluTT6zNP0lD8DDinVjqC8";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Retana";
char pass[] = "violetaeshermosa";

int measurePin = A0;
int ledPower = D2;
// Set your LED and physical button pins here
const int puriPin = D1;
const int btnPin = D6;

// Tiempos de muestreo
unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;
unsigned long previousTime = 0;
long intervalOn = 300000; //5 minutos
int flag = 0;

//Cantidades medidas
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;



// Set the typical output voltage in Volts when there is zero dust. 
static float Voc = 0.6;

// Use the typical sensitivity in units of V per 100ug/m3.
const float K = 0.5;

int ReCnctFlag;  // Reconnection Flag
int ReCnctCount = 0;  // Reconnection counter

BlynkTimer timer;

//Llamo función para revisar estado del botón
void checkPhysicalButton();

//Defino los estados iniciales del botón y del purificador
int puriState = LOW;
int btnState = LOW;

// Every time we connect to the cloud...
BLYNK_CONNECTED() {
  // Request the latest state from the server
  Blynk.syncVirtual(V2);
    ReCnctCount = 0;

}


// When App button is pushed - switch the state
BLYNK_WRITE(V2) {
  puriState = param.asInt();
  digitalWrite(puriPin, puriState);
}

void checkPhysicalButton()
{
  if (digitalRead(btnPin) == HIGH) {
    // btnState is used to avoid sequential toggles
    if (btnState != HIGH) {
      // Toggle LED state
      puriState = !puriState;
      digitalWrite(puriPin, puriState);
      
        // Update Button Widget
      Blynk.virtualWrite(V2, puriState);
    }
    btnState = HIGH;
  } else {
    btnState = LOW;
  }
}

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendSensor()
{

unsigned long currentTime = millis();

/******************
* Dust
*****************/
//for (int i = 0; i<50; i++){
  
// Turn on the dust sensor LED by setting digital pin LOW.
digitalWrite(ledPower,LOW);

// Wait 0.28ms before taking a reading of the output voltage as per spec.
delayMicroseconds(samplingTime);

// Record the output voltage. 
voMeasured = analogRead(measurePin);

//This operation takes around 40 microseconds.
delayMicroseconds(deltaTime);

// Turn the dust sensor LED off by setting digital pin HIGH.
digitalWrite(ledPower,HIGH);

  // Wait for remainder of the 10ms cycle = 10000 - 280 - 40 microseconds.
delayMicroseconds(sleepTime);


calcVoltage = voMeasured / 1024 * 5.0 ;

float dV = calcVoltage - Voc;
if (dV < 0 ){
  dV = 0;
  Voc = calcVoltage;
  }
  
dustDensity = dV / K* 100.0;

if ( dustDensity < 0)
{
dustDensity = 0.00;
}

 // avg += dustDensity;
//}

//avg /= 50;


//Condicional de encendido
//if ( dustDensity > 80 || puriState == HIGH){

//  digitalWrite(puriPin, HIGH);
  
//  }
//  else {
//    digitalWrite(puriPin, puriState);
//    }

if (puriState == HIGH){
    digitalWrite(puriPin, HIGH); //Enciendo el purificador
    if (currentTime - previousTime > intervalOn && flag == 1){ //Si entro aquí por mucho polvo detectado entonces lo dejo encendido por 5 minutos el flag me confirma que llegué ahi por <80
      puriState = LOW; //Cumplidos los 5 minutos cambio el estado del purificador a LOW
      flag = 0; //Y cambio el flag a 0
      }  
  }
else { //Si el estado del purificador está en LOW
    if (dustDensity > 80){ //Me fijo si hay mucho polvo si es asi. RECOMIENDO COLOCAR UN LED ROJO PARA CUANDO SEA ACTIVACIÓN POR MUCHO POLVO Y UNO VERDE CUANDO ACTIVACIÓN MANUAL
      previousTime = currentTime; //Tomo el tiempo en que se detectó mucho polvo
      puriState = HIGH; //Cambio el estado del putificador a HIGH
      flag = 1; //Cambio el flag a 1
      }
    digitalWrite(puriPin, puriState);
  }


Serial.println("Raw Signal Value (0-1023):");
Serial.println(voMeasured);

Serial.println("Voltage:");
Serial.println(calcVoltage);

Serial.println("Dust Density micro gram/m^3:");
Serial.println(dustDensity);


Blynk.virtualWrite(V5,dustDensity);

///////////////////////////////
}


void setup()
{
// Debug console
Serial.begin(9600);

WiFi.begin(ssid, pass);
Blynk.config(auth);
Blynk.connect();
//Blynk.begin(auth, ssid, pass);
// You can also specify server:
//Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
//Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

pinMode(btnPin, INPUT);
pinMode(puriPin, OUTPUT);
digitalWrite(puriPin, puriState);
digitalWrite(btnPin, btnState);

// Setup a function to be called every second
timer.setInterval(1000L, sendSensor);
timer.setInterval(1000L, checkPhysicalButton);

pinMode(ledPower,OUTPUT);
///////////////
}

void loop()
{
  timer.run();
  
  if (Blynk.connected()) {  // If connected run as normal
    Blynk.run();
  }
    else if (ReCnctFlag == 0) {  
    unsigned long currentTime = millis();
    /******************
    * Dust
    *****************/
    // Turn on the dust sensor LED by setting digital pin LOW.
    digitalWrite(ledPower,LOW);

    // Wait 0.28ms before taking a reading of the output voltage as per spec.
    delayMicroseconds(samplingTime);

    // Record the output voltage. 
    voMeasured = analogRead(measurePin);

    //This operation takes around 40 microseconds.
    delayMicroseconds(deltaTime);

    // Turn the dust sensor LED off by setting digital pin HIGH.
    digitalWrite(ledPower,HIGH);

    // Wait for remainder of the 10ms cycle = 10000 - 280 - 40 microseconds.
    delayMicroseconds(sleepTime);


    calcVoltage = voMeasured / 1024 * 5.0 ;

    float dV = calcVoltage - Voc;
    if (dV < 0 ){
      dV = 0;
      Voc = calcVoltage;
    }
  
    dustDensity = dV / K* 100.0;

    if ( dustDensity < 0)
    {
      dustDensity = 0.00;
    }


    if (digitalRead(btnPin) == HIGH) {
    // btnState is used to avoid sequential toggles
    if (btnState != HIGH) {
      // Toggle LED state
      puriState = !puriState;
    }
    btnState = HIGH;
    } else {
      btnState = LOW;
    }


    //Condicional de encendido
    if (puriState == HIGH){
      digitalWrite(puriPin, HIGH); //Enciendo el purificador
      if (currentTime - previousTime > intervalOn && flag == 1){ //Si entro aquí por mucho polvo detectado entonces lo dejo encendido por 5 minutos el flag me confirma que llegué ahi por <80
        puriState = LOW; //Cumplidos los 5 minutos cambio el estado del purificador a LOW
        flag = 0; //Y cambio el flag a 0
        }  
    }
    else { //Si el estado del purificador está en LOW
      if (dustDensity > 80){ //Me fijo si hay mucho polvo si es asi. RECOMIENDO COLOCAR UN LED ROJO PARA CUANDO SEA ACTIVACIÓN POR MUCHO POLVO Y UNO VERDE CUANDO ACTIVACIÓN MANUAL
        previousTime = currentTime; //Tomo el tiempo en que se detectó mucho polvo
        puriState = HIGH; //Cambio el estado del putificador a HIGH
        flag = 1; //Cambio el flag a 1
        }
      digitalWrite(puriPin, puriState);
    }


    Serial.println("Raw Signal Value (0-1023):");
    Serial.println(voMeasured);

    Serial.println("Voltage:");
    Serial.println(calcVoltage);

    Serial.println("Dust Density micro gram/m^3:");
    Serial.println(dustDensity);
      
      
    // If NOT connected and not already trying to reconnect, set timer to try to reconnect in 30 seconds
    
    ReCnctFlag = 1;  // Set reconnection Flag
    Serial.println("Starting reconnection timer in 30 seconds...");
    timer.setTimeout(30000L, []() {  // Lambda Reconnection Timer Function
      ReCnctFlag = 0;  // Reset reconnection Flag
      ReCnctCount++;  // Increment reconnection Counter
      Serial.print("Attempting reconnection #");
      Serial.println(ReCnctCount);
      Blynk.connect();  // Try to reconnect to the server
      });  // END Timer Function{
      
      }


}
