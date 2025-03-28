import serial
from datetime import datetime
import os
from PIL import Image
import csv

# Configuration
SERIAL_PORT = 'COM3'  # Replace with your port
BAUD_RATE = 115200
SAVE_DIR = 'captured_frames'
os.makedirs(SAVE_DIR, exist_ok=True)

def rgb565_to_rgb888(data):
    pixels = []
    for i in range(0, len(data), 2):
        if i+1 >= len(data):
            break  # Handle odd-length data
        pixel = (data[i] << 8) | data[i+1]
        r = (pixel >> 11) & 0x1F
        g = (pixel >> 5) & 0x3F
        b = pixel & 0x1F
        pixels.append((r << 3, g << 2, b << 3))
    return pixels

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
buffer = bytearray()

try:
    while True:
        # Wait for START marker
        line = ser.read_until(b'\n').decode().strip()
        if line != "START":
            continue
            
        # Read metadata
        width = int(ser.read_until(b'\n').decode().strip())
        height = int(ser.read_until(b'\n').decode().strip())
        fb_len = int(ser.read_until(b'\n').decode().strip())
        
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
        end_marker = ser.read_until(b'\n').decode().strip()
        
        if len(img_data) != fb_len or end_marker != "END":
            print("Frame incomplete or corrupted, skipping...")
            continue
        
        # Convert pixels to RGB888
        pixels = rgb565_to_rgb888(img_data)

        # Save pixels to a CSV file
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        csv_filename = os.path.join(SAVE_DIR, f"frame_{timestamp}.csv")
        with open(csv_filename, mode='w', newline='') as csv_file:
            writer = csv.writer(csv_file)
            writer.writerow(["R", "G", "B"])  # Write header
            writer.writerows(pixels)  # Write pixel data
        print(f"Saved pixel data to {csv_filename}")

        # Optionally save the image as well
        img = Image.new('RGB', (width, height))
        img.putdata(pixels)
        img_filename = os.path.join(SAVE_DIR, f"frame_{timestamp}.png")
        img.save(img_filename)
        print(f"Saved image to {img_filename} ({width}x{height})")

except KeyboardInterrupt:
    ser.close()
    print("\nStopped")