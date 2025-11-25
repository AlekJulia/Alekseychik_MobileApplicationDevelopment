#include <iostream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>

using namespace std;


int main()
{
    setlocale(LC_ALL, "Russian");

    const int states = 3; // кол-во состояний

    double Q[states][states] = {
        { -0.5, 0.4, 0.1 },
        { 0.7, -1, 0.3 },
        { 0.2, 0.5, -0.7 }
    };

    vector<int> N_values = { 10, 1000, 10000, 100000 };

    int start_state = 1; 

    // генератор случайных чисел
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> uniform01(0.0, 1.0);

    cout << fixed << setprecision(6);

    for (int N_jumps : N_values) {
        // время, которое было проведено в состоянии 
        vector<double> time_in_state(states, 0.0);
        // всё время моделирования
        double total_time = 0.0;

        // текущее состояние
        int state = start_state;
        // число переходов
        int jumps_done = 0;

        while (jumps_done < N_jumps) {
            // лямбда - интенсивность выхода из текущего состояния
            double lambda = -Q[state][state];

            // время прибывания в текущем состоянии tau ~ Exp(lambda), среднее время пребывания = 1/lambda
            double alpha = uniform01(gen);      // равномерное число (0,1)
            double tau = -log(alpha) / lambda;  // формула τ = -(1/λ)*ln(α)

            time_in_state[state] += tau; // прибавляем время прибывания в сост
            total_time += tau; // прибавляем время к общему таймеру

            // вероятность перехода
            double r = uniform01(gen); // равномерное случ число (0, 1)
            double accum = 0.0; 
            int next_state = state;

            // перебираем состояния кроме текущего
            for (int j = 0; j < states; ++j) { 
                if (j == state) continue; // пропускаем текущее состояние
                double pij = Q[state][j] / lambda;  // вероятность перейти в j
                accum += pij;
                if (r < accum) { // если пропали в интервал, то выбираем это состояние 
                    next_state = j;
                    break;
                }
            }

            // новое состояние
            state = next_state;
            ++jumps_done; // увеличием счётчик состояний
        } 

        cout << "============================================\n";
        cout << "Количесво переходов = " << N_jumps << "\n";
        cout << "Всё время моделирования = " << total_time << "\n";

        if (total_time > 0.0) {
            for (int i = 0; i < states; ++i) {
                double pi_emp = time_in_state[i] / total_time;
                cout << "Состояние " << (i + 1) << ": суммарное время = " << time_in_state[i]
                    << ",  вероятность = " << pi_emp << "\n";
            }
        } 
    } 

    return 0;
}