
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

/* ************************************************************** *
 * Constants
 * ************************************************************** */
 
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define POLL_TIMEOUT (3 * 1000) /* 3 seconds */
#define MAX_BUF 64

typedef enum GPIODirections {
	GPIO_In,
	GPIO_Out
} GPIODirection;

typedef enum GPIOValues {
	GPIO_Low,
	GPIO_High
} GPIOValue;

typedef enum GPIOEdges {
	GPIO_Rising,
	GPIO_Falling,
	GPIO_Both,
	GPIO_None
} GPIOEdge;

typedef unsigned int GPIO;
typedef int GPIOFD;


const char * pcGPIOEdgeStr_Rising  = "rising";
const char * pcGPIOEdgeStr_Falling = "falling";
const char * pcGPIOEdgeStr_Both    = "both";
const char * pcGPIOEdgeStr_None    = "none";


/**
 * @brief Export GPIO pin
 *
 * @param gpio The GPIO pin number to export
 * @return 0 on success, < 0 otherwise
 */
int gpio_export(GPIO gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
 
	return 0;
}


/**
 * @brief Unexport GPIO pin
 *
 * @param gpio The GPIO pin number to unexport
 * @return 0 on success, < 0 otherwise
 */
int gpio_unexport(GPIO gpio)
{
	int fd, len;
	char buf[MAX_BUF];
 
	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}
 
	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}


/**
 * @brief Set the direction of gpio
 *
 * @param gpio The GPIO pin number to set
 * @param dir GPIO direction to set
 * @return 0 on success, < 0 otherwise
 */
int gpio_setDirection(GPIO gpio, GPIODirection xDir)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
 
	if (xDir == GPIO_Out)
		write(fd, "out", 4);
	else if (xDir == GPIO_In)
		write(fd, "in", 3);
 
	close(fd);
	return 0;
}


/**
 * @brief Set the value of GPIO pin
 *
 * @param gpio The GPIO pin number to set
 * @param value to set (high or low)
 * @return 0 on success, < 0 otherwise
 */
int gpio_setValue(GPIO gpio, GPIOValue value)
{
	int fd, len;
	char buf[MAX_BUF];
 
	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}
 
	if (value == GPIO_High)
		write(fd, "1", 2);
	else if (value == GPIO_Low)
		write(fd, "0", 2);
 
	close(fd);
	return 0;
}


/**
 * @brief Get the value of GPIO pin
 *
 * @param gpio The GPIO pin number to set
 * @param value GPIO value read
 * @return 0 on success, < 0 otherwise
 */
int gpio_getValue(GPIO gpio, GPIOValue *value)
{
	int fd, len;
	char buf[MAX_BUF];
	char ch;

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		perror("gpio/get-value");
		return fd;
	}
 
	read(fd, &ch, 1);

	if (ch != '0') {
		*value = GPIO_High;
	} else {
		*value = GPIO_Low;
	}
 
	close(fd);
	return 0;
}


/**
 * @brief Set the interrupt edge of GPIO pin
 *
 * @param gpio The GPIO pin number to set
 * @param edge GPIO edge to set
 * @return 0 on success, < 0 otherwise
 */
int gpio_setEdge(GPIO gpio, GPIOEdge edge)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);
 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-edge");
		return fd;
	}
 
	if (edge == GPIO_Falling) {
		write(fd, pcGPIOEdgeStr_Falling, strlen(pcGPIOEdgeStr_Falling) + 1);
	} else if (edge == GPIO_Rising) {
		write(fd, pcGPIOEdgeStr_Rising, strlen(pcGPIOEdgeStr_Rising) + 1);
	} else if (edge == GPIO_Both) {
		write(fd, pcGPIOEdgeStr_Both, strlen(pcGPIOEdgeStr_Both) + 1);
	} else {
		write(fd, pcGPIOEdgeStr_None, strlen(pcGPIOEdgeStr_None) + 1);
	}

	close(fd);
	return 0;
}


/**
 * @brief Open the file descriptor of GPIO pin
 *
 * @param gpio The GPIO pin number
 * @return 0 on success, < 0 otherwise
 */
GPIOFD gpio_open(GPIO gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	len = snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);
 
	fd = open(buf, O_RDONLY | O_NONBLOCK );
	if (fd < 0) {
		perror("gpio/fd_open");
	}
	return fd;
}


/**
 * @brief Close the file descriptor of GPIO pin
 *
 * @param fd file descriptor of GPIO pin
 * @return 0 on success, < 0 otherwise
 */
int gpio_close(GPIOFD fd)
{
	return close(fd);
}

/****************************************************************
 * Main
 ****************************************************************/
int main(int argc, char **argv, char **envp)
{
	struct pollfd fdset[2];
	int nfds = 2;
	int gpio_fd, timeout, rc;
	char buf[MAX_BUF];
	GPIO gpio;
	int len;



	if (argc < 2) {
		printf("Usage: gpio-int <gpio-pin>\n\n");
		printf("Waits for a change in the GPIO pin voltage level or input on stdin\n");
		exit(-1);
	}

	gpio = atoi(argv[1]);

	gpio_export(gpio);
	gpio_setDirection(gpio, 0);
	gpio_setEdge(gpio, "rising");
	gpio_fd = gpio_open(gpio);

	timeout = POLL_TIMEOUT;
 
	while (1) {
		memset((void*)fdset, 0, sizeof(fdset));

		fdset[0].fd = STDIN_FILENO;
		fdset[0].events = POLLIN;
      
		fdset[1].fd = gpio_fd;
		fdset[1].events = POLLPRI;

		rc = poll(fdset, nfds, timeout);      

		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
      
		if (rc == 0) {
			printf(".");
		}
            
		if (fdset[1].revents & POLLPRI) {
			len = read(fdset[1].fd, buf, MAX_BUF);
			printf("\npoll() GPIO %d interrupt occurred\n", gpio);
		}

		if (fdset[0].revents & POLLIN) {
			(void)read(fdset[0].fd, buf, 1);
			printf("\npoll() stdin read 0x%2.2X\n", (unsigned int) buf[0]);
		}

		fflush(stdout);
	}

	gpio_close(gpio_fd);
	return 0;
}
