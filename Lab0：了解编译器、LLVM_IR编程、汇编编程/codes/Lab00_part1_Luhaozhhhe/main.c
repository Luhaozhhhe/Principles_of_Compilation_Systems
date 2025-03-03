#include<stdio.h>
#include<time.h>

int main() {
    int a, b, i, t, n;
    double totalTime = 0.00000; // 用于存储总时间
    int trials = 10000; // 测试次数

    scanf("%d", &n); // 读取输入
    clock_t start, end;

    for (int trial = 0; trial < trials; trial++) {
        start = clock(); // 每次试验开始时记录时间

        a = 0;
        b = 1;
        i = 1;

        while (i < n) {
            t = b;
            b = a + b;
            a = t;
            i = i + 1;
        }

        end = clock(); // 每次试验结束时记录时间
        totalTime += (double)(end - start) / CLOCKS_PER_SEC; // 累加每次试验的运行时间
    }

    printf("平均运行时间为：%f 秒\n", totalTime / trials); // 输出平均运行时间
    return 0;
}
