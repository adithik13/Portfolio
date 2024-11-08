from SDS00015 import plot_subplot1  
from SDS00017 import plot_subplot2

import matplotlib.pyplot as plt

file_path_1 = "wu_venv/csv_files/SDS00015.csv"  
file_path_2 = "wu_venv/csv_files/SDS00017.csv"  

fig, axs = plt.subplots(2, 1, sharex=True, figsize=(10, 8))  

plot_subplot1(axs[0], file_path_1)  
plot_subplot2(axs[1], file_path_2)  

plt.tight_layout()

for ax in axs:
    ax.spines['top'].set_visible(False)     
    ax.spines['right'].set_visible(False)   
    ax.spines['left'].set_visible(False)    
    ax.spines['bottom'].set_visible(False)  

def add_scale_bar(ax, x_length, y_length, x_label, y_label, x_start, y_start):
    ax.plot([x_start, x_start + x_length], [y_start, y_start], color='black', lw=1.5)
    ax.text(x_start + x_length / 2, y_start - 0.005, x_label, 
            ha='center', va='top', fontsize=10)
    ax.plot([x_start, x_start], [y_start, y_start + y_length], color='black', lw=1.5)
    ax.text(x_start - 0.0005, y_start + y_length / 2, y_label,  
            ha='right', va='center', fontsize=10)

axs[0].set_xlim(0.005, 0.0175)
axs[1].set_xlim(0.005, 0.0175)
axs[1].set_ylim(-0.03, 0.03)

add_scale_bar(axs[0], x_length=0.001, y_length=0.04, x_label='1 ms', y_label='40 mV', x_start=0.005, y_start=-0.15)

add_scale_bar(axs[1], x_length=0.001, y_length=0.01, x_label='1 ms', y_label='10 mV', x_start=0.005, y_start=-0.025)

plt.subplots_adjust(left=0.15, right=0.85, bottom=0.15, top=0.85)

plt.savefig('15_17_plots.png', dpi=1000, bbox_inches='tight')
plt.show()
