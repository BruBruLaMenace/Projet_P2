#include "lib_tar.h"
//lseek permet de bouger
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//size donne la taille des donne mais je dois bouger par block
// imaginons taille 1222 je dois bouger de 3 block

#define BLOCK_SIZE 512
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
int check_archive(int tar_fd) {
    int n_headers = 0;
    char last = '\0';
    unsigned int checksum = 0;
    while(true){
        
        tar_header_t header = get_header(tar_fd);
        
        if (header.name[0] == last) {
            // End of archive
            break;
        }       
        // Check magic value
        if (strncmp(header.magic, "ustar", 5) != 0 || header.magic[5] != last) {
            return -1;  // Invalid magic value
        }
        // Check version value
        if (strncmp(header.version, "00", 2) != 0 || header.version[2] != '\0') {
            return -2;  // Invalid version value
        }  
        
        // Check checksum
        checksum = 0;
        for (int i = 0; i < sizeof(header); i++) {
            checksum += ((unsigned char *)(&header))[i];
        }

        // Subtract the checksum field in the header
        checksum -= octalToDecimal(header.chksum);
        if (checksum != octalToDecimal(header.chksum)) {
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
    char last = '\0';
    while (1) {
        tar_header_t header = get_header(tar_fd);
        
        // Check for the end of the archive
        if (header.name[0] == last) {
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
    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
}




tar_header_t get_header(int tar_fd){

    tar_header_t header;
    if (read(tar_fd, &header,sizeof(header)) { //read avance avance d un lbock
        perror("Error reading header");
        return 1;
    }
    return header
       
     
}

int next_header(int tar_fd,tar_header_t heady){
    
    tar_header_t headino;
    int size = octalToDecimal(heady.size);
    int reste = (size % 512 != 0) ? 1 : 0; //si reste rajoute 1blockd e mouvement
    int movement = (size /512) + reste;
    lseek(tar_fd, movement*BLOCK_SIZE ,SEEK_CUR);
    headino = get_header(tar_fd);
    return headino;
        
}

int octalToDecimal(const char *str) {
    return strtol(str, NULL, 8);
}

