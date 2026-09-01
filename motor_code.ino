/*
  Controle de Motor DC via MOSFET - Arduino Nano
  Baseado no esquema fornecido (SCH.png)
  Versao completa - sem sensor Hall (nao conectado)

  Sensores:
   - DS18B20 (Temperatura)    -> D3
   - INA219 (Corrente/Tensão) -> I2C (A4=SDA / A5=SCL)
     (requer resistores de pull-up de 4,7k em SDA e SCL para 5V,
      caso o modulo nao tenha pull-up proprio)

  Indicadores:
   - LED D11 -> acende quando o motor esta RODANDO
   - LED D10 -> acende quando o motor esta PARADO
                (pisca rapido se parado por falha de seguranca)

  Atuador:
   - Motor DC via MOSFET (Q1) -> D9 (PWM no gate, R1=220R série, R2=10k pull-down)

  Bibliotecas necessárias (Gerenciador de Bibliotecas):
   - OneWire
   - DallasTemperature
   - Adafruit INA219 (+ Adafruit BusIO)

  Comandos via Serial (9600 baud):
   - Vxxx  -> define velocidade (0-255), ex: V150
   - S     -> para o motor
   - R     -> reseta uma falha de seguranca (temperatura ou corrente)
*/

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_INA219.h>

// ---------- Pinos ----------
#define PIN_ONEWIRE   3   // DS18B20
#define PIN_MOTOR     9   // Gate do MOSFET (PWM)
#define PIN_LED_RUN   11  // LED - motor rodando
#define PIN_LED_STOP  10  // LED - motor parado

// ---------- Parâmetros de segurança ----------
const float TEMP_MAX = 70.0;        // °C - desliga motor acima disso
const float CORRENTE_MAX_A = 3.0;   // A  - desliga motor acima disso (ajuste conforme o motor/fusivel novos)
const unsigned long INTERVALO_LEITURA = 1000; // ms

// ---------- Objetos ----------
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature sensores(&oneWire);
Adafruit_INA219 ina219;

unsigned long ultimaLeitura = 0;

// ---------- Controle do motor ----------
int velocidadeMotor = 0;     // 0-255 (duty cycle do PWM)
bool motorHabilitado = true; // false quando trava por segurança
bool ina219OK = false;       // true se o INA219 foi encontrado no setup

// ---------- Piscada do LED de falha ----------
unsigned long ultimoPiscar = 0;
bool estadoPiscaLed = false;

void setup() {
  Serial.begin(9600);

  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_LED_RUN, OUTPUT);
  pinMode(PIN_LED_STOP, OUTPUT);

  analogWrite(PIN_MOTOR, 0); // motor desligado no início

  Wire.begin();
  sensores.begin();

  ina219OK = ina219.begin();
  if (!ina219OK) {
    Serial.println(F("Falha ao encontrar o INA219! Seguindo sem leitura de corrente/tensao."));
  }

  Serial.println(F("Sistema iniciado."));
  Serial.println(F("Comandos: Vxxx (velocidade 0-255) | S (parar) | R (reset de falha)"));
}

void loop() {
  lerComandoSerial();

  unsigned long agora = millis();
  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = agora;

    // --- Temperatura (DS18B20) ---
    sensores.requestTemperatures();
    float temperatura = sensores.getTempCByIndex(0);

    // --- Corrente / Tensão (INA219, só se detectado) ---
    float tensao = 0;
    float corrente_mA = 0;
    float potencia_mW = 0;
    if (ina219OK) {
      tensao = ina219.getBusVoltage_V();
      corrente_mA = ina219.getCurrent_mA();
      potencia_mW = ina219.getPower_mW();
    }

    // --- Lógica de segurança ---
    bool faltaCorrente = ina219OK && ((corrente_mA / 1000.0) > CORRENTE_MAX_A);
    if (temperatura > TEMP_MAX || faltaCorrente) {
      motorHabilitado = false;
      Serial.println(F("!! FALHA: motor desligado por seguranca !!"));
    }

    // --- Log ---
    Serial.print(F("Temp: ")); Serial.print(temperatura); Serial.print(F(" C"));
    if (ina219OK) {
      Serial.print(F(" | Tensao: ")); Serial.print(tensao); Serial.print(F(" V"));
      Serial.print(F(" | Corrente: ")); Serial.print(corrente_mA); Serial.print(F(" mA"));
      Serial.print(F(" | Potencia: ")); Serial.print(potencia_mW); Serial.print(F(" mW"));
    } else {
      Serial.print(F(" | INA219 nao detectado"));
    }
    Serial.print(F(" | PWM Motor: ")); Serial.println(velocidadeMotor);
  }

  bool motorRodando = motorHabilitado && velocidadeMotor > 0;
  bool emFalha = !motorHabilitado;

  // --- LED de motor rodando ---
  digitalWrite(PIN_LED_RUN, motorRodando ? HIGH : LOW);

  // --- LED de motor parado (pisca rapido se for falha de segurança) ---
  if (!motorRodando) {
    if (emFalha) {
      // pisca rapido (a cada 150ms) para indicar falha
      if (millis() - ultimoPiscar >= 150) {
        ultimoPiscar = millis();
        estadoPiscaLed = !estadoPiscaLed;
        digitalWrite(PIN_LED_STOP, estadoPiscaLed ? HIGH : LOW);
      }
    } else {
      digitalWrite(PIN_LED_STOP, HIGH); // parado normal - aceso fixo
    }
  } else {
    digitalWrite(PIN_LED_STOP, LOW);
  }

  // --- Aplica PWM no motor (via gate do MOSFET) ---
  analogWrite(PIN_MOTOR, motorHabilitado ? velocidadeMotor : 0);
}

void lerComandoSerial() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 'V' || c == 'v') {
      int v = Serial.parseInt();
      v = constrain(v, 0, 255);
      velocidadeMotor = v;
      Serial.print(F("Nova velocidade: ")); Serial.println(velocidadeMotor);
    } else if (c == 'R' || c == 'r') {
      motorHabilitado = true;
      Serial.println(F("Motor reabilitado (reset de falha)."));
    } else if (c == 'S' || c == 's') {
      velocidadeMotor = 0;
      Serial.println(F("Motor parado."));
    }
  }
}
