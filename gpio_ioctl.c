#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h> 

#define GPIO_SET_PIN  _IOW('a', 1, int32_t*)
#define GPIO_SET_HIGH _IOW('a', 2, int32_t*)
#define GPIO_SET_LOW  _IOW('a', 3, int32_t*)

int main() {
    int fd;
    int32_t gpio_pin, value;

    fd = open("/dev/my_gpioo", O_RDWR); 
    if (fd < 0) {
        perror("Can not open driver");
        return -1;
    }
    printf("GPIO: ");
    scanf("%d", &gpio_pin);
    if (ioctl(fd, GPIO_SET_PIN, &gpio_pin) < 0) { 
        perror("Error setting GPIO pin");
        close(fd);
        return -1;
    }
    printf("GPIO %d selected\n", gpio_pin);

    // Vòng lặp điều khiển
    while (1) {
        printf("1: HIGH\n2: LOW\nElse: Exit\n");
        scanf("%d", &value);

        if (value == 1) {
            if (ioctl(fd, GPIO_SET_HIGH, &value) < 0) { 
                perror("Error setting HIGH");
            } else {
                printf("GPIO %d set to HIGH\n", gpio_pin);
            }
        } else if (value == 2) {
            if (ioctl(fd, GPIO_SET_LOW, &value) < 0) { 
                perror("Error setting LOW");
            } else {
                printf("GPIO %d set to LOW\n", gpio_pin);
            }
        } else {
            printf("Exiting...\n");
            break;
        }
    }

    close(fd);
    return 0;
}
