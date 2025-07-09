#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "font.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define VRX_PIN 26

// Array para armazenar os valores de tensão para plotar a curva
float tensao_history[128];
int history_index = 0;

// Filtro de média móvel para suavizar os dados
#define FILTER_SIZE 8
float adc_buffer[FILTER_SIZE];
int buffer_index = 0;

// Função para calcular média móvel
float smooth_adc_value(uint16_t raw_value) {
    // Adiciona o novo valor ao buffer
    adc_buffer[buffer_index] = (float)raw_value;
    buffer_index = (buffer_index + 1) % FILTER_SIZE;
    
    // Calcula a média dos valores no buffer
    float sum = 0.0f;
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += adc_buffer[i];
    }
    
    return sum / FILTER_SIZE;
}

int main()
{
  // Inicialização do I2C
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // Inicialização do ADC
  adc_init();
  adc_gpio_init(VRX_PIN);

  // Inicialização do display
  ssd1306_t ssd;
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
  ssd1306_config(&ssd);
  ssd1306_send_data(&ssd);

  // Limpa o display
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  // Inicializa o array de histórico com valores médios
  for (int i = 0; i < 128; i++) {
    tensao_history[i] = 1.65f; // Valor médio (3.3V / 2)
  }
  
  // Inicializa o buffer do filtro com valores médios
  for (int i = 0; i < FILTER_SIZE; i++) {
    adc_buffer[i] = 2048.0f; // Valor médio do ADC (4095 / 2)
  }

  while (true)
  {
    // Lê o valor do ADC (0 a 4095)
    adc_select_input(0);
    uint16_t vrx_value = adc_read();
    
    // Aplica o filtro de média móvel
    float smoothed_adc = smooth_adc_value(vrx_value);
    
    // Converte para tensão (0 a 3.3V)
    float tensao = smoothed_adc * 3.3f / 4095.0f;
    
    // Armazena no histórico
    tensao_history[history_index] = tensao;
    history_index = (history_index + 1) % 128;

    // Limpa o display
    ssd1306_fill(&ssd, false);

    // Desenha o título
    ssd1306_draw_string(&ssd, "TENSAO JOYSTICK", 0, 0);
    
    // Mostra o valor atual da tensão
    char tensao_str[20];
    sprintf(tensao_str, "%.2fV", tensao);
    ssd1306_draw_string(&ssd, tensao_str, 0, 16);

    // Plota a curva de tensão com suavização
    // A área de plotagem vai de y=32 até y=62 (30 pixels de altura)
    // Começa a partir do eixo Y (x=20) e termina antes da legenda do eixo X
    for (int x = 0; x < 100; x++) { // 100 = 120 - 20 (largura até antes da legenda - margem do eixo)
      int hist_idx = (history_index - 100 + x + 128) % 128;
      float valor = tensao_history[hist_idx];
      
      // Converte tensão (0-3.3V) para posição Y (32-62)
      int y = 62 - (int)((valor / 3.3f) * 30);
      
      // Garante que y está dentro dos limites (não passa do eixo X)
      if (y < 32) y = 32;
      if (y > 62) y = 62;
      
      // Ajusta a posição X para começar após o eixo Y e terminar antes da legenda
      int plot_x = x + 20;
      
      // Garante que não passa além do eixo Y (x=20)
      if (plot_x < 20) plot_x = 20;
      
      // Desenha o ponto na curva
      ssd1306_pixel(&ssd, plot_x, y, true);
      
      // Adiciona alguns pixels adjacentes para suavizar a linha
      if (x > 0) {
        int prev_hist_idx = (history_index - 100 + x - 1 + 128) % 128;
        float prev_valor = tensao_history[prev_hist_idx];
        int prev_y = 62 - (int)((prev_valor / 3.3f) * 30);
        if (prev_y < 32) prev_y = 32;
        if (prev_y > 62) prev_y = 62;
        
        // Desenha pixels intermediários para suavizar
        if (abs(y - prev_y) <= 2) {
          for (int py = (prev_y < y ? prev_y : y); py <= (prev_y > y ? prev_y : y); py++) {
            if (py >= 32 && py <= 62) {
              ssd1306_pixel(&ssd, plot_x, py, true);
            }
          }
        }
      }
    }

    // Desenha os eixos do gráfico
    ssd1306_vline(&ssd, 20, 32, 62, true); // Eixo Y (Tensão) - linha vertical
    ssd1306_hline(&ssd, 20, 127, 62, true); // Eixo X (Tempo) - linha horizontal
    
    // Desenha legendas dos eixos
    ssd1306_draw_string(&ssd, "T", 110, 55); // Tempo (eixo X)

    // Atualiza o display
    ssd1306_send_data(&ssd);

    sleep_ms(500); // Atualiza mais rapidamente para suavidade
  }
}
