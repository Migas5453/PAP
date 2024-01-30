#include <Servo.h>
#include <dht.h>
#include <LiquidCrystal.h>

Servo ServoPort;
Servo ServoPrat;

const int PosHoPart = 58;    //Posicao horizontal da prateleira
const int PosDePart = 90;    //Posicao deitada da prateleira
const int PosOpenPort = 0;    //Posicao horizontal da prateleira
const int PosClosePort = 90;    //Posicao deitada da prateleira
const int RefTemp = 13;      //Temperatura referencia

dht DHT;

const int PSpir = A1;        //Pino do spir
const int PDht11 = A0;       //Pino do dht11
const int PinServoPrat = 8;  //Pino do servo da prateleira
const int PinServoPort = 9;  //Pino do servo da porta
const int PBotao = 2;        //Pino do botao

const int PLuz = 11;         //Pino da luz de presenca
const int PLuzPorta = 10;    //Pino da luz da porta aberta
const int PArref = 4;        //Pino do arref (led verde)
const int PBuz = 7;          //Pino do buzzer

int VSpir = 500;        //Variavel do spir a 0


bool VBuz = 0;         //Variavel bo buzzer a 0
int VTemp = 0;         //Variavel da temp a 0
int VHum = 0;          //Variavel da hum a 0
bool VLuz = 0;         //Variavel da luz de presenca a 0
bool VLuzPorta = 0;    //Variavel da luz da porta aberta a 0
bool VArref = 0;       //Variavel do arref (led verde) a 0
int VServoPort = PosOpenPort;    //Variavel do servo da porta a 0
int VServoPrat = PosHoPart;    //Variavel do servo da prateleira a PosHoPart
int Vtone = 0;         //Variavel da frequencia a 0

LiquidCrystal lcd(12, 6, 5, 3, A2, A4);  //inciar lcd

float leitura_sensor = 0;     // le sensor


bool VBotao = false;       //Variavel do botao para abrir e fechar porta
bool VestadoPorta = true;  // estado da porta True == Aberta

unsigned long momentodeClique = 0;     //momento do clique


long tempoDeEspera = 10000;    //tempo de espera apos clicar no botao
unsigned long tempoAnterior = 0;    //momento da ultima mudanca do servo
long tempoDeTroca = 3000;     //tempo para trocar de posicao
long lcdUpdateTime = 0;     //ultima atualizacao do lcd
long tempoDeAviso = 0;    //aviso do buzzer
unsigned long ultimaDetecaoMovimento = 0;   //ultima detecao de movimento
const unsigned long tempoDesligarLCD = 5000; // tempo para o lcd desligar se nao houver movimento

void setup()
{
  Serial.begin(9600);  
  ServoPort.attach(PinServoPort);   //define a porta ao pino
  ServoPrat.attach(PinServoPrat);   //define a porta ao pino
  pinMode(PBotao, INPUT);     //   define o botao como entrada
  pinMode(PSpir, INPUT);    //define o sensor de movimento como entrada
  pinMode(PDht11, INPUT);   //define o dht11 como entrada
  pinMode(PLuz, OUTPUT);   //define a luz de presenca como saida
  pinMode(PLuzPorta, OUTPUT);   //define a luz da porta como saida
  pinMode(PBuz, OUTPUT);    //define o buzzer como saida
  pinMode(PArref, OUTPUT);    //define a luz de arref (led verde) como saida
  lcd.begin(16, 2);
  lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Tem:");
      lcd.print(VTemp);
      lcd.print("C Hum:");
      lcd.print(VHum);
      lcd.print("% ");
      lcd.setCursor(0, 1);

      ServoPrat.write(PosDePart);    // Atualiza a posição do servo da prateleira      
      lcd.print("Coloque Garrafas");
      delay(3000); 
}

