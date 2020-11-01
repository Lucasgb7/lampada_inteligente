#include <WiFi.h>
#include <analogWrite.h>
#include "Ultrasonic.h"

//Definindo as credenciais da rede Wi-Fi
const char *ssid = "Assembly";
const char *password = "#code1304";

// Define a porta do server como 80
WiFiServer server(80);

// Variavel para armazenar o HTTP request
String header;

// Variaveis auxiliares
static unsigned int counter = 0;
int lightValue, distanceInit, distanceValue, ledColor = 1, delayTimer = 5, brightness = 99, positionBrightness, dutyCycle = 255;
bool powerButtonState = false, ledOn = false, buttonOn = false;
hw_timer_t * timer = NULL;
String Sbrightness = "99";

// Define as saídas para seus pinos GPIO
const int redPin = 13;    // R = 330 Ohm
const int greenPin = 12;   // R = 330 Ohm
const int bluePin = 14;    // R = 330 Ohm
const int photoresistor = 33;  // R = 10K Ohm
const int powerButton = 32;   // R = 10K Ohm
const int trig = 26;
const int echo = 25;

// Sensor ultrasonico
Ultrasonic ultrasonic(trig, echo);
// Retorna o tempo atual em milisegundos
unsigned long currentTime = millis();
// Retorna o tempo anterior
unsigned long previousTime = 0;
// Define o tempo maximo em milisegundos (exemplo: 2000ms = 2s)
const long timeoutTime = 2000;
// Configuração das propriedados do PWM
const int freq = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;
const int resolution = 8;

// funções de cor para o LED RGB
void calculateDutyCycle();
void redColor();
void blueColor();
void greenColor();
void blueColor();
void whiteColor();
void yellowColor();
void magentaColor();
void cyanColor();
void blackOut();

// Funções de interrupção do temporizador
void startTimer();
void stopTimer();
void cb_timer();

// Leitura do sensor ultrasonico
int hcsr04();

// Funções da aplicação
void switchLED();
void personDetection();
void monitor();

