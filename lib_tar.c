#include "lib_tar.h"
//lseek permet de bouger
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//size donne la taille des donne mais je dois bouger par block
// imaginons taille 1222 je dois bouger de 3 block

#define ERROR_INDICATOR "ERROR"  // Define an error indicator
#define BLOCK_SIZE 512


int octalToDecimal(const char *str) {
    return strtol(str, NULL, 8);
}

tar_header_t get_header(int tar_fd){

    tar_header_t header;
    if (read(tar_fd, &header, sizeof(header)) < 0) {
        perror("Error reading header");
        strncpy(header.name, ERROR_INDICATOR, sizeof(header.name));
        // Ensure null-termination
        header.name[sizeof(header.name) - 1] = '\0';
    }
    return header;
}


tar_header_t next_header(int tar_fd,tar_header_t heady){
    tar_header_t headino;
    int size = octalToDecimal(heady.size);
    int reste = (size % 512 != 0) ? 1 : 0; //si reste rajoute 1blockd e mouvement
    int movement = (size /512) + reste;
    lseek(tar_fd, movement * BLOCK_SIZE ,SEEK_CUR);
    headino = get_header(tar_fd);
    return headino;
}

/*tar_header_t next_header(int tar_fd, tar_header_t current_header) {
    tar_header_t next_header;
    int size = octalToDecimal(current_header.size);

    // Calculer le nombre de blocs à sauter pour atteindre le prochain en-tête
    // Ajouter 1 pour tenir compte de l'en-tête actuel
    int blocks_to_skip = (size / BLOCK_SIZE) + (size % BLOCK_SIZE ? 1 : 0) + 1;

    // Se déplacer vers le prochain en-tête
    if (lseek(tar_fd, blocks_to_skip * BLOCK_SIZE, SEEK_CUR) == -1) {
        perror("Error moving to next header");
        // Gestion d'erreur, peut-être retourner un en-tête vide ou signaler une erreur
    }

    // Lire le prochain en-tête
    if (read(tar_fd, &next_header, sizeof(next_header)) < 0) {
        perror("Error reading next header");
        // Gestion d'erreur
    }

    return next_header;
}*/







/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
void print_header_info(int tar_fd) {
    char last = '\0';
    while(1) {
        tar_header_t header = get_header(tar_fd);
        
        if (header.name[0] == last) {
            break; // End of archive
        }

        printf("Magic: '%.*s', Version: '%.*s'\n", TMAGLEN, header.magic, TVERSLEN, header.version);

        header = next_header(tar_fd, header);
    }
}

int check_archive(int tar_fd) {
    int n_headers = 0;
    char last = '\0';
    // print_header_info(tar_fd);
    tar_header_t header = get_header(tar_fd);
    while(1){
        
        printf("Name: %s\n", header.name);
        printf("Mode: %s\n", header.mode);
        printf("UID: %s\n", header.uid);
        printf("Magic: %.6s\n", header.magic);
        printf("Version: %.2s\n", header.version);

        
        if (header.name[0] == '\0') {
            break; // End of archive
        }

        // Check magic value (POSIX format)
        if (strncmp(header.magic, TMAGIC , TMAGLEN ) != 0) {
            //printf("Magic: '%.6s'\n", header.magic);
            return -1;  // Invalid magic value
        }

        // Check version value
        if (strncmp(header.version, TVERSION , TVERSLEN) != 0) {
            return -2;  // Invalid version value
        }  
        
         // Check checksum
        unsigned int checksum = 0;
        char original_chksum[8];
        strncpy(original_chksum, header.chksum, 8); // Save original checksum
        memset(header.chksum, ' ', 8); // Set checksum field to spaces for calculation

        for (int i = 0; i < sizeof(header); i++) {
            checksum += ((unsigned char *)(&header))[i];
        }

        if (checksum != TAR_INT(original_chksum)) {
            return -3;  // Invalid checksum
        }

        n_headers++;
        header = next_header(tar_fd, header);
    }

    return n_headers;
}


