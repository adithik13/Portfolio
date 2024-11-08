from SDS00023_Current import plot_subplot1  # import the functions from each file that correspond to the plots 
from SDS00027_Current import plot_subplot2

import matplotlib.pyplot as plt  # import matplotlib to create visualizations 

# plot ch2 of the data, as that is the representation of voltage
file_path_1 = "wu_venv/0924Sciatic/SDS00023.csv"  # path to SDS00009 on my computer, replace with path on yours to run 
file_path_2 = "wu_venv/0924Sciatic/SDS00027.csv"  

# define subplots, since we are putting 2 graphs on one screen 
fig, axs = plt.subplots(2, 1, sharey=True, figsize=(10, 6))  # 2 rows & 1 column, sharing an x axis

plot_subplot1(axs[0], file_path_1)  # let axs[0] be the first plot, of SDS00009.csv
plot_subplot2(axs[1], file_path_2)  # let axs[1] be the second plot, of SDS00012.csv 

# loop through each plot to remove the box: 
for ax in axs:
    ax.spines['top'].set_visible(False)     
    ax.spines['right'].set_visible(False)   
    ax.spines['left'].set_visible(False)    
    ax.spines['bottom'].set_visible(False)  

plt.tight_layout()  # prevents components from overlapping, not necessary, but a good precaution 

# add a scale bar based off x and y axis measurements 
def add_scale_bar(ax, x_length, y_length, x_label, y_label, x_start, y_start):
    # horizontal line 
    ax.plot([x_start, x_start + x_length], [y_start, y_start], color='black', lw=1.5)
    ax.text(x_start + x_length / 2, y_start - y_length / 3, x_label, 
            ha='center', va='top', fontsize=10)  # Adjusted y position
    
    # vertical line 
    ax.plot([x_start, x_start], [y_start, y_start + y_length], color='black', lw=1.5)
    ax.text(x_start - x_length / 3, y_start + y_length / 2, y_label,  
            ha='center', va='center', fontsize=10)  

# Adding a scale bar to the second subplot with defined lengths and labels for x and y

# Scale bar for the bottom subplot (axs[1])
# Adjust x_start, y_start, x_length, y_length accordingly based on the scale of your data
add_scale_bar(axs[1], x_length=0.0025, y_length=1, x_label='2.5 ms', y_label='1 A', x_start=0.025, y_start=-2.5)

# crop the graph to desired limits
axs[0].set_xlim(0.025, 0.050)  # include negative space
axs[1].set_xlim(0.025, 0.050)  

# add margins to plot
plt.subplots_adjust(left=0.15, right=0.85, bottom=0.15, top=0.85)  # Reduced left margin slightly

#plt.grid(True)

#fig.suptitle('SDS00023 Vs SDS00027 Current', fontsize=16)  # Adjust the title text and fontsize as needed

# save and show the plots
plt.savefig('23_27_Current_Plot.png', dpi=1000, bbox_inches='tight')  # dpi=1000 for very high resolution
plt.show()
