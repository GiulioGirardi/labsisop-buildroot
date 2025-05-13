/**
 * @brief A Linux user space program that communicates with the xtea_driver LKM.
 * It sends an encryption command to the LKM and reads the encrypted response.
 * The device must be called /dev/xtea_driver.
 *
 * Based on Derek Molloy's simple_driver test code (http://www.derekmolloy.ie/)
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH 512               ///< The buffer length for input/output

int main() {
    int ret, fd;
    char receive[BUFFER_LENGTH];         ///< The receive buffer from the LKM
    char command[BUFFER_LENGTH];         ///< The command to send to the LKM

    printf("Starting xtea_driver test code example...\n");

    // Open the device with read/write access
    fd = open("/dev/xtea_driver", O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device...");
        return errno;
    }

    // Prompt user for the encryption command
    printf("Enter an encryption command (e.g., 'enc f0e1d2c3 b4a59687 78695a4b 3c2d1e0f 16 aabbccddeeff00112233445566778899'):\n");
    if (fgets(command, BUFFER_LENGTH, stdin) == NULL) {
        perror("Failed to read input...");
        close(fd);
        return errno;
    }

    // Remove trailing newline
    command[strcspn(command, "\n")] = '\0';

    // Validate command length
    if (strlen(command) == 0) {
        fprintf(stderr, "Error: Empty command\n");
        close(fd);
        return EINVAL;
    }

    printf("Writing command to the device: [%s]\n", command);

    // Send the command to the LKM
    ret = write(fd, command, strlen(command));
    if (ret < 0) {
        perror("Failed to write the command to the device.");
        close(fd);
        return errno;
    }

    printf("Press ENTER to read back from the device...\n");
    getchar();

    printf("Reading from the device...\n");

    // Read the response from the LKM
    ret = read(fd, receive, BUFFER_LENGTH);
    if (ret < 0) {
        perror("Failed to read the response from the device.");
        close(fd);
        return errno;
    }

    // Display the received data in hexadecimal format
    printf("The received encrypted data (%d bytes) is:\n", ret);
    for (int i = 0; i < ret; i++) {
        printf("%02x", (unsigned char)receive[i]);
        if (i % 16 == 15) printf("\n"); // New line every 16 bytes
        else printf(" ");
    }
    if (ret % 16 != 0) printf("\n");

    printf("End of the program\n");

    close(fd);
    return 0;
}