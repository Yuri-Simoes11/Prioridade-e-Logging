#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/device.h>
#include <stdint.h>

// Configuração do botão
#define BUTTON_NODE DT_NODELABEL(user_button_0)

static const struct gpio_dt_spec button =
    GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

/* Estrutura para registrar o callback da interrupcao */
static struct gpio_callback button_cb;

// --- Configuração de LEDs via DeviceTree ---
#define LED_A_NODE DT_ALIAS(led0)  // LED verde
#define LED_B_NODE DT_ALIAS(led2)  // LED vermelho

static const struct gpio_dt_spec ledA = GPIO_DT_SPEC_GET(LED_A_NODE, gpios);
static const struct gpio_dt_spec ledB = GPIO_DT_SPEC_GET(LED_B_NODE, gpios);

// --- Prioridades e tempos ---
#define PRIO_THREAD_A 5   // Maior prioridade (número menor)
#define PRIO_THREAD_B 7   // Menor prioridade
#define TEMPO_A_MS   1500   // Thread A dorme
#define TEMPO_B_MS   1000   // Thread B dorme

// Variáveis para a interrupção
#define DEBOUNCE_MS 500
static int64_t last_press_time;

// ----------------------------------------------------
// INTERRUPÇÃO ISR
// ----------------------------------------------------
static void button_pressed(const struct device *port,
                           struct gpio_callback *cb,
                           gpio_port_pins_t pins)
{
    int64_t now = k_uptime_get();

    if (now - last_press_time >= DEBOUNCE_MS) {
        last_press_time = now;
        printk("\n>>> ISR: BOTAO PRESSIONADO! <<<\n \n");
    }
}

// ----------------------------------------------------
// THREAD A — Tarefa curta, alta prioridade
// ----------------------------------------------------
void thread_A(void *p1, void *p2, void *p3)
{
    while (1) {

        gpio_pin_set_dt(&ledA, 1);  // Liga LED verde

        // Mede o tempo de processamento
        uint32_t inicio = k_cycle_get_32();

        // Simula processamento rápido 
        for (volatile int i = 0; i < 200000; i++) {}

        uint32_t fim = k_cycle_get_32();

        uint32_t ciclos = fim - inicio;
        uint64_t tempo_us = k_cyc_to_us_floor64(ciclos);

        printk("Thread A: %u ciclos, %llu us\n",
               ciclos, (unsigned long long)tempo_us);
        
        gpio_pin_set_dt(&ledA, 0);  // Desliga LED verde
   
        k_msleep(TEMPO_A_MS);       // Dorme — libera CPU


    }
}

// ----------------------------------------------------
// THREAD B — Tarefa longa, baixa prioridade
// ----------------------------------------------------
void thread_B(void *p1, void *p2, void *p3)
{
    while (1) {
        gpio_pin_set_dt(&ledB, 1);  // Liga LED vermelho

        uint32_t inicio = k_cycle_get_32();

        // Simula processamento mais longo 
        for (volatile int i = 0; i < 10000000; i++) {
            if (i % 100000 == 0) {
                // Aqui não imprimimos nada — apenas ocupamos a CPU
            }
        }

        uint32_t fim = k_cycle_get_32();

        uint32_t ciclos = fim - inicio;
        uint64_t tempo_us = k_cyc_to_us_floor64(ciclos);

        printk("Thread B: %u ciclos, %llu us\n",
        ciclos, (unsigned long long)tempo_us);

        gpio_pin_set_dt(&ledB, 0);  // Desliga LED vermelho
        k_msleep(TEMPO_B_MS);       // Dorme um pouco
    }
}

// ----------------------------------------------------
// Definição das threads
// ----------------------------------------------------
K_THREAD_DEFINE(a_tid, 512, thread_A, NULL, NULL, NULL,
                PRIO_THREAD_A, 0, 0);
K_THREAD_DEFINE(b_tid, 512, thread_B, NULL, NULL, NULL,
                PRIO_THREAD_B, 0, 0);

// ----------------------------------------------------
// Função principal
// ----------------------------------------------------
void main(void){

    int ret;
    // Inicializa GPIOs dos LEDs
    if (!device_is_ready(ledA.port) || !device_is_ready(ledB.port)) {
        return;
    }
    gpio_pin_configure_dt(&ledA, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledB, GPIO_OUTPUT_INACTIVE);

    // Verificar se o botão está disponível
    if (!gpio_is_ready_dt(&button)) {
        printk("ERRO: Botão não está disponível!\n");
        return;
    }
    
    // Configurar botão como entrada COM pull-up forçado
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);
    
    printk("Botão configurado: Porto %s, Pino %d (COM PULL-UP)\n", 
           button.port->name, button.pin);

    /* Inicializa a estrutura do callback */
    gpio_init_callback(&button_cb,
                       button_pressed,
                       BIT(button.pin));

    /* Registra o callback na porta GPIO */
    ret = gpio_add_callback(button.port, &button_cb);

    if (ret < 0) {
        printk("Erro ao registrar callback: %d\n", ret);
        return 0;
    }

    /* Habilita a interrupcao na transicao para o estado ativo */
    ret = gpio_pin_interrupt_configure_dt(
        &button, GPIO_INT_EDGE_TO_ACTIVE);

    if (ret < 0) {
        printk("Erro ao configurar interrupcao: %d\n", ret);
        return 0;
    }

    printk("=== Demonstração de Preempção com LEDs ===\n");
    printk("Thread A (vermelho): prioridade %d\n", PRIO_THREAD_A);
    printk("Thread B (verde): prioridade %d\n", PRIO_THREAD_B);

    while (1) {
        k_sleep(K_FOREVER); // Main dorme para liberar CPU.
    }
}
