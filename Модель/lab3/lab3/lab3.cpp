#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <iomanip>

using namespace std;

int main() {

    setlocale(LC_ALL, "Russian");

    // ==== ПАРАМЕТРЫ СИСТЕМЫ ====
    double lambda = 0.8;    // интенсивность входящего потока
    double mu = 1.0;        // интенсивность обслуживания
    double rho = lambda / mu; // нагрузка системы

    // параметры остановки (количество ОБСЛУЖЕННЫХ заявок)
    vector<int> target_completed_list = { 10, 100, 1000, 10000 };

    // ==== ГЕНЕРАТОР СЛУЧАЙНЫХ ЧИСЕЛ ====
    mt19937 rng(chrono::high_resolution_clock::now().time_since_epoch().count());

    cout << fixed << setprecision(4);
    cout << "Параметры системы: lambda = " << lambda << ", mu = " << mu << ", rho = " << rho << endl;
    cout << "Теоретическое среднее число заявок в системе: " << rho << endl << endl;

    for (int target_completed : target_completed_list) {
        cout << "============================================" << endl;
        cout << "Целевое количество обслуженных заявок: " << target_completed << endl;

        // ==== ПЕРЕМЕННЫЕ ДЛЯ МОДЕЛИРОВАНИЯ ====
        double t = 0.0;         // текущее время
        int x = 0;              // текущее количество заявок в системе
        int completed = 0;      // счетчик обслуженных заявок
        int arrived = 0;        // счетчик поступивших заявок 

        // histogram[i] = суммарное время, которое система провела с i заявками
        vector<double> histogram(1, 0.0);

        // ==== ОСНОВНОЙ ЦИКЛ МОДЕЛИРОВАНИЯ ====
        while (completed < target_completed) {
            // Генерируем время до следующего события

            // Время до следующего поступления заявки
            double tau_arrival = exponential_distribution<double>(lambda)(rng);

            // Время до следующего ухода заявки
            // Свойство минимума: если x заявок, каждая уходит с интенсивностью mu,
            // то первая уйдет через Exp(mu * x)
            double tau_departure = (x > 0) ? exponential_distribution<double>(mu * x)(rng) : INFINITY;

            // Выбираем ближайшее событие
            double dt = min(tau_arrival, tau_departure);

            // Увеличиваем размер histogram если нужно
            if (x >= (int)histogram.size()) {
                histogram.resize(x + 1, 0.0);
            }

            // Добавляем время, проведенное в текущем состоянии
            histogram[x] += dt;
            t += dt;

            // Обрабатываем событие
            if (tau_arrival < tau_departure) {
                // Пришла новая заявка
                x++;
                arrived++;
            }
            else {
                // Ушла обслуженная заявка
                x--;
                completed++;
            }
        }

        // ==== ВЫЧИСЛЕНИЕ СТАТИСТИК ====
        double total_time = t;

        // Вычисляем эмпирическое среднее число заявок
        double empirical_mean = 0.0;
        for (int i = 0; i < histogram.size(); i++) {
            empirical_mean += i * (histogram[i] / total_time);
        }

        cout << "Общее время моделирования: " << total_time << endl;
        cout << "Поступило заявок: " << arrived << endl;
        cout << "Обслужено заявок: " << completed << endl;
        cout << "Теоретическое среднее: " << rho << endl;
        cout << "Эмпирическое среднее: " << empirical_mean << endl;

        // ==== СРАВНЕНИЕ РАСПРЕДЕЛЕНИЙ ====
        cout << "\nСравнение распределений:" << endl;
        cout << setw(5) << "i" << setw(12) << "Эмпир." << setw(12) << "Теор." << setw(12) << "Разность" << endl;

        double kolmogorov_distance = 0.0;
        double empirical_cdf = 0.0;
        double theoretical_cdf = 0.0;

        for (int i = 0; i < histogram.size(); i++) {
            double empirical_prob = histogram[i] / total_time;
            double theoretical_prob = pow(rho, i) * exp(-rho) / tgamma(i + 1);
            double difference = fabs(empirical_prob - theoretical_prob);

            // Вычисляем кумулятивные вероятности для расстояния Колмогорова
            empirical_cdf += empirical_prob;
            theoretical_cdf += theoretical_prob;
            double cdf_difference = fabs(empirical_cdf - theoretical_cdf);
            kolmogorov_distance = max(kolmogorov_distance, cdf_difference);

            // Выводим только значимые состояния
            if (empirical_prob > 1e-4 || theoretical_prob > 1e-4) {
                cout << setw(5) << i
                    << setw(12) << empirical_prob
                    << setw(12) << theoretical_prob
                    << setw(12) << difference << endl;
            }
        }

        cout << "Расстояние Колмогорова: " << kolmogorov_distance << endl;
        cout << endl;
    }

    // ==== ДОПОЛНИТЕЛЬНЫЕ ЭКСПЕРИМЕНТЫ ====
    cout << "============================================" << endl;
    cout << "ДОПОЛНИТЕЛЬНЫЕ ЭКСПЕРИМЕНТЫ" << endl;
    cout << "============================================" << endl;

    vector<pair<double, double>> parameters = {
        {0.5, 1.0},   // Низкая нагрузка (ρ = 0.5)
        {1.5, 1.0},   // Средняя нагрузка (ρ = 1.5)  
        {3.0, 1.0}    // Высокая нагрузка (ρ = 3.0)
    };

    int experiment_target = 10000;

    for (const auto& params : parameters) {
        double exp_lambda = params.first;
        double exp_mu = params.second;
        double exp_rho = exp_lambda / exp_mu;

        cout << "\nЭксперимент: lambda = " << exp_lambda << ", mu = " << exp_mu << ", rho = " << exp_rho << endl;

        // Сбрасываем переменные
        double t = 0.0;
        int x = 0;
        int completed = 0;
        vector<double> histogram(1, 0.0);

        while (completed < experiment_target) {
            double tau_arrival = exponential_distribution<double>(exp_lambda)(rng);
            double tau_departure = (x > 0) ? exponential_distribution<double>(exp_mu * x)(rng) : INFINITY;
            double dt = min(tau_arrival, tau_departure);

            if (x >= (int)histogram.size()) {
                histogram.resize(x + 1, 0.0);
            }

            histogram[x] += dt;
            t += dt;

            if (tau_arrival < tau_departure) {
                x++;
            }
            else {
                x--;
                completed++;
            }
        }

        // Вычисляем статистику
        double empirical_mean = 0.0;
        for (int i = 0; i < histogram.size(); i++) {
            empirical_mean += i * (histogram[i] / t);
        }

        cout << "Теоретическое среднее: " << exp_rho << endl;
        cout << "Эмпирическое среднее: " << empirical_mean << endl;

        // Находим наиболее вероятные состояния
        cout << "Наиболее вероятные состояния: ";
        double max_prob = 0.0;
        int most_probable = 0;

        for (int i = 0; i < histogram.size(); i++) {
            double prob = histogram[i] / t;
            if (prob > max_prob) {
                max_prob = prob;
                most_probable = i;
            }
        }
        cout << most_probable << " (вероятность " << max_prob << ")" << endl;
    }

    return 0;
}