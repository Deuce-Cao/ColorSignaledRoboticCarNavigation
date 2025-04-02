import serial

# Configuration
SERIAL_PORT = 'COM3'  # Replace with your port
BAUD_RATE = 115200
TIMEOUT = 2

def read_serial_data(ser, start_marker, end_marker):
    """
    Reads data from the serial port between start and end markers.
    Converts hexadecimal strings into raw binary data.
    """
    data = bytearray()
    collecting = False
    while True:
        line = ser.readline()
        # print(f"Raw line: {line}")  # Debug: Print raw serial data
        try:
            decoded_line = line.decode('utf-8').strip()
            if decoded_line == start_marker:
                collecting = True
                print("Start collecting data")
                data = bytearray()  # Start collecting data
            elif decoded_line == end_marker:
                print("End collecting data")
                break  # Stop collecting data
            elif collecting:
                # Convert hexadecimal string to raw binary data
                data.extend(bytearray.fromhex(decoded_line))
        except UnicodeDecodeError as e:
            print(f"Decode error: {e}")  # Debug: Print decode error
    return data

def read_raw_bytes_from_serial(ser):
    """
    Reads the image data sent by sendImageToPC.
    """
    print("Waiting for image data...")

    # Wait for START marker
    while True:
        line = ser.read_until(b'\n').decode('utf-8').strip()
        if line == "START":
            break

    # Read metadata
    width = int(ser.read_until(b'\n').decode('utf-8').strip())
    height = int(ser.read_until(b'\n').decode('utf-8').strip())
    fb_len = int(ser.read_until(b'\n').decode('utf-8').strip())

    print(f"Image metadata: width={width}, height={height}, length={fb_len}")

    # Read image data
    received = 0
    img_data = bytearray()
    while received < fb_len:
        chunk = ser.read(fb_len - received)
        if not chunk:
            break  # Timeout
        img_data.extend(chunk)
        received += len(chunk)

    # Verify END marker
    end_marker = ser.read_until(b'\n').decode('utf-8').strip()
    if len(img_data) != fb_len or end_marker != "END":
        raise ValueError("Frame incomplete or corrupted")

    print("Image data received successfully")
    return width, height, fb_len, img_data

def compare_raw_bytes(raw1, raw2):
    """
    Compares two raw byte arrays and prints mismatches.
    """
    if len(raw1) != len(raw2):
        print(f"Data length mismatch: {len(raw1)} vs {len(raw2)}")
        return False

    mismatches = 0
    for i, (byte1, byte2) in enumerate(zip(raw1, raw2)):
        if byte1 != byte2:
            print(f"Mismatch at byte {i}: {byte1:#04x} vs {byte2:#04x}")
            mismatches += 1

    if mismatches == 0:
        print("All raw bytes match!")
        return True
    else:
        print(f"Total mismatches: {mismatches}")
        return False

# Main script
if __name__ == "__main__":
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)

    try:
        # Read raw bytes from sendImageToPC
        width, height, fb_len, raw_bytes_from_sendImageToPC = read_raw_bytes_from_serial(ser)

        # Read raw bytes from Serial.printf
        print("Waiting for raw bytes from Serial.printf...")
        raw_bytes_from_color_detector = read_serial_data(ser, "RAW_START", "RAW_END")

        # Compare the two raw byte datasets
        print("Comparing raw bytes...")
        compare_raw_bytes(raw_bytes_from_sendImageToPC, raw_bytes_from_color_detector)

    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        ser.close()