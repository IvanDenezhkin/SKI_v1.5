#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include <SPI.h>
#include <Ethernet.h>
#define BUFSIZ 100
#include <SD.h>

#include <RTC.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
RTC  time;
#define ONE_WIRE_BUS 2 // порт для датчиков DS18b20
 
unsigned int s = 10; // время логинга
unsigned int Menuexit = 1;
unsigned int MenuStyle = 0;

// Введите MAC-адрес и IP-адрес вашего контроллера
// IP-адрес должен соответствовать вашей локальной сети
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDA, 0x02 };
IPAddress ip(192,168,0,10); //<<< IP-АДРЕС ВВОДИТЬ СЮДА!!!
EthernetServer server(80);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int menuButton = 14;
int buttonLeft = 15;
int buttonRight = 16;
int selectButton = 17;
int fflag=1;

Adafruit_PCD8544 display = Adafruit_PCD8544(22, 24, 26, 28, 30); 
// pin 22 - LCD reset (RST)
// pin 24 - LCD chip select (CS)
// pin 26 - Data/Command select (D/C)
// pin 28 - Serial data out (DIN)
// pin 30 - Serial clock out (SCLK)

float t[8] = {0,1,2,3,4,5,6,7};
float tc[8] = {0,0,0,0,0,0,0,0};
float alert[8]={37.5,37.5,37.5,37.5,37.5,37.5,37.5,37.5};
float delta = 1.0;
int Notification =0;
int Data[5]={0,0,0,0,0};
unsigned int Notification_status[8]={0,0,0,0,0,0,0,0};


void setup() {
  /*for(int i=0;i<8;i++){ //при первой прошивке затереть память нулями
    EEPROM.put(i*4,tc[i]);
    }*/
  for(int i=0;i<8;i++){EEPROM.get(i*4,tc[i]);} // чтение из памяти коэфициента коррекции
  
  pinMode (menuButton,INPUT);
  pinMode (buttonLeft,INPUT);
  pinMode (buttonRight,INPUT);
  pinMode (selectButton, INPUT);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  display.begin();
  display.setContrast(50);
  display.clearDisplay();
  
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
 
  
  while(1){
  Serial1.println("AT+CPAS");
  if (Serial1.find("0")) break;
  
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("Loading ...");
  display.display();
  display.clearDisplay(); // очищаем дисплей
  delay(100);  
 }
  Serial1.println("AT+CMGF=1");
  delay(100);
  Serial1.println("AT+CSCS=\"GSM\"");
  delay(100);
  
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(0,20);
  display.println("Loading OK");
  display.display();
  delay(1000); 
  display.clearDisplay(); // очищаем дисплей
   
  display.setCursor(0,20);
  display.println("CKI v.1.5");
  display.display();
  delay(1000); // задержка в секунду
  display.clearDisplay(); // очищаем дисплей

  
  SD.begin(4);
  Ethernet.begin(mac, ip);
  server.begin();
  
  sensors.begin();
  sensors.setResolution(12);
  
  time.begin(RTC_DS1302,5,6,7);  // на базе чипа DS1302, вывод RST, вывод CLK, вывод DAT
  
}

void loop() {
  
  if(digitalRead(menuButton) == HIGH){displayMenu();}
  Menuexit=1;
  
 sensors.requestTemperatures(); // Запрос температуры датчиков
 for(int i =0; i<8;i++){
  t[i]= (sensors.getTempCByIndex(i)+tc[i]);
 }

for(int i =0; i<8;i++){                                                           // проверяем мин и макс значения температуры и отправляем СМС
  if(t[i]<= (alert[i]-delta)|| t[i]>= (alert[i]+delta)) {
    if(Notification_status[i]==1){SMS(i,t[i]);}
    }
  }
 
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  
  switch(MenuStyle){
    case 0:
 display.print(" TEMPERATURE:");
 for(int i=0,pos=10 ;i<8;i++,pos+=10){
    if(t[i]<=-127){ display.print("XXX");} // если нет сигнала вывод ошибки
    
    else{
      if(i<4){                  // вывод первых 4х в 1й колонке
      display.setCursor(0,pos);
      if(Notification_status[i]==1){display.drawLine(0, pos+6, 38 , pos+6, BLACK);}
      if(pos>35){pos=0;}
      display.print(i+1);
      display.print("-");
      display.print(t[i],1);}
      
      else{display.setCursor(44,pos); // вывод вторых во 2й колонке
      if(Notification_status[i]==1){display.drawLine(44, pos+6, 80 , pos+6, BLACK);}
      if(pos>35){pos=0;}
      display.print(i+1);
      display.print("-");
      display.print(t[i],1);}}}
      display.display();
      delay(1000);
      display.clearDisplay();
      break;

      case 1:
      
    for(int i=0;i<8;i++){
    if(digitalRead(menuButton) == HIGH){i=7;displayMenu();}
    if(t[i]<=-127){ display.print("XXX");} // если нет сигнала вывод ошибки
    else{
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print(time.gettime("H:i"));
  display.setCursor(60,0);
  if(Notification_status[i]==1){display.print("ON");}
  display.print("#");
  display.print(i+1);
  display.setTextSize(3);
  display.setCursor(0,15);
  display.print(t[i],1);
  display.display();
  delay(2000);
  display.clearDisplay(); // очищаем дисплей
  }
    }
      break;
  }
  
  
  
  // запись лога каждые s мин
  time.gettime("H:i:s");
  //Serial.println(time.gettime("d-m-Y,H:i:s"));
  if (time.minutes%s==0 && time.seconds<=3) {
  String timer = time.gettime("d-m-Y, H:i:s");
  String temperatureString = ""; // logging data to sd card

  // read three sensors and append to the string:
  for (int i = 0; i < 8; i++) {
    float sensort = t[i];
    temperatureString += String(sensort);
    if (i < 8) {
    temperatureString += "*C, ";
    }
  }
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(timer);
    dataFile.println(temperatureString);
    dataFile.close();
    // print to the serial port too:
   //Serial.println(temperatureString);
  }
    else {
    //Serial.println("error opening datalog.txt");
  } 
  }
 EthernetGO ();
  }

  
