import sys
import time
import serial
from pynput import keyboard

# Configure the serial connection
# TODO: also can choose port automatically (by searching the name of paired device) 
# COM13 выбран сейчас для взаимодействия с Arduino HC-05, подробнее 
# можно посмотреть в настройках bluetooth в Windows нажав "Подробнее"
ser = serial.Serial('COM13', baudrate=9600, timeout=1)

# Эти значения взяты из файла для Arduino
# channel 1 == steer
# channel 2 == speed
AVG_CH1 = 1555
MIN_CH1 = 1179
MAX_CH1 = 1921 

AVG_CH2 = 1560
MIN_CH2 = 1070
MAX_CH2 = 1832

cur_steer = AVG_CH1
cur_speed = AVG_CH2

def write_command(time_stamp, steer, speed):
    # Format the command as "time,steer,speed\n"
    command = f"{time_stamp},{steer},{speed}\n"
    ser.write(command.encode())  # Write the command as bytes
    print(f"Sent: {command.strip()}")  # For debugging

# Обработчики для каждой клавиши
def handle_w_pressed():
    global cur_speed
    cur_speed = AVG_CH2 + 200
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("W нажата")

def handle_s_pressed():
    global cur_speed
    cur_speed = AVG_CH2 - 400
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("S нажата")

def handle_a_pressed():
    global cur_steer
    cur_steer = AVG_CH1 + 200
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("A нажата")

def handle_d_pressed():
    global cur_steer
    cur_steer = AVG_CH1 - 200
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("D нажата")

def handle_w_released():
    global cur_speed
    cur_speed = AVG_CH2
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("W отпущена")

def handle_s_released():
    global cur_speed
    cur_speed = AVG_CH2
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("S отпущена")

def handle_a_released():
    global cur_steer
    cur_steer = AVG_CH1
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("A отпущена")

def handle_d_released():
    global cur_steer
    cur_steer = AVG_CH1
    current_time = int(time.time() * 1000)
    write_command(current_time, cur_steer, cur_speed)
    print("D отпущена")

# Словарь для хранения состояний клавиш
keys_pressed = {
    'w': False,
    'a': False,
    's': False,
    'd': False
}

def on_press(key):
    try:
        if key.char == 'w' and not keys_pressed['w']:
            keys_pressed['w'] = True
            handle_w_pressed()
        elif key.char == 'a' and not keys_pressed['a']:
            keys_pressed['a'] = True
            handle_a_pressed()
        elif key.char == 's' and not keys_pressed['s']:
            keys_pressed['s'] = True
            handle_s_pressed()
        elif key.char == 'd' and not keys_pressed['d']:
            keys_pressed['d'] = True
            handle_d_pressed()
        elif key.char == 'q':
            print("\nПрограмма завершена.")
            sys.exit(0)  # Выход из программы
    except AttributeError:
        pass

def on_release(key):
    try:
        if key.char == 'w' and keys_pressed['w']:
            keys_pressed['w'] = False
            handle_w_released()
        elif key.char == 'a' and keys_pressed['a']:
            keys_pressed['a'] = False
            handle_a_released()
        elif key.char == 's' and keys_pressed['s']:
            keys_pressed['s'] = False
            handle_s_released()
        elif key.char == 'd' and keys_pressed['d']:
            keys_pressed['d'] = False
            handle_d_released()
    except AttributeError:
        pass

def main():
    with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
        listener.join()

if __name__ == "__main__":
    main()
