#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "ipc.h"
#include <dirent.h>

#define BUFFER_SIZE 8192
#define COMMAND_SIZE 1024
#define VALUE_SIZE 4096

#define STRING_SIZE 256

#define ADMIN_INFO_FILENAME "admin_info.csv"

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

void *unix_main(void *args) {
    struct sockaddr_un addr;
    struct sockaddr_un from;
    int fd;
    int ret = 0;

    socklen_t fromlen = sizeof(from);
	int ok = 1;
	int len;
	

	if ((fd = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		ok = 0;
	}

	if (ok) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SERVER_SOCK_FILE);
		unlink(SERVER_SOCK_FILE);
		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("bind");
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
		printf("recvfrom: %s\n", buff);

        if (GetCommandAndValue(buff, command, value) != 0) {
            printf("GetCommandAndValue error\n");
            printf("buff = %s\n", buff);
            printf("command = %s\n", command);
            printf("value = %s\n", value);
            continue;
        }

        printf("buff = %s\n", buff);
        printf("command = %s\n", command);
        printf("value = %s\n", value);

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
                printf("Logged in successfuly\n");
                strcpy(buff, "Logged in successfuly\n");
                gotLogin = 1;
            } else {
                strcpy(buff, "Failed login\n");
                printf("Failed login %d\n", ret);
            }
        }

        //4. cerere resetare nume si parola
        if (gotLogin == 1 && strcmp(command, "reset") == 0 && strlen(name) > 0 && strlen(password) > 0) {
            ChangeAdminInfo(name, password);
            printf("gotreset");
        }

        //5. cerere verificare putere parola
        if (strcmp(command, "passpow") == 0) {
            ret = GetPasswordPower(password);
            printf("password power = %d\n", ret);
            sprintf(buff, "Power = %d\n", ret);
        }

        //6. cerere profil nume
        if (strcmp(command, "profile") == 0) {
            GetNameProfile(name, value);
            printf("name profile: %s\n", value);
            strcpy(buff, value);
        }

        //7. listeaza id-uri useri
        if (strcmp(command, "list") == 0) {
            ListDir(buff);
        }

		if (send(fd, buff, strlen(buff)+1, 0) == -1) {
			perror("send");
			//ok = 0;
		}
		printf ("sent iccExchangeAPDU\n");

		ret = sendto(fd, buff, strlen(buff)+1, 0, (struct sockaddr *)&from, fromlen);
		if (ret < 0) {
			perror("sendto");
			break;
		}
	}

	if (fd >= 0) {
		close(fd);
	}

    pthread_exit(NULL);
}

int main() {
    pthread_t ptUnix;

    unlink(SERVER_SOCK_FILE);
    pthread_create(&ptUnix, NULL, unix_main, SERVER_SOCK_FILE);

    pthread_join(ptUnix, NULL);
    
    return 0;
}