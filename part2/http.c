
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "http.h"

#define BUFSIZE 512

const char *get_mime_type(const char *file_extension) {
    if (strcmp(".txt", file_extension) == 0) {
        return "text/plain";
    } else if (strcmp(".html", file_extension) == 0) {
        return "text/html";
    } else if (strcmp(".jpg", file_extension) == 0) {
        return "image/jpeg";
    } else if (strcmp(".png", file_extension) == 0) {
        return "image/png";
    } else if (strcmp(".pdf", file_extension) == 0) {
        return "application/pdf";
    }

    return NULL;
}

int read_http_request(int fd, char *resource_name) {
    // copy file descriptor
    int fd_copy = dup(fd);
    if (fd_copy == -1){
        perror("dup");
        return -1;
    }
    // open stream system call
    FILE *socket_stream = fdopen(fd_copy , "r"); 
    if (socket_stream == NULL){
        perror("fdopen");
        fclose(socket_stream);
        return -1;
    }
    // Disable usual stdio buffering
    if (setvbuf(socket_stream, NULL , _IONBF , 0) != 0){
        perror("setvbuf");
        fclose(socket_stream);
        return -1;
    }


    char buf[BUFSIZE];
    fgets(buf, BUFSIZE, socket_stream);
    char *token = strtok(buf, " ");
    
    token = strtok(NULL , " ");
    strcpy(resource_name, token);

    while(fgets(buf, BUFSIZE, socket_stream) != NULL) { 
        if (strcmp(buf, "\r\n") == 0) {
            break;
        }
    }
    
    if (fclose(socket_stream) != 0) {
        perror("fclose");
        return -1;
    }
    return 0;
}

int write_http_response(int fd, const char *resource_path) {

    char buf[BUFSIZE] = "";

    struct stat statbuf;
    if(stat(resource_path,&statbuf) == -1){
        snprintf(buf,BUFSIZE, "HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        if (write(fd, buf, strlen(buf) + 1) == -1) {
            perror("write");
            return -1;
        }
        if (errno != ENOENT){
            perror("ENOENT");
            return -1;
        }
        return -1;
    }

    int fp = open(resource_path, O_RDONLY);
    if(fp == -1){
        perror("open");
        return -1;
    }

    const char *ptr = resource_path;
    const char *file_type;
    for(int i = strlen(resource_path); i >= 0; i--){
        file_type = &ptr[i];
        if (ptr[i] == '.'){
            file_type = &ptr[i];
            
            break;
        }
    }

    const char *Filetype = get_mime_type(file_type);

    snprintf(buf,BUFSIZE, "HTTP/1.0 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", Filetype,statbuf.st_size);
    if (write(fd, buf, strlen(buf)) == -1) {
        perror("write");
        return -1;
    }

    int bytes_read;
   int count = 0;


    while(count < statbuf.st_size){
        bytes_read = read(fp,buf,BUFSIZE);

        if (bytes_read == -1) {
            perror("read");
            close(fp);
            return -1;
        }
        if(write(fd,buf,bytes_read) == -1){
            perror("write");
            close(fp);
            return -1;
        }
        count += bytes_read;
    }
    return 0;

}