void EthernetGO (){
  
  char clientline[BUFSIZ];
  int index = 0;

  //SD.remove("datalog.txt");
EthernetClient client = server.available();
  if (client) {
   boolean currentLineIsBlank = true;
   index = 0;
    while (client.connected()) {
   if (client.available()) {
  char c = client.read();
  
  if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZ) 
            index = BUFSIZ -1;
 
          // continue to read more data!
          continue;
        }
 clientline[index] = 0;
 
         if (strstr(clientline, "GET /log") != 0) {
        
          File dataFile = SD.open("datalog.txt");
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Disposition: attachment; datalog=default-download-datalod.txt;");
          client.println();
          int16_t c;
          while ((c = dataFile.read()) > 0) {
              // uncomment the serial to debug (slow!)
              client.print((char)c);
          }
          dataFile.close();
          break;
          }
        if (strstr(clientline, "GET / ") != 0)
         {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 10");  // refresh the page automatically every 60 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<title> CKI-1 </title>");
          client.println("<h1> Control system v.1.5 </h1>");
          
          for(int i = 0; i<8;i++){
          client.println("<br>");
          client.print("Temperatura #");
          client.print(i+1);
          client.print(" = ");
          client.print(t[i],1);
          client.print(" *C ");
          client.println("</br>");
          }
          client.println("");
          client.println("to download log print http://192.168.0.10/log "); 
          client.println("</html>");
          break;
        }
        
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(100);
    // close the connection:
    client.stop();
    }
}


void displayMenu(){
  display.clearDisplay(); // очищаем дисплей
  int flag=0;
  while(Menuexit !=0){
     if(digitalRead(buttonRight) == HIGH){
        flag++;
        delay(250);
       }
      if(digitalRead(buttonLeft) == HIGH){
      flag--;
      delay(250);
     }
    if (flag<0) flag=7;
    if (flag>7) flag=0;
     switch(flag){
       case 0:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Change Triger TEMPETARURE");
  if(digitalRead(selectButton) == HIGH){TemperatureSet();delay(250);}
  display.display();
       break;
       case 1:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Correction of t*");
  if(digitalRead(selectButton) == HIGH){Correction();delay(250);}
  display.display();
       break;
       case 2:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Set delta");
  if(digitalRead(selectButton) == HIGH){SetDelta();}
  display.display();
         break;
   case 3:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("MENU STYLE");
  if(MenuStyle == 0){
  display.println("MINI");}
  else {display.println("BIG");}
  
  if(digitalRead(selectButton) == HIGH){
    if(MenuStyle == 0){
    MenuStyle = 1;delay(250);}
    else {MenuStyle = 0;delay(250);}
  }
  display.display();
       break;
    case 4:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Set Log Time");
  if(digitalRead(selectButton) == HIGH){SetLog();}
  display.display();
         break;
    case 5:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Set Time&Data");
  if(digitalRead(selectButton) == HIGH){SetData();}
  display.display();
         break;
    case 6:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Delete LOG");
  if(digitalRead(selectButton) == HIGH){deletedata();}
  display.display();
         break;
    case 7:
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Exit");
  if(digitalRead(selectButton) == HIGH){
    Menuexit=0;
    break;
  }
  display.display();
        break;
}
}
}


