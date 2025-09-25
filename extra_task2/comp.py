import subprocess
import matplotlib.pyplot as plt
import random
import statistics
import time
import math
import numpy as np

# --- Настройки ---
EXECUTABLE_OLD = "./sort"      # Оригинальная программа (часть 1)
EXECUTABLE_NEW = "./sort_m"        # Новая программа с qsort (часть 2)  
INPUT_FILE = "data"            
OUTPUT_FILE = "result"         
NUM_ELEMENTS = 2**20           # 1,048,576 элементов
MAX_CUTOFF = 1000              # Максимальный размер отсечки
INITIAL_CUTOFF_STEP = 1        
LOGARITHMIC_THRESHOLD = 50     
NUM_RUNS = 3                   # Уменьшим для скорости сравнения
MIN_RUNS = 2                   

def generate_input_file(filename, num_elements):
    """Генерирует файл со случайными числами."""
    print(f"Генерация входного файла '{filename}' с {num_elements} элементами...")
    with open(filename, 'w') as f:
        f.write(f"{num_elements}\n")
        for _ in range(num_elements):
            f.write(f"{random.randint(0, 1000000)} ")
    print("Генерация завершена.")

def run_sorting(executable, cutoff_size, run_number):
    """Запускает программу сортировки и возвращает время выполнения."""
    command = [executable, INPUT_FILE, OUTPUT_FILE, str(cutoff_size)]
    
    try:
        result = subprocess.run(
            command, 
            capture_output=True,
            text=True,
            check=True,
            timeout=300
        )
        
        time_str = result.stdout.strip()
        time_val = float(time_str)
        
        print(f"  {executable} Запуск {run_number}: {time_val:.4f} сек")
        return time_val
        
    except subprocess.TimeoutExpired:
        print(f"  {executable} Запуск {run_number}: ТАЙМАУТ")
        return None
    except subprocess.CalledProcessError as e:
        print(f"  {executable} Запуск {run_number}: ОШИБКА - {e.stderr.strip()}")
        return None
    except ValueError:
        print(f"  {executable} Запуск {run_number}: ОШИБКА формата вывода")
        return None

def generate_cutoff_sequence(max_cutoff, initial_step, log_threshold):
    """Генерирует последовательность значений cutoff."""
    cutoffs = []
    
    # Линейная часть
    current = 1
    while current <= log_threshold and current <= max_cutoff:
        cutoffs.append(current)
        current += initial_step
    
    # Логарифмическая часть
    if current <= max_cutoff:
        num_log_points = min(15, max_cutoff - log_threshold)
        log_start = math.log(log_threshold + 1)
        log_end = math.log(max_cutoff)
        
        for i in range(1, num_log_points + 1):
            log_value = log_start + (log_end - log_start) * i / num_log_points
            cutoff_value = int(round(math.exp(log_value)))
            
            if cutoff_value <= max_cutoff and cutoff_value not in cutoffs:
                cutoffs.append(cutoff_value)
    
    if max_cutoff not in cutoffs:
        cutoffs.append(max_cutoff)
    
    return sorted(cutoffs)

def main():
    """Основная функция для запуска эксперимента сравнения."""
    generate_input_file(INPUT_FILE, NUM_ELEMENTS)

    cutoff_sequence = generate_cutoff_sequence(MAX_CUTOFF, INITIAL_CUTOFF_STEP, LOGARITHMIC_THRESHOLD)
    
    print(f"Будет протестировано {len(cutoff_sequence)} значений cutoff")
    
    # Данные для обеих программ
    results_old = {'cutoffs': [], 'times': [], 'stds': []}  # sort_m
    results_new = {'cutoffs': [], 'times': [], 'stds': []}  # sort
    
    # Тестируем обе программы
    for executable, results in [(EXECUTABLE_OLD, results_old), (EXECUTABLE_NEW, results_new)]:
        print(f"\n=== Тестирование {executable} ===")
        
        cutoff_sizes = []
        execution_times = []
        std_deviations = []
        
        for i, current_cutoff in enumerate(cutoff_sequence):
            print(f"\n[{i+1}/{len(cutoff_sequence)}] task_size = {current_cutoff}")
            
            run_times = []
            successful_runs = 0
            
            for run in range(1, NUM_RUNS + 1):
                time_val = run_sorting(executable, current_cutoff, run)
                if time_val is not None:
                    run_times.append(time_val)
                    successful_runs += 1
                
                if run < NUM_RUNS:
                    time.sleep(0.2)
            
            if successful_runs >= MIN_RUNS:
                avg_time = statistics.mean(run_times)
                std_dev = statistics.stdev(run_times) if len(run_times) > 1 else 0
                
                cutoff_sizes.append(current_cutoff)
                execution_times.append(avg_time)
                std_deviations.append(std_dev)
                
                print(f"  Усредненное время: {avg_time:.4f} сек (±{std_dev:.4f} сек)")
        
        results['cutoffs'] = cutoff_sizes
        results['times'] = execution_times
        results['stds'] = std_deviations

    # Построение графиков сравнения
    plot_comparison(results_old, results_new)

