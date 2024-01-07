#include <Servo.h> // Inclui a biblioteca Servo para controlar servomotores
#include <LiquidCrystal_I2C.h> // Inclui a biblioteca LiquidCrystal_I2C para controlar um LCD

Servo meuServo1; // Cria um objeto Servo chamado meuServo1
Servo meuServo2; // Cria um objeto Servo chamado meuServo2

LiquidCrystal_I2C lcd(0x27, 16, 2); // Cria um objeto lcd. O endereço 0x27 pode variar

int posicaoPrimaria = 0; // Define a posição primária do servo
int posicaoSecundaria = 90; // Define a posição secundária do servo
int tempo = 1; // Define o tempo de espera entre movimentos do servo

unsigned long tempoAnterior = 0; // Variável para guardar o tempo anterior
unsigned long intervalo = 20; // Define o intervalo de tempo

float leitura_sensor = 0; // Variável para guardar a leitura do sensor
int temperatura = 0; // Variável para guardar a temperatura
float temperaturaAnterior = 0.0; // Variável para guardar a temperatura anterior
int botaoPin = 2; // Define o pino do botão

const int servo1Pin = 9; // Define o pino do servo 1
const int servo2Pin = 8; // Define o pino do servo 2
const int piezoPin = 7; // Define o pino do piezo

bool emPosicaoSecundaria = false; // Variável booleana para verificar se o servo está na posição secundária
unsigned long tempoEspera = 0; // Variável para guardar o tempo de espera

void setup() { // Função setup, executada uma vez quando o programa começa
  meuServo1.attach(servo1Pin); // Anexa o servo 1 ao pino definido
  meuServo2.attach(servo2Pin); // Anexa o servo 2 ao pino definido
  meuServo1.write(posicaoPrimaria); // Move o servo 1 para a posição primária
  meuServo2.write(posicaoPrimaria); // Move o servo 2 para a posição primária
  meuServo1.write(posicaoSecundaria); // Move o servo 1 para a posição secundária
  meuServo2.write(posicaoSecundaria); // Move o servo 2 para a posição secundária

  Serial.begin(9600); // Inicia a comunicação serial a 9600 bps
  pinMode(A0, INPUT); // Define o pino A0 como entrada
  pinMode(13, OUTPUT); // Define o pino 13 como saída
  pinMode(4, OUTPUT); // Define o pino 4 como saída
  pinMode(10, OUTPUT); // Define o pino 10 como saída
  pinMode(5, INPUT); // Define o pino 5 como entrada
  pinMode(botaoPin, INPUT); // Define o pino do botão como entrada
  pinMode(11, OUTPUT); // Define o pino 11 como saída
  pinMode(piezoPin, OUTPUT); // Define o pino do piezo como saída

  lcd.init(); // Inicia o LCD
  lcd.backlight(); // Liga a luz de fundo do LCD
  lcd.clear(); // Limpa o LCD
  lcd.setCursor(0, 0); // Posiciona o cursor no início do LCD
  lcd.print("Sistema a Ligar"); // Imprime "Sistema a Ligar" no LCD

  digitalWrite(7, LOW); // Define o pino 7 como LOW
}

void lersensor() { // Função para ler o sensor
  leitura_sensor = analogRead(A0); // Lê o valor analógico do pino A0
  botaoPin = digitalRead(2); // Lê o valor digital do pino 2
  temperatura = map(leitura_sensor, 20, 358, -40, 125); // Mapeia a leitura do sensor para uma temperatura
}

void controloTemperatura() { // Função para controlar a temperatura
  if (botaoPin == LOW) { // Se o botão estiver pressionado
    digitalWrite(10, LOW); // Define o pino 10 como LOW

    if (temperatura >= 11) { // Se a temperatura for maior ou igual a 11
      digitalWrite(13, LOW); // Define o pino 13 como LOW
      digitalWrite(4, HIGH); // Define o pino 4 como HIGH
    } else if (temperatura <= 10) { // Se a temperatura for menor ou igual a 10
      digitalWrite(13, HIGH); // Define o pino 13 como HIGH
      digitalWrite(4, LOW); // Define o pino 4 como LOW
    }

    if (temperatura != temperaturaAnterior || temperatura == 0) { // Se a temperatura mudou ou é 0
      lcd.clear(); // Limpa o LCD
      lcd.setCursor(0, 0); // Posiciona o cursor no início do LCD
      lcd.print("Temperatura: "); // Imprime "Temperatura: " no LCD
      lcd.print(temperatura); // Imprime a temperatura no LCD
      temperaturaAnterior = temperatura; // Atualiza a temperatura anterior
    }

    lcd.setCursor(0, 1); // Posiciona o cursor na segunda linha do LCD

    if (temperatura > 11) { // Se a temperatura for maior que 11
      lcd.print("Muito Quente"); // Imprime "Muito Quente" no LCD
    } else if (temperatura < 10) { // Se a temperatura for menor que 10
      lcd.print("Muito Frio"); // Imprime "Muito Frio" no LCD
    }
  }
}

