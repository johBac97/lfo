#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>

#define SCRAMBLED_OFFSET    (1)
#define LENGTH_SCRAMBLED_FILENAME   (5)


int encrypt_file(char* filename , int offset) 
{
    // Encrypts file with ceaser cipher
    
    FILE *f;
    int c; 
    char* temp;
    char* new_filename;

    f = fopen(filename, "r+");

    // Check if we were able to open the file
    if (f == NULL) {
        printf("Unable to open file:\t%d\n" , errno);
        return 1; // ERROR
    }


    temp = malloc(12);
    
    fseek(f, -9 , SEEK_END);
    fgets(temp , 10 , f);
    if (strcmp(temp , "scrambled") == 0) {
        // File is already scrambled return.
        return 2;
    }

    // First add filename to end of file along with the legnth of the filename
    fseek(f , 0 , SEEK_END);
    fputs(filename , f);  
    fputc( (unsigned)strlen(filename) , f);
    fseek(f , 0 , SEEK_SET);

    while ( ( c = fgetc(f)) != EOF) {
        // Increment byte with offset, if larger than 255 do modulo
        c = (c + offset) % 256;

        // Move cursor back one step and write
        fseek(f , -1 , SEEK_CUR);

        fputc(c , f ); 
    }

    // Now finally add 8 bytes identifying that this file is scrambled
    fputs("scrambled", f);

    // Close file
    fclose(f); 

    // Now generate new random filename
    new_filename = malloc(LENGTH_SCRAMBLED_FILENAME + 1); 
    for (int i = 0; i < LENGTH_SCRAMBLED_FILENAME; i++)
        new_filename[i] = rand() % 26 + 'a';
    new_filename[LENGTH_SCRAMBLED_FILENAME] = '\0';

    rename(filename, new_filename);

    return 0;
}


int decrypt_file(char* filename , int offset) 
{
    // Encrypts file with ceaser cipher
    
    FILE *f;
    int c; 
    size_t filename_length;
    char* orig_filename, *temp;
    long file_length;

    
    f = fopen(filename, "r+");

    // Check if we were able to open the file
    if (f == NULL) {
        printf("Unable to open file:\t%d\n" , errno);
        return 1;  // ERROR: unable to open file
    }

    // Program supports filenames up to 255 characters long
    orig_filename = malloc(255);

    temp = malloc(12);
    
    // First check to see that file is really scrambled before trying to decrypt
    fseek(f, -9 , SEEK_END);
    fgets(temp , 10 , f);
    if (strcmp(temp , "scrambled") != 0) {
        // File is not scrambled return.
        return 2;
    }

    // Also get file length while we are here
    file_length = ftell(f);

    // reset cursor  
    fseek(f , 0 , SEEK_SET);

    // will read and "unscramble" filename as well but doesnt matter file is truncated later
    while ( ( c = fgetc(f)) != EOF) {
        // Increment byte with offset, if larger than 255 do modulo
        c = (c - offset + 256 ) % 256;

        // Move cursor back one step and write
        fseek(f , -1 , SEEK_CUR);

        fputc(c , f ); 
    }

    fseek(f , -10 , SEEK_END);
   
    // Read this byte which is the lengt of the filename
    filename_length = fgetc(f);
    
    // Move cursor to beginning of filename and read
    fseek(f , -(filename_length + 10) , SEEK_END);
    fgets(orig_filename , filename_length + 1, f); 
    
    // Close file
    fclose(f); 

    // Truncate file
    truncate(filename , file_length - (filename_length + 10));     

    // And rename file to its original name
    rename(filename, orig_filename);

    free(orig_filename);
    free(temp);

    return 0;
}

int main(int argc, char* argv[] ) 
{

    int operation_status;
    time_t t;

    // Expects mode, either encrypt or decrypt and a filename to operate on 
    if (argc != 3) {
        printf("usage: %s -e|-d filname\n" , argv[0]);
        exit(1);
    }

    // Seed random number generator
    srand( (unsigned) time(&t)); 

    if (strcmp( argv[1] , "-e") == 0) {
        // Encrypt file
        operation_status = encrypt_file(argv[2], SCRAMBLED_OFFSET);

    } else if (strcmp(argv[1] , "-d") == 0) {
        // Decrypt file
        operation_status = decrypt_file(argv[2] , SCRAMBLED_OFFSET);
    } else {
        printf("Invalid mode\n");
        exit(1);
    }

    return 0;
}
