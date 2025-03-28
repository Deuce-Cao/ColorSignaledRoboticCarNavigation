import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import colorsys

#Load data
#df = pd.read_csv('captured_frames/frame_20250328_135552_167830.csv')
#df = pd.read_csv('captured_frames/frame_20250328_135557_317341.csv')
#df = pd.read_csv('captured_frames/frame_20250328_135607_617282.csv')
#df = pd.read_csv('captured_frames/frame_20250328_135602_469441.csv')

# QQVGA dimensions
width, height = 160, 120

# Calculate center 50% region
x_start, x_end = width // 4, 3 * width // 4  # Middle 50% horizontally
y_start, y_end = height // 4, 3 * height // 4  # Middle 50% vertically

# Reshape the data into a 2D array (row-major order)
pixels = df[['R', 'G', 'B']].values.reshape((height, width, 3))

# Extract the center 50% region
center_pixels = pixels[y_start:y_end, x_start:x_end].reshape(-1, 3)

# Convert the center pixels to a DataFrame
center_df = pd.DataFrame(center_pixels, columns=['R', 'G', 'B'])

# print the count for R value > 200, count for G > 100, count for B > 200
print("Count of R > 200:", len(center_df[center_df['R'] > 200]))
print("Count of G > 150:", len(center_df[center_df['G'] > 150]))
print("Count of B > 200:", len(center_df[center_df['B'] > 200]))

# # Create histograms
# plt.figure(figsize=(12, 6))

# # Plot each channel with different colors and transparency
# plt.hist(center_df['R'], bins=256, color='red', alpha=0.7, label='Red', range=(0, 255))
# plt.hist(center_df['G'], bins=256, color='green', alpha=0.7, label='Green', range=(0, 255))
# plt.hist(center_df['B'], bins=256, color='blue', alpha=0.7, label='Blue', range=(0, 255))

# # Add labels and title
# plt.title('RGB Channel Distribution', fontsize=14)
# plt.xlabel('Color Value (0-255)', fontsize=12)
# plt.ylabel('Frequency', fontsize=12)
# plt.legend()
# plt.grid(True, alpha=0.3)

# # Show plot
# plt.tight_layout()
# plt.show()





def rgb_to_hsv(row):
    """Convert RGB values (0-255) to HSV (Hue 0-360)"""
    r, g, b = [x / 255 for x in [row['R'], row['G'], row['B']]]
    h, s, v = colorsys.rgb_to_hsv(r, g, b)
    return h * 360, s, v

# Convert to HSV
center_df[['Hue', 'Sat', 'Val']] = center_df.apply(rgb_to_hsv, axis=1, result_type='expand')

#print count for (Hue < 30 + Hue > 330), count for (120 < Hue <190), count for (200 < Hue < 240) 
print("Count of Hue < 30 or Hue > 330:", len(center_df[(center_df['Hue'] < 30) | (center_df['Hue'] > 330)]))
print("Count of 120 < Hue < 190:", len(center_df[(center_df['Hue'] > 120) & (center_df['Hue'] < 190)]))
print("Count of 200 < Hue < 240:", len(center_df[(center_df['Hue'] > 200) & (center_df['Hue'] < 240)]))

# Print the average Hue value
print("Average Hue value (center 50%):", center_df['Hue'].mean())

# Create Hue histogram with color-coded bars
plt.figure(figsize=(14, 7))
n_bins = 360  # One bin per degree
bin_edges = range(0, 361, 1)

# Create histogram
counts, bins, patches = plt.hist(center_df['Hue'], bins=bin_edges, edgecolor='none')

# Color each bin according to its Hue value
norm = mcolors.Normalize(vmin=0, vmax=360)
cmap = plt.cm.get_cmap('hsv')

for bin_, patch in zip(bins[:-1], patches):
    hue = bin_ + 0.5  # Use bin center for color
    patch.set_facecolor(cmap(norm(hue)))

# Add colorbar
sm = plt.cm.ScalarMappable(cmap=cmap, norm=norm)
sm.set_array([])
cbar = plt.colorbar(sm, ax=plt.gca(), orientation='vertical')
cbar.set_label('Hue Value (Degrees)', rotation=270, labelpad=20)

# Format plot
plt.title('Hue Distribution (Center 50% of Image)', fontsize=16)
plt.xlabel('Hue Value (Degrees)', fontsize=12)
plt.ylabel('Frequency', fontsize=12)
plt.xlim(0, 360)
plt.grid(alpha=0.3)
plt.tight_layout()
plt.show()