
# NETUNO — Sistema de Monitoramento Ambiental Aquático

## 1. Apresentação

O **Netuno** é um sistema de monitoramento ambiental aquático desenvolvido com o objetivo de realizar a aquisição, o processamento e a transmissão de parâmetros físico-químicos relacionados à qualidade da água.

O projeto integra sistemas embarcados, instrumentação eletrônica e tecnologias de comunicação, utilizando uma arquitetura baseada em microcontrolador ESP32, sensores específicos, conversores analógico-digitais e comunicação sem fio.

Seu desenvolvimento teve origem no contexto do Curso Técnico em Eletrônica do Colégio Técnico da Universidade Federal de Minas Gerais (COLTEC/UFMG), na disciplina de Microcontroladores, com data de criação informada em 15 de outubro de 2025.

A proposta contempla a integração entre hardware e software para possibilitar o acompanhamento de variáveis ambientais e fornecer uma estrutura tecnológica aplicável a atividades de monitoramento e análise da qualidade da água.

## 2. Objetivos

O objetivo principal do Netuno é desenvolver uma solução integrada para monitoramento de parâmetros ambientais aquáticos por meio de instrumentação eletrônica e sistemas embarcados.

Entre seus objetivos específicos estão:

- Realizar medições de temperatura, potencial hidrogeniônico (pH) e sólidos dissolvidos totais (TDS).
- Integrar sensores e módulos de aquisição a uma unidade de processamento baseada em ESP32.
- Implementar rotinas de processamento e conversão dos sinais obtidos.
- Transmitir as informações por meio de comunicação sem fio.
- Disponibilizar uma estrutura de integração para processamento e visualização dos dados.
- Desenvolver uma arquitetura modular que permita futuras expansões de hardware e software.

## 3. Parâmetros monitorados

A versão de firmware documentada contempla a aquisição de três parâmetros relacionados à qualidade da água.

### 3.1. Temperatura

A medição de temperatura é realizada por um termopar tipo K conectado ao módulo MAX6675.

O módulo realiza a leitura do sinal proveniente do termopar e disponibiliza a informação ao ESP32 por meio de comunicação digital.

O firmware utiliza a função de leitura do MAX6675 para obter a temperatura em graus Celsius.

### 3.2. Potencial hidrogeniônico (pH)

O monitoramento do pH é realizado por meio de uma sonda eletroquímica conectada a um módulo de condicionamento de sinal.

O sinal analógico disponibilizado pelo módulo é adquirido pelo conversor ADS1115, utilizando seu canal de entrada 0.

O firmware aplica uma calibração linear baseada em duas referências de tensão correspondentes aos valores de pH 4 e pH 7.

Essa relação é utilizada para estimar o potencial hidrogeniônico da amostra analisada.

### 3.3. Sólidos dissolvidos totais (TDS)

A medição de TDS utiliza um módulo específico para monitoramento da qualidade da água, conectado ao conversor ADS1115.

A aquisição é realizada pelo canal de entrada 1 do conversor.

O firmware processa a tensão obtida e aplica parâmetros de referência e compensação térmica para calcular uma estimativa de sólidos dissolvidos totais.

O resultado é disponibilizado como valor numérico inteiro.

## 4. Arquitetura de hardware

A arquitetura de hardware é organizada em torno do ESP32, responsável pela aquisição das medições, execução das rotinas de processamento e comunicação com a infraestrutura externa.

### 4.1. Unidade de processamento

**ESP32**

Microcontrolador responsável pela execução do firmware e pela coordenação das operações do sistema.

Suas atribuições incluem:

- Inicialização dos periféricos.
- Comunicação com os módulos de aquisição.
- Processamento das medições.
- Gerenciamento da conexão Wi-Fi.
- Comunicação com o broker MQTT.
- Organização e transmissão dos dados.

### 4.2. Conversor analógico-digital

**ADS1115**

Conversor analógico-digital de 16 bits utilizado para aquisição dos sinais provenientes dos módulos de pH e TDS.

Sua utilização permite a leitura dos sinais analógicos e sua disponibilização ao microcontrolador para processamento.

### 4.3. Módulo de temperatura

**MAX6675 e termopar tipo K**

Conjunto responsável pela aquisição da temperatura.

O MAX6675 realiza a conversão do sinal proveniente do termopar e disponibiliza a medição ao ESP32 por meio de comunicação digital.

### 4.4. Instrumentação de pH

**Sonda de pH e módulo de condicionamento**

Conjunto utilizado para detectar o potencial hidrogeniônico da amostra e disponibilizar um sinal compatível com o sistema de aquisição.

A identificação comercial exata dos componentes depende da confirmação dos modelos utilizados.

### 4.5. Instrumentação de TDS

**Módulo de medição TDS e sonda**

Conjunto responsável pela obtenção do sinal utilizado para estimar a concentração de sólidos dissolvidos totais.

