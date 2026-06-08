/*
 * =====================================================
 *  SISTEMA IoT — MONITORAMENTO DE CÁPSULA ESPACIAL
 *  FIAP | Global Solution 2026 | 1º Semestre
 *  Disciplina: Sistemas Embarcados / IoT
 * =====================================================
 *
 *  COMPONENTES:
 *    DHT22  → Temperatura interna     (Pino D2)
 *    LDR    → Luminosidade            (Pino A0)
 *    SW-420 → Vibração / Impacto      (Pino D3)
 *    LCD    → Display 16x2 I2C        (SDA=A4, SCL=A5)
 *    LED R  → Alerta temperatura      (Pino D8)
 *    LED A  → Alerta vibração         (Pino D9)
 *    Buzzer → Alarme sonoro           (Pino D10)
 *
 *  BIBLIOTECAS NECESSÁRIAS (instalar via Library Manager):
 *    - DHT sensor library (Adafruit)
 *    - Adafruit Unified Sensor
 *    - LiquidCrystal I2C (Frank de Brabander)
 *
 *  COMO USAR NO WOKWI:
 *    1. Acesse wokwi.com → New Project → Arduino Uno
 *    2. Cole este código no editor
 *    3. Adicione os componentes conforme esquema do relatório
 *    4. Clique em "Start Simulation"
 * =====================================================
 */

#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ── Pinos ───────────────────────────────────────────
#define DHTPIN        2
#define DHTTYPE       DHT22
#define LDR_PIN       A0
#define VIB_PIN       3
#define LED_TEMP      8
#define LED_VIB       9
#define BUZZER_PIN    10

// ── Limiares de Segurança ───────────────────────────
#define TEMP_MAX      35.0   // °C — acima → alerta de calor
#define TEMP_MIN      15.0   // °C — abaixo → alerta de frio
#define LUZ_BAIXA     200    // 0–1023 — abaixo → módulo escuro
#define LUZ_MEDIA     600    // 0–1023 — abaixo → penumbra
#define VIB_DEBOUNCE  500    // ms — janela anti-ruído de vibração
#define VIB_RESET     2000   // ms — tempo para resetar alerta de vibração
#define CICLO_LCD     2000   // ms — intervalo de alternância de telas
#define CICLO_LEITURA 500    // ms — intervalo de leitura dos sensores

// ── Objetos ─────────────────────────────────────────
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ── Variáveis de Estado ─────────────────────────────
float        temperatura  = 22.0;
int          luminosidade = 512;
bool         vibracao     = false;
unsigned long ultimaVib   = 0;
unsigned long ultimaTela  = 0;
unsigned long ultimaLeitura = 0;
int          telaAtual    = 0;   // 0=Temp  1=Luz  2=Vibração
int          cicloTotal   = 0;   // contador de ciclos para log serial

// ── Caractere personalizado: símbolo de satélite ───
byte iconSat[8] = {
  0b00100,
  0b01110,
  0b10101,
  0b01110,
  0b00100,
  0b01010,
  0b10001,
  0b00000
};

// ══════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);

  // Configuração de pinos
  pinMode(LED_TEMP,   OUTPUT);
  pinMode(LED_VIB,    OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIB_PIN,    INPUT);

  // Garante atuadores desligados no inicio
  digitalWrite(LED_TEMP,   LOW);
  digitalWrite(LED_VIB,    LOW);
  noTone(BUZZER_PIN);

  // Inicialização do Display LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, iconSat);

  // Tela de splash
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(0);
  lcd.print(" CAPSULA FIAP ");
  lcd.write(0);
  lcd.setCursor(0, 1);
  lcd.print("  Iniciando...  ");
  delay(2000);
  lcd.clear();

  // Inicialização do sensor DHT22
  dht.begin();
  delay(500); // aguarda sensor estabilizar

  // Cabeçalho do log serial
  Serial.println(F("=============================================="));
  Serial.println(F(" SISTEMA IoT — MONITORAMENTO CAPSULA ESPACIAL"));
  Serial.println(F(" FIAP | Global Solution 2026"));
  Serial.println(F("=============================================="));
  Serial.println(F("Ciclo | Temp(C) | Lux(0-1023) | Vib | Status"));
  Serial.println(F("----------------------------------------------"));
}

// ══════════════════════════════════════════════════
void loop() {
  unsigned long agora = millis();

  // Leitura dos sensores a cada CICLO_LEITURA ms
  if (agora - ultimaLeitura >= CICLO_LEITURA) {
    ultimaLeitura = agora;
    lerSensores();
    avaliarAlertas();
    enviarSerial();
    cicloTotal++;
  }

  // Atualização do display no intervalo próprio
  exibirLCD();
}

