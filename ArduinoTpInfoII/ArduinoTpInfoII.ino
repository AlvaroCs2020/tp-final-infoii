#include <SPI.h>		// incluye libreria interfaz SPI
#include <SD.h>			// incluye libreria para tarjetas SD
#include <EEPROM.h>
#include <RTClib.h>
#include <Wire.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#define LOWV_ADDRESS 0
#define HIGHV_ADDRESS 10
#define SENS_TEMP 2 //pin de entrada de sensor de temperatura 
#define SENS_CURR A3 //pin de entrada analogica de sensor de corriente 20amp
#define SENS_V1 A2 //pin de entrada analogica de sensor de voltaje de la carga que esta conectada, cuando esta conectada 
#define SENS_V2 A1 //pin de entrada analogica de sensor de voltaje de bateria 
#define SENS_V3 A0 //pin de entrada analogica de sensor de voltaje que esta entregando el cargador
#define SENS_V1 A2 //pin de entrada analogica de sensor de voltaje de la carga que esta conectada, cuando esta conectada 
#define SENS_V2 A1 //pin de entrada analogica de sensor de voltaje de bateria 
#define SENS_V3 A0 //pin de entrada analogica de sensor de voltaje que esta entregando el cargador
#define RELE_1 6 //rele para separación de bateria se circuito 
#define RELE_2 7 //rele para paso de cargador hacia la carga  
#define RELE_3 8//rele para separación de bateria se circuito 
#define RELE_4 9//rele para separación de bateria se circuito
#define PULSE_UP 2 //boton de pulsador para subir valor 
#define PULSE_DW 3 //boton de pulsador para bajar valor 
#define PULSE_ENTER 4 //boton de pulsador para selecionar valor 
#define PULSE_SET 5 //boton de pulsador para salbar o guardar valor  
#define DHTTYPE DHT11
using namespace std;

class DevUtils
{
  public:
    static float truncateFloat(float value, int decimalPlaces);
    static String completeZerosItoString(int n);
};

static float DevUtils::truncateFloat(float value, int decimalPlaces)
{
  float multiplier = pow(10, decimalPlaces);
  return round(value * multiplier) / multiplier;
}
static String DevUtils :: completeZerosItoString(int n)
{
  return n < 10 ? String("0") + String(n) : String(n); 
}
class FileManagement
{
  public:
    FileManagement();
    void UpdateTxt(float currentV, String date, String time);
  private:
    int const SSpin = 10;
    File archivo;			// objeto archivo del tipo File
};

FileManagement ::FileManagement ()
{
  if (!SD.begin(SSpin)) // inicializacion de tarjeta SD
  {			
  //ACA VA LLAMADA AL LCD
    return;				
  }
}

void FileManagement :: UpdateTxt(float currentV, String date, String time)
{
  archivo = SD.open("DATA.txt", FILE_WRITE);
  if(archivo)
  {
    archivo.print(currentV);
    archivo.print(";");
    archivo.print(date);
    archivo.print(";");
    archivo.println(time);
  }
  archivo.close();
}


class EEpromManagement
{
  public:
    EEpromManagement();
    void setLowV(float);
    void setHighV(float);
    float getHighV();
    float getLowV();

  private:
    int lowAddress;
    int highAddress;
};

EEpromManagement :: EEpromManagement()
{
  this->lowAddress = LOWV_ADDRESS;
  this->highAddress = HIGHV_ADDRESS;
}

void EEpromManagement :: setHighV(float voltage)
{
  EEPROM.put(this->highAddress, DevUtils::truncateFloat(voltage, 2));
}
void EEpromManagement :: setLowV(float voltage)
{
  EEPROM.put(this->lowAddress, DevUtils::truncateFloat(voltage, 2));
}
float EEpromManagement :: getHighV()
{
  float value;
  EEPROM.get(this->highAddress, value);
  return value;
}
float EEpromManagement :: getLowV()
{
  float value;
  EEPROM.get(this->lowAddress, value);
  return value;
}

class RtcManagement
{
  private:
      RTC_DS3231 rtc;
      DateTime date;
  public:
      RtcManagement(); 
      String getDate();
      String getTime();
      DateTime now();
};

