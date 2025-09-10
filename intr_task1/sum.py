import subprocess
import matplotlib.pyplot as plt
import re
import numpy as np
import time

def run_program_multiple_times(N, num_threads, num_runs=10):
    """Запускает программу несколько раз и усредняет результаты"""
    slow_times = []
    fast_times = []
    
    for run in range(num_runs):
        try:
            cmd = ["./sum", str(N), str(num_threads)]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
            
            if result.returncode == 0:
                output = result.stdout
                
                # Парсим время выполнения
                slow_match = re.search(r'SLOW VARIANT\s*.*?time: ([0-9.]+)', output, re.DOTALL)
                fast_match = re.search(r'FAST VARIANT\s*.*?time: ([0-9.]+)', output, re.DOTALL)
                
                if slow_match and fast_match:
                    slow_times.append(float(slow_match.group(1)))
                    fast_times.append(float(fast_match.group(1)))
                else:
                    print(f"Ошибка парсинга для {num_threads} потоков, запуск {run+1}")
                    
            else:
                print(f"Ошибка выполнения для {num_threads} потоков, запуск {run+1}: {result.stderr}")
                
        except subprocess.TimeoutExpired:
            print(f"Таймаут для {num_threads} потоков, запуск {run+1}")
        except Exception as e:
            print(f"Исключение для {num_threads} потоков, запуск {run+1}: {e}")
        
        # Небольшая пауза между запусками
        time.sleep(0.1)
    
    # Усредняем результаты
    avg_slow = np.mean(slow_times) if slow_times else 0
    avg_fast = np.mean(fast_times) if fast_times else 0
    std_slow = np.std(slow_times) if slow_times else 0
    std_fast = np.std(fast_times) if fast_times else 0
    
    return avg_slow, avg_fast, std_slow, std_fast, len(slow_times)

def run_benchmark(N, max_threads, num_runs=10):
    """Запускает полный бенчмарк"""
    threads = list(range(1, max_threads + 1))
    slow_avg_times = []
    fast_avg_times = []
    slow_std = []
    fast_std = []
    successful_runs = []
    
    print(f"Запуск бенчмарка для N={N}, потоков=1..{max_threads}, {num_runs} запусков на конфигурацию")
    print("=" * 70)
    
    for num_threads in threads:
        print(f"Тестируем {num_threads} потоков...")
        avg_slow, avg_fast, std_slow, std_fast, success_count = run_program_multiple_times(N, num_threads, num_runs)
        
        slow_avg_times.append(avg_slow)
        fast_avg_times.append(avg_fast)
        slow_std.append(std_slow)
        fast_std.append(std_fast)
        successful_runs.append(success_count)
        
        print(f"  Потоков: {num_threads} | Успешных запусков: {success_count}/{num_runs}")
        print(f"  Slow: {avg_slow:.6f} ± {std_slow:.6f} сек")
        print(f"  Fast: {avg_fast:.6f} ± {std_fast:.6f} сек")
        print("-" * 50)
    
    return threads, slow_avg_times, fast_avg_times, slow_std, fast_std, successful_runs