A identificação comercial exata do módulo e da sonda depende da confirmação dos modelos adquiridos.

## 5. Arquitetura de software

O sistema contempla tecnologias de programação, comunicação e integração, distribuídas entre o firmware embarcado e a infraestrutura de processamento.

### 5.1. Firmware embarcado

**Linguagem: C++**

O firmware é executado no ESP32 e concentra as operações de aquisição, processamento e transmissão das medições.

Suas principais responsabilidades são:

- Inicialização dos módulos e periféricos.
- Estabelecimento da conexão Wi-Fi.
- Gerenciamento da comunicação MQTT.
- Aquisição dos dados dos sensores.
- Aplicação dos cálculos de pH e TDS.
- Organização das informações em formato JSON.
- Publicação periódica das medições.

### 5.2. Processamento e integração

**Ambiente: Node-RED**

O Node-RED é utilizado como ambiente de integração dos componentes do sistema.

Sua arquitetura baseada em fluxos permite estabelecer conexões entre os serviços de comunicação, processamento e visualização.

A integração pode utilizar nós de entrada MQTT, nós Function para execução de JavaScript e outros componentes destinados ao encaminhamento e tratamento das informações.

### 5.3. Processamento em JavaScript

**Linguagem: JavaScript**

O JavaScript é utilizado nos nós Function do Node-RED para executar operações sobre as mensagens recebidas.

Entre as operações previstas estão:

- Interpretação do conteúdo recebido.
- Análise da estrutura das mensagens.
- Validação dos campos e tipos de dados.
- Organização das informações.
- Encaminhamento dos dados para os demais componentes da integração.

O arquivo `netuno_processor.js` contém uma implementação de referência para processamento e validação das mensagens publicadas pelo firmware.

Essa implementação foi elaborada com base na estrutura de comunicação identificada no código C++ documentado e não deve ser confundida com uma recuperação do código-fonte original da aplicação Node-RED.

### 5.4. Visualização de dados

**Ferramenta: Grafana**

O Grafana integra a arquitetura de visualização prevista para o acompanhamento das informações monitoradas.

Sua utilização permite estruturar interfaces gráficas destinadas à apresentação e análise das medições.

A configuração efetiva dos painéis depende da integração com os serviços responsáveis pelo recebimento e armazenamento dos dados.

## 6. Comunicação e protocolos

### 6.1. Wi-Fi

A comunicação sem fio é realizada por meio da interface Wi-Fi do ESP32.

O firmware utiliza a biblioteca `WiFi.h` para estabelecer a conexão com a rede configurada.

### 6.2. MQTT

O MQTT é utilizado como protocolo de comunicação entre o dispositivo embarcado e os componentes externos da arquitetura.

O firmware utiliza a biblioteca PubSubClient para gerenciar a conexão, as inscrições e as publicações.

Os tópicos configurados são:

| Tópico | Finalidade |
|---|---|
| `sensores/netuno/dados` | Publicação das medições realizadas pelo dispositivo |
| `sensores/netuno/atuadores` | Recebimento de mensagens destinadas ao dispositivo |

A configuração utiliza a porta 8883.

O firmware realiza a publicação periódica das medições e mantém uma inscrição no tópico destinado às mensagens de entrada.

A rotina de recebimento implementada atualmente realiza a leitura e a apresentação do conteúdo recebido. O controle efetivo de atuadores não está implementado na versão analisada.

### 6.3. JSON

O JSON é utilizado como formato de serialização das informações transmitidas pelo dispositivo.

A estrutura publicada pelo firmware contém os seguintes campos:

| Campo | Tipo | Descrição |
|---|---|---|
| `temperatura` | Numérico | Temperatura em graus Celsius |
| `ph` | Numérico | Valor calculado de pH |
| `tds` | Numérico inteiro | Estimativa de sólidos dissolvidos totais |

A utilização desse formato permite a interoperabilidade entre o firmware embarcado e os componentes de processamento da arquitetura.

## 7. Processamento das medições

O firmware executa rotinas específicas para transformar os sinais adquiridos em valores correspondentes aos parâmetros monitorados.

### 7.1. Aquisição

As leituras são realizadas por meio do MAX6675 e do ADS1115.

O intervalo de aquisição configurado na versão documentada é de 5 segundos.

### 7.2. Cálculo de pH

O cálculo utiliza os parâmetros de calibração definidos no firmware, considerando as referências de tensão associadas aos valores de pH 4 e pH 7.

A partir desses parâmetros, é estabelecida uma relação linear utilizada para estimar o valor de pH.

### 7.3. Cálculo de TDS

O cálculo de TDS utiliza a tensão obtida pelo conversor analógico-digital, uma referência de condutividade configurada e um fator de compensação relacionado à temperatura.

O resultado é convertido para um valor inteiro antes de sua transmissão.

### 7.4. Organização e transmissão

