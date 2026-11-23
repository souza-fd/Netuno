/*
 * ============================================================================
 * PROJETO: NETUNO — SISTEMA DE MONITORAMENTO DA QUALIDADE DA ÁGUA
 * PLATAFORMA: ESP32
 * ============================================================================
 *
 * DESCRIÇÃO
 * Firmware destinado à aquisição, ao processamento e à transmissão de
 * parâmetros físico-químicos da água. O sistema utiliza um ESP32 para
 * realizar a leitura de temperatura, pH e sólidos dissolvidos totais (TDS),
 * organizando os resultados em mensagens JSON e transmitindo-os por MQTT.
 *
 * PLATAFORMA E COMPONENTES DECLARADOS
 * - Microcontrolador ESP32
 * - Módulo MAX6675 e termopar para medição de temperatura
 * - Conversor analógico-digital ADS1115 para leitura de pH e TDS
 * - Comunicação Wi-Fi para conexão à rede
 * - Comunicação MQTT para transmissão e recebimento de mensagens
 * - Cliente Wi-Fi Secure para transporte com TLS
 *
 * MAPEAMENTO DE CONEXÕES
 * MAX6675:
 * - SO: 19
 * - CS: 5
 * - SCK: 18
 *
 * ADS1115:
 * - Canal 0: sinal analógico do sensor de pH
 * - Canal 1: sinal analógico do sensor de TDS
 * - Comunicação com o ESP32 pelo barramento I2C
 *
 * ORGANIZAÇÃO DO PROGRAMA
 * 1. Bibliotecas e configurações de comunicação
 * 2. Instâncias dos clientes Wi-Fi e MQTT
 * 3. Configuração dos sensores e parâmetros de calibração
 * 4. Variáveis de controle de tempo
 * 5. Callback para recebimento de mensagens MQTT
 * 6. Funções de conexão Wi-Fi e MQTT
 * 7. Inicialização do sistema (setup)
 * 8. Aquisição, processamento e publicação dos dados (loop)
 *
 * AQUISIÇÃO E PROCESSAMENTO
 * O MAX6675 fornece a leitura de temperatura em graus Celsius. O ADS1115
 * realiza a conversão dos sinais analógicos associados aos sensores de pH
 * e TDS. O firmware converte os valores digitais em tensão e aplica as
 * fórmulas de calibração e compensação definidas no código.
 *
 * COMUNICAÇÃO MQTT
 * O dispositivo publica os dados no tópico:
 * sensores/netuno/dados
 *
 * Também realiza a inscrição no tópico:
 * sensores/netuno/atuadores
 *
 * Os dados dos sensores são organizados em JSON, contendo os campos
 * temperatura, ph e tds. O recebimento de mensagens é implementado por
 * callback, mas a lógica de acionamento dos atuadores permanece comentada.
 *
 * PARÂMETROS DE CALIBRAÇÃO
 * - pH 7: 2,50 V
 * - pH 4: 3,00 V
 * - EC_REF: 500,0
 * - V_REF_CAL: 2,5 V
 *
 * PERIODICIDADE
 * A publicação dos dados é realizada em intervalos de 5.000 ms (5 segundos),
 * conforme definido pela variável interval.
 *
 * SEGURANÇA E COMUNICAÇÃO
 * A conexão MQTT utiliza WiFiClientSecure na porta 8883. Entretanto,
 * setInsecure() desativa a validação do certificado do servidor. Portanto,
 * esta implementação não valida a identidade do broker por certificado.
 *
 * OBSERVAÇÕES TÉCNICAS
 * - A conversão do ADS1115 deve ser compatibilizada com o ganho configurado.
 * - Os cálculos de pH e TDS dependem da calibração dos circuitos e sensores.
 * - A fórmula de TDS requer tratamento para tensão igual a zero.
 * - A leitura de temperatura deve ser validada antes de sua utilização.
 * - As rotinas de conexão Wi-Fi e MQTT utilizam laços bloqueantes.
 * - O código documentado não representa, por si só, validação experimental.
 *
 * ============================================================================
 */


