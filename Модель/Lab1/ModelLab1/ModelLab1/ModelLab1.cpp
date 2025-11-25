#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std;

int main()
{
    setlocale(LC_ALL, "Russian");
    // Создаём генератор случайных чисел с seed от времени
    random_device rd;
    mt19937 gen(rd()); // генератор, инициализированный случайным seed
    uniform_real_distribution<double> dist(0, 1); // диапазон [0.0, 1.0)

    // количество совершения события
    int x0 = 0, x1 = 0, x2 = 0, x3 = 0, x4 = 0; // заменить на словарь
    // всего событий
    int amount = 100000;

    int start = 4; // с какого состояния начинаем

    int m = 5; // размерность матрицы

    // матрица Маркова
    double matrix[5][5] = { {0, 0.5, 0.5, 0, 0},
                            {0, 0, 0.1, 0.5, 0.4},
                            {0, 0, 0.1, 0, 0.9},
                            {0, 0, 0.25, 0.05, 0.7},
                            {0.7, 0.3, 0, 0, 0} }; // вставить значения из отчёта

    
    for (int k = 0; k < amount; ++k) {
        // случайное число
        double x = dist(gen);
       // cout << "Вероятность " << x << endl;

        for (int i = 0; i < m; i++) {
            //cout << matrix[start][i] << " "; // проходим по строке
            x -= matrix[start][i];
            if (x < 0) { 
                start = i;
              //  cout << "переход в состояние " << start + 1 << endl;

                switch (start) {
                    case 0: 
                        ++x0;
                        break;
                    case 1:
                        ++x1;
                        break;
                    case 2:
                        ++x2;
                        break;
                    case 3:
                        ++x3;
                        break;
                    case 4:
                        ++x4;
                        break;
                }
                break;         
            }
        }

    }

    cout << "Количество всех событий: " << amount << endl;
    cout << endl;
    cout << "Количество свершений события 1: " << x0 << endl;
    cout << "Вероятность свершения события 1: " << x0 / double(amount) << endl;
    cout << endl;
    cout << "Количество свершений события 2: " << x1 << endl;
    cout << "Вероятность свершения события 2: " << x1 / double(amount) << endl;
    cout << endl;
    cout << "Количество свершений события 3: " << x2 << endl;
    cout << "Вероятность свершения события 3: " << x2 / double(amount) << endl;
    cout << endl;
    cout << "Количество свершений события 4: " << x3 << endl;
    cout << "Вероятность свершения события 4: " << x3 / double(amount) << endl;
    cout << endl;
    cout << "Количество свершений события 5: " << x4 << endl;
    cout << "Вероятность свершения события 5: " << x4 / double(amount) << endl;

    return 0;
}
