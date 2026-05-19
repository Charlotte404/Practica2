#include <Arduino.h>

/* ============================
   PINES
   ============================ */
#define LED_PIN      2
#define BTN_UP_PIN   47
#define BTN_DOWN_PIN 48

/* ============================
   PARÁMETROS
   ============================ */
#define TIMER_INTERVAL_US 1000
#define DEBOUNCE_TIME_MS  20

#define FREQ_MIN 1
#define FREQ_MAX 20

/* ============================
   VARIABLES
   ============================ */

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool tick_1ms = false;

uint32_t led_counter = 0;
bool led_state = false;
uint8_t led_frequency = 2;
uint8_t last_printed_frequency = 0;

/* Debounce */
uint8_t btn_up_state = HIGH;
uint8_t btn_down_state = HIGH;

uint8_t btn_up_last = HIGH;
uint8_t btn_down_last = HIGH;

uint16_t btn_up_debounce = 0;
uint16_t btn_down_debounce = 0;

/* ============================
   ISR
   ============================ */
void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);
    tick_1ms = true;
    portEXIT_CRITICAL_ISR(&timerMux);
}

/* ============================
   SETUP
   ============================ */
void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("Sistema iniciado");
    Serial.println("Frecuencia inicial: 2 Hz");

    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, TIMER_INTERVAL_US, true);
    timerAlarmEnable(timer);
}

/* ============================
   LOOP
   ============================ */
void loop()
{
    if (tick_1ms)
    {
        portENTER_CRITICAL(&timerMux);
        tick_1ms = false;
        portEXIT_CRITICAL(&timerMux);

        /* ===== CONTROL LED ===== */
        uint32_t half_period = 500 / led_frequency;

        if (++led_counter >= half_period)
        {
            led_state = !led_state;
            digitalWrite(LED_PIN, led_state);
            led_counter = 0;
        }

        /* ===== LECTURA BOTONES ===== */
        uint8_t up = digitalRead(BTN_UP_PIN);
        uint8_t down = digitalRead(BTN_DOWN_PIN);

        // SUBIR
        if (up != btn_up_last)
            btn_up_debounce = 0;
        else if (btn_up_debounce < DEBOUNCE_TIME_MS)
            btn_up_debounce++;
        else if (up != btn_up_state)
        {
            btn_up_state = up;
            if (btn_up_state == LOW && led_frequency < FREQ_MAX)
                led_frequency++;
        }
        btn_up_last = up;

        // BAJAR
        if (down != btn_down_last)
            btn_down_debounce = 0;
        else if (btn_down_debounce < DEBOUNCE_TIME_MS)
            btn_down_debounce++;
        else if (down != btn_down_state)
        {
            btn_down_state = down;
            if (btn_down_state == LOW && led_frequency > FREQ_MIN)
                led_frequency--;
        }
        btn_down_last = down;
    }

    /* ===== IMPRIMIR SI CAMBIA FRECUENCIA ===== */
    if (led_frequency != last_printed_frequency)
    {
        Serial.print("Frecuencia actual: ");
        Serial.print(led_frequency);
        Serial.println(" Hz");

        last_printed_frequency = led_frequency;
    }
}