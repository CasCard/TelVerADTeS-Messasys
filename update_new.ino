#include <SoftwareSerial.h>
#include <TinyGPS.h>
SoftwareSerial GPS(2,3); // RX, TX
//Initialization of Each pins and Module
TinyGPS gps;
void gpsdump(TinyGPS &gps);
bool feedgps();
void getGPS();
long lat, lon;
float LAT, LON;
int pinButton = 6; 
int pinButton_panic = A1;
int mqSixValue;
int panicValue;
int input_sensor = A5;
int buz_pin = 13;
int xpin = 11;                  // x-axis of the accelerometer
int ypin = 10;                  // y-axis
int zpin = 9;                   // z-axis
String latitude="No Range..";
String longitude="No Range..";
int temp=0,i;
int track=0;
int trig=A3;
int echo=A4;
long duration;
int distance;

void setup() 
{
  //Pin configurations of Ultra Sonic
  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);
  //Pin configurations of Trigger Pins from RF Transmitter and Reciever
  pinMode(pinButton, INPUT);
  pinMode(pinButton_panic, OUTPUT);

   // Starting of GPS and Our Unit
  Serial.begin(9600);
  GPS.begin(9600);
  Serial.begin(9600);
  Serial.println("Accident Detection,Messaging,Tracking and Security System");
  delay(2000);
  gsm_init();
  
  Serial.println("AT+CNMI=2,2,0,0,0");
  Serial.println("GPS Initializing");
  Serial.println("  No GPS Range  ");
  delay(2000);
  Serial.print("Latitude : ");
  Serial.print(LAT/1000000,8);
  Serial.print(" :: Longitude : ");
  Serial.println(LON/1000000,8);
  get_gps();
  Serial.println("GPS Range Discovered");
  Serial.println("GPS is Ready");
  delay(2000);
  
  Serial.println("Configuring Network");
  delay(2000);
  Serial.println("Done!...");
  Serial.flush();
  Serial.flush();
  Serial.println("System Ready");
  temp=0;
// End of Setup
}

void loop()
{
  //Checking Status Of Trigger Pins of RF and Panic Switch(To inform if any false case)
  int stateButton = digitalRead(pinButton);
  panicValue = analogRead(pinButton_panic);
  float voltagePanic = panicValue * (5.0 / 1023.0);

  if(stateButton==1 && voltagePanic<=3.00)
  {
    serialEvent();
    get_gps();
    tracking();
  }
   
  if(voltagePanic>=4.00 && stateButton==0){
    serialEvent();
    get_gps();
    panictracking();
  }
  //End

  //Setting Up Ultrasonic Module
  digitalWrite(trig,LOW);
  delayMicroseconds(2);

  digitalWrite(trig,HIGH);
  delayMicroseconds(10);
  digitalWrite(trig,LOW);
// Finding distance using distance formula(with velocity of sound and duration(in cm))
  duration=pulseIn(echo,HIGH);
  distance=duration*0.034/2;
/*  
 *   (Trigger live distance )
 Serial.print("Distance :");
  Serial.println(distance); 
*/
 // Trigger if SMS and HTTP 
if(distance==3)
  {
    for(int f=0;f<1;f++){
     get_gps();
     airLocation();
     delay(2000);  // sets the LED on
    }
  }

  /*
   * For car tilt system
   * Connected to Triple Axis Accelerometer
   */
if(digitalRead(zpin)==0){
  for(int m=0;m<1;m++){
    get_gps();
    carTilt();
  }
  Serial.print(digitalRead(zpin));
 // print a tab between values:
  Serial.print("\t");
  Serial.println("Accident !!");
}
delay(1000);

// Defined for MQ6 Gas Sensor : Check for the conc. values 0-1024
  mqSixValue = analogRead(input_sensor);
   if (mqSixValue >= 750)
  {
    digitalWrite(buz_pin, HIGH);
    gasLeak();
    delay(1000);  // sets the LED on
  }
   else
  {
  digitalWrite(buz_pin, LOW);    // sets the LED off
  }
  //For Vehicle Tracking 
  //Track<space>Your Vehicle Number  (Send from phone to defined number in AT)
  vehicle_tracking();
  if(track==1){
    get_gps();
    vehicleLocation();
  }
  
 /*
  * Theft Security System 
  * Check for the Analog Outputs and Trigger 
  * */
 
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
   int sensorValue_two = analogRead(A2);
  float voltage_two = sensorValue_two * (5.0 / 1023.0);
 if (voltage == 5.00 && voltage_two <= 4.90){
    get_gps();
    theftSecurity();
    delay(8000);
  }else if (voltage <= 4.90 && voltage_two==5.00){
     get_gps();
    theftSecurity();
    delay(8000);
  }else if(voltage < 4.90 && voltage_two < 4.90){
       get_gps();
      theftSecurityPlus();
      delay(8000);
  }
  //End of Loop
}
// State Check
void serialEvent()
{
  int stateButton = digitalRead(pinButton);
//  int stateButton_panic = digitalRead(pinButton_panic);
  while(stateButton){
    if(stateButton==1)
    {
      temp=1;
      break;
    }
    else
    temp=0;
   
  }
}