Após a aquisição e o processamento, os valores são organizados em uma estrutura JSON.

O firmware realiza a serialização das informações e publica a mensagem no tópico MQTT destinado aos dados dos sensores.

## 8. Bibliotecas e dependências

O firmware utiliza as seguintes bibliotecas:

| Biblioteca | Finalidade |
|---|---|
| `WiFi.h` | Gerenciamento da conexão Wi-Fi |
| `Wire.h` | Comunicação pelo barramento I²C |
| `WiFiClientSecure.h` | Comunicação por conexão TLS |
| `Adafruit_ADS1X15.h` | Integração com o conversor ADS1115 |
| `max6675.h` | Comunicação com o módulo MAX6675 |
| `PubSubClient.h` | Implementação da comunicação MQTT |
| `ArduinoJson.h` | Construção e serialização de mensagens JSON |

A execução do firmware depende da instalação das bibliotecas compatíveis com a plataforma ESP32 e com os módulos utilizados.

## 9. Configuração e execução

A execução do sistema depende da disponibilidade dos componentes de hardware, das bibliotecas utilizadas e da infraestrutura de comunicação.

### 9.1. Requisitos de hardware

- Microcontrolador ESP32.
- Conversor analógico-digital ADS1115.
- Módulo MAX6675.
- Termopar tipo K.
- Sonda e módulo de pH.
- Módulo e sonda TDS.
- Alimentação elétrica compatível com os componentes.

### 9.2. Requisitos de software

- Ambiente de desenvolvimento compatível com ESP32.
- Arduino IDE ou ambiente equivalente.
- Bibliotecas utilizadas pelo firmware.
- Broker MQTT acessível.
- Ambiente Node-RED para integração.
- JavaScript para processamento nos nós Function.

### 9.3. Configurações de conexão

As credenciais de Wi-Fi e MQTT devem ser fornecidas pelo responsável pela instalação.

Os parâmetros de conexão precisam corresponder à rede e ao broker utilizados no ambiente de execução.

Credenciais, senhas e informações privadas não devem ser disponibilizadas em repositórios públicos.

## 10. Segurança da comunicação

O firmware utiliza a biblioteca `WiFiClientSecure` para estabelecer uma conexão TLS com o broker MQTT.

Entretanto, a versão analisada utiliza o método `setInsecure()`, que desativa a validação do certificado apresentado pelo servidor.

Dessa forma, embora a comunicação utilize uma conexão TLS, a autenticação da identidade do broker por meio de certificado não está implementada nessa versão.

Para uma implantação com validação de certificado, recomenda-se configurar uma autoridade certificadora confiável por meio do método `setCACert()`.

Também é necessário manter as credenciais de acesso protegidas e evitar sua inclusão em arquivos disponibilizados publicamente.

## 11. Componentes não identificados na versão atual

Durante o levantamento dos materiais do projeto, também foram mencionados sensores de turbidez, transdutores de pressão e um sensor de temperatura DS18B20.

Entretanto, esses componentes não foram identificados na versão de firmware analisada.

Por esse motivo, não são considerados parâmetros implementados na documentação funcional atual.

A inclusão desses componentes na arquitetura depende da confirmação de sua utilização e da identificação de suas respectivas rotinas de aquisição e processamento.

## 12. Estado de desenvolvimento

Conforme informado na documentação de registro do programa, o Netuno foi declarado como protótipo funcional em validação técnica.

A documentação deste repositório descreve as características identificadas no firmware disponibilizado e as tecnologias relacionadas à arquitetura do projeto.

A presença de ferramentas ou componentes na descrição da arquitetura não constitui, isoladamente, comprovação de que todas as funcionalidades de integração tenham sido implementadas, testadas ou estejam operacionais.

A validação completa depende de testes de comunicação, aquisição, calibração, processamento e integração entre os componentes físicos e computacionais.

## 13. Contexto acadêmico

**Instituição:** Colégio Técnico da Universidade Federal de Minas Gerais (COLTEC/UFMG).

**Curso:** Técnico em Eletrônica.

**Disciplina de origem:** Microcontroladores.

**Data de criação informada:** 15 de outubro de 2025.

O projeto está relacionado ao desenvolvimento de conhecimentos e aplicações nas áreas de eletrônica, instrumentação, programação de sistemas embarcados, aquisição de dados e comunicação entre dispositivos.

## 14. Licenciamento e atribuição

As bibliotecas e ferramentas de terceiros utilizadas no desenvolvimento possuem licenças próprias, que devem ser observadas conforme os termos de distribuição de cada projeto.

A definição da licença do Netuno deve considerar a titularidade dos direitos autorais e a autorização dos responsáveis pelo programa.

Os créditos de autoria e colaboração devem corresponder às contribuições efetivamente realizadas no desenvolvimento do sistema.

---

**Netuno — Sistema de Monitoramento Ambiental Aquático**
