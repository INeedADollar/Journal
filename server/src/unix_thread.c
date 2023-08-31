#include "logger.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define BUFFER_SIZE 8192
#define COMMAND_SIZE 1024
#define VALUE_SIZE 4096

#define STRING_SIZE 256

#define ADMIN_INFO_FILENAME "admin_info.csv"

#define SERVER_SOCK_FILE "server.sock"

int GetCommandAndValue(char buf[BUFFER_SIZE], char command[COMMAND_SIZE], char value[VALUE_SIZE]) {
    int i = 0;

    for (i = 0; i < BUFFER_SIZE; i++) {
        if (buf[i] == '\n' || buf[i] == '\0') { 
            return 1; 
        
        } else if (buf[i] == '$') {
            command[i] = '\0';
            break;
        }

        command[i] = buf[i];
    }

    i++;
    for (int j = 0; j < BUFFER_SIZE && j < VALUE_SIZE; j++, i++) {
        if (buf[i] == '\n' || buf[i] == '\0') {
            value[j] = '\0';
            break;
        }

        value[j] = buf[i];
    }

    if (strlen(command) == 0) return 2;

    return 0;
}

int TryLogin(char name[STRING_SIZE], char password[STRING_SIZE]) {
    FILE *file = fopen(ADMIN_INFO_FILENAME, "r");
    char fname[STRING_SIZE];
    char fpassword[STRING_SIZE];
    char line[STRING_SIZE];


    fgets(line, STRING_SIZE, file);
    fclose(file);

    //printf("login line = %s\n\n", line);

    int i;
    for (i = 0; i < STRING_SIZE; i++) {
        if (line[i] == '\r' || line[i] == '\n' || line[i] == '\0') return 1;
        else if (line[i] == ',') {
            fname[i] = '\0';
            break;
        }

        fname[i] = line[i];
    }

    i++;
    for (int j = 0; j < STRING_SIZE && i < STRING_SIZE; j++, i++) {
        if (line[i] == '\r' || line[i] == '\n' || line[i] == '\0') {
            fpassword[j] = '\0';
            break;
        }

        fpassword[j] = line[i];
    }

    //printf("file %s <--> %s\n", fname, fpassword);
    //printf("given %s <--> %s\n", name, password);

    /*for (int i = 0; i < STRING_SIZE; i++) {
        if (name[i] != fname[i] || password[i] != fpassword[i]) {
            return 2;
        }
    }*/

    if (strcmp(name, fname) == 0 && strcmp(password, fpassword) == 0) return 0;

    return 2;
}

void ChangeAdminInfo(char name[STRING_SIZE], char password[STRING_SIZE]) {
    FILE *file = fopen(ADMIN_INFO_FILENAME, "w");
    fprintf(file, "%s,%s\n", name, password);
    fclose(file);
}

int getPasswordScore(int upper, int lower, int digits, int signs) {
    return (upper > 0) + (lower > 0) + (digits > 0) + (signs > 0);
}

int GetPasswordPower(char password[STRING_SIZE]) {
    int upper = 0;
    int lower = 0;
    int digits = 0;
    int signs = 0;

    for (int i = 0; i < STRING_SIZE; i++) {
        if (password[i] == '\0') break;
        if (password[i] >= 'A' && password[i] <= 'Z') upper++;
        else if (password[i] >= 'a' && password[i] <= 'z') lower++;
        else if (password[i] >= '0' && password[i] <= '9') digits++;
        else if (password[i] > 32) signs++;
    }

    return getPasswordScore(upper, lower, digits, signs);
}

void GetNameProfile(char name[STRING_SIZE], char str[STRING_SIZE]) {
    int consonants = 0;
    int symbols = 0;
    int vowels = 0;
    int digits = 0;

    for (int i = 0; i < STRING_SIZE; i++) {
        char ch = name[i];
        
        if (ch == '\0' || ch == '\n') {
            break;
        }

        if (strchr("aeiouAEIOU", ch) != NULL) vowels++;
        else if (strchr("0123456789", ch) != NULL) digits++;
        else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) consonants++;
        else symbols++;
    }

    sprintf(str, "consonants: %d\nsymbols: %d\nvowels: %d\ndigits: %d\n", consonants, symbols, vowels, digits);
}

