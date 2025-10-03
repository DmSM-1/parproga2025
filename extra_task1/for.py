import subprocess
import re
import matplotlib.pyplot as plt
import numpy as np


thread_counts = np.arange(1, 5, 1)

strategies = ["static,  1", "static,  4", "dynamic, 1", "dynamic, 4", "guided 1", "guided 4", "default"]

num_runs = 10000

results = {s: [] for s in strategies}

time_re = re.compile(r'\((.*?)\): time: ([0-9.eE+-]+)')

for threads in thread_counts:
    print(f"Running with {threads} threads...")
    temp_times = {s: [] for s in strategies}

    for run in range(num_runs):
        proc = subprocess.run(["./for", str(threads)], capture_output=True, text=True)
        output = proc.stdout

        for line in output.splitlines():
            match = time_re.search(line)
            if match:
                strategy_name = match.group(1).strip()
                time_value = float(match.group(2))
                temp_times[strategy_name].append(time_value)

    for s in strategies:
        avg_time = np.mean(temp_times[s]) if temp_times[s] else 0.0
        results[s].append(avg_time)

plt.figure(figsize=(10, 6))
for s in strategies:
    plt.plot(thread_counts, results[s], marker='o', label=s)

plt.xlabel("Number of threads")
plt.ylabel("Average Time (s)")
plt.title(f"OpenMP schedule comparison (averaged over {num_runs} runs)")
plt.xticks(thread_counts)
plt.grid(True)
plt.legend()
plt.savefig("for.png")
plt.show()
