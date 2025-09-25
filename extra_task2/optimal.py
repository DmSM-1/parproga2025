import subprocess
import matplotlib.pyplot as plt
import random
import statistics
import time
import math
import numpy as np

# --- Настройки ---
EXECUTABLE = "./sort_m"          # Имя вашей скомпилированной программы
INPUT_FILE = "data"            # Исходный файл с данными
OUTPUT_FILE = "result"         # Файл для результата сортировки
NUM_ELEMENTS = 2**22           # 1,048,576 элементов, как в задании
MAX_CUTOFF = 2**22               # Максимальный размер отсечки для теста
INITIAL_CUTOFF_STEP = 1        # Начальный шаг изменения размера отсечки
LOGARITHMIC_THRESHOLD = 1     # Порог, после которого шаг становится логарифмическим
NUM_RUNS = 5                 # Количество запусков для усреднения
MIN_RUNS = 3                   # Минимальное количество успешных запусков

def generate_input_file(filename, num_elements):
    """Генерирует файл со случайными числами."""
    print(f"Генерация входного файла '{filename}' с {num_elements} элементами...")
    with open(filename, 'w') as f:
        f.write(f"{num_elements}\n")  # Сначала пишем количество элементов
        for _ in range(num_elements):
            f.write(f"{random.randint(0, 1000000)} ")
    print("Генерация завершена.")

def run_sorting(cutoff_size, run_number):
    """Запускает программу сортировки и возвращает время выполнения."""
    command = [EXECUTABLE, INPUT_FILE, OUTPUT_FILE, str(cutoff_size)]
    
    try:
        # Запускаем процесс и ждем его завершения
        result = subprocess.run(
            command, 
            capture_output=True,
            text=True,
            check=True,
            timeout=300  # Таймаут 5 минут на случай зависания
        )
        
        # Получаем время из стандартного вывода
        time_str = result.stdout.strip()
        time_val = float(time_str)
        
        print(f"  Запуск {run_number}: {time_val:.4f} сек")
        return time_val
        
    except subprocess.TimeoutExpired:
        print(f"  Запуск {run_number}: ТАЙМАУТ (превышено 5 минут)")
        return None
    except subprocess.CalledProcessError as e:
        print(f"  Запуск {run_number}: ОШИБКА - {e.stderr.strip()}")
        return None
    except ValueError:
        print(f"  Запуск {run_number}: ОШИБКА - неверный формат вывода: '{result.stdout.strip()}'")
        return None

def generate_cutoff_sequence(max_cutoff, initial_step, log_threshold):
    """
    Генерирует последовательность значений cutoff.
    До log_threshold - линейный шаг, после - логарифмический.
    """
    cutoffs = []
    
    # Линейная часть (мелкие значения)
    current = 1
    while current <= log_threshold and current <= max_cutoff:
        cutoffs.append(current)
        current += initial_step
    
    # Логарифмическая часть (крупные значения)
    if current <= max_cutoff:
        # Определяем количество точек для логарифмической шкалы
        num_log_points = min(20, max_cutoff - log_threshold)  # Не более 20 точек
        
        # Создаем логарифмически распределенные точки
        log_start = math.log(log_threshold + 1)
        log_end = math.log(max_cutoff)
        
        for i in range(1, num_log_points + 1):
            # Равномерно распределяем точки в логарифмическом пространстве
            log_value = log_start + (log_end - log_start) * i / num_log_points
            cutoff_value = int(round(math.exp(log_value)))
            
            # Убеждаемся, что значение в пределах и уникально
            if cutoff_value <= max_cutoff and cutoff_value not in cutoffs:
                cutoffs.append(cutoff_value)
    
    # Добавляем максимальное значение, если его еще нет
    if max_cutoff not in cutoffs:
        cutoffs.append(max_cutoff)
    
    return sorted(cutoffs)

