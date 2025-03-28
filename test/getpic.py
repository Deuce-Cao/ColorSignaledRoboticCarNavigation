import serial
from datetime import datetime
import time
from PIL import Image

# Configuration
SERIAL_PORT = 'COM3'  # Replace with your port (COM3, /dev/ttyUSB0, etc.)
BAUD_RATE = 115200
IMAGE_WIDTH = 160
IMAGE_HEIGHT = 120
SAVE_DIR = 'captured_frames'

# Create save directory
import os
os.makedirs(SAVE_DIR, exist_ok=True)

# RGB565 to RGB888 conversion
def rgb565_to_rgb888(data):
    pixels = []
    for i in range(0, len(data), 2):
        pixel = (data[i] << 8) | data[i+1]
        r = (pixel >> 11) & 0x1F
        g = (pixel >> 5) & 0x3F
        b = pixel & 0x1F
        pixels.append((r << 3, g << 2, b << 3))
    return pixels

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
buffer = bytearray()
state = "WAIT_HEADER"
frame_data = bytearray()

try:
    while True:
        # Read serial data
        data = ser.read(ser.in_waiting or 1)
        buffer.extend(data)

        # State machine for frame parsing
        while True:
            if state == "WAIT_HEADER":
                # Look for header bytes 0xAA 0xBB
                if len(buffer) >= 2:
                    if buffer[0] == 0xAA and buffer[1] == 0xBB:
                        print("Header found")
                        buffer = buffer[2:]
                        frame_data = bytearray()
                        state = "READ_FRAME"
                        
                    else:
                        buffer.pop(0)
                else:
                    break

            elif state == "READ_FRAME":
                # Look for footer 0xCC 0xDD
                footer_pos = buffer.find(bytes([0xCC, 0xDD]))
                if footer_pos >= 0:
                    # Extract frame data and checksum
                    frame_data.extend(buffer[:footer_pos])
                    buffer = buffer[footer_pos+2:]
                    
                    # Get checksum (last byte after footer)
                    if len(buffer) >= 1:
                        received_checksum = buffer[0]
                        buffer = buffer[1:]
                        
                        # Verify checksum
                        calculated_checksum = 0
                        for byte in frame_data:
                            calculated_checksum ^= byte
                            
                        if calculated_checksum == received_checksum:
                            # Convert to image
                            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
                            filename = f"{SAVE_DIR}/frame_{timestamp}.png"
                            
                            # Convert RGB565 to Image
                            pixels = rgb565_to_rgb888(frame_data)
                            img = Image.new('RGB', (IMAGE_WIDTH, IMAGE_HEIGHT))
                            img.putdata(pixels)
                            img.save(filename)
                            print(f"Saved {filename}")
                        else:
                            print("Checksum error!")
                        
                        frame_data = bytearray()
                        state = "WAIT_HEADER"
                    else:
                        break
                else:
                    # Add to frame data if footer not found
                    frame_data.extend(buffer)
                    buffer.clear()
                    break

except KeyboardInterrupt:
    ser.close()
    print("\nStopped")