def plot_comparison(results_old, results_new):
    """Строит графики сравнения двух программ."""
    
    # 1. Основной график сравнения
    plt.figure(figsize=(15, 10))
    
    # Графики для обеих программ
    plt.plot(results_old['cutoffs'], results_old['times'], 'b-', linewidth=2, 
             label=f'{EXECUTABLE_OLD} (merge sort)', marker='o')
    plt.plot(results_new['cutoffs'], results_new['times'], 'r-', linewidth=2, 
             label=f'{EXECUTABLE_NEW} (hybrid)', marker='s')
    
    # Области отклонений
    plt.fill_between(results_old['cutoffs'], 
                    [t - s for t, s in zip(results_old['times'], results_old['stds'])],
                    [t + s for t, s in zip(results_old['times'], results_old['stds'])],
                    alpha=0.2, color='blue')
    plt.fill_between(results_new['cutoffs'], 
                    [t - s for t, s in zip(results_new['times'], results_new['stds'])],
                    [t + s for t, s in zip(results_new['times'], results_new['stds'])],
                    alpha=0.2, color='red')
    
    plt.xlabel('Размер отсечки (task_size)')
    plt.ylabel('Время выполнения, секунды')
    plt.title('Сравнение производительности: Pure Merge Sort vs Hybrid (Merge + QSort)')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # 2. График ускорения (speedup)
    plt.figure(figsize=(15, 8))
    
    # Вычисляем ускорение для общих значений cutoff
    common_cutoffs = sorted(set(results_old['cutoffs']) & set(results_new['cutoffs']))
    speedups = []
    valid_cutoffs = []
    
    for cutoff in common_cutoffs:
        idx_old = results_old['cutoffs'].index(cutoff)
        idx_new = results_new['cutoffs'].index(cutoff)
        
        time_old = results_old['times'][idx_old]
        time_new = results_new['times'][idx_new]
        
        if time_old > 0 and time_new > 0:
            speedup = time_old / time_new
            speedups.append(speedup)
            valid_cutoffs.append(cutoff)
    
    plt.plot(valid_cutoffs, speedups, 'g-', linewidth=3, marker='D', markersize=8)
    plt.axhline(y=1, color='gray', linestyle='--', label='Нет ускорения')
    plt.xlabel('Размер отсечки (task_size)')
    plt.ylabel('Ускорение (раз)')
    plt.title('Ускорение Hybrid версии относительно Pure Merge Sort')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # 3. График в логарифмическом масштабе
    plt.figure(figsize=(15, 10))
    
    plt.semilogx(results_old['cutoffs'], results_old['times'], 'b-', linewidth=2, 
                 label=f'{EXECUTABLE_OLD} (merge sort)', marker='o')
    plt.semilogx(results_new['cutoffs'], results_new['times'], 'r-', linewidth=2, 
                 label=f'{EXECUTABLE_NEW} (hybrid)', marker='s')
    
    plt.xlabel('Размер отсечки (task_size) - логарифмическая шкала')
    plt.ylabel('Время выполнения, секунды')
    plt.title('Сравнение производительности (логарифмическая шкала)')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # 4. График разницы во времени
    plt.figure(figsize=(15, 8))
    
    time_differences = []
    diff_cutoffs = []
    
    for cutoff in common_cutoffs:
        idx_old = results_old['cutoffs'].index(cutoff)
        idx_new = results_new['cutoffs'].index(cutoff)
        
        time_old = results_old['times'][idx_old]
        time_new = results_new['times'][idx_new]
        
        time_differences.append(time_old - time_new)
        diff_cutoffs.append(cutoff)
    
    plt.bar(diff_cutoffs, time_differences, alpha=0.7, color='orange')
    plt.xlabel('Размер отсечки (task_size)')
    plt.ylabel('Разница во времени (старая - новая), секунды')
    plt.title('Выигрыш во времени Hybrid версии')
    plt.grid(True, alpha=0.3)
    
    plt.tight_layout()
    plt.show()
    
    # Вывод статистики сравнения
    print_statistics(results_old, results_new, valid_cutoffs, speedups)

def print_statistics(results_old, results_new, common_cutoffs, speedups):
    """Выводит статистику сравнения."""
    print("\n" + "="*80)
    print("СТАТИСТИКА СРАВНЕНИЯ")
    print("="*80)
    
    # Находим лучшее время для каждой программы
    best_time_old = min(results_old['times'])
    best_cutoff_old = results_old['cutoffs'][results_old['times'].index(best_time_old)]
    
    best_time_new = min(results_new['times'])
    best_cutoff_new = results_new['cutoffs'][results_new['times'].index(best_time_new)]
    
    # Вычисляем общее ускорение
    overall_speedup = best_time_old / best_time_new if best_time_new > 0 else 0
    time_saved = best_time_old - best_time_new
    
    print(f"\n{EXECUTABLE_OLD} (Pure Merge Sort):")
    print(f"  Лучшее время: {best_time_old:.4f} сек при task_size = {best_cutoff_old}")
    
    print(f"\n{EXECUTABLE_NEW} (Hybrid Merge + QSort):")
    print(f"  Лучшее время: {best_time_new:.4f} сек при task_size = {best_cutoff_new}")
    
    print(f"\n РЕЗУЛЬТАТЫ СРАВНЕНИЯ:")
    print(f"  Ускорение: {overall_speedup:.2f}x")
    print(f"  Экономия времени: {time_saved:.4f} сек ({time_saved/best_time_old*100:.1f}%)")
    
    if speedups:
        avg_speedup = statistics.mean(speedups)
        max_speedup = max(speedups)
        print(f"  Среднее ускорение: {avg_speedup:.2f}x")
        print(f"  Максимальное ускорение: {max_speedup:.2f}x")
    
    print(f"  Оптимальный task_size для {EXECUTABLE_OLD}: {best_cutoff_old}")
    print(f"  Оптимальный task_size для {EXECUTABLE_NEW}: {best_cutoff_new}")
    
    
    print("="*80)

if __name__ == "__main__":
    main()