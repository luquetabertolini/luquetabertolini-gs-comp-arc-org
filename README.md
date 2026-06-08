Sistema IoT — Monitoramento de Causa Espacial
FIAP | Global Solution 2026 | 1º Semestre
Disciplina: Sistemas Embarcados / IoT

Descricao do Projeto
Este projeto consiste em um sistema de telemetria e monitoramento em tempo real para as condicoes internas de uma capsula espacial. Utilizando o ecossistema Arduino, o circuito monitora variaveis criticas de temperatura, luminosidade e vibracao/impacto, acionando alertas visuais (LEDs), sonoros (Buzzer) e exibindo os dados mastigados em um display LCD 16x2 com comunicacao I2C, alem de enviar logs detalhados via Serial (Telemetria).

O sistema foi adaptado com sucesso para simulacao tanto no Wokwi quanto no Tinkercad.

Componentes Utilizados e Conexoes

Sensores e Entradas:

DHT22 / DHT11 (Temperatura): Conectado ao pino digital D2. Monitora o clima interno da capsula.

LDR (Sensor de Luz): Conectado ao pino analogico A0 (com resistor pull-down de 10kOhm). Monitora a incidencia de luz ou escuridao no modulo.

SW-420 / Sensor de Inclinacao-Tilt (Vibracao): Conectado ao pino digital D3. Detecta impactos estruturais e trepidacoes anormais.

Atuadores e Saidas:

Display LCD 16x2 I2C: Conectado aos pinos A4 (SDA) e A5 (SCL). Exibe os dados alternando as telas a cada 2 segundos.

LED Vermelho (Alerta Termico): Conectado ao pino digital D8 (com resistor de 220Ohm).

LED Amarelo (Alerta de Impacto): Conectado ao pino digital D9 (com resistor de 220Ohm).

Buzzer Piezoeletrico (Alarme Sonoro): Conectado ao pino digital D10.

Logica de Funcionamento e Regras de Negocio

O sistema opera baseado em faixas de seguranca estritas. Dependendo dos valores lidos, as saidas se comportam da seguinte forma:

Monitoramento de Temperatura

Abaixo de 15.0C: Status FRIO CRITICO. O LED Vermelho acende e o Buzzer emite um bip medio (600Hz por 150ms).

Entre 15.0C e 35.0C: Status NORMAL. Condicao nominal, LEDs e Buzzer desligados.

Acima de 35.0C: Status CALOR CRITICO. O LED Vermelho acende e o Buzzer emite um bip medio (600Hz por 150ms).

Monitoramento de Vibracao (Impacto)

Sem impacto (Estavel): Status ESTAVEL. LED Amarelo desligado.

Impacto Detectado: Status IMPACTO/FALHA. O LED Amarelo acende e o Buzzer emite um som grave (400Hz por 100ms).

Nota de Seguranca: O alarme de impacto possui uma trava (debounce e reset automatico de 2 segundos) para garantir que mesmo impactos rapidos sejam vistos e registrados pelo operador.

Modo de Emergencia Critica (Combinado)

Condicao: Se houver falha de temperatura (muito quente/frio) E um impacto/vibracao ao mesmo tempo.

Comportamento: Ambos os LEDs acendem e o Buzzer entra em modo de estridencia maxima, tocando um alarme agudo e rapido (1000Hz por 200ms). O Monitor Serial reporta o estado de EMERGENCIA.

Monitoramento de Luminosidade
A luminosidade varia de 0 a 1023 e atualiza dinamicamente o status no LCD:

Menor que 200: Modulo ESCURO

Entre 200 e 599: Modulo PENUMBRA

Maior ou igual a 600: Modulo ILUMINADO

Visualizacao de Dados (LCD e Serial)

Display LCD
O display alterna automaticamente a cada 2 segundos entre tres telas:

Tela 1: Temperatura atual (C) e o status do clima.

Tela 2: Valor bruto da luz (0-1023) e o status de iluminacao do modulo.

Tela 3: Condicao estrutural (Estavel ou Impacto/Falha).

Telemetria Serial (9600 Baud)
A cada 500ms, o sistema envia uma linha de log formatada para monitoramento externo:
Ciclo | Temp(C) | Lux(0-1023) | Vib | Status

Como Executar o Projeto

Copie o codigo do arquivo .ino.

Monte o circuito em um simulador (como o Tinkercad ou Wokwi) seguindo o mapeamento de pinos descrito na secao de componentes.

Certifique-se de incluir as seguintes bibliotecas no seu ambiente Arduino:

DHT sensor library (Adafruit)

Adafruit Unified Sensor

LiquidCrystal I2C

Inicie a simulacao e abra o Monitor Serial para acompanhar a telemetria.

Integrantes:
Lucca Bertolini - RM: 569552
Raphaello Caffettani - RM: 572334

Link do circuito:
https://wokwi.com/projects/466300753761911809