// ============================================================================
// 1. BIBLIOTECAS E CONFIGURAÇÕES DE COMUNICAÇÃO
// ============================================================================
// Bibliotecas para comunicação Wi-Fi, I2C, MQTT e TLS, além da aquisição
// de temperatura pelo MAX6675, leitura analógica pelo ADS1115 e manipulação
// de dados no formato JSON.

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <max6675.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// ============================================================================
// 2. CONFIGURAÇÕES DE REDE E MQTT
// ============================================================================
// Parâmetros utilizados para conexão à rede Wi-Fi e ao broker MQTT.
// As credenciais devem ser configuradas localmente e não devem ser
// disponibilizadas em repositórios públicos.

// Configuração da rede Wi-Fi
const char* ssid = "SEU_SSID";
const char* password = "SUA_SENHA_WIFI";

// Configuração do broker MQTT
const char* mqtt_server = "ENDERECO_DO_BROKER";
const int mqtt_port = 8883;
const char* mqtt_user = "USUARIO_MQTT";
const char* mqtt_pass = "SENHA_MQTT";

// Tópicos MQTT
const char* mqtt_topic_publish = "sensores/netuno/dados";
const char* mqtt_topic_subscribe = "sensores/netuno/atuadores";

// Identificação exclusiva do cliente MQTT
const char* clientID = "NETUNO_SEGURO";


// ============================================================================
// 3. INSTÂNCIAS DOS CLIENTES DE COMUNICAÇÃO
// ============================================================================
// WiFiClientSecure fornece o transporte de rede utilizado pelo cliente MQTT.
// PubSubClient implementa a comunicação por publicação e assinatura.

WiFiClientSecure espClientSecure;
PubSubClient client(espClientSecure);


// ============================================================================
// 4. CONFIGURAÇÃO DOS SENSORES E MAPEAMENTO DE CONEXÕES
// ============================================================================
// MAX6675: leitura de temperatura por termopar, utilizando os pinos
// SO=19, CS=5 e SCK=18.
// ADS1115: conversão dos sinais analógicos dos sensores de pH e TDS,
// utilizando os canais 0 e 1, respectivamente.

// Configuração do MAX6675
int soPin = 19;
int csPin = 5;
int sckPin = 18;

MAX6675 thermocouple(sckPin, csPin, soPin);

// Configuração do ADS1115
Adafruit_ADS1115 ads;

#define PH_ADC_CHANNEL 0
#define TDS_ADC_CHANNEL 1

// Parâmetros utilizados na conversão analógico-digital
#define ADS_VREF 5.00
#define ADS_MAX_VALUE 32767.0


// ============================================================================
// 5. VARIÁVEIS E CONSTANTES DE CALIBRAÇÃO
// ============================================================================
// Parâmetros utilizados na conversão das tensões obtidas pelos sensores
// em valores estimados de pH e TDS. Os valores correspondem à calibração
// definida na implementação e devem ser validados experimentalmente.

// Referências de tensão para calibração do pH
const float voltageAt7_pH = 2.50;
const float voltageAt4_pH = 3.00;

// Inclinação da relação linear entre tensão e pH
const float slope_pH =
    (voltageAt4_pH - voltageAt7_pH) / (4.0 - 7.0);

// Constantes utilizadas no cálculo de TDS
const float EC_REF = 500.0;
const float V_REF_CAL = 2.5;


// ============================================================================
// 6. VARIÁVEIS DE CONTROLE DE TEMPO
// ============================================================================
// Controle da periodicidade de aquisição e publicação dos dados.
// A variável lastMsg armazena o instante da última execução da rotina,
// enquanto interval define o intervalo entre as publicações.

long lastMsg = 0;

// Intervalo de publicação: 5.000 milissegundos (5 segundos)
long interval = 5000;


