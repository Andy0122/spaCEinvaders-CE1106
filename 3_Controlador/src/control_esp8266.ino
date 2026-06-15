/*
 * Controlador para el Botón de Movimiento y Disparo del Juego
 * 
 * Funcionalidades:
 * - Botón de Movimiento (D1):
 *   - Toque simple: Mueve a la izquierda (envía 'L')
 *   - Doble toque: Mueve a la derecha (envía 'R')
 *   Mantener presionado: Envía ráfagas continuas ('L' o 'R' según el último toque)
 * 
 * - Botón de Disparo (D2):
 *   - Toque simple: Dispara una bala (envía 'S')
 *   Mantener presionado: Dispara ráfagas continuas ('S')
 */

 /*
#define BOTON_MOVIMIENTO_PIN D1
#define BOTON_DISPARO_PIN D2

// --- Variables para el Botón de Movimiento ---
int movementButtonState = HIGH;
int lastMovementButtonState = HIGH;
unsigned long lastDebounceTime = 0;
#define DEBOUNCE_DELAY 50

int pressCount = 0;
unsigned long lastPressTime = 0;
#define DOUBLE_PRESS_TIMEOUT 250 // Tiempo para decidir si es 1 o 2 toques
#define HOLD_REPEAT_DELAY 50     // Velocidad a la que se envían las letras al mantener presionado

unsigned long lastHoldSendTime = 0;
bool actionSentForThisClick = false; // Para saber si ya mandamos un toque rápido

// --- Variables para el Botón de Disparo ---
int fireButtonState = HIGH;
int lastFireButtonState = HIGH;
unsigned long lastFireDebounceTime = 0;
unsigned long lastFireSendTime = 0;
#define FIRE_REPEAT_DELAY 200 // Milisegundos entre cada bala si se mantiene presionado disparar

void setup() {
  Serial.begin(9600);
  pinMode(BOTON_MOVIMIENTO_PIN, INPUT_PULLUP);
  pinMode(BOTON_DISPARO_PIN, INPUT_PULLUP);
}

void loop() {
  handleMovementButton();
  handleFireButton();
}

void handleMovementButton() {
  int reading = digitalRead(BOTON_MOVIMIENTO_PIN);

  // 1. Antirrebote básico
  if (reading != lastMovementButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != movementButtonState) {
      movementButtonState = reading;
      
      // Si el botón acaba de ser PRESIONADO
      if (movementButtonState == LOW) {
        pressCount++;
        lastPressTime = millis();
        actionSentForThisClick = false; // Reiniciamos el estado para el nuevo toque
      }
    }
  }
  lastMovementButtonState = reading;

  // 2. Lógica de "Mantener" y "Click + Mantener"
  if (pressCount == 1) {
    // Si ha pasado el tiempo para un doble click y el botón SIGUE presionado (Hold 1 toque)
    if (movementButtonState == LOW && (millis() - lastPressTime) > DOUBLE_PRESS_TIMEOUT) {
      if ((millis() - lastHoldSendTime) > HOLD_REPEAT_DELAY) {
        Serial.print('L'); // Enviar ráfaga de Izquierda
        lastHoldSendTime = millis();
        actionSentForThisClick = true;
      }
    } 
    // Si soltó el botón antes de hacer el doble click (Toque simple)
    else if (movementButtonState == HIGH && (millis() - lastPressTime) > DOUBLE_PRESS_TIMEOUT) {
      if (!actionSentForThisClick) {
        Serial.print('L'); // Enviar un solo pasito a la izquierda
      }
      pressCount = 0; // Reiniciamos todo
    }
  } 
  else if (pressCount >= 2) {
    // Si ya van 2 toques y el botón está presionado (Click + Hold)
    if (movementButtonState == LOW) {
      if ((millis() - lastHoldSendTime) > HOLD_REPEAT_DELAY) {
        Serial.print('R'); // Enviar ráfaga de Derecha
        lastHoldSendTime = millis();
        actionSentForThisClick = true;
      }
    }
    // Si soltó después de 2 toques
    else if (movementButtonState == HIGH && (millis() - lastPressTime) > DOUBLE_PRESS_TIMEOUT) {
      if (!actionSentForThisClick) {
        Serial.print('R'); // Enviar un solo pasito a la derecha
      }
      pressCount = 0; // Reiniciamos todo
    }
  }
}

void handleFireButton() {
  int reading = digitalRead(BOTON_DISPARO_PIN);

  if (reading != lastFireButtonState) {
    lastFireDebounceTime = millis();
  }

  if ((millis() - lastFireDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != fireButtonState) {
      fireButtonState = reading;
    }
  }
  lastFireButtonState = reading;

  // Lógica de Fuego (Toque simple o Fuego Automático al mantener)
  if (fireButtonState == LOW) {
    if ((millis() - lastFireSendTime) > FIRE_REPEAT_DELAY) {
      Serial.print('S');
      lastFireSendTime = millis();
    }
  }
}

*/