def find_optimal_parameters(cutoff_sizes, execution_times, std_deviations):
    """Находит оптимальные параметры на основе результатов."""
    results = []
    
    # 1. Просто минимальное время
    min_time_idx = np.argmin(execution_times)
    results.append({
        'type': 'Абсолютный минимум',
        'cutoff': cutoff_sizes[min_time_idx],
        'time': execution_times[min_time_idx],
        'std': std_deviations[min_time_idx],
        'score': execution_times[min_time_idx]
    })
    
    # 2. Минимум с учетом стабильности (время + отклонение)
    stability_scores = [t + s for t, s in zip(execution_times, std_deviations)]
    stability_idx = np.argmin(stability_scores)
    results.append({
        'type': 'С учетом стабильности',
        'cutoff': cutoff_sizes[stability_idx],
        'time': execution_times[stability_idx],
        'std': std_deviations[stability_idx],
        'score': stability_scores[stability_idx]
    })
    
    # 3. Минимум среди наиболее стабильных (низкое отклонение)
    low_std_threshold = np.percentile(std_deviations, 25)  # Нижние 25% по отклонению
    low_std_indices = [i for i, std in enumerate(std_deviations) if std <= low_std_threshold]
    
    if low_std_indices:
        low_std_times = [execution_times[i] for i in low_std_indices]
        best_low_std_idx = low_std_indices[np.argmin(low_std_times)]
        results.append({
            'type': 'Среди стабильных',
            'cutoff': cutoff_sizes[best_low_std_idx],
            'time': execution_times[best_low_std_idx],
            'std': std_deviations[best_low_std_idx],
            'score': execution_times[best_low_std_idx]
        })
    
    return results

