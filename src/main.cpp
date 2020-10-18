#include <WiFi.h>
#include <analogWrite.h>

// Definindo as credenciais da rede Wi-Fi
const char* ssid = "Assembly";
const char* password = "#code1304";

// Define a porta do server como 80
WiFiServer server(80);

// Variavel para armazenar o HTTP request
String header;

// Auxiliar variables to store the current output state
String redString = "0";
String greenString = "0";
String blueString = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;
int lightValue, lightInit;
String color;

// Define as saídas para seus pinos GPIO
const int photoresistor = 35;
const int bluePin = 4;
const int greenPin = 2;
const int redPin = 15;

// Estado do LED
bool led_on = false;  

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

//Funções responsáveis por executar o brilho selecionado
void redColor(){
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
}
void blueColor(){
  digitalWrite(bluePin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
}
void greenColor(){
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
}
void whiteColor(){
  digitalWrite(bluePin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, HIGH);
}
void yellowColor(){
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, HIGH);
}
void magentaColor(){
  digitalWrite(bluePin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
}
void cyanColor(){
  digitalWrite(bluePin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(redPin, LOW);
}

void blackOut(){
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
}

void setup() {
  Serial.begin(115200);
  // Inicaliza as variaveis de saida
  pinMode(photoresistor, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  // Define as saidas desligadas
  digitalWrite(photoresistor, LOW);
  digitalWrite(bluePin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, LOW);
  // Começa a leitura do valor de luminosidade
  lightInit = analogRead(photoresistor);
  // Conecta a rede Wi-Fi com o SSID e password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop()
{
  
  WiFiClient client = server.available();   // Servidor disponivel para acesso
  // Leitura de Luminosidade
  /*
  lightValue = analogRead(photoresistor);
  if(lightValue - lightInit <  50){         
      whiteColor();
  }else{
      blackOut();
  }
  */
  if (client) {                             // Caso haja alguma conexao
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // Enquanto o cliente ficar conectado
      currentTime = millis();
      if (client.available()) {             // Caso haver algum dado para ler
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Botões para escolher a cor
            if (header.indexOf("white") != -1) {
              Serial.println("BRANCO");
              whiteColor();
            }
            if (header.indexOf("red") != -1) {
              Serial.println("VERMELHO");
              redColor();
            }
            if (header.indexOf("green") != -1) {
              Serial.println("VERDE");
              greenColor();
            }
            if (header.indexOf("blue") != -1) {
              Serial.println("AZUL");
              blueColor();
            }
            if (header.indexOf("yellow") != -1) {
              Serial.println("AMARELO");
              yellowColor();
            }
            if (header.indexOf("magenta") != -1) {
              Serial.println("MAGENTA");
              magentaColor();
            }
            if (header.indexOf("cyan") != -1) {
              Serial.println("CYANO");
              cyanColor();
            }

            // Pagina Web em HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button1 { background-color: #FFFFFF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button2 { background-color: #FF0000; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button3 { background-color: #FFFF00; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button4 { background-color: #00FF00; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button5 { background-color: #00FFFF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button6 { background-color: #0000FF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button7 { background-color: #FF00FF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");

            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");

            // Cabeçalho da pagina WEB
            client.println("</style></head>");
            client.println("<body><center><h1>Prototipo de Lampada Inteligente</h1></center>");

            // Botões para escolher uma cor
            client.println("<form><center>"); 
            client.println("<p> Escolha uma cor </p></center>");
            client.println("<div class=\"btn-group\">");
            client.println("<button class=\"button1\" name=\"colorLED\" value=\"white\" type=\"submit\"> </button>");
            client.println("<button class=\"button2\" name=\"colorLED\" value=\"red\" type=\"submit\"> </button>");
            client.println("<button class=\"button3\" name=\"colorLED\" value=\"yellow\" type=\"submit\"> </button>");
            client.println("<button class=\"button4\" name=\"colorLED\" value=\"green\" type=\"submit\"> </button>");
            client.println("<button class=\"button5\" name=\"colorLED\" value=\"cyan\" type=\"submit\"> </button>");
            client.println("<button class=\"button6\" name=\"colorLED\" value=\"blue\" type=\"submit\"> </button>");
            client.println("<button class=\"button7\" name=\"colorLED\" value=\"magenta\" type=\"submit\"> </button>");
            client.println("</div></form></body></html>");
            // The HTTP response ends with another blank line

            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Limpa o cabecalho
    header = "";
    // Encerra a conexao
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    }
}