void setup(){
  Serial.begin(115200);
  // Inializa as variaveis de saida e entrada
  pinMode(echo, INPUT);
  pinMode(trig, OUTPUT);
  pinMode(powerButton, INPUT);
  // Configura as funcionalidades PWM do LED
  ledcSetup(redChannel, freq, resolution);
  ledcSetup(greenChannel, freq, resolution);
  ledcSetup(blueChannel, freq, resolution);
  // Vincula os canais com as portas a serem controladas
  ledcAttachPin(redPin, redChannel);
  ledcAttachPin(greenPin, greenChannel);
  ledcAttachPin(bluePin, blueChannel);
  // Iniciando leitura dos sensores analógicos
  lightValue = analogRead(photoresistor); // Sensor LDR
  distanceInit = hcsr04();                // Sensor Ultrasonico
  // Conecta a rede Wi-Fi com o SSID e password
  Serial.print("Conectando a rede ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("Endereco IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available(); // Servidor disponivel para acesso
  // Leitura de luminosidade
  lightValue = analogRead(photoresistor);
  // Leitura de distancia
  distanceValue = hcsr04();
  // Caso tenha sido ligado pelo botão, a deteccao é desligada
  if(!buttonOn)
    personDetection();
  // Ajusta cor e brilho do LED
  switchLED();
  // Clique do botão físico
  if(digitalRead(powerButton)){
    if(ledOn){
      ledOn = false;
      buttonOn = false;
    }else{
      ledOn = true;
      buttonOn = true;
    }
    switchLED();
    delay(500);
  }
  if (client){ // Caso haja alguma conexao
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Cliente novo.");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // Enquanto o cliente ficar conectado
      currentTime = millis();
      if (client.available()){ // Caso haver algum dado para ler
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') { // caso o byte seja uma nova linha
          // Caso a linha atual esteja em branco, pega dois caracteres de nova linha de uma vez
          // esse é o fim do requeste do cliente HTTP, então envia uma resposta:
          if (currentLine.length() == 0) {
            // HTTP headers sempre começam o código de resposta (e.g. HTTP/1.1 200 OK)
            // e o content-type, entao o cliente sabe oque está recebendo, entao uma linha em branco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Conexao: fechada");
            client.println();
            // Recebe brilho do servidor
            if(header.indexOf("brightness") != -1){
              positionBrightness = header.indexOf("brightness") + 11;
              Sbrightness = header.substring(positionBrightness, positionBrightness + 2);
              brightness = Sbrightness.toInt();
              calculateDutyCycle();
            }
            // Botões para escolher a cor
            if (header.indexOf("white") != -1){
              ledColor = 1;
            }
            if (header.indexOf("red") != -1){
              ledColor = 2;
            }
            if (header.indexOf("green") != -1){
              ledColor = 3;
            }
            if (header.indexOf("blue") != -1){
              ledColor = 4;
            }
            if (header.indexOf("yellow") != -1){
              ledColor = 5;
            }
            if (header.indexOf("magenta") != -1){
              ledColor = 6;
            }
            if (header.indexOf("cyan") != -1){
              ledColor = 7;
            }
            if (header.indexOf("LEDoff") != -1){
                ledOn = false;
                buttonOn = false;
            }
            if (header.indexOf("LEDon") != -1){
                ledOn = true;
                buttonOn = true;
            }
            // Pagina Web em HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".buttonPower { border: 2px solid #000000; border-radius: 50%; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button1 { background-color: #FFFFFF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button2 { background-color: #FF0000; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button3 { background-color: #FFFF00; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button4 { background-color: #00FF00; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button5 { background-color: #00FFFF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button6 { background-color: #0000FF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".button7 { background-color: #FF00FF; border: 2px solid #000000;; color: black; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer; }");
            client.println(".slidecontainer { width: 100%; }");
            client.println(".slider {-webkit-appearance: none;width: 100%;height: 15px;border-radius: 5px;background: #d3d3d3;outline: none;opacity: 0.7;-webkit-transition: .2s;transition: opacity .2s;}");
            client.println(".slider::-webkit-slider-thumb {-webkit-appearance: none;appearance: none;width: 25px;height: 25px;border-radius: 50%;background: #333;cursor: pointer;}");
            client.println(".slider::-moz-range-thumb {width: 25px;height: 25px;border-radius: 50%;background: #333;cursor: pointer;}");

            // Cabeçalho da pagina WEB
            client.println("</style></head>");
            client.println("<body><center><h1>Lampada Inteligente</h1><h5>by Jonath & Lucas</h5></center><br>");

            // Corpo
            // Botao de Power
            client.println("<form id=\"myForm\"><center>");
            client.println("<h3>Ligar/Desligar</h3>");
            client.println("<div class=\"btn-group\">");
            if(ledOn){
              client.println("<button class=\"buttonPower\" name=\"power\" value=\"LEDoff\" type=\"submit\">ON/OFF</button>");
            }else{
              client.println("<button class=\"buttonPower\" name=\"power\" value=\"LEDon\" type=\"submit\">ON/OFF</button>");
            }
            client.println("</div><br>");
            // Slider de Brilho
            client.println("<h3>Brilho</h3>");
            client.println("<div class=\"slidecontainer\">");
            client.print("<input name=\"brightness\" type=\"range\" min=\"10\" max=\"99\" value=\"");
            client.print(brightness);
            client.println("\" class=\"slider\" id=\"myRange\">");
            client.println("</div><br>");

            // Botoes de Cores
            client.println("<h3>Cor</h3></center>");
            client.println("<div class=\"btn-group\">");
            client.println("<button class=\"button1\" name=\"colorLED\" value=\"white\" type=\"submit\"> </button>");
            client.println("<button class=\"button2\" name=\"colorLED\" value=\"red\" type=\"submit\"> </button>");
            client.println("<button class=\"button3\" name=\"colorLED\" value=\"yellow\" type=\"submit\"> </button>");
            client.println("<button class=\"button4\" name=\"colorLED\" value=\"green\" type=\"submit\"> </button>");
            client.println("<button class=\"button5\" name=\"colorLED\" value=\"cyan\" type=\"submit\"> </button>");
            client.println("<button class=\"button6\" name=\"colorLED\" value=\"blue\" type=\"submit\"> </button>");
            client.println("<button class=\"button7\" name=\"colorLED\" value=\"magenta\" type=\"submit\"> </button>");
            client.println("</div></form>");

            // Script para o slider
            client.println("<script>");
            client.println("var slider = document.getElementById(\"myRange\"); slider.onmouseup = function() { document.getElementById(\"myForm\").submit(); }; slider.ontouchend = function() { document.getElementById(\"myForm\").submit(); };");
            client.println("</script></body></html>");
            break;
          }
          else { // caso ter uma nova linha, limpa a linha atual
            currentLine = "";
          }
        } else if (c != '\r') { // se possuir algum outro dado, retorna o caracter
          currentLine += c; // adiciona ele ao final da linha atual
        }
      }
    }
    // Limpa o cabecalho
    header = "";
    // Encerra a conexao
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }

  //monitor();
  delay(10);
}