void ListDir(char buff[BUFFER_SIZE]) {
    DIR *jorunalDir = opendir("./journals");
    struct dirent *dir;

    buff[0] = '\0';
    if (jorunalDir) {
        while ((dir = readdir(jorunalDir)) != NULL) {
            strcat(buff, dir->d_name);
        }
        closedir(jorunalDir);
    }

}

void *unix_thread(void *args) {
    struct sockaddr_un addr;
    struct sockaddr_un from;
    int fd;
    int ret = 0;

    socklen_t fromlen = sizeof(from);
	int ok = 1;
	int len;
	
    unlink(SERVER_SOCK_FILE);
	if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
		log_error("socket");
		ok = 0;
	}

	if (ok) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SERVER_SOCK_FILE);
		unlink(SERVER_SOCK_FILE);
		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			log_error("bind");
			ok = 0;
		}
	}

    char buff[BUFFER_SIZE];
    char command[COMMAND_SIZE];
    char value[VALUE_SIZE];

    char name[STRING_SIZE];
    char password[STRING_SIZE];
    int gotName = 0;
    int gotPassword = 0;
    int gotLogin = 0;

	while ((len = recvfrom(fd, buff, BUFFER_SIZE, 0, (struct sockaddr *)&from, &fromlen)) > 0) {
		log_debug("recvfrom: %s", buff);

        if (GetCommandAndValue(buff, command, value) != 0) {
            log_error("GetCommandAndValue error");
            log_debug("buff = %s", buff);
            log_debug("command = %s", command);
            log_debug("value = %s", value);
            continue;
        }

        log_debug("buff = %s", buff);
        log_debug("command = %s", command);
        log_debug("value = %s", value);

        strcpy(buff, "ok\0");

        //1. cerere nume
        if (strcmp(command, "name") == 0) {
            strcpy(name, value);
            gotName = 1;
        }

        //2. cerere parola
        if (strcmp(command, "password") == 0) {
            strcpy(password, value);
            gotPassword = 1;
        }

        //3. cerere login
        if (strcmp(command, "login") == 0 && gotLogin == 0) {
            ret = TryLogin(name, password);
            if (ret == 0) {
                log_info("Logged in successfuly");
                strcpy(buff, "Logged in successfuly");
                gotLogin = 1;
            } else {
                strcpy(buff, "Failed login");
                log_error("Failed login %d", ret);
            }
        }

        //4. cerere resetare nume si parola
        if (gotLogin == 1 && strcmp(command, "reset") == 0 && strlen(name) > 0 && strlen(password) > 0) {
            ChangeAdminInfo(name, password);
            log_debug("gotreset");
        }

        //5. cerere verificare putere parola
        if (strcmp(command, "passpow") == 0) {
            ret = GetPasswordPower(password);
            log_debug("password power = %d", ret);
            sprintf(buff, "Power = %d", ret);
        }

        //6. cerere profil nume
        if (strcmp(command, "profile") == 0) {
            GetNameProfile(name, value);
            log_debug("name profile: %s", value);
            strcpy(buff, value);
        }

        //7. listeaza id-uri useri
        if (strcmp(command, "list") == 0) {
            ListDir(buff);
        }

		if (send(fd, buff, strlen(buff)+1, 0) == -1) {
			log_error("send %s", strerror(errno));
			//ok = 0;
		}
		log_info ("sent iccExchangeAPDU");

		ret = sendto(fd, buff, strlen(buff)+1, 0, (struct sockaddr *)&from, fromlen);
		if (ret < 0) {
			log_error("sendto %s", strerror(errno));
			break;
		}
	}

	if (fd >= 0) {
		close(fd);
	}

    pthread_exit(NULL);
}
