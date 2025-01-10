#include <Servo.h>

#define SERIAL_PORT_SPEED 9600

#define PRINT_COOLDOWN_MILLIS 1000
bool need_to_write_intervention_detected_message = false;
bool need_to_write_car_is_controlled_by_remote_message = false;

#define INTERVENTION_DELTA 100
#define INTERVENTION_COOLDOWN_MILLIS 10000

#define RC_CH1_INPUT 3 // steer
#define RC_CH1_OUTPUT 10 // steer

#define RC_CH2_INPUT 6 // speed
#define RC_CH2_OUTPUT 9 // speed

unsigned long cur_time = 0;
unsigned long last_print_time = 0;
unsigned long last_intervention_time = 0;

Servo servo_channel_1; // steer
Servo servo_channel_2; // speed

#define AVG_CH1 1555
#define MIN_CH1 1179
#define MAX_CH1 1921 

#define AVG_CH2 1560
#define MIN_CH2 1070
#define MAX_CH2 1832

struct Command {
  unsigned long time;
  int steer;
  int speed;
};

// Function to check if there was a command from remote
bool remoteIntervention(int value, int avg_value) {
  // no signal from remote
  if (value == 0) {
    return false;
  }

  // if we received a strong value from remote, update last intervention time
  if (abs(value - avg_value) > INTERVENTION_DELTA) {
    need_to_write_intervention_detected_message = true;
    last_intervention_time = cur_time;
    return true;
  }

  // if intervention cooldown is still active but weak,
  // count it as an intervention (so car can be controlled by remote)
  // but do not update last_intervention_time
  if (cur_time - last_intervention_time < INTERVENTION_COOLDOWN_MILLIS) {
    need_to_write_car_is_controlled_by_remote_message = true;
    return true;
  }

  return false;
}

// Function to parse a command string into a Command structure
bool parseCommand(const String& commandString, struct Command& cmd) {
  int firstComma = commandString.indexOf(',');
  int secondComma = commandString.indexOf(',', firstComma + 1);

  if (firstComma == -1 || secondComma == -1) {
    // Invalid format, return false
    return false;
  }

  // Extract and convert each part of the command
  cmd.time = commandString.substring(0, firstComma).toInt();
  cmd.steer = commandString.substring(firstComma + 1, secondComma).toInt();
  cmd.speed = commandString.substring(secondComma + 1).toInt();

  return true;
}

void setup() {
  // Назначить пины D9 и D10 на 244 Гц 10bit
  // https://alexgyver.ru/lessons/pwm-overclock/
  // TCCR1A = 0b00000011;  // 10bit
  // TCCR1B = 0b00001011;  // x64 fast pwm

  // the default setting on HC-05 is typically at 9600 baud/second
  Serial.begin(SERIAL_PORT_SPEED); //ensure serial monitor is also set to 9600 baud

  pinMode(RC_CH1_INPUT, INPUT);
  servo_channel_1.attach(RC_CH1_OUTPUT, 0, 3000);

  pinMode(RC_CH2_INPUT, INPUT);
  servo_channel_2.attach(RC_CH2_OUTPUT, 0, 3000);

  cur_time = millis();
}

void loop() {
  cur_time = millis();

  // Handle control from remote (if intervention happened)
  int remote_channel_1_value = pulseIn(RC_CH1_INPUT, HIGH);
  if (remoteIntervention(remote_channel_1_value, AVG_CH1)) {
    servo_channel_1.writeMicroseconds(
      constrain(remote_channel_1_value, MIN_CH1, MAX_CH1)
    );
  }
  int remote_channel_2_value = pulseIn(RC_CH2_INPUT, HIGH);
  if (remoteIntervention(remote_channel_2_value, AVG_CH2)) {
    servo_channel_2.writeMicroseconds(
      constrain(remote_channel_2_value, MIN_CH2, MAX_CH2)
    );
  }

  // Handle control from console (or received from bluetooth)
  if (Serial.available() > 0) {
    String commandString = Serial.readStringUntil('\n');
    unsigned long millis_since_last_intervention = cur_time - last_intervention_time;

    // Ignore command if still under cooldown, otherwise apply
    if (millis_since_last_intervention < INTERVENTION_COOLDOWN_MILLIS) {
      Serial.println(
        "Command ignored because intervention happened recently\nIt happened " + 
        String(millis_since_last_intervention) + " millis ago and cooldown is " + 
        String(INTERVENTION_COOLDOWN_MILLIS)
      );
    } else {
      Command newCommand;

      if (parseCommand(commandString, newCommand)) {
        Serial.println("newCommand channel1 value: " + String(newCommand.steer));
        servo_channel_1.writeMicroseconds(
          constrain(newCommand.steer, MIN_CH1, MAX_CH1)
        );

        Serial.println("newCommand channel2 value: " + String(newCommand.speed));
        servo_channel_2.writeMicroseconds(
          constrain(newCommand.speed, MIN_CH2, MAX_CH2)
        );
      } else {
        Serial.println("Failed to parse command");
      }
    }
  }

  // Output current PWM values to console (or send via bluetooth)
  if (cur_time - last_print_time > PRINT_COOLDOWN_MILLIS) {
    Serial.println("Channel1 value: " + String(servo_channel_1.readMicroseconds()));
    Serial.println("Channel2 value: " + String(servo_channel_2.readMicroseconds()));

    if (need_to_write_car_is_controlled_by_remote_message) {
      Serial.println("Car can be controlled by remote now");
      need_to_write_car_is_controlled_by_remote_message = false;
    }
    if (need_to_write_intervention_detected_message) {
      Serial.println("New intervention from remote detected");
      need_to_write_intervention_detected_message = false;
    }
    Serial.println();

    last_print_time = cur_time;
  }
}