// ---------------------------------------<<<< Início Loop <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
void loop()
{

  lerEntradas();
  
  // Ver se alguém carregou para abrir ou fechar porta <<<<<<<<<<<<<<<<<<<<<<<<<<
  if (VBotao == HIGH && !VestadoPorta) // Abrir Porta <<<<<<<<<<<<<<<<<
  {
    momentodeClique = millis();   //regista o momento do clique do botao
    changePortaStatus();
    VLuz = 1; // Luz de ver vinho       
    VLuzPorta = 1; // Luz que indica porta aberta
  }
  else if (VBotao == HIGH && VestadoPorta) // Fechar Porta <<<<<<<<<<<<<<<<<
  {
    changePortaStatus();
    VLuz = 0; // Luz de ver vinho       
    VLuzPorta = 0; // Luz que indica porta Fechada
    delay(200);
  }

  if (VestadoPorta && ((millis()- momentodeClique) > tempoDeEspera)) // Porta aberta há muito tempo
  {
    VBuz=1;
    Vtone = 440;   //frequencia do buzzer
    if (VBotao)
    {
      Vtone = 0;   //desliga o buzzer
      tempoDeAviso = 0;    //reinicia o momento do aviso
    }
  }
  
  // --------------------------------

  if (!VestadoPorta && ((millis() - tempoAnterior) >= tempoDeTroca)) // Rodar garrafa a um certo tempo
  {
    changeSecondServoPos(); // Se a porta está fechada;
    tempoAnterior = millis();   //atualiza o momento da ultima troca
  }
  if (!VestadoPorta)
  {
    controloTemperatura();
  }

  // ------------------------------------------
  atualizarLCD();    //atualiza o lcd
  atualizarSaidas();
  
}
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^Fim de loop ---------------------<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void controloTemperatura()
{
  int chk = DHT.read11(PDht11);  //reinicia a leitura do dht11
  VTemp = DHT.temperature;     //atualiza a temp
  VHum = DHT.humidity;        //atualiza a hum

  if (VTemp > RefTemp)
  {
    VArref = HIGH;    //ativa o arref (led verde)
  }
  else if (VTemp < RefTemp)
  {
    VArref = LOW;     //desativa o arred (led verde)
  }
}

void atualizarLCD()
{
      if (VSpir > 430)
    { 
      lcd.display();
      lcd.setCursor(0, 0);
      lcd.print("Tem:");
      lcd.print(VTemp);
      lcd.print("C Hum:");
      lcd.print(VHum);
      lcd.print("% ");
;
      }
      
      else if (VSpir <= 100) // Seninguém estiver à frente
      {
      lcd.noDisplay();
      lcd.clear();

      // lcd.setCursor(0, 0);
      // lcd.print("VPir: "); // Teste de Pir
      // lcd.print(VSpir);// Teste de Pir
      // delay(1000);
        }

}

void lerEntradas()
{
  VSpir = analogRead(PSpir);   //le o estado do sensor de movimento
  VBotao = digitalRead(PBotao);  //le o estado do botao

} 

void atualizarSaidas()
{
  digitalWrite(PLuz, VLuz);   // Atualiza o estado do pino da luz principal
  digitalWrite(PLuzPorta, VLuzPorta);    // Atualiza o estado do pino da luz da porta
  digitalWrite(PArref, VArref);   // Atualiza o estado do pino do arref (led verde)
  digitalWrite(PBuz, VBuz);    // Atualiza o estado do pino do buzzer
  ServoPort.write(VServoPort);    // Atualiza a posição do servo da porta
  ServoPrat.write(VServoPrat);    // Atualiza a posição do servo da prateleira
  if (VBuz == 1)
    tone(PBuz, Vtone);    //ativa o buzzer
  else
    noTone(PBuz);   //desativa o buzzer
}


void changePortaStatus()
{
  
  if (VestadoPorta )
  {
    VestadoPorta = false;  //porta fica fechada
    VServoPort = PosClosePort;  //servo da porta destrancado
 
    VServoPrat = PosDePart;  
    // lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Porta Fechada :)");
    delay(1000);
    VLuz = 0;  // Apago luz de ver o vinho
    VLuzPorta = 0;   // Apago luz da porta 
    VBuz = 0;

  }
  else
  {
    VestadoPorta = true;  //porta fica Aberta
    //VServoPort = PosOpenPort;
    VServoPort = PosHoPart; 
    // lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("Porta Aberta! ");
    delay(1000);
    VLuz = 1; // Ligo de ver o vinho 
    VLuzPorta = 1; // Ligo luz da porta 

  }
}

void changeSecondServoPos()
{
  if (VServoPrat == PosHoPart)
  {
    VServoPrat = PosDePart;
  }
  else
  {
    VServoPrat = PosHoPart;
  }
}