RtcManagement :: RtcManagement(){

  if (! rtc.begin()) {				// si falla la inicializacion del modulo
  //Serial.println("Modulo RTC no encontrado !");	// muestra mensaje de error
  }
  this->date = rtc.now();
  //rtc.adjust(DateTime(__DATE__, __TIME__));	
  	
}
String RtcManagement :: getDate (){
  
  String y =  DevUtils::completeZerosItoString(date.year());
  String m =  DevUtils::completeZerosItoString(date.month());
  String d =  DevUtils::completeZerosItoString(date.day());
   
  String strdate = d +"-"+ m +"-"+ y ;
  return strdate;
}
String RtcManagement :: getTime(void){
  String h =  DevUtils::completeZerosItoString(date.hour());
  String m =  DevUtils::completeZerosItoString(date.minute());
  String s =  DevUtils::completeZerosItoString(date.second());
  String time = h + m + s;
  return time;
}
DateTime RtcManagement :: now()
{
  return rtc.now();
}

class SensorsManagement
{
  public:
    SensorsManagement();
    float getVoltageMeassure();
    float getCurrentMeassure();
    int controlButtons();
    float getTemperature();
  private:
    DHT * tempSensor;
};
SensorsManagement :: SensorsManagement()
{
  pinMode(SENS_CURR, INPUT);
  pinMode(SENS_V2, INPUT);
  pinMode(SENS_V3, INPUT);
  pinMode(SENS_TEMP, INPUT);
  pinMode(PULSE_DW, INPUT_PULLUP);
  pinMode(PULSE_ENTER, INPUT_PULLUP);
  pinMode(PULSE_SET, INPUT_PULLUP);
  tempSensor = new DHT(SENS_TEMP, DHTTYPE);
  tempSensor->begin();
}
float SensorsManagement :: getVoltageMeassure()
{
  float meassure = 0;
  for(int i = 0; i < 50; i++)
  {
    meassure += (25.0 *(float) analogRead(SENS_V2) / 1023.0);
    delay(10);
  }
  return abs(meassure/50.0);
}
float SensorsManagement :: getCurrentMeassure()
{
  float meassure = 0;
  float v;
  for(int i = 0; i < 50; i++)
  {
    v = (float) analogRead(SENS_CURR) * 5.0 / 1023.0;    
    meassure += (v - 2.5) / 0.185;
    delay(10);
  }
  return abs(meassure/50.0);
}
int SensorsManagement :: controlButtons()
{
  if( 
      //!digitalRead(PULSE_UP)     ||
      !digitalRead(PULSE_DW)     ||
      !digitalRead(PULSE_ENTER)  ||
      !digitalRead(PULSE_SET)    
    )
  {
    delay(500);
    return 1;
  }
  return 0;
}
float SensorsManagement ::getTemperature()
{
  return this->tempSensor->readTemperature();
}
class LcdManagement
{
    public:
      LcdManagement();
      void showDisplay(float v, float t, float i, bool charging);
      void showDisplay2(float vLow, float vHigh);
      void showAuxiliar(String message);
    private:
      LiquidCrystal_I2C *lcd;
};

LcdManagement :: LcdManagement()
{
  Wire.begin();
  lcd = new LiquidCrystal_I2C(0x27,20,4);
  lcd->init();
  lcd->backlight();
}

