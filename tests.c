#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    printf("Opening file: %s\n", argv[1]);
    int fd = open(argv[1], O_RDONLY);
    
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    // Test de check_archive
    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);
    if (ret < 0) {
        close(fd);
        return -1; // Arrêter si l'archive n'est pas valide
    }

    // Réinitialiser la position du descripteur de fichier pour les tests suivants
    lseek(fd, 512, SEEK_SET);

    // Utiliser un chemin valide dans l'archive TAR
    char *test_path = "bonjour/"; 
    // on a mis l'offset à 512 car on voulait voir
    // le deuxième fichier, l'offset est à changer par un multiple de 512
    // en fonction de la posiiton du fichier qu'on regarder

    // Test de exists
    ret = exists(fd, test_path);
    printf("exists for %s returned %d\n", test_path, ret);   
    lseek(fd, 512, SEEK_SET); // Réinitialiser à nouveau

    // Test de is_file
    ret = is_file(fd, test_path);
    printf("is_file for %s returned %d\n", test_path, ret);
    lseek(fd, 512, SEEK_SET);

    // Test de is_dir
    ret = is_dir(fd, test_path);
    printf("is_dir for %s returned %d\n", test_path, ret);
    lseek(fd, 512, SEEK_SET);

    // Test de is_symlink
    ret = is_symlink(fd, test_path);
    printf("is_symlink for %s returned %d\n", test_path, ret);

    size_t no_entries = 10;  // Adjust as needed. Set this to the maximum expected number of entries.
    char *entries[no_entries];  // Array of pointers to store entry names
    for (size_t i = 0; i < no_entries; i++) {
        entries[i] = malloc(512 * sizeof(char));  // Allocate memory for each entry name
    }

    // Make sure test_path is a directory with a trailing slash.
    ret = list(fd, test_path, entries, &no_entries);  // Call the list function
    printf("list for %s returned %d, no_entries: %zu\n", test_path, ret, no_entries);

    // Print the entries
    for (size_t i = 0; i < no_entries; i++) {
        printf("Entry %zu: %s\n", i, entries[i]);
        free(entries[i]);  // Free the allocated memory after printing
    }

    size_t file_offset = 0; // Starting at the beginning of the file
    size_t read_length = 512; // Adjust this to the expected size of the file you are reading
    uint8_t *read_buffer = malloc(read_length); // Allocate a buffer for the file contents

    if (read_buffer == NULL) {
        perror("malloc");
        // Free previously allocated entries
        for (size_t i = 0; i < no_entries; i++) {
            free(entries[i]);
        }
        close(fd);
        return -1;
    }

    // Initialize read_length with the size of the buffer allocated
    ssize_t read_result = read_file(fd, test_path, file_offset, read_buffer, &read_length);

    if (read_result >= 0) {
        // Successfully read the file, now print its contents
        printf("Read file '%s' with result %zd, read_length: %zu\n", test_path, read_result, read_length);
        debug_dump(read_buffer, read_length);
    } else {
        // Handle errors
        if (read_result == -1) {
            printf("File '%s' not found in the archive.\n", test_path);
        } else if (read_result == -2) {
            printf("Offset %zu is outside the file's total length.\n", file_offset);
        } else {
            printf("Unknown error occurred while reading the file.\n");
        }
    }

    free(read_buffer);


    close(fd);
    return 0;
}