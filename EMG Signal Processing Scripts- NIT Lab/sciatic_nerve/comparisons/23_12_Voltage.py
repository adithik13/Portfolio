from SDS00023_Voltage import plot_subplot1  # Import the functions from each file that correspond to the plots 
from SDS00012_Voltage import plot_subplot2

import matplotlib.pyplot as plt  # Import matplotlib to create visualizations 

# Plot ch2 of the data, as that is the representation of voltage
file_path_1 = "wu_venv/0924Sciatic/SDS00023.csv"  # Path to SDS00023 on your computer
file_path_2 = "wu_venv/csv_files/SDS00012.csv"  

# Define subplots: 2 rows & 1 column, sharing the y-axis
fig, axs = plt.subplots(2, 1, sharey=True, figsize=(10, 6))

plot_subplot1(axs[0], file_path_1)  # First plot
plot_subplot2(axs[1], file_path_2)  # Second plot 

# Remove the box around each plot
for ax in axs:
    ax.spines['top'].set_visible(False)     
    ax.spines['right'].set_visible(False)   
    ax.spines['left'].set_visible(False)    
    ax.spines['bottom'].set_visible(False)  

plt.tight_layout()  # Prevent overlapping components

# Function to add scale bars
def add_scale_bar(ax, x_length, y_length, x_label, y_label, 
                 x_start=-0.0005, y_start=-0.05, 
                 x_offset_label=-0.0003, x_label_offset=-0.05):
    # Horizontal line 
    ax.plot([x_start, x_start + x_length], [y_start, y_start], color='black', lw=1.5)
    ax.text(
        x_start + x_length / 2, 
        y_start + x_label_offset,  # Adjusted vertical offset
        x_label, 
        ha='center', 
        va='top', 
        fontsize=10
    )
    
    # Vertical line 
    ax.plot([x_start, x_start], [y_start, y_start + y_length], color='black', lw=1.5)
    ax.text(
        x_start + x_offset_label, 
        y_start + y_length / 2, 
        y_label,  
        ha='center', 
        va='center', 
        fontsize=10
    )

# Adding scale bars to each subplot

# Scale bar for the bottom subplot (axs[1])
add_scale_bar(
    axs[1], 
    x_length=0.0005, 
    y_length=0.5, 
    x_label='0.5 ms', 
    y_label='500 mV', 
    x_start=0.001, 
    y_start=-0.9, 
    x_offset_label=-0.0003, 
    x_label_offset=-0.05  # Increased negative vertical offset
)

# Optional: Uncomment and adjust the top subplot's scale bar as needed
# add_scale_bar(
#     axs[0], 
#     x_length=0.001, 
#     y_length=0.5, 
#     x_label='1 ms', 
#     y_label='500 mV', 
#     x_start=0.030, 
#     y_start=-0.8, 
#     x_offset_label=-0.0003, 
#     x_label_offset=-0.05  # Adjust as needed
# )

# Crop the graph to desired limits
axs[0].set_xlim(0.03, 0.040)  # Include negative space
axs[1].set_xlim(0.001, 0.007)  

# Add margins to plot
plt.subplots_adjust(left=0.15, right=0.85, bottom=0.15, top=0.85)  # Reduced left margin slightly

# Save and show the plots
plt.savefig('23_12_Voltage_Plot.png', dpi=1000, bbox_inches='tight')  # High resolution
plt.show()
