#include <Servo.h>
#include <LiquidCrystal.h> // Inclui a biblioteca LiquidCrystal_I2C para controlar um LCD

Servo ServoPort; // Cria um objeto servo para o primeiro servo (pino 9)
Servo ServoPrat; // Cria um objeto servo para o segundo servo (pino 8)

// Constantes
const int PosHoPart = 90;  // Posição Horiz da prateleira
const int PosDePart = 58; // Posição Deitada da prateleira
const int RefTemp = 11;   // temperatura referencia a manter (11ºC)
// ---------

// Entradas
const int PSpir = 13;
const int PDht11 = A0;
const int PFDCPorta = 1;
const int PinServoPrat = 8;
const int PinServoPort = 9;
const int PBotao = 2;
// ---------

// Saidas
const int PLuz = 11;
const int PLuzPorta = 10;
const int PArref = 4;
const int PBuz = 7;
// ---------

// Variaveis
bool VSpir = 0; // se for igual a 1 ha movimento
bool VBotao = 0;
bool VBuz = 0;
int VTemp = 0; // Variável para guardar a temperatura
bool VLuz = 0;
bool VLuzPorta = 0;
bool VArref = 0;
bool VFDCPorta = 0;
int VServoPort = 0;
int VServoPrat = 0;
int Vtone = 0;
String VLcd1 = "";
String VLcd2 = "";
// ---------

LiquidCrystal lcd(12, 6, 5, 3, A2, A4); // Cria um objeto lcd. O endereço 0x27 pode variar
// Variaveis de Controlo
float leitura_sensor = 0; // Variável para guardar a leitura do sensor
bool estadoPorta = false; // Estado atual do primeiro servo
unsigned long momentodeClique = 0;
bool botaoPressionado = false;
long tempoDeEspera = 4000;
unsigned long tempoAnterior = 0;
long tempoDeTroca = 4000; // Tempo de troca de posição do segundo servo
long lcdUpdateTime = 0;
long tempoDeAviso = 0;
// ---------

void setup()
{
  Serial.begin(9600);
  ServoPort.attach(PinServoPort); // O primeiro servo está conectado ao pino digital 9
  ServoPrat.attach(PinServoPrat); // O segundo servo está conectado ao pino digital 8
  pinMode(PBotao, INPUT);         // Configura o pino do botão como entrada
  pinMode(PSpir, INPUT);
  pinMode(PDht11, INPUT);
  pinMode(PLuz, OUTPUT);
  pinMode(PLuzPorta, OUTPUT);
  pinMode(PBuz, OUTPUT);
  pinMode(PArref, OUTPUT);
  lcd.begin(16, 2);
  lcd.clear(); // Limpa o LCD
 
}

// ------------------------------------------------------- LOOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void loop()
{
  lerEntradas();
  if (VBotao == HIGH && !botaoPressionado)
  {
    momentodeClique = millis();
    botaoPressionado = true;
  }
  if (botaoPressionado && momentodeClique + tempoDeEspera <= millis())
  {
    changePortaStatus(); // Função para alterar o estado da porta
    momentodeClique = 0;
    botaoPressionado = false;
  }
  // Verifica se passaram 10 segundos desde a última troca de posição do segundo servo
  if (!estadoPorta && millis() - tempoAnterior >= tempoDeTroca)
  {
    changeSecondServoPos();   // Troca a posição do segundo servo
    tempoAnterior = millis(); // Atualiza o tempo da última troca
  }
  if (!estadoPorta)
  {
    controloTemperatura();
  }
  if (estadoPorta && tempoDeAviso != 0 && tempoDeAviso + 10000 <= millis())
  {
    Vtone = 440;
    if (VBotao)
    {
      Vtone = 0;
      tempoDeAviso = 0;
    }
  }
  atualizarLCD();
  atualizarSaidas();
}
// ------------------------------------------------------- Fim de LOOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void controloTemperatura()
{
  VLcd2 = "";
  VLcd1 = "Temperatura: "; // Imprime "Temperatura: " no LCD
  VLcd1 += VTemp;          // Imprime a temperatura no LCD
  if (VTemp > RefTemp)
  {                         // Se a temperatura for maior q 11
    VArref = HIGH;          // Define o pino PArref como HIGH
    VLcd2 = "Muito Quente"; // Imprime "Muito Quente" no LCD
  }
  else if (VTemp < RefTemp)
  {                       // Se a temperatura for menor ou igual a 10
    VArref = LOW;         // Define o pino PArref como LOW
    VLcd2 = "Muito Frio"; // Imprime "Muito Frio" no LCD
  }
}

void atualizarLCD()
{
  if (lcdUpdateTime + 500 < millis())
  {
    lcd.clear();         // Limpa o LCD
    lcd.setCursor(0, 0); // Posiciona o cursor na segunda linha do LCD
    lcd.print(VLcd1);
    lcd.setCursor(0, 1); // Posiciona o cursor na segunda linha do LCD
    lcd.print(VLcd2);
    lcdUpdateTime = millis();
  }
}

void lerEntradas()
{
  VFDCPorta = digitalRead(PFDCPorta);
  VSpir = digitalRead(PSpir);
  VBotao = digitalRead(PBotao);
  VTemp = 15;
  VServoPort = ServoPort.read();
  VServoPrat = ServoPrat.read();
}

void atualizarSaidas()
{
  LCDpoupanca();
  digitalWrite(PSpir, VSpir);
  digitalWrite(PLuz, VLuz);
  digitalWrite(PLuzPorta, VLuzPorta);
  digitalWrite(PArref, VArref);
  digitalWrite(PBuz, VBuz);
  ServoPort.write(VServoPort);
  ServoPrat.write(VServoPrat);
  if (Vtone != 0)
    tone(PBuz, Vtone);
  else
    noTone(PBuz);
}

void LCDpoupanca()
{
  if (VSpir) {lcd.display();return;}
  lcd.noDisplay();
}

void changePortaStatus()
{
  if (!estadoPorta)
  {
    estadoPorta = true; // Atualiza o estado para Porta Aberta
    VServoPort = 0;
    VLuz = 1;
    VServoPrat = PosHoPart;
    VLcd1 = "Porta Aberta"; // Imprime "Porta Aberta " no LCD
    VLcd2 = "";
    VLuzPorta = 1;
    VArref = 0;
    tempoDeAviso = millis();
  }
  else
  {
    VServoPort = 90;     // Move o primeiro servo de volta para 90 graus (porta trancada)
    estadoPorta = false; // Atualiza o estado para Porta Fechada
    controloTemperatura();
    VLuz = 0;
    VLuzPorta = 0;
    VBuz = 0; // Para o piezo
    VTemp = 0;
  }
}

void changeSecondServoPos()
{
  if (VServoPrat == PosHoPart)
  {
    VServoPrat = PosDePart; // Move o segundo servo para 90 graus
  }
  else
  {
    VServoPrat = PosHoPart; // Move o segundo servo de volta para 0 graus
  }
}