// ── Leitura dos Sensores ────────────────────────────
void lerSensores() {
  // Temperatura — DHT22
  float t = dht.readTemperature();
  if (!isnan(t)) {
    temperatura = t;
  }
  // Em caso de falha de leitura (NaN), mantém o ultimo valor válido

  // Luminosidade — LDR (leitura analógica 0–1023)
  luminosidade = analogRead(LDR_PIN);

  // Vibração — SW-420 com lógica de debounce
  if (digitalRead(VIB_PIN) == HIGH) {
    unsigned long agora = millis();
    if (agora - ultimaVib > VIB_DEBOUNCE) {
      vibracao  = true;
      ultimaVib = agora;
    }
  } else {
    // Auto-reset após VIB_RESET ms sem novo sinal
    if (millis() - ultimaVib > VIB_RESET) {
      vibracao = false;
    }
  }
}

// ── Avaliação de Alertas e Controle de Atuadores ───
void avaliarAlertas() {
  bool alertaTemp = (temperatura > TEMP_MAX || temperatura < TEMP_MIN);
  bool alertaVib  = vibracao;

  // LEDs indicadores
  digitalWrite(LED_TEMP, alertaTemp ? HIGH : LOW);
  digitalWrite(LED_VIB,  alertaVib  ? HIGH : LOW);

  // Buzzer com frequência proporcional à gravidade
  if (alertaTemp && alertaVib) {
    // EMERGÊNCIA CRÍTICA — dois alertas simultâneos
    tone(BUZZER_PIN, 1000, 200);
  } else if (alertaTemp) {
    // Alerta de temperatura isolado
    tone(BUZZER_PIN, 600, 150);
  } else if (alertaVib) {
    // Alerta de vibração isolado
    tone(BUZZER_PIN, 400, 100);
  } else {
    // Sistema nominal — sem alarme
    noTone(BUZZER_PIN);
  }
}

// ── Exibição no LCD (alterna 3 telas a cada 2s) ────
void exibirLCD() {
  unsigned long agora = millis();
  if (agora - ultimaTela < CICLO_LCD) return;
  ultimaTela = agora;

  lcd.clear();

  switch (telaAtual) {

    case 0: // ── TELA 1: Temperatura ─────────────────
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperatura, 1);
      lcd.print((char)223); // ° grau
      lcd.print("C");

      lcd.setCursor(0, 1);
      if (temperatura > TEMP_MAX)
        lcd.print("!! CALOR CRITICO");
      else if (temperatura < TEMP_MIN)
        lcd.print("!! FRIO CRITICO ");
      else
        lcd.print("Status: NORMAL  ");
      break;

    case 1: // ── TELA 2: Luminosidade ─────────────────
      lcd.setCursor(0, 0);
      lcd.print("Luz:");
      lcd.print(luminosidade);
      lcd.print("/1023   ");

      lcd.setCursor(0, 1);
      if (luminosidade < LUZ_BAIXA)
        lcd.print("Modulo: ESCURO  ");
      else if (luminosidade < LUZ_MEDIA)
        lcd.print("Modulo: PENUMBRA");
      else
        lcd.print("Modulo: ILUMINDO");
      break;

    case 2: // ── TELA 3: Vibração ─────────────────────
      lcd.setCursor(0, 0);
      lcd.print("Vibracao/Impacto");
      lcd.setCursor(0, 1);
      lcd.print(vibracao ? "!! IMPACTO/FALHA" : "Status: ESTAVEL ");
      break;
  }

  telaAtual = (telaAtual + 1) % 3;
}

// ── Log Serial (telemetria para monitoramento) ──────
void enviarSerial() {
  // Formato: Ciclo | Temp | Luz | Vibração | Status
  Serial.print(cicloTotal);
  Serial.print(F("     | "));
  Serial.print(temperatura, 1);
  Serial.print(F("    | "));
  Serial.print(luminosidade);
  Serial.print(F("         | "));
  Serial.print(vibracao ? F("SIM") : F("NAO"));
  Serial.print(F(" | "));

  // Determina status geral do sistema
  bool alertaTemp = (temperatura > TEMP_MAX || temperatura < TEMP_MIN);
  if (alertaTemp && vibracao)
    Serial.println(F("[EMERGENCIA] TEMPERATURA + VIBRACAO"));
  else if (temperatura > TEMP_MAX)
    Serial.println(F("[ALERTA] TEMPERATURA ALTA"));
  else if (temperatura < TEMP_MIN)
    Serial.println(F("[ALERTA] TEMPERATURA BAIXA"));
  else if (vibracao)
    Serial.println(F("[ALERTA] VIBRACAO DETECTADA"));
  else
    Serial.println(F("[OK] SISTEMA NOMINAL"));
}
