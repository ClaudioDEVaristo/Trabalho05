# 🌧️ Sistema de Alerta Inteligente para Chuvas Fortes e Enchentes

Este projeto simula um sistema embarcado inteligente para monitoramento de riscos ambientais, como enchentes e chuvas intensas, utilizando a Raspberry Pi Pico (BitDogLab). O sistema utiliza sensores analógicos, alertas visuais e sonoros, e exibe informações em tempo real por meio de um display OLED.

## 🔧 Tecnologias Utilizadas

- **Placa:** Raspberry Pi Pico W (BitDogLab)
- **RTOS:** FreeRTOS (para multitarefa)
- **Display:** OLED SSD1306 via I2C
- **Sensor:** Joystick analógico (simula sensores ambientais)
- **Alerta Sonoro:** Buzzer via PWM
- **Alerta Visual:** LED RGB e Matriz de LEDs (via PIO)
- **Comunicação:** Filas (Queue) do FreeRTOS

## 🧠 Funcionalidades

- **Leitura de sensores (joystick)** simulando intensidade da chuva (eixo X) e nível da água (eixo Y).
- **Display OLED** mostra:
  - Níveis gráficos da chuva e do nível da água.
  - Alertas como "ALERTA DE CHUVAS FORTES" e "ALERTA DE ENCHENTE".
- **Buzzer** emite beeps distintos conforme o alerta.
- **LED vermelho** acende em situações críticas.
- **Matriz de LEDs** exibe um símbolo de exclamação com diferentes cores, dependendo da gravidade.
- **Reset USB via botão físico** para entrar em modo BOOTSEL.

## 📦 Estrutura de Código

- `main.c`: Inicialização do sistema e criação das tarefas.
- `vJoystickTask`: Leitura do joystick e envio dos dados pela fila.
- `vDisplayTask`: Exibição dos dados e alertas no display OLED.
- `vMatrizTask`: Controle da matriz de LEDs e do buzzer com base nos alertas.
- `vLedRedTask`: Controle do LED vermelho.
- `config_matriz.h`: Configuração da matriz de LEDs.
- `filas.pio`: Programa PIO para a matriz.
- `lib/`: Biblioteca para controle do display SSD1306 e fontes.

## 🚨 Critérios de Alerta

| Parâmetro | Limite | Ação |
|-----------|--------|------|
| Eixo X ≥ 80 | Chuva Forte | Exibe alerta, buzina com intervalo longo, matriz exibe "!" vermelho |
| Eixo Y ≥ 70 | Enchente | Exibe alerta, buzina com intervalo rápido, matriz exibe "!" azul |
| Normal | - | Exibe níveis gráficos no display |


