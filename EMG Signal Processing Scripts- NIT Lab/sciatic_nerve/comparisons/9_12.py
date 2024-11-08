from SDS00009 import plot_subplot1  # import the functions from each file that correspond to the plots 
from SDS00012 import plot_subplot2

import matplotlib.pyplot as plt  # import matplotlib to create visualizations 

file_path_1 = "wu_venv/csv_files/SDS00009.csv"  # path to SDS00009 on my computer, replace with path on yours to run 
file_path_2 = "wu_venv/csv_files/SDS00012.csv"  

# define subplots, since we are putting 2 graphs on one screen 
fig, axs = plt.subplots(2, 1, sharex=True, sharey=True, figsize=(10, 6))  # 2 rows & 1 column, sharing an x axis

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
def add_scale_bar(ax, x_length, y_length, x_label, y_label):
    # start near origin
    x_start = -0.0005  
    y_start = -0.05
    
    # horizontal line 
    ax.plot([x_start, x_start + x_length], [y_start, y_start], color='black', lw=1.5)
    ax.text(x_start + x_length / 2, y_start - 0.03, x_label, 
            ha='center', va='top', fontsize=10)  # Adjusted y position
    
    # vertical line 
    ax.plot([x_start, x_start], [y_start, y_start + y_length], color='black', lw=1.5)
    ax.text(x_start - 0.0008, y_start + y_length / 2, y_label,  
            ha='center', va='center', fontsize=10)  

# adds the scale bar to the plot with defined length and width & labels for x and y 
add_scale_bar(axs[1], x_length=0.0005, y_length=0.045, x_label='0.5 ms', y_label='50 mV')

# crop the graph to desired limits
axs[0].set_xlim(-0.001, 0.01)  # include negative space
axs[1].set_xlim(-0.001, 0.01)  

# add margins to plot
plt.subplots_adjust(left=0.15, right=0.85, bottom=0.15, top=0.85)  # Reduced left margin slightly

#plt.grid(True)

# save and show the plots
plt.savefig('9_12_plots.png', dpi=1000, bbox_inches='tight')  # dpi=1000 for very high resolution
plt.show()