// ============================================================================
// 7. CALLBACK DE RECEBIMENTO DE MENSAGENS MQTT
// ============================================================================
// callback() processa as mensagens recebidas no tópico inscrito pelo cliente.
// O conteúdo recebido é convertido em String e apresentado no monitor serial.
// A lógica de acionamento dos atuadores permanece comentada nesta versão.

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");

  // Conversão do payload recebido para String
  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.println(message);

  // LÓGICA DE ACIONAMENTO DOS ATUADORES
  // O trecho abaixo representa um exemplo de controle de bomba,
  // mas não está ativo na implementação atual.

  /*
  if (message == "LIGAR_BOMBA") {
    digitalWrite(PINO_RELE_BOMBA, HIGH);
    Serial.println("Bomba ligada!");
  } else if (message == "DESLIGAR_BOMBA") {
    digitalWrite(PINO_RELE_BOMBA, LOW);
    Serial.println("Bomba desligada!");
  }
  */
}


// ============================================================================
// 8. CONEXÃO À REDE WI-FI
// ============================================================================
// setup_wifi() inicia a conexão com a rede configurada e aguarda até que
// o ESP32 obtenha conexão. Ao final, apresenta o endereço IP atribuído.
// A rotina utiliza espera bloqueante enquanto a conexão não é estabelecida.

void setup_wifi() {

  delay(10);

  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  // Inicia a conexão Wi-Fi
  WiFi.begin(ssid, password);

  // Aguarda o estabelecimento da conexão
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");

  // Exibe o endereço IP atribuído ao ESP32
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}


// ============================================================================
// 9. CONEXÃO E RECONEXÃO COM O BROKER MQTT
// ============================================================================
// reconnect_mqtt() tenta restabelecer a conexão com o broker enquanto
// o cliente estiver desconectado. Após uma conexão bem-sucedida, realiza
// a inscrição no tópico destinado às mensagens dos atuadores.
// Em caso de falha, apresenta o código de estado e aguarda cinco segundos
// antes de realizar uma nova tentativa.

void reconnect_mqtt() {

  while (!client.connected()) {

    Serial.print("Tentando conectar ao MQTT Broker...");

    // Autenticação e conexão com o broker
    if (client.connect(clientID, mqtt_user, mqtt_pass)) {

      Serial.println("conectado!");

      // Inscrição no tópico de comunicação dos atuadores
      client.subscribe(mqtt_topic_subscribe);

      Serial.print("Subscrito ao tópico: ");
      Serial.println(mqtt_topic_subscribe);

    } else {

      Serial.print("falhou, rc=");
      Serial.print(client.state());

      Serial.println(" tentando novamente em 5 segundos");

      delay(5000);
    }
  }
}


// ============================================================================
// 10. INICIALIZAÇÃO DO SISTEMA
// ============================================================================
// setup() configura a comunicação serial, estabelece a conexão Wi-Fi,
// configura o cliente MQTT e inicializa o conversor ADS1115.
// Também define a função de callback responsável pelo recebimento
// de mensagens MQTT e prepara o módulo MAX6675.

void setup() {

  // Inicialização da comunicação serial
  Serial.begin(115200);

  // Conexão à rede Wi-Fi
  setup_wifi();

  // Configuração do cliente TLS.
  // setInsecure() desativa a validação do certificado do servidor.
  // Para implantação com validação de identidade, recomenda-se setCACert().
  espClientSecure.setInsecure();

  // Configuração do broker MQTT
  client.setServer(mqtt_server, mqtt_port);

  // Registro da função de recebimento de mensagens
  client.setCallback(callback);

  // Inicialização do conversor ADS1115
  Serial.println("Inicializando ADS1115 (I2C)...");

  if (!ads.begin()) {

    Serial.println("Falha ao encontrar o ADS1115! Verifique as conexões.");

    // Interrompe a execução caso o conversor não seja encontrado
    while (1);
  }

  // Preparação do módulo MAX6675
  Serial.println("Inicializando MAX6675 (SPI)...");

  delay(500);
}


// ============================================================================
// 11. LOOP PRINCIPAL: AQUISIÇÃO, PROCESSAMENTO E TRANSMISSÃO
// ============================================================================
// loop() mantém a comunicação MQTT ativa e executa periodicamente a rotina
// de aquisição dos sensores. Os valores obtidos são convertidos, processados,
// organizados em JSON e publicados no tópico destinado aos dados do Netuno.

