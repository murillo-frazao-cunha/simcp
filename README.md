# Contador de Passageiros

Projeto desenvolvido para realizar a contagem de passageiros em um ônibus utilizando sensores ultrassônicos e comunicação Wi-Fi.

## Hardware

* **Arduino Mega 2560** — responsável pelo processamento e controle dos componentes.
* **ESP8266** — utilizado para realizar a conexão Wi-Fi e comunicação com a API.
* **2x Sensor Ultrassônico** — utilizados para detectar a entrada e saída de passageiros.
* **LED RGB** — utilizado para indicar visualmente a quantidade de passageiros.
* Jumpers e demais componentes necessários para as conexões.

## Funcionamento

Os sensores ultrassônicos monitoram a entrada e a saída do ônibus. Quando uma pessoa é detectada entrando, o contador é incrementado. Quando uma pessoa é detectada saindo, o contador é decrementado.

A quantidade de passageiros é enviada para uma API através do **ESP8266**, utilizando uma conexão Wi-Fi.

O LED RGB indica a ocupação atual do ônibus:

| Passageiros | LED      |
| ----------- | -------- |
| 0–41        | Verde    |
| 42–51       | Azul     |
| 52+         | Vermelho |

## Tecnologias

* Arduino / C++
* ESP8266
* Arduino Mega 2560
* Wi-Fi
* Sensores ultrassônicos
* API HTTP

## Projeto

Projeto desenvolvido com o objetivo de monitorar a ocupação de ônibus em tempo real por meio de sensores e comunicação com um servidor.
