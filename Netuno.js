
/*
 * ============================================================
 * NETUNO — PROCESSAMENTO E VALIDAÇÃO DE DADOS
 * ============================================================
 *
 * Projeto: Sistema de monitoramento Netuno
 * Plataforma: Node-RED
 * Linguagem: JavaScript
 * Integração: MQTT / JSON
 *
 * DESCRIÇÃO:
 * Recebe os dados publicados pelo ESP32 através do MQTT,
 * interpreta o conteúdo JSON, verifica a integridade dos
 * valores e organiza as informações para processamento
 * pelos demais nós do Node-RED.
 *
 * ORIGEM DOS DADOS:
 * Firmware ESP32 — arquivo Netuno.ino
 *
 * TÓPICO MQTT:
 * sensores/netuno/dados
 *
 * CAMPOS RECEBIDOS:
 * temperatura -> MAX6675
 * ph          -> ADS1115, canal 0
 * tds         -> ADS1115, canal 1
 *
 * OBSERVAÇÃO:
 * Este código deve ser inserido em um nó Function
 * conectado a um nó MQTT In no Node-RED.
 *
 * ============================================================
 */


/* ============================================================
 * 1. CONFIGURAÇÕES
 * ============================================================
 */

// Nomes dos campos enviados pelo firmware.
const CAMPOS_OBRIGATORIOS = [
    "temperatura",
    "ph",
    "tds"
];

// Limites físicos utilizados na validação do pH.
const PH_MINIMO = 0;
const PH_MAXIMO = 14;


/* ============================================================
 * 2. VERIFICAÇÃO DA MENSAGEM
 * ============================================================
 */

// Verifica se o payload foi recebido.
if (
    msg.payload === undefined ||
    msg.payload === null
) {
    node.warn("Netuno: mensagem recebida sem conteúdo.");
    return null;
}


/* ============================================================
 * 3. INTERPRETAÇÃO DO PAYLOAD
 * ============================================================
 */

// O MQTT pode entregar o conteúdo como Buffer,
// texto JSON ou objeto JavaScript.
let dados;

try {

    if (Buffer.isBuffer(msg.payload)) {

        // Converte Buffer para texto UTF-8.
        dados = JSON.parse(
            msg.payload.toString("utf8")
        );

    } else if (typeof msg.payload === "string") {

        // Interpreta o texto como JSON.
        dados = JSON.parse(msg.payload);

    } else if (
        typeof msg.payload === "object" &&
        !Array.isArray(msg.payload)
    ) {

        // Aceita objetos já interpretados.
        dados = msg.payload;

    } else {

        node.warn(
            "Netuno: formato de payload não reconhecido."
        );

        return null;
    }

} catch (erro) {

    node.error(
        "Netuno: falha na interpretação do JSON: " +
        erro.message,
        msg
    );

    return null;
}


/* ============================================================
 * 4. VALIDAÇÃO DA ESTRUTURA JSON
 * ============================================================
 */

// Verifica se todos os campos esperados existem.
for (const campo of CAMPOS_OBRIGATORIOS) {

    if (
        !Object.prototype.hasOwnProperty.call(
            dados,
            campo
        )
    ) {

        node.warn(
            `Netuno: campo obrigatório ausente: ${campo}`
        );

        return null;
    }
}


/* ============================================================
 * 5. VALIDAÇÃO DOS TIPOS DE DADOS
 * ============================================================
 */

// Os três campos devem ser valores numéricos válidos.
for (const campo of CAMPOS_OBRIGATORIOS) {

    if (
        typeof dados[campo] !== "number" ||
        !Number.isFinite(dados[campo])
    ) {

        node.warn(
            `Netuno: valor numérico inválido em ${campo}.`
        );

        return null;
    }
}


/* ============================================================
 * 6. VALIDAÇÃO DOS VALORES DOS SENSORES
 * ============================================================
 */

// Verifica se o pH está dentro da escala convencional.
if (
    dados.ph < PH_MINIMO ||
    dados.ph > PH_MAXIMO
) {

    node.warn(
        "Netuno: valor de pH fora da faixa de 0 a 14."
    );

    return null;
}


// O TDS não deve apresentar valores negativos.
if (dados.tds < 0) {

    node.warn(
        "Netuno: valor de TDS negativo."
    );

    return null;
}


// O firmware envia o TDS como número inteiro.
if (!Number.isInteger(dados.tds)) {

    node.warn(
        "Netuno: o valor de TDS não é um número inteiro."
    );

    return null;
}


/* ============================================================
 * 7. ORGANIZAÇÃO DOS DADOS
 * ============================================================
 */

// Cria um objeto padronizado com os campos
// utilizados pelo firmware do Netuno.
const dadosValidados = {

    temperatura: dados.temperatura,

    ph: dados.ph,

    tds: dados.tds

};


/* ============================================================
 * 8. REGISTRO DE INFORMAÇÕES DA MENSAGEM
 * ============================================================
 */

// Armazena informações complementares sem alterar
// os nomes e valores do payload principal.
msg.netuno = {

    origem: "ESP32",

    protocolo: "MQTT",

    formato: "JSON",

    topico: msg.topic || null,

    recebidoEm: new Date().toISOString(),

    validado: true

};


/* ============================================================
 * 9. PREPARAÇÃO DA SAÍDA
 * ============================================================
 */

// Substitui o payload original pelo objeto validado.
msg.payload = dadosValidados;


/* ============================================================
 * 10. FINALIZAÇÃO
 * ============================================================
 */

// Encaminha a mensagem para o próximo nó do fluxo.
return msg;
