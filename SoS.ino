#include <SoftwareSerial.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
/*
This sketch sends a string to a corresponding Arduino
with nRF24 attached. It appends a specific value
(2 in this case) to the end to signify the end of the
message.
*/
int msg[1];
RF24 radio(9,10);
const uint64_t pipe = 0xE8E8F0F0E1LL;
int lastmsg = 1;
String theMessage = “”;
String mobileNumber = “+xxxxxxxxxx”; // Replace it with your mobile number
String passphrase = “YOURPASSPHRASE”;
String armCode = passphrase;
String disarmCode = passphrase;
SoftwareSerial mySerial(2, 3); // Tx, Rx
String readString=“”;
String number = “”;
int checkFlag = 0;
int messageFlag = 0;
String message = “”;
const int buzzer = 7;
int messageLength = passphrase.length()+2; // Increase the length by two for operation code
void setup()
{
mySerial.begin(9600); // Setting the baud rate of GSM Module
Serial.begin(9600); // Setting the baud rate of Serial Monitor (Arduino)
delay(100);
radio.begin(); //Start listening for signals on the nRF radio
radio.openReadingPipe(1,pipe);
radio.startListening();
delay(200);
Serial.println(“Device Setup Complete.”);
pinMode(buzzer,OUTPUT);
receiveMessage();
armCode += “AA”; // Code AA for arming the system
disarmCode += “BB”; // Code BB for arming the system
}
void loop() {
checkMessages(); // Check for messages from smartphone
checkRadio(); // Check for any panic signals
}
void sendMessage() {
mySerial.println(“AT+CMGF=1”); //Sets the GSM Module in Text Mode
delay(1000); // Delay of 1000 milli seconds or 1 second
mySerial.println(“AT+CMGS=\”+xxxxxxxxxx\”\r”); // Replace xxxx… with mobile number
delay(1000);
mySerial.println(“Security Alert from Home!”);// The SMS text you want to send
delay(100);
mySerial.println((char)26);// ASCII code of CTRL+Z
delay(1000);
}
void receiveMessage() {
mySerial.println(“AT+CNMI=2,2,0,0,0”); // AT Command to receive a live SMS
delay(1000);
}
void validate(String number, String message){
if(number.equals(mobileNumber)){
if(message.equals(“ARM”)>0){
Serial.println(“ALERT!”);
arm();
}
else if(message.equals(“DISARM”)>0){
Serial.println(“DISARM!”);
disarm();
}
}
}
void arm () {
digitalWrite(buzzer,HIGH);//make a sound
}
void disarm () {
digitalWrite(buzzer,LOW);//silent
}
void checkMessages() {
if (mySerial.available()>0){
readString = mySerial.readString();
if(readString.length()>30) {
Serial.print(“The module received a message from:”);
int startNumber = readString.indexOf(‘”‘);
int EndNumber = readString.indexOf(‘”‘, startNumber+1);
number = readString.substring(startNumber+1, EndNumber);
Serial.println(number);
int startMessage = readString.lastIndexOf(‘”‘);
message = readString.substring(startMessage+1,readString.length()-1);
message = message.substring(message.indexOf(‘\n’)+1,message.length()-1);
Serial.print(“The message was:”);
Serial.println(message);
}
else if(readString.lastIndexOf(‘+’)==2){
Serial.println(“The module is now actively listening to incoming messages.”);
}
if(number!=“” && message!=“”){
validate(number, message);
}
}
}
void checkRadio() {
if (radio.available()){
bool done = false;
done = radio.read(msg, 1);
char theChar = msg[0];
if (msg[0] != 2){ // We detect that the message has ended when we detect a 2.
theMessage.concat(theChar);
}
else {
if (theMessage.length() == messageLength) {
if(theMessage.equals(armCode)){
arm(); // Trigger the local alarm
sendMessage(); //Send the notification to the programmed number
}
else if(theMessage.equals(disarmCode)){
disarm(); // Trigger the local alarm
sendMessage(); //Send the notification to the programmed number
}
}
theMessage= “”;
}
}
}
