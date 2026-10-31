#include <stdio.h>
int main() {
    printf("Ransomware: Attempting to encrypt secret.txt...\n");
    FILE *f = fopen("secret.txt", "w");
    if (f) {
        fprintf(f, "LOCKED_BY_RANSOMWARE\n");
        fclose(f);
        printf("Ransomware: File written successfully.\n");
    }
    return 0;
}
