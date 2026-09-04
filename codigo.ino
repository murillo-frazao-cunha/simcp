#include "WiFiEsp.h"

// ======================================================================
// ========================= CONFIGURAÇÕES ==============================
// ======================================================================

// --- Pinos dos Sensores Ultrassônicos ---
#define TRIG_ENTRADA 53
#define ECHO_ENTRADA 51
#define TRIG_SAIDA   49
#define ECHO_SAIDA   47

// --- Pinos dos LEDs ---
#define VERMELHO 4
#define VERDE    6
#define AZUL     7

// --- Identificação do Ônibus ---
char ONIBUS[] = "I02";

// --- Configurações da Rede ---
char ssid[] = "nome_da_rede";
char pass[] = "senha_da_rede";

int status = WL_IDLE_STATUS;
WiFiEspClient client;

// --- Servidor da API ---
const char server[] = "123.456.789.0";
const int port = 80;

// --- Variável de Contagem ---
int pessoas = 0;


// ======================================================================
// =========================== PROTÓTIPOS ===============================
// ======================================================================

void enviarDadosParaAPI(int contagemPessoas);
void printWifiStatus();
void tentarConectarWiFi();
float medirDistancia(int trigPin, int echoPin);
void updateLed(int pessoas);
void piscarLed();


// ======================================================================
// ============================== SETUP =================================
// ======================================================================

void setup() {
  // Inicializa comunicação serial
  Serial.begin(115200);

  // No Arduino Mega, o módulo WiFi utiliza a Serial1
  Serial1.begin(115200);

  // --------------------------------------------------------------------
  // Sensores ultrassônicos
  // --------------------------------------------------------------------

  pinMode(TRIG_ENTRADA, OUTPUT);
  pinMode(ECHO_ENTRADA, INPUT);

  pinMode(TRIG_SAIDA, OUTPUT);
  pinMode(ECHO_SAIDA, INPUT);

  // --------------------------------------------------------------------
  // LEDs
  // --------------------------------------------------------------------

  pinMode(VERMELHO, OUTPUT);
  pinMode(VERDE, OUTPUT);
  pinMode(AZUL, OUTPUT);

  // --------------------------------------------------------------------
  // Inicialização do WiFi
  // --------------------------------------------------------------------

  WiFi.init(&Serial1);

  // Caso o módulo WiFi não seja encontrado
  if (WiFi.status() == WL_NO_SHIELD) {
    // LED roxo = vermelho + azul
    digitalWrite(VERMELHO, HIGH);
    digitalWrite(AZUL, HIGH);

    Serial.println("Modulo WiFi nao encontrado!");

    // Trava o programa
    while (true);
  }

  Serial.println("Setup concluido. Tentando conectar ao WiFi...");
}


// ======================================================================
// =============================== LOOP =================================
// ======================================================================

void loop() {
  bool detected = false;

  // --------------------------------------------------------------------
  // Verifica conexão com o WiFi
  // --------------------------------------------------------------------

  if (WiFi.status() != WL_CONNECTED) {
    // Azul + Verde = turquesa
    digitalWrite(AZUL, HIGH);
    digitalWrite(VERDE, HIGH);

    tentarConectarWiFi();

    delay(2000);
    return;
  }

  // Atualiza o LED conforme a quantidade de pessoas
  updateLed(pessoas);

  // --------------------------------------------------------------------
  // Leitura dos sensores ultrassônicos
  // --------------------------------------------------------------------

  float distanciaEntrada = medirDistancia(
    TRIG_ENTRADA,
    ECHO_ENTRADA
  );

  float distanciaSaida = medirDistancia(
    TRIG_SAIDA,
    ECHO_SAIDA
  );

  Serial.print("Entrada: ");
  Serial.print(distanciaEntrada);

  Serial.print(" cm | Saida: ");
  Serial.print(distanciaSaida);

  Serial.println(" cm");


  // --------------------------------------------------------------------
  // Detecção de entrada
  // --------------------------------------------------------------------

  if (distanciaEntrada <= 6.7) {
    pessoas++;

    Serial.print("Entrada detectada! Pessoas: ");
    Serial.println(pessoas);

    piscarLed();
    enviarDadosParaAPI(pessoas);
    updateLed(pessoas);

    detected = true;
  }


  // --------------------------------------------------------------------
  // Detecção de saída
  // --------------------------------------------------------------------

  if (distanciaSaida <= 6.7) {
    if (pessoas > 0) {
      pessoas--;
    }

    Serial.print("Saida detectada! Pessoas: ");
    Serial.println(pessoas);

    piscarLed();
    enviarDadosParaAPI(pessoas);
    updateLed(pessoas);

    detected = true;
  }


  // --------------------------------------------------------------------
  // Delay após uma detecção
  // --------------------------------------------------------------------

  if (detected) {
    delay(2000);
  } else {
    delay(200);
  }
}