void estadodobotao() { // Função para verificar o estado do botão
  digitalWrite(10, HIGH); // Define o pino 10 como HIGH
  digitalWrite(13, HIGH); // Define o pino 13 como HIGH
  digitalWrite(4, LOW); // Define o pino 4 como LOW
  digitalWrite(11, HIGH); // Liga a lâmpada

  lcd.clear(); // Limpa o LCD
  lcd.setCursor(0, 1); // Posiciona o cursor na segunda linha do LCD
  lcd.print("Porta Aberta "); // Imprime "Porta Aberta " no LCD
  temperatura = 0; // Reseta a temperatura
  temperaturaAnterior = 0; // Reseta a temperatura anterior
  meuServo1.write(posicaoSecundaria); // Move o servo 1 para a posição secundária
  meuServo2.write(posicaoSecundaria); // Move o servo 2 para a posição secundária
}

void loop() { // Função loop, executada repetidamente
  unsigned long tempoAtual = millis(); // Obtém o tempo atual

  if (!emPosicaoSecundaria) { // Se o servo não estiver na posição secundária
    if (tempoAtual - tempoAnterior >= intervalo) { // Se o tempo atual menos o tempo anterior for maior ou igual ao intervalo
      tempoAnterior = tempoAtual; // Atualiza o tempo anterior
      int pos = meuServo1.read(); // Lê a posição do servo 1
      if (pos < posicaoSecundaria) { // Se a posição for menor que a posição secundária
        meuServo1.write(pos + tempo); // Move o servo 1 para a posição atual mais o tempo
        meuServo2.write(pos + tempo); // Move o servo 2 para a posição atual mais o tempo
      } else {
        emPosicaoSecundaria = true; // Define que o servo está na posição secundária
        tempoEspera = tempoAtual + 3000; // Define o tempo de espera para 3 segundos
      }
    }
  } 
  else {
    if (tempoAtual >= tempoEspera) { // Se o tempo atual for maior ou igual ao tempo de espera
      int pos = meuServo1.read(); // Lê a posição do servo 1
      if (pos > posicaoPrimaria) { // Se a posição for maior que a posição primária
        meuServo1.write(pos - tempo); // Move o servo 1 para a posição atual menos o tempo
        meuServo2.write(pos - tempo); // Move o servo 2 para a posição atual menos o tempo
      } 
      else {
        emPosicaoSecundaria = false; // Define que o servo não está na posição secundária
        tempoEspera = tempoAtual + 3000; // Define o tempo de espera para 3 segundos
      }
    }

    lersensor(); // Chama a função para ler o sensor
    controloTemperatura(); // Chama a função para controlar a temperatura


    if(digitalRead(2) == HIGH) { // Se o botão estiver pressionado
  estadodobotao(); // Chama a função para verificar o estado do botão
  unsigned long inicioEspera = millis(); // Obtém o tempo atual
  while (millis() - inicioEspera < 5000) { // Enquanto o tempo atual menos o tempo de início da espera for menor que 5000 ms (5 segundos)
    if (digitalRead(2) == LOW) { // Se o botão não estiver pressionado
      break; // Sai do loop
    }
  }
  if (millis() - inicioEspera >= 5000) { // Se o tempo atual menos o tempo de início da espera for maior ou igual a 5000 ms (5 segundos)
    tone(piezoPin, 1000); // Toca um tom no piezo
  }
}
else { // Se o botão não estiver pressionado
  digitalWrite(2, LOW); // Define o pino 2 como LOW
  digitalWrite(11, LOW); // Define o pino 11 como LOW
  noTone(piezoPin); // Para o piezo
}
  }
}