def main():
    """Основная функция для запуска эксперимента."""
    # 1. Генерируем входной файл
    generate_input_file(INPUT_FILE, NUM_ELEMENTS)

    # 2. Генерируем последовательность значений cutoff
    cutoff_sequence = generate_cutoff_sequence(MAX_CUTOFF, INITIAL_CUTOFF_STEP, LOGARITHMIC_THRESHOLD)
    
    print(f"Будет протестировано {len(cutoff_sequence)} значений cutoff:")
    print(f"Линейная часть (1-{LOGARITHMIC_THRESHOLD}): {[c for c in cutoff_sequence if c <= LOGARITHMIC_THRESHOLD]}")
    print(f"Логарифмическая часть (> {LOGARITHMIC_THRESHOLD}): {[c for c in cutoff_sequence if c > LOGARITHMIC_THRESHOLD]}")

    cutoff_sizes = []
    execution_times = []
    std_deviations = []  # Стандартное отклонение для отображения разброса
    all_runs_data = []   # Все данные запусков для детального анализа

    print(f"\n--- Начало эксперимента (усреднение по {NUM_RUNS} запускам) ---")
    
    # 3. Запускаем C-программу для каждого значения из последовательности
    for i, current_cutoff in enumerate(cutoff_sequence):
        print(f"\n[{i+1}/{len(cutoff_sequence)}] Параметр отсечки (task_size) = {current_cutoff}")
        
        run_times = []
        successful_runs = 0
        
        # Выполняем несколько запусков для усреднения
        for run in range(1, NUM_RUNS + 1):
            time_val = run_sorting(current_cutoff, run)
            
            if time_val is not None:
                run_times.append(time_val)
                successful_runs += 1
            
            # Небольшая пауза между запусками для стабилизации системы
            if run < NUM_RUNS:
                time.sleep(0.5)
        
        # Проверяем, достаточно ли успешных запусков
        if successful_runs >= MIN_RUNS:
            # Вычисляем среднее значение и стандартное отклонение
            avg_time = statistics.mean(run_times)
            if len(run_times) > 1:
                std_dev = statistics.stdev(run_times)
            else:
                std_dev = 0
            
            cutoff_sizes.append(current_cutoff)
            execution_times.append(avg_time)
            std_deviations.append(std_dev)
            all_runs_data.append(run_times)
            
            print(f"  Усредненное время: {avg_time:.4f} сек (±{std_dev:.4f} сек)")
            print(f"  Успешных запусков: {successful_runs}/{NUM_RUNS}")
        else:
            print(f"  ПРЕРВАНО: недостаточно успешных запусков ({successful_runs}/{MIN_RUNS})")
            continue

    print("\n--- Эксперимент завершен ---")

    # 4. Проверяем, есть ли данные для построения графика
    if not cutoff_sizes:
        print("Не удалось собрать данные для построения графика.")
        return
    
    # 5. Находим оптимальные параметры
    optimal_results = find_optimal_parameters(cutoff_sizes, execution_times, std_deviations)
    
    # 6. Сохраняем сырые данные в файл для дальнейшего анализа
    with open("experiment_results.csv", "w") as f:
        f.write("cutoff_size,avg_time,std_dev,all_runs\n")
        for i, cutoff in enumerate(cutoff_sizes):
            runs_str = ",".join([f"{t:.4f}" for t in all_runs_data[i]])
            f.write(f"{cutoff},{execution_times[i]:.4f},{std_deviations[i]:.4f},{runs_str}\n")
    
    # 7. Строим график с ошибками
    plt.figure(figsize=(14, 8))
    
    # Основной график
    plt.plot(cutoff_sizes, execution_times, 'b-', linewidth=2, label='Среднее время')
    plt.fill_between(cutoff_sizes, 
                    [t - s for t, s in zip(execution_times, std_deviations)],
                    [t + s for t, s in zip(execution_times, std_deviations)],
                    alpha=0.2, label='±1 стандартное отклонение')
    
    # Различные маркеры для линейной и логарифмической частей
    linear_cutoffs = [c for c in cutoff_sizes if c <= LOGARITHMIC_THRESHOLD]
    linear_times = [execution_times[cutoff_sizes.index(c)] for c in linear_cutoffs]
    
    log_cutoffs = [c for c in cutoff_sizes if c > LOGARITHMIC_THRESHOLD]
    log_times = [execution_times[cutoff_sizes.index(c)] for c in log_cutoffs]
    
    plt.scatter(linear_cutoffs, linear_times, color='green', s=50, zorder=5, label='Линейный шаг')
    plt.scatter(log_cutoffs, log_times, color='red', s=50, zorder=5, label='Логарифмический шаг')
    
    # Вертикальная линия, показывающая границу перехода
    plt.axvline(x=LOGARITHMIC_THRESHOLD, color='gray', linestyle='--', alpha=0.7, label=f'Граница перехода ({LOGARITHMIC_THRESHOLD})')
    
    # Помечаем оптимальные точки разными цветами и формами
    colors = ['gold', 'orange', 'purple']
    markers = ['*', 's', 'D']
    
    for i, result in enumerate(optimal_results):
        plt.scatter(result['cutoff'], result['time'], 
                   color=colors[i % len(colors)], 
                   marker=markers[i % len(markers)], 
                   s=200, zorder=10, 
                   label=f"{result['type']} (cutoff={result['cutoff']})")
    
    plt.title(f'Зависимость времени выполнения от параметра отсечки (task_size)\n'
              f'Усреднение по {NUM_RUNS} запускам, {NUM_ELEMENTS} элементов')
    plt.xlabel('Размер отсечки (task_size)')
    plt.ylabel('Время выполнения, секунды')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # Дополнительный график - стандартное отклонение
    plt.figure(figsize=(14, 6))
    plt.bar(cutoff_sizes, std_deviations, alpha=0.7, color='orange')
    plt.axvline(x=LOGARITHMIC_THRESHOLD, color='gray', linestyle='--', alpha=0.7)
    
    # Помечаем оптимальные точки на графике отклонений
    for i, result in enumerate(optimal_results):
        plt.axvline(x=result['cutoff'], color=colors[i % len(colors)], 
                   linestyle='-', alpha=0.8, linewidth=2,
                   label=f"{result['type']} (cutoff={result['cutoff']})")
    
    plt.title('Стандартное отклонение времени выполнения по запускам')
    plt.xlabel('Размер отсечки (task_size)')
    plt.ylabel('Стандартное отклонение, секунды')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    # График в логарифмическом масштабе для лучшей визуализации больших значений
    plt.figure(figsize=(14, 8))
    plt.semilogx(cutoff_sizes, execution_times, 'b-', linewidth=2, marker='o', label='Среднее время')
    plt.fill_between(cutoff_sizes, 
                    [t - s for t, s in zip(execution_times, std_deviations)],
                    [t + s for t, s in zip(execution_times, std_deviations)],
                    alpha=0.2)
    plt.axvline(x=LOGARITHMIC_THRESHOLD, color='gray', linestyle='--', alpha=0.7, label=f'Граница перехода ({LOGARITHMIC_THRESHOLD})')
    
    # Помечаем оптимальные точки
    for i, result in enumerate(optimal_results):
        plt.scatter(result['cutoff'], result['time'], 
                   color=colors[i % len(colors)], 
                   marker=markers[i % len(markers)], 
                   s=200, zorder=10, 
                   label=f"{result['type']} (cutoff={result['cutoff']})")
    
    plt.title(f'Зависимость времени выполнения (логарифмическая шкала cutoff)\n'
              f'Усреднение по {NUM_RUNS} запускам, {NUM_ELEMENTS} элементов')
    plt.xlabel('Размер отсечки (task_size) - логарифмическая шкала')
    plt.ylabel('Время выполнения, секунды')
    plt.grid(True, alpha=0.3)
    plt.legend()
    
    plt.show()
    
    # 8. Выводим подробную статистику
    print("\n" + "="*70)
    print("РЕЗУЛЬТАТЫ ЭКСПЕРИМЕНТА")
    print("="*70)
    
    print(f"\nОбщая статистика:")
    print(f"Всего тестируемых значений отсечки: {len(cutoff_sizes)}")
    print(f"Диапазон значений cutoff: {min(cutoff_sizes)} - {max(cutoff_sizes)}")
    print(f"Общее время эксперимента: ~{len(cutoff_sizes) * NUM_RUNS * 2} секунд")
    
    print(f"\n Лучшие результаты по разным критериям:")
    print("-" * 70)
    
    for i, result in enumerate(optimal_results):
        print(f"{i+1}. {result['type']}:")
        print(f"   └── task_size = {result['cutoff']}")
        print(f"   └── Время: {result['time']:.4f} сек (±{result['std']:.4f} сек)")
        print(f"   └── Оценка: {result['score']:.4f}")
    
    print("-" * 70)
    
    best_result = optimal_results[0]  # Абсолютный минимум
    stable_result = optimal_results[1]  # С учетом стабильности
    
    print(f"   Используйте task_size = {best_result['cutoff']}")
    print(f"   Ожидаемое время: {best_result['time']:.4f} сек")
    

    if len(optimal_results) > 2:
        low_std_result = optimal_results[2]
        print(f"\n3. Для максимальной предсказуемости:")
        print(f"   Используйте task_size = {low_std_result['cutoff']}")
        print(f"   Стандартное отклонение: {low_std_result['std']:.4f} сек")
    
    # Анализ поведения
    print(f"\n АНАЛИЗ ПОВЕДЕНИЯ:")
    print("-" * 70)
    
    # Находим, где начинается плато производительности
    min_time = min(execution_times)
    threshold_time = min_time * 1.05  # +5% от минимума
    
    plateau_cutoffs = [cutoff for cutoff, time in zip(cutoff_sizes, execution_times) 
                      if time <= threshold_time]
    
    if plateau_cutoffs:
        plateau_start = min(plateau_cutoffs)
        plateau_end = max(plateau_cutoffs)
        print(f"Область стабильной производительности (+5% от минимума):")
        print(f"   task_size от {plateau_start} до {plateau_end}")
        print(f"   Это {len(plateau_cutoffs)} из {len(cutoff_sizes)} тестируемых значений")

    print("="*70)

if __name__ == "__main__":
    main()