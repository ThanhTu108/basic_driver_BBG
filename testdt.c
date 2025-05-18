#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>


#define GPIO_NUMBER_MAGIC 'G'
#define GPIO_SET_PIN _IOW(GPIO_NUMBER_MAGIC, 1, struct gpio_config)
#define GPIO_SET_VALUE _IOW(GPIO_NUMBER_MAGIC, 2, int)

struct gpio_config {
    int bank;
    int num;
};

int main() {
    int fd;
    struct gpio_config config;
    int value;

    fd = open("/dev/gpio_device", O_RDWR);
    if (fd < 0) {
        perror("Can not open driver");
        return -1;
    }

    printf("GPIO bank (0-3): ");
    scanf("%d", &config.bank);
    printf("GPIO number (0-31): ");
    scanf("%d", &config.num);

    if (ioctl(fd, GPIO_SET_PIN, &config) < 0) {
        perror("Error setting GPIO pin");
        close(fd);
        return -1;
    }
    printf("GPIO%d_%d selected\n", config.bank, config.num);

    while (1) {
        printf("1: HIGH\n2: LOW\nElse: Exit\nChoice: ");
        scanf("%d", &value);

        if (value == 1) {
            if (ioctl(fd, GPIO_SET_VALUE, &value) < 0) {
                perror("Error setting HIGH");
            } else {
                printf("GPIO%d_%d set to HIGH\n", config.bank, config.num);
            }
        } else if (value == 2) {
            value = 0;
            if (ioctl(fd, GPIO_SET_VALUE, &value) < 0) {
                perror("Error setting LOW");
            } else {
                printf("GPIO%d_%d set to LOW\n", config.bank, config.num);
            }
        } else {
            printf("Exiting...\n");
            break;
        }
    }

    close(fd);
    return 0;
}
