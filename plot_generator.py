import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("output.csv")

df['timestamp'] = pd.to_numeric(df['timestamp'])

t0 = df['timestamp'].iloc[0]
df['t'] = df['timestamp'] - t0

real = df[df['type'] == 'real']
forecast = df[df['type'] == 'forecast']

plt.figure()

plt.bar(real['t'], real['value'], label='Real')
plt.plot(forecast['t'], forecast['value'], color='red', linestyle='--', label='Forecast')

for i, (x, y) in enumerate(zip(real['t'], real['value'])):
    if i % 2 == 0:
        offset = 1 if i % 4 == 0 else 3
        plt.text(x, y + offset, f'{y:.1f}', ha='center', va='bottom', fontsize=8)

for i, (x, y) in enumerate(zip(forecast['t'], forecast['value'])):
    if i % 2 == 0:
        plt.text(x, y + 4, f'{y:.1f}', ha='center', va='bottom', fontsize=8, color='red')

plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Value")
plt.title("Real vs Forecast")

plt.show()