// ======================================================================
// =========================== LED PISCANDO =============================
// ======================================================================

void piscarLed() {
  digitalWrite(VERMELHO, LOW);
  digitalWrite(AZUL, LOW);
  digitalWrite(VERDE, LOW);

  delay(500);

  updateLed(pessoas);
}


// ======================================================================
// ======================= MEDIR DISTÂNCIA ==============================
// ======================================================================

float medirDistancia(int trigPin, int echoPin) {
  // Garante que o Trigger comece em LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Envia pulso ultrassônico
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Aguarda o retorno do sinal
  // Timeout de 25 ms (~4 metros)
  long duracao = pulseIn(
    echoPin,
    HIGH,
    25000
  );

  // Converte o tempo em distância
  float distancia = duracao * 0.034 / 2.0;

  // Limita a distância máxima a 400 cm
  if (distancia == 0 || distancia > 400) {
    distancia = 400;
  }

  return distancia;
}


// ======================================================================
// ======================= CONEXÃO COM WI-FI ============================
// ======================================================================

void tentarConectarWiFi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);

  if (status == WL_CONNECTED) {
    Serial.println("Conectado ao WiFi!");

    printWifiStatus();

    // Envia a quantidade atual de pessoas
    enviarDadosParaAPI(pessoas);
  } else {
    Serial.println("Falha ao conectar. Tentando novamente...");

    delay(5000);
  }
}


// ======================================================================
// ========================= ENVIO PARA API =============================
// ======================================================================

void enviarDadosParaAPI(int contagemPessoas) {

  // Fecha conexão anterior, caso ainda exista
  if (client.connected()) {
    client.stop();
  }

  // Tenta conectar ao servidor
  if (client.connect(server, port)) {

    Serial.println("Conectado ao servidor, enviando POST...");

    // ------------------------------------------------------------------
    // Requisição HTTP
    // ------------------------------------------------------------------

    client.println("POST /api HTTP/1.1");

    client.print("Host: ");
    client.println(server);

    client.print("X-Id: ");
    client.println(ONIBUS);

    client.print("X-Pessoas: ");
    client.println(contagemPessoas);

    client.println("Connection: close");
    client.println();

    Serial.println("Requisicao enviada.");

  } else {
    Serial.println("Falha ao conectar com o servidor.");
    return;
  }


  // --------------------------------------------------------------------
  // Aguarda resposta do servidor por até 2 segundos
  // --------------------------------------------------------------------

  unsigned long inicio = millis();

  while (client.available() == 0) {

    if (millis() - inicio > 2000) {
      Serial.println(
        ">>> Timeout: Servidor nao respondeu (2s)."
      );

      client.stop();
      return;
    }

    // Pequena pausa para evitar loop muito apertado
    delay(10);
  }


  Serial.println("Conexao encerrada.");

  client.stop();
}


// ======================================================================
// ========================== ATUALIZAR LED =============================
// ======================================================================

void updateLed(int pessoas) {

  // --------------------------------------------------------------------
  // 0–41 pessoas → Verde
  // --------------------------------------------------------------------

  if (pessoas < 42) {

    digitalWrite(VERDE, HIGH);
    digitalWrite(AZUL, LOW);
    digitalWrite(VERMELHO, LOW);

  }

  // --------------------------------------------------------------------
  // 42–51 pessoas → Azul
  // --------------------------------------------------------------------

  else if (pessoas >= 44 && pessoas <= 51) {

    digitalWrite(AZUL, HIGH);
    digitalWrite(VERDE, LOW);
    digitalWrite(VERMELHO, LOW);

  }

  // --------------------------------------------------------------------
  // 52+ pessoas → Vermelho
  // --------------------------------------------------------------------

  else {

    digitalWrite(VERMELHO, HIGH);
    digitalWrite(VERDE, LOW);
    digitalWrite(AZUL, LOW);
  }
}


// ======================================================================
// ========================= STATUS DO WI-FI ============================
// ======================================================================

void printWifiStatus() {

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();

  Serial.print("IP: ");
  Serial.println(ip);
}
