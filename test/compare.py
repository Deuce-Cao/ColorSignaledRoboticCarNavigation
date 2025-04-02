import serial
import numpy as np

# Configuration
SERIAL_PORT = 'COM3'  # Replace with your port
BAUD_RATE = 115200
TIMEOUT = 2

def read_serial_rgb888(ser, start_marker, end_marker):
    """
    Reads RGB888 data sent by the ColorDetector between start and end markers.
    """
    rgb_data = []
    collecting = False
    while True:
        line = ser.readline()
        try:
            decoded_line = line.decode('utf-8').strip()
            if decoded_line == start_marker:
                collecting = True
                print("Start collecting RGB888 data")
                rgb_data = []  # Start collecting data
            elif decoded_line == end_marker:
                print("End collecting RGB888 data")
                break  # Stop collecting data
            elif collecting:
                # Parse the RGB values from the line
                r, g, b = map(int, decoded_line.split(","))
                rgb_data.append((r, g, b))
        except (UnicodeDecodeError, ValueError) as e:
            print(f"Error parsing line: {line}, Error: {e}")
    return rgb_data

def parse_rgb565_to_rgb888(fb_buf):
    """
    Converts raw RGB565 data to RGB888 format.
    """
    rgb888 = []
    for i in range(0, len(fb_buf), 2):
        pixel = (fb_buf[i] << 8) | fb_buf[i + 1]
        r = ((pixel >> 11) & 0x1F) * 255 // 31
        g = ((pixel >> 5) & 0x3F) * 255 // 63
        b = (pixel & 0x1F) * 255 // 31
        rgb888.append((r, g, b))
    return rgb888

def read_image_from_serial(ser):
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
    return width, height, img_data

def compare_rgb_data(rgb1, rgb2):
    """
    Compares two lists of RGB values and prints mismatches.
    """
    if len(rgb1) != len(rgb2):
        print(f"Data length mismatch: {len(rgb1)} vs {len(rgb2)}")
        return False

    mismatches = 0
    for i, (pixel1, pixel2) in enumerate(zip(rgb1, rgb2)):
        if pixel1 != pixel2:
            print(f"Mismatch at pixel {i}: {pixel1} vs {pixel2}")
            mismatches += 1

    if mismatches == 0:
        print("All pixels match!")
        return True
    else:
        print(f"Total mismatches: {mismatches}")
        return False

# Main script
if __name__ == "__main__":
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)

    try:
        # Read raw bytes from sendImageToPC
        print("Reading raw bytes from sendImageToPC...")
        width, height, read_image_from_sendImageToPC = read_image_from_serial(ser)

        # Convert raw RGB565 data to RGB888
        rgb888_from_sendImageToPC = parse_rgb565_to_rgb888(read_image_from_sendImageToPC)

        # Read RGB888 data from ColorDetector
        print("Reading RGB888 data from ColorDetector...")
        rgb888_from_color_detector = read_serial_rgb888(ser, "RGB_START", "RGB_END")

        # Compare the two RGB888 datasets
        print("Comparing RGB888 data...")
        compare_rgb_data(rgb888_from_sendImageToPC, rgb888_from_color_detector)

    except KeyboardInterrupt:
        print("Exiting...")
    finally:
        ser.close()