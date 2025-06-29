#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

bool flag = false;

void* waiter(void* arg) {
    printf("%s() -> waiting for flag to be true\n", __func__);
    while (!flag) {
        // Busy waiting — not safe!
    }
    printf("%s() -> flag is true\n", __func__);
    return NULL;
}

void* setter(void* arg) {
    // simulate some work

    sleep(1);
    printf("%s() -> setting flag to true\n", __func__);
    flag = true;
    printf("%s() -> flag set to true\n", __func__);
    return NULL;
}

int main() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, waiter, NULL);
    pthread_create(&t2, NULL, setter, NULL);
    pthread_join(t2, NULL);
    printf("%s() -> t2 joined\n", __func__);
    pthread_join(t1, NULL);
    printf("%s() -> t1 joined\n", __func__);

    return 0;
}

