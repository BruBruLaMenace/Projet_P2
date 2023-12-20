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
    int fd_init = fd;
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
    char *test_path = "lib_tar.c";

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

    close(fd);
    return 0;
}