// GSM SetUP
void gsm_init()
{
  
  Serial.println("Finding GSM Module...");
  boolean at_flag=1;
  while(at_flag)
  {
    Serial.println("AT");
    while(Serial.available()>0)
    {
      if(Serial.find("OK"))
      at_flag=0;
    }
    
    delay(1000);
  }

  
  Serial.println("GSM Module Connected");
  delay(1000);
 
  Serial.println("Disabling ECHO");
  boolean echo_flag=1;
  while(echo_flag)
  {
    Serial.println("ATE0");
    while(Serial.available()>0)
    {
      if(Serial.find("OK"))
      echo_flag=0;
    }
    delay(1000);
  }

  
  Serial.println("Echo OFF");
  delay(1000);
 
  Serial.println("Finding Network..");
  boolean net_flag=1;
  while(net_flag)
  {
    Serial.println("AT+CPIN?");
    while(Serial.available()>0)
    {
      if(Serial.find("+CPIN: READY"))
      net_flag=0;
    }
    delay(1000);
  }
  
  Serial.println("Network Found..");
  delay(1000);
  
}
// Retrieving GPS Data (Coordinates)
void get_gps()
{
bool newdata = false;
  unsigned long start = millis();
  // Every 1 seconds we print an update
  while (millis() - start < 1000)
  {
    if (feedgps ()){
      newdata = true;
    }
  }
  if (newdata)
  {
    gpsdump(gps);
  }
  Serial.print("Latitude : ");
  Serial.print(LAT/1000000,8);
  Serial.print(" :: Longitude : ");
  Serial.println(LON/1000000,8);
}

bool feedgps(){
  while (GPS.available())
  {
    if (gps.encode(GPS.read()))
      return true;
  }
  return 0;
}
void gpsdump(TinyGPS &gps)
{
  //byte month, day, hour, minute, second, hundredths;
  gps.get_position(&lat, &lon);
  LAT = lat;
  LON = lon;
  {
    feedgps(); // If we don't feed the gps during this long 
//routine, we may drop characters and get checksum errors
  }
}
// AT initialization
void init_sms()
{
  Serial.println("AT+CMGF=1");
  delay(400);
  Serial.println("AT+CMGS=\"+919495707364\"");   // use your 10 digit cell no. here
  delay(400);
}

// SMS Content

void send_data(String message)
{
  Serial.println(message);
  delay(200);
}
// Coordinate Transmission
void send_coord(float message)
{
  Serial.println(message/1000000,8);
  delay(200);
}

void send_sms()
{
  Serial.write(26);
}

void serial_status()
{
  Serial.println("Message Sent");
  delay(2000);
  Serial.println("System Ready");
  return;
}