void LcdManagement :: showDisplay(float v, float t, float i, bool charging)
{
  lcd->clear();
  lcd->setCursor(0,0);
  lcd->print("Voltaje:");
  lcd->print(v);
  lcd->setCursor(0,1);
  lcd->print("Corriente:");
  lcd->print(i);
  lcd->setCursor(0,2);
  lcd->print(charging ? "Cargando!": "En descarga...");  
  lcd->setCursor(0,3);
  lcd->print("Temp:");
  lcd->print(t);
  lcd->setCursor(10,3);
}
void LcdManagement :: showAuxiliar(String message)
{
  lcd->clear();
  delay(100);
  lcd->setCursor(0,0);
  lcd->print("Llego un mensaje:");
  lcd->setCursor(0,1);
  lcd->print(message);
  delay(1500);
}
void LcdManagement :: showDisplay2(float vLow, float vHigh)
{
  lcd->clear();
  lcd->setCursor(0,0);
  lcd->print("Voltaje LOW:");
  lcd->print(vLow);
  lcd->setCursor(0,1);
  lcd->print("Voltaje HIGH:");
  lcd->print(vHigh);
}
class ReleManagement
{
  public:
    ReleManagement();
    int controlBatteryCharge(float vActual,float vLow,float vHigh);
};
ReleManagement :: ReleManagement()
{
  pinMode(RELE_1, OUTPUT);//corte BATERIA
  pinMode(RELE_2, OUTPUT);//paso a cargador
  pinMode(RELE_3, OUTPUT);//cooler enfriador
  pinMode(RELE_4, OUTPUT);//auxiliar
}
int ReleManagement ::controlBatteryCharge(float vActual,float vLow,float vHigh)
{
  int reten = 0;
  if (vActual > vHigh ){
    reten = 0;
    digitalWrite(RELE_1,HIGH);//cuando esta por encima del limite superior deja conectado el artefacto que consume   
  }
  else if(vActual > vLow && reten == 0){
    digitalWrite(RELE_1,HIGH);//cuando esta entremedio de los valores, y reten es 0 deja conectada la carga, 
  }                           // pero cuando reten es 1 por que llego a ser mas bajo el actual que el nivel minimo, 
  else{                       // deja conectado el cargador, por que pasa directamente a la ultima condicion.
    reten = 1;
    digitalWrite(RELE_1,LOW);
  }

  return reten;
}

//Global variables

String serialData = ""; 
FileManagement *fileManager;
EEpromManagement *eepromManager;
RtcManagement *rtcManager;
SensorsManagement * sensorsManager;
LcdManagement * lcdManager;
ReleManagement * releManager;
static float highV;
static float lowV;

void setup() 
{
  Serial.begin(115200);		// inicializa monitor serie a 9600 bps
  
  fileManager = new FileManagement();
  eepromManager = new EEpromManagement();
  rtcManager = new RtcManagement();
  sensorsManager = new SensorsManagement();
  lcdManager = new LcdManagement();
  releManager = new ReleManagement();

  highV = eepromManager->getHighV();
  lowV = eepromManager->getLowV();
  Serial.print(highV);

  digitalWrite(RELE_1,HIGH);
  digitalWrite(RELE_2,HIGH);
  digitalWrite(RELE_3,HIGH);
  digitalWrite(RELE_4,HIGH);
}

void loop() 
{
  float voltageMeassured = sensorsManager->getVoltageMeassure();
  float currentMeassured = sensorsManager->getCurrentMeassure();
  bool currentScreen = true;
  //Data Recolection

  DateTime now = rtcManager->now();
  const int intervalMinutes = 1; 
  unsigned int lastExecution = 0;

  if (now.unixtime() - lastExecution >= intervalMinutes * 60) 
  {
    fileManager->UpdateTxt(voltageMeassured, rtcManager->getDate(), rtcManager->getTime());
    lastExecution = now.unixtime();
  }

  //Rele control
  int charging = releManager->controlBatteryCharge(voltageMeassured, lowV, highV);
  
  //Input control

  if(sensorsManager->controlButtons())
  {
    currentScreen =! currentScreen;
    delay(1000);
  }
  Serial.println(sensorsManager->getTemperature());
  //Data to the user
  if(currentScreen)
    lcdManager->showDisplay(voltageMeassured, sensorsManager->getTemperature(), currentMeassured, charging);
  else
    lcdManager->showDisplay2(lowV,highV);
}
void serialEvent()
{
  //Recepción de datos Seriales
  
  while (Serial.available()) 
  {              
    char CaracterEntrada = Serial.read();
    serialData += CaracterEntrada;   
    if (CaracterEntrada == '\n') {         
      Serial.println(serialData);
      if(String(serialData[0]) == String("V")) 
      {
        Serial.println("Es un comando");
        String sValueSent = "";
        for(int i=2; i < 7; i++)
          sValueSent += serialData[i];
        if(String(serialData[1]) == String("H"))
          eepromManager->setHighV(sValueSent.toFloat());
        else if(String(serialData[1]) == String("L"))
          eepromManager->setLowV(sValueSent.toFloat());
        
        Serial.println(sValueSent.toFloat());
      }
      lcdManager->showAuxiliar(serialData);
      serialData = "";                        
    }
  }
}