/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {

    while (1) {
        tar_header_t header = get_header(tar_fd);
        
        // Check for the end of the archive
        if (header.name[0] == '\0') {
            break;
        }

        // Check if the header's path matches the specified path
        if (strcmp(header.name, path) == 0) {
            return 1; // Entry exists
        }
        
        // Move to the next header
        header = next_header(tar_fd, header);
    }

    return 0; // Entry not found
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    char last = '\0';
    //lseek(tar_fd, 0, SEEK_SET);  // Ensure we start at the beginning of the file

    while (1) {
        tar_header_t header = get_header(tar_fd);
        
        // Check for the end of the archive
        if (header.name[0] == last) {
            break;
        }

        // Check if the header's path matches the specified path and if it's a directory
        if (strcmp(header.name, path) == 0 && header.typeflag == DIRTYPE) {
            return 1; // Directory exists
        }
        
        // Move to the next header
        header = next_header(tar_fd, header);
    }
    // not found
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    char last = '\0';
    tar_header_t header = get_header(tar_fd);
    while (1) {
        
        
        // Check for the end of the archive
        if (header.name[0] == last) {
            break;
        }

        // Check if the header's path matches the specified path and if it's a regular file
        if (strcmp(header.name, path) == 0 && (header.typeflag == REGTYPE || header.typeflag == AREGTYPE)) {
            return 1; // File exists
        }
        
        // Move to the next header
        header = next_header(tar_fd, header);
    }

    return 0; // File not found
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    char last = '\0';
    lseek(tar_fd, 0, SEEK_SET);  // Ensure we start at the beginning of the file

    while (1) {
        tar_header_t header = get_header(tar_fd);
        
        // Check for the end of the archive
        if (header.name[0] == last) {
            break;
        }

        // Check if the header's path matches the specified path and if it's a symlink
        if (strcmp(header.name, path) == 0 && header.typeflag == SYMTYPE) {
            return 1; // Symlink exists
        }
        
        // Move to the next header
        header = next_header(tar_fd, header);
    }

    return 0; // Symlink not found
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
char last = '\0';
    size_t path_len = strlen(path);
    size_t max_entries = *no_entries;
    *no_entries = 0;

    lseek(tar_fd, 0, SEEK_SET);  // Start at the beginning of the file

    while (1) {
        tar_header_t header = get_header(tar_fd);
        
        // Check for the end of the archive
        if (header.name[0] == last) {
            break;
        }

        // Check if the entry is a direct child of the specified path
        if (strncmp(header.name, path, path_len) == 0) {
            char *entry_name = header.name + path_len;
            
            // Check if the entry is directly in the specified directory (not in a subdirectory)
            if (entry_name[0] != '\0' && (entry_name[0] != '/' || entry_name[1] == '\0')) {
                if (*no_entries < max_entries) {
                    strncpy(entries[*no_entries], header.name, BLOCK_SIZE);
                    entries[*no_entries][BLOCK_SIZE - 1] = '\0';  // Ensure null-termination
                    (*no_entries)++;
                }
            }
        }
        
        // Move to the next header
        header = next_header(tar_fd, header);
    }

    return *no_entries > 0 ? 1 : 0; // Return 1 if any entries found, 0 otherwise
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    char last = '\0';
    lseek(tar_fd, 0, SEEK_SET);  // Start at the beginning of the file

    while (1) {
        tar_header_t header = get_header(tar_fd);
        
        // Check for the end of the archive
        if (header.name[0] == last) {
            return -1;  // File not found
        }

        // Check if the header's path matches the specified path
        if (strcmp(header.name, path) == 0 && (header.typeflag == REGTYPE || header.typeflag == AREGTYPE)) {
            size_t file_size = octalToDecimal(header.size);

            // Check if offset is within the file size
            if (offset >= file_size) {
                return -2;  // Offset outside the file total length
            }

            lseek(tar_fd, BLOCK_SIZE * ((offset / BLOCK_SIZE) + 1), SEEK_SET); // Move to the start of file data
            ssize_t to_read = file_size - offset > *len ? *len : file_size - offset;
            ssize_t read_bytes = read(tar_fd, dest, to_read);
            
            if (read_bytes < 0) {
                return -1; // Error in reading file
            }

            *len = read_bytes;
            return file_size - offset > *len ? file_size - offset - *len : 0; // Return remaining bytes
        }
        
        // Move to the next header
        header = next_header(tar_fd, header);
    }

    return -1;  // File not found
}