void tracking()
{
    init_sms();
   
    send_data("Accident Alert !");
    Serial.print("\nLatitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
    // String Concatenate
  float latitude = LAT;
  float longitude = LON;
  String buf;
  buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
  buf += String(latitude/1000000,7);
  buf += F(",");
  buf += String(longitude/1000000,7);
  buf +=F("&mode=driving");
  Serial.print("\nMap:");
  send_data(buf);
  send_sms();
  delay(2000);
  serial_status();

  // HTTP Requests  
  Serial.println("AT+CGATT?");
  delay(100);
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  //NSP Settings
  Serial.println("AT+SAPBR=3,1,\"APN\",\"idea\"");
  delay(2000);
  Serial.println("AT+SAPBR=1,1");
  delay(2000);
  Serial.println("AT+HTTPINIT");
  delay(2000); 
 // HTTP URL Posting    
    String url;
    url +=F("AT+HTTPPARA=\"URL\",\"http://www.mylifedimensions.com/test/accident_alert.php?lat=");
    url +=String(latitude/1000000,7);
    url += F("&lon=");
    url +=String(longitude/1000000,7);
    url += F("&typ=Accident%20Alert&lin=");
    url += F("http://maps.google.com/maps?");
    url += String(latitude/1000000,7);
    url += F(",");
    url += String(longitude/1000000,7);
    url +=F("&mode=driving\"");

    Serial.println(url); 
    delay(8000);
    Serial.println("AT+HTTPACTION=0");
    delay(7000);
    // read server response
    Serial.println("AT+HTTPREAD"); 
    delay(2000);
    Serial.println("");
    Serial.println("AT+HTTPTERM");
    delay(1000);
    Serial.println("");
    delay(10000);
    Serial.println(url); 
    delay(5000);
    Serial.println("AT+HTTPACTION=0");
    delay(6000);
   
    Serial.println("Server Upload Successful");
}
void panictracking()
{
    init_sms();
    send_data("False Alert !");
    Serial.print("\nLatitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
     buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
     buf += String(latitude/1000000,7);
     buf += F(",");
     buf += String(longitude/1000000,7);
     buf +=F("&mode=driving");
     Serial.print("\nDrive:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
     send_sms();
     delay(2000);
     serial_status();
    
  Serial.println("AT+CGATT?");
  delay(100);
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  Serial.println("AT+SAPBR=3,1,\"APN\",\"idea\"");
  delay(2000);
  Serial.println("AT+SAPBR=1,1");
  delay(2000);
  Serial.println("AT+HTTPINIT");
  delay(2000); 
    
    String url;
    url +=F("AT+HTTPPARA=\"URL\",\"http://www.mylifedimensions.com/test/accident_alert.php?lat=");
    url +=String(latitude/1000000,7);
    url += F("&lon=");
    url +=String(longitude/1000000,7);
    url += F("&typ=False%20Alert&lin=");
    url += F("http://maps.google.com/maps?");
    url += String(latitude/1000000,7);
    url += F(",");
    url += String(longitude/1000000,7);
    url +=F("&mode=driving\"");

    Serial.println(url); 
    delay(8000);
    Serial.println("AT+HTTPACTION=0");
    delay(7000);
    // read server response
    Serial.println("AT+HTTPREAD"); 
    delay(2000);
    Serial.println("");
    Serial.println("AT+HTTPTERM");
    delay(1000);
    Serial.println("");
    delay(10000);
    Serial.println(url); 
    delay(5000);
    Serial.println("AT+HTTPACTION=0");
    delay(6000);
    Serial.println("Server Upload Successful");
}
void vehicle_tracking()
{
  while(Serial.available())
  {
   if(Serial.find("Track KL46D5782")){
     track=1;
    }
       else
       track=0;
    }
}
void carTilt(){
   init_sms();
   send_data("Car Tilt");
    //send_data("Your Vehicle Current Location is:");
    Serial.print("\nLatitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
     buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
  buf += String(latitude/1000000,7);
  buf += F(",");
  buf += String(longitude/1000000,7);
  buf +=F("&mode=driving");
     Serial.print("\nDrive:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
    send_sms();
    delay(2000);
    serial_status();

  Serial.println("AT+CGATT?");
  delay(100);
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  Serial.println("AT+SAPBR=3,1,\"APN\",\"idea\"");
  delay(2000);
  Serial.println("AT+SAPBR=1,1");
  delay(2000);
  Serial.println("AT+HTTPINIT");
  delay(2000); 
    
    String url;
    url +=F("AT+HTTPPARA=\"URL\",\"http://www.mylifedimensions.com/test/accident_alert.php?lat=");
    url +=String(latitude/1000000,7);
    url += F("&lon=");
    url +=String(longitude/1000000,7);
    url += F("&typ=Car%20Tilt&lin=");
    url += F("http://maps.google.com/maps?");
    url += String(latitude/1000000,7);
    url += F(",");
    url += String(longitude/1000000,7);
    url +=F("&mode=driving\"");

    Serial.println(url); 
    delay(8000);
    Serial.println("AT+HTTPACTION=0");
    delay(7000);
    // read server response
    Serial.println("AT+HTTPREAD"); 
    delay(2000);
    Serial.println("");
    Serial.println("AT+HTTPTERM");
    delay(1000);
    Serial.println("");
    delay(10000);
    Serial.println(url); 
    delay(5000);
    Serial.println("AT+HTTPACTION=0");
    delay(6000);
    Serial.println("Server Upload Successful");
}
void theftSecurity(){
    init_sms();
   //send_data("Vehicle Number:KL46D5782");
    //send_data("Your Vehicle Current Location is:");
    send_data("Single Line Cut!");
    Serial.print("Latitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
     buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
     buf += String(latitude/1000000,7);
     buf += F(",");
     buf += String(longitude/1000000,7);
     buf +=F("&mode=driving");
     Serial.print("\nDrive:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
     send_sms(); 
     delay(2000); 
     serial_status(); 
  
}
void theftSecurityPlus(){
    init_sms();
    send_data("Line 1&2 Cut!");
    Serial.print("Latitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
     buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
     buf += String(latitude/1000000,7);
     buf += F(",");
     buf += String(longitude/1000000,7);
     buf +=F("&mode=driving");
     Serial.print("Map:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
     send_sms(); 
      delay(2000);
      serial_status(); 

 
}

void vehicleLocation(){
  init_sms();
   send_data("Currently At");
    //send_data("Your Vehicle Current Location is:");
    Serial.print("\nLatitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
     buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
  buf += String(latitude/1000000,7);
  buf += F(",");
  buf += String(longitude/1000000,7);
  buf +=F("&mode=driving");
     Serial.print("\nLocation Map:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
    send_sms();
    delay(2000);
    serial_status();

    
}

void airLocation(){
  init_sms();
   send_data("Airline@");
    //send_data("Your Vehicle Current Location is:");
    Serial.print("\nLatitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
    buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
  buf += String(latitude/1000000,7);
  buf += F(",");
  buf += String(longitude/1000000,7);
  buf +=F("&mode=driving");
     Serial.print("\nLocation Map:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
    send_sms();
    delay(2000);
    serial_status();

Serial.println("AT+CGATT?");
  delay(100);
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
  Serial.println("AT+SAPBR=3,1,\"APN\",\"idea\"");
  delay(2000);
  Serial.println("AT+SAPBR=1,1");
  delay(2000);
  Serial.println("AT+HTTPINIT");
  delay(2000); 
    
   String url;
    url +=F("AT+HTTPPARA=\"URL\",\"http://www.mylifedimensions.com/test/accident_alert.php?lat=");
    url +=String(latitude/1000000,7);
    url += F("&lon=");
    url +=String(longitude/1000000,7);
    url += F("&typ=Airline@&lin=");
    url += F("http://maps.google.com/maps?");
    url += String(latitude/1000000,7);
    url += F(",");
    url += String(longitude/1000000,7);
    url +=F("&mode=driving\"");
    
    Serial.println(url); 
    delay(8000);
    Serial.println("AT+HTTPACTION=0");
    delay(7000);
    // read server response
    Serial.println("AT+HTTPREAD"); 
    delay(2000);
    Serial.println("");
    Serial.println("AT+HTTPTERM");
    delay(1000);
    Serial.println("");
    delay(10000);
    Serial.println(url); 
    delay(5000);
    Serial.println("AT+HTTPACTION=0");
    delay(6000);
    Serial.println("Server Upload Successful");

    
}

void gasLeak()
{
    init_sms();
   send_data("Gas Leak!");
    //send_data("Your Vehicle Current Location is:");
    if (mqSixValue >= 950)
  {
    Serial.print("Concentration Level Very High");
    send_data("Concentration Level Very High");
     // sets the LED on
  }
    Serial.print("\nLatitude:");
    send_coord(LAT);
    Serial.print("\nLongitude:");
    send_coord(LON);
     float latitude = LAT;
     float longitude = LON;
     String buf;
   buf += F("http://maps.google.com/maps?saddr=My%20location&daddr=");
  buf += String(latitude/1000000,7);
  buf += F(",");
  buf += String(longitude/1000000,7);
  buf +=F("&mode=driving");
     Serial.print("\nMap:");
     send_data(buf);
    //send_data("Sorry for false alert \nThank You");
    send_sms();
    delay(2000);
    serial_status();

    Serial.println("AT+CGATT?");
  delay(100);
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(2000);
   Serial.println("AT+SAPBR=3,1,\"APN\",\"idea\"");
  delay(2000);
   Serial.println("AT+SAPBR=1,1");
  delay(2000);
    Serial.println("AT+HTTPINIT");
    delay(2000); 
    
   String url;
    url +=F("AT+HTTPPARA=\"URL\",\"http://www.mylifedimensions.com/test/accident_alert.php?lat=");
    url +=String(latitude/1000000,7);
    url += F("&lon=");
    url +=String(longitude/1000000,7);
    url += F("&typ=Gas%20Leak&lin=");
    url += F("http://maps.google.com/maps?");
    url += String(latitude/1000000,7);
    url += F(",");
    url += String(longitude/1000000,7);
    url +=F("&mode=driving\"");
    
    Serial.println(url); 
    delay(8000);
    Serial.println("AT+HTTPACTION=0");
    delay(7000);
    // read server response
    Serial.println("AT+HTTPREAD"); 
    delay(2000);
    Serial.println("");
    Serial.println("AT+HTTPTERM");
    delay(1000);
    Serial.println("");
    delay(10000);
    Serial.println(url); 
    delay(5000);
    Serial.println("AT+HTTPACTION=0");
    delay(6000);
    Serial.println("Server Upload Successful");
}
// End Of Program


