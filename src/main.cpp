#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>

constexpr uint8_t ONE_WIRE_PIN = 4;
constexpr uint8_t FAN_PWM_PIN = 15;
constexpr uint8_t PWM_CHANNEL = 0;
constexpr uint8_t PWM_RESOLUTION = 8;
constexpr uint32_t PWM_FREQUENCY = 25000;
constexpr float FAN_ON_TEMPERATURE = 25.5f;
constexpr float FAN_OFF_TEMPERATURE = 24.5f;
constexpr float SPEED2_TEMPERATURE = 30.0f;
constexpr float SPEED3_TEMPERATURE = 35.0f;
constexpr uint8_t SPEED1_PWM = 85;
constexpr uint8_t SPEED2_PWM = 170;
constexpr uint8_t SPEED3_PWM = 255;
constexpr unsigned long SENSOR_INTERVAL_MS = 1000;

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature temperatureSensor(&oneWire);

bool fanActive = false;
unsigned long lastSensorRead = 0;

uint8_t calculatePwm(float temperature) {
    if (!fanActive || temperature < FAN_OFF_TEMPERATURE) {
        return 0;
    }

    if (temperature >= SPEED3_TEMPERATURE) {
        return SPEED3_PWM;
    }

    if (temperature >= SPEED2_TEMPERATURE) {
        return SPEED2_PWM;
    }

    return SPEED1_PWM;
}

uint8_t currentSpeedLevel(uint8_t pwmValue) {
    if (pwmValue >= SPEED3_PWM) {
        return 3;
    }
    if (pwmValue >= SPEED2_PWM) {
        return 2;
    }
    if (pwmValue > 0) {
        return 1;
    }
    return 0;
}

void updateFanState(float temperature) {
    if (!fanActive && temperature >= FAN_ON_TEMPERATURE) {
        fanActive = true;
    } else if (fanActive && temperature < FAN_OFF_TEMPERATURE) {
        fanActive = false;
    }
}

void setup() {
    Serial.begin(115200);
    temperatureSensor.begin();
    ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(FAN_PWM_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 0);

    Serial.println("Smart Fan initialized");
}

void loop() {
    const unsigned long now = millis();
    if (now - lastSensorRead < SENSOR_INTERVAL_MS) {
        return;
    }
    lastSensorRead = now;

    temperatureSensor.requestTemperatures();
    const float temperature = temperatureSensor.getTempCByIndex(0);

    if (temperature == DEVICE_DISCONNECTED_C) {
        fanActive = false;
        ledcWrite(PWM_CHANNEL, 0);
        Serial.println("Temperature: SENSOR ERROR | PWM: 0 | Speed: 0% | Fan: OFF");
        return;
    }

    updateFanState(temperature);
    const uint8_t pwmValue = calculatePwm(temperature);
    ledcWrite(PWM_CHANNEL, pwmValue);

    const uint8_t speedPercentage = static_cast<uint8_t>((pwmValue * 100UL) / 255UL);
    const uint8_t speedLevel = currentSpeedLevel(pwmValue);
    Serial.print("Temperature: ");
    Serial.print(temperature, 1);
    Serial.print(" C | PWM: ");
    Serial.print(pwmValue);
    Serial.print(" | Speed: ");
    Serial.print(speedPercentage);
    Serial.print("% | Fan: ");
    if (speedLevel == 0) {
        Serial.println("OFF");
    } else {
        Serial.print("SPEED ");
        Serial.println(speedLevel);
    }
}
