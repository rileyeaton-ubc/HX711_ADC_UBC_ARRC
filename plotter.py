import pandas as pd
import plotly.express as px

# Load data from CSV
data = pd.read_csv('F:\data_11.csv', header=None, names=['timestamp', 'weight'])

# Create dot plot
fig = px.scatter(
    data, 
    x='timestamp', 
    y='weight', 
    title='Weight vs. Time',
    labels={'timestamp': 'Time (ms)', 'weight': 'Weight (grams)'},
    opacity=0.7
)

# Show the plot
fig.show()