def plot_results(threads, slow_avg, fast_avg, slow_std, fast_std, N):
    """Строит графики с error bars"""
    fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(18, 6))
    
    # График 1: Время выполнения с error bars
    ax1.errorbar(threads, slow_avg, yerr=slow_std, fmt='o-', 
                label='Slow variant', capsize=5, capthick=2, linewidth=2, markersize=8)
    ax1.errorbar(threads, fast_avg, yerr=fast_std, fmt='s-', 
                label='Fast variant', capsize=5, capthick=2, linewidth=2, markersize=8)
    ax1.set_xlabel('Количество потоков')
    ax1.set_ylabel('Время выполнения (секунды)')
    ax1.set_title(f'Время выполнения (N = {N})\nс ошибками стандартного отклонения')
    ax1.grid(True, alpha=0.3)
    ax1.legend()
    
    # График 2: Ускорение
    slow_single = slow_avg[0]
    fast_single = fast_avg[0]
    
    slow_speedup = [slow_single / t for t in slow_avg]
    fast_speedup = [fast_single / t for t in fast_avg]
    
    # Пропагация ошибок для ускорения
    slow_speedup_err = [slow_speedup[i] * np.sqrt((slow_std[0]/slow_single)**2 + (slow_std[i]/slow_avg[i])**2) 
                       for i in range(len(threads))]
    fast_speedup_err = [fast_speedup[i] * np.sqrt((fast_std[0]/fast_single)**2 + (fast_std[i]/fast_avg[i])**2) 
                       for i in range(len(threads))]
    
    ax2.errorbar(threads, slow_speedup, yerr=slow_speedup_err, fmt='o-', 
                label='Slow variant', capsize=5, capthick=2, linewidth=2, markersize=8)
    ax2.errorbar(threads, fast_speedup, yerr=fast_speedup_err, fmt='s-', 
                label='Fast variant', capsize=5, capthick=2, linewidth=2, markersize=8)
    ax2.plot(threads, threads, '--', label='Идеальное ускорение', color='gray', alpha=0.7)
    ax2.set_xlabel('Количество потоков')
    ax2.set_ylabel('Ускорение')
    ax2.set_title(f'Ускорение (N = {N})\nотносительно 1 потока')
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    
    # График 3: Отношение производительности
    ratio = [slow_avg[i] / fast_avg[i] for i in range(len(threads))]
    ratio_err = [ratio[i] * np.sqrt((slow_std[i]/slow_avg[i])**2 + (fast_std[i]/fast_avg[i])**2) 
                for i in range(len(threads))]
    
    ax3.errorbar(threads, ratio, yerr=ratio_err, fmt='^-', 
                color='purple', capsize=5, capthick=2, linewidth=2, markersize=8)
    ax3.axhline(y=1.0, linestyle='--', color='red', alpha=0.7, label='Равная производительность')
    ax3.set_xlabel('Количество потоков')
    ax3.set_ylabel('Отношение Slow/Fast')
    ax3.set_title('Во сколько раз Fast вариант быстрее Slow')
    ax3.grid(True, alpha=0.3)
    ax3.legend()
    
    plt.tight_layout()
    plt.savefig(f'openmp_benchmark_avg_N{N}.png', dpi=300, bbox_inches='tight')
    plt.show()
    
    # Вывод подробной таблицы результатов
    print("\n" + "=" * 100)
    print("ИТОГОВЫЕ РЕЗУЛЬТАТЫ (усреднено по 10 запускам):")
    print("=" * 100)
    print("Потоки | Slow время (с) ± std | Fast время (с) ± std | Slow ускорение | Fast ускорение | Отношение")
    print("-" * 100)
    
    for i, t in enumerate(threads):
        slow_sp = slow_avg[0] / slow_avg[i] if i > 0 else 1.0
        fast_sp = fast_avg[0] / fast_avg[i] if i > 0 else 1.0
        ratio = slow_avg[i] / fast_avg[i]
        
        print(f"{t:6} | {slow_avg[i]:6.4f} ± {slow_std[i]:6.4f} | {fast_avg[i]:6.4f} ± {fast_std[i]:6.4f} | "
              f"{slow_sp:12.2f} | {fast_sp:13.2f} | {ratio:8.2f}x")

def main():
    # Параметры бенчмарка
    N = 100000
    max_threads = 11
    num_runs = 5
    
    # Компилируем программу
    print("Компилируем программу...")
    compile_cmd = ["gcc", "-o", "sum", "sum.c", "-fopenmp", "-O3"]  # Добавил -O3 для оптимизации
    compile_result = subprocess.run(compile_cmd, capture_output=True, text=True)
    
    if compile_result.returncode != 0:
        print("Ошибка компиляции:")
        print(compile_result.stderr)
        return
    
    print("Компиляция успешна! Запускаем бенчмарк...")
    
    # Запускаем бенчмарк
    results = run_benchmark(N, max_threads, num_runs)
    
    # Строим графики
    plot_results(*results[:5], N)

if __name__ == "__main__":
    main()