// Regra de três para passar a porcentagem de brilho para um valor do duty cycle
void calculateDutyCycle(){
  dutyCycle = (255 * brightness) / 100;
}

// Funções responsáveis por definir as cores do led RGB
void redColor(){
  ledcWrite(redChannel, dutyCycle);
  ledcWrite(greenChannel, 0);
  ledcWrite(blueChannel, 0);
}
void blueColor(){
  ledcWrite(redChannel, 0);
  ledcWrite(greenChannel, 0);
  ledcWrite(blueChannel, dutyCycle);
}
void greenColor(){
  ledcWrite(redChannel, 0);
  ledcWrite(greenChannel, dutyCycle);
  ledcWrite(blueChannel, 0);
}
void whiteColor(){
  ledcWrite(redChannel, dutyCycle);
  ledcWrite(greenChannel, dutyCycle);
  ledcWrite(blueChannel, dutyCycle);
}
void yellowColor(){
  ledcWrite(redChannel, dutyCycle);
  ledcWrite(greenChannel, dutyCycle);
  ledcWrite(blueChannel, 0);
}
void magentaColor(){
  ledcWrite(redChannel, dutyCycle);
  ledcWrite(greenChannel, 0);
  ledcWrite(blueChannel, dutyCycle);
}
void cyanColor(){
  ledcWrite(redChannel, 0);
  ledcWrite(greenChannel, dutyCycle);
  ledcWrite(blueChannel, dutyCycle);
}
void blackOut(){
  ledcWrite(redChannel, 0);
  ledcWrite(greenChannel, 0);
  ledcWrite(blueChannel, 0);
}

// Inicia a interrupção para o temporizador
void startTimer(){
    //inicialização do timer. Parametros:
    /* 0 - seleção do timer a ser usado, de 0 a 3.
      80 - prescaler. O clock principal do ESP32 é 80MHz. Dividimos por 80 para ter 1us por tick.
    true - true para contador progressivo, false para regressivo
    */
    timer = timerBegin(0, 80, true);

    /*conecta à interrupção do timer
     - timer é a instância do hw_timer
     - endereço da função a ser chamada pelo timer
     - edge=true gera uma interrupção
    */
    timerAttachInterrupt(timer, &cb_timer, true);

    /* - o timer instanciado no inicio
       - o valor em us para 1s
       - auto-reload. true para repetir o alarme
    */
    timerAlarmWrite(timer, 1000000, true); 

    //ativa o alarme
    timerAlarmEnable(timer);
}

// Para o temporizador
void stopTimer(){
    timerEnd(timer);
    timer = NULL;
    ledOn = false;
}

// Função do temporizador
void cb_timer(){
    Serial.print("Timer: ");
    Serial.print(counter);
    Serial.println(" segundos");

    if (counter == delayTimer){
        stopTimer();
    }
    counter++;
}

// Função de leitura do sensor ultrasonico
int hcsr04(){
    digitalWrite(trig, LOW); 
    delayMicroseconds(2); 
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW); 
    return (ultrasonic.Ranging(CM)); // Converte tempo do echo em "cm"
}

// Função para trocar as cores do LED
void switchLED(){
  if(ledOn){
    switch (ledColor)
      {
      case 2:
        redColor();
        break;
      case 3:
        greenColor();
        break;
      case 4:
        blueColor();
        break;
      case 5:
        yellowColor();
        break;
      case 6:
        magentaColor();
        break;
      case 7:
        cyanColor();
        break;
      default:
        whiteColor();
        break;
      }
  }else
    blackOut();
}

// Função para a ativação do LED a partir da luminosidade e do sensor ultrasonico
void personDetection(){
  if (lightValue < 1000){ // se o quarto estiver escuro
    if(distanceValue < distanceInit){ // se tiver pessoa no quarto
      ledOn = true;
      counter = 0;
      startTimer();
    }
  }
}

// Função para monitoramento das variáveis para o ambiente de desenvolvimento
void monitor(){
  Serial.print("Luminosidade: ");
  Serial.println(lightValue);
  Serial.print("Distancia inicial: ");
  Serial.println(distanceInit);
  Serial.print("Distancia atual: ");
  Serial.println(distanceValue);
  Serial.print("Brilho: ");
  Serial.println(Sbrightness);
  delay(500);
}