import pandas as pd
import plotly.express as px

# Load data from CSV
data = pd.read_csv('D:\data_2_1.csv', header=1, names=['timestamp', 'weight_1', 'weight_2'])

# Melt the data to a long format for easier plotting with Plotly Express
data_melted = data.melt(id_vars='timestamp', value_vars=['weight_1', 'weight_2'], var_name='Load Cell #', value_name='Weight')

# Create a line plot with both weight columns as separate lines
fig = px.line(
    data_melted,
    x='timestamp',
    y='Weight',
    color='Load Cell #',  # Differentiate lines by load cell
    title='Weight vs. Time',
    labels={'timestamp': 'Time (ms)', 'Weight': 'Weight (grams)', 'Load Cell #': 'Weight Column'},
    markers=True  # Add markers to the lines
)

# Show the plot
fig.show()
