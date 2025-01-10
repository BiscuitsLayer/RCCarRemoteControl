import time
import serial

# Configure the serial connection
# TODO: also can choose port automatically (by searching the name of paired device) 
ser = serial.Serial('COM13', baudrate=9600, timeout=1)

def write_command(time_stamp, steer, speed):
    # Format the command as "time,steer,speed\n"
    command = f"{time_stamp},{steer},{speed}\n"
    ser.write(command.encode())  # Write the command as bytes
    print(f"Sent: {command.strip()}")  # For debugging

try:
    # Send commands based on user input
    while True:
        # Get user input for steer and speed
        try:
            steer = int(input("Enter steer value: "))
            speed = int(input("Enter speed value: "))
        except ValueError:
            print("Please enter valid integer values for steer and speed.")
            continue

        # Get the current time in milliseconds
        current_time = int(time.time() * 1000)

        # Write the command
        write_command(current_time, steer, speed)

        # Optional: add a delay if needed
        # time.sleep(1)

except KeyboardInterrupt:
    print("Stopping script.")
finally:
    ser.close()