void loop() {

  // Verifica a conexão MQTT e executa a reconexão quando necessário
  if (!client.connected()) {
    reconnect_mqtt();
  }

  // Mantém a comunicação MQTT ativa e processa mensagens recebidas
  client.loop();

  // Controle do intervalo de publicação
  long now = millis();

  if (now - lastMsg > interval) {

    lastMsg = now;


    // ========================================================================
    // 12. AQUISIÇÃO DOS DADOS DOS SENSORES
    // ========================================================================
    // Leitura da temperatura pelo MAX6675 e aquisição dos sinais analógicos
    // de pH e TDS pelos canais definidos no conversor ADS1115.

    // Temperatura em graus Celsius
    double tempC = thermocouple.readCelsius();

    // Leitura analógica do sensor de pH
    int16_t adcValue_pH = ads.readADC_SingleEnded(PH_ADC_CHANNEL);

    // Conversão da leitura digital em tensão
    float voltage_pH =
        (float)adcValue_pH * ADS_VREF / ADS_MAX_VALUE;


    // ========================================================================
    // 13. CÁLCULO DO pH
    // ========================================================================
    // Conversão da tensão obtida pelo ADS1115 em valor estimado de pH,
    // utilizando a relação linear estabelecida pelos pontos de calibração.

    float phValue =
        7.0 + ((voltage_pH - voltageAt7_pH) / slope_pH);


    // ========================================================================
    // 14. LEITURA E PROCESSAMENTO DO TDS
    // ========================================================================
    // Aquisição do sinal analógico do sensor de TDS, conversão para tensão
    // e aplicação do fator de compensação de temperatura.
    // O valor final é estimado a partir da constante de referência EC_REF.

    // Leitura analógica do sensor de TDS
    int16_t adcValue_TDS =
        ads.readADC_SingleEnded(TDS_ADC_CHANNEL);

    // Conversão da leitura digital em tensão
    float voltage_TDS =
        (float)adcValue_TDS * ADS_VREF / ADS_MAX_VALUE;

    // Fator de compensação de temperatura
    float temp_compensated_factor =
        1.0 + 0.02 * (tempC - 25.0);

    // Cálculo da condutividade elétrica compensada
    float EC_compensated =
        (V_REF_CAL * EC_REF) / voltage_TDS / temp_compensated_factor;

    // Estimativa do valor de TDS
    float tdsValue = EC_compensated / 2.0;


    // ========================================================================
    // 15. ORGANIZAÇÃO DOS DADOS EM JSON
    // ========================================================================
    // Criação de um documento JSON contendo os valores de temperatura,
    // pH e TDS. Os valores de temperatura e pH são convertidos para
    // duas casas decimais por truncamento; o TDS é convertido para inteiro.

    StaticJsonDocument<256> doc;

    // Inserção dos parâmetros no documento JSON
    doc["temperatura"] = float(int(tempC * 100)) / 100;
    doc["ph"] = float(int(phValue * 100)) / 100;
    doc["tds"] = int(tdsValue);


    // ========================================================================
    // 16. SERIALIZAÇÃO E EXIBIÇÃO DOS DADOS
    // ========================================================================
    // Conversão do documento JSON para uma sequência de caracteres,
    // armazenada em um buffer para posterior publicação MQTT.

    char output[256];

    serializeJson(doc, output);

    // Exibição das informações no monitor serial
    Serial.print("Publicando no tópico '");
    Serial.print(mqtt_topic_publish);
    Serial.print("': ");

    Serial.println(output);

    // Exibe a tensão obtida na leitura do sensor de pH
    Serial.println(voltage_pH);


    // ========================================================================
    // 17. PUBLICAÇÃO DOS DADOS NO MQTT
    // ========================================================================
    // Envia o documento JSON serializado ao broker MQTT, utilizando
    // o tópico definido para publicação dos dados dos sensores.

    client.publish(mqtt_topic_publish, output);

    Serial.println("--------------------");
  }
}


// ============================================================================
// FIM DO FIRMWARE — NETUNO
// ============================================================================