void TemperatureSet(){
  
 for(int number_of_ds=-1;number_of_ds!=8;){
  if(digitalRead(selectButton) == HIGH){number_of_ds++;delay(250);}
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("SET ALARM:");
  display.print("TEMP #");
  display.print(number_of_ds+1);
  display.print("- ");
  display.println(t[number_of_ds],1);
  display.println("");
  display.print("min ");
  display.println((alert[number_of_ds])-delta,1);
  display.print("max ");
  display.println((alert[number_of_ds])+delta,1);
  display.print("Alarm ");
  if(Notification_status[number_of_ds] == 1){display.print("ON");}
  else {display.print("OFF");}
  display.display();
   if(digitalRead(buttonRight) == HIGH){alert[number_of_ds]+= 0.1; delay(150);}
   if(digitalRead(buttonLeft) == HIGH){alert[number_of_ds]-= 0.1; delay(150);}
   if(digitalRead(menuButton) == HIGH){
      if(Notification_status[number_of_ds] == 0){Notification_status[number_of_ds] = 1;} else {Notification_status[number_of_ds] = 0;}
      delay(200);
      }
   }
   exit;
}
void Correction(){
  
 for(int number_of_ds=-1;number_of_ds!=8;){
  if(digitalRead(selectButton) == HIGH){
   EEPROM.put(number_of_ds*4,tc[number_of_ds]);
    number_of_ds++;
    delay(250);
    }
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.print("SET CORRECTION");
  display.print("TEMP #");
  display.print(number_of_ds+1);
  display.print("- ");
  display.println(t[number_of_ds],1);
  display.println(tc[number_of_ds],1);
  display.print("after correction ");
  display.println((t[number_of_ds])+tc[number_of_ds],1);
  display.display();
   if(digitalRead(buttonRight) == HIGH){tc[number_of_ds]+= 0.1; delay(250);}
   if(digitalRead(buttonLeft) == HIGH){tc[number_of_ds]-= 0.1; delay(250);}
   }
   exit;
}
void SetDelta(){
  delay(250);
  while(Menuexit==1){
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Set Delta:");
  display.println(delta);
  display.display();
  if(digitalRead(selectButton) == HIGH) {delay(150); break;} 
  if(digitalRead(buttonRight) == HIGH){delta+= 0.1; delay(250);}
  if(digitalRead(buttonLeft) == HIGH){delta-= 0.1; delay(250);}
  }
}

void SetLog(){
  delay(250);
  while(Menuexit==1){
  if(s<1)s=60;
  if(s>=61)s=1;
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("Log every:");
  display.print(s);
  display.print(" min");
  display.display();
  
  if(digitalRead(selectButton) == HIGH) {delay(150); break;} 
  if(digitalRead(buttonRight) == HIGH){s+= 1; delay(250);}
  if(digitalRead(buttonLeft) == HIGH){s-= 1; delay(250);}
  }
}


void SMS(int number, float temp ){
  
while(1){
  Serial1.println("AT+CPAS");
  if (Serial1.find("0")) break;
  delay(100);  
 }
 Serial1.println("AT+CMGS=\"+380991025313\"");
 delay(100);
 Serial1.print("Temperatura v kamere #");
 delay(100);
 Serial1.print(number+1);
 delay(100);
 Serial1.print(" = ");
 delay(100);
 Serial1.print(temp,1);  // и переменную со значением
 delay(100);
 Serial1.print((char)26);          // символ завершающий передачу*/
 Notification_status[number] = 0;
 }
      
      
void SetData(){
for(int i=-1;i!=5;){
  if(digitalRead(selectButton) == HIGH){i++;delay(250);}
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("SET DATA:");
  display.println("m:h,d-m-y");
  for(int s=0; s<5;s++){display.print(Data[s]); display.print(":");}
  display.display();
  if(digitalRead(buttonRight) == HIGH){Data[i]+=1; delay(100);}
  if(digitalRead(buttonLeft) == HIGH){Data[i]-=1; delay(100);}
  
  if(Data[0]>59){Data[0]=0;} // ограничение для минут
  if(Data[0]<0){Data[0]=59;} 
  
  if(Data[1]>23){Data[1]=0;} // ограничение для часов
  if(Data[1]<0){Data[1]=23;} 

  if(Data[2]>31){Data[2]=0;} // ограничение для дней 
  if(Data[2]<0){Data[2]=31;} 

  if(Data[3]>12){Data[3]=0;} // ограничение для месяца
  if(Data[3]<0){Data[3]=12;} 
  
  if(Data[4]>99){Data[4]=0;} // ограничение для года
  if(Data[4]<0){Data[4]=99;} 
  
  if(i==5){time.settime(0,Data[0],Data[1],Data[2],Data[3],Data[4],0);} // time.settime(сек,мин,час,д,м,г,дн)
   }
 }

void deletedata(){
  delay(500);
while(1){
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("DELETE ???");
  display.display();
  if(digitalRead(selectButton) == HIGH) {
  SD.remove("datalog.txt");
  display.clearDisplay(); // очищаем дисплей
  display.fillScreen(WHITE);
  display.setCursor(0,0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.println("LOG CLEARED");
  display.display();
  delay(1000); 
  break;
  }
 }
}
