#include <lib/Scheduler.cpp>
#include <string>
#include <iostream>
#include <cmath>

std::string operator*(const std::string& st, int a) {
    std::string ans;
    for (int i = 0; i < a; i++) {
        for (auto& el : st) {
            ans.push_back(el);
        }
    }
    return ans;
}

int main() {
    struct S {
        S(float a, float b) : a(a), b(b) {}

        float operator()(int c) {
            return static_cast<float>(c) + a + b;
        }

        float a;
        float b;
    };
    S s(2.5, 3.6);
    TTaskScheduler s1;
    int d = 4;
    auto id1 = s1.add(s, d);
    std::cout << s1.getResult<float>(id1);
}