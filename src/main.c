#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>

#include<sys/stat.h>


#define SCRAMBLED_OFFSET    (1)
#define LENGTH_SCRAMBLED_FILENAME   (5)


struct settings {
    char* target;
    short unsigned is_dir;
    short unsigned keep_name; 
    short unsigned encrypt;
    short unsigned scramble_offset;
    char* dir;
};


int encrypt_file(struct settings *s) 
{
    // Encrypts file with ceaser cipher
    
    // Function assumes we are in same dir as file
    
    FILE *f;
    int c, offset; 
    char* temp;
    char* new_filename;
    char* filename;

    filename = s->target;
    offset = s->scramble_offset;

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

    free(temp);
    free(new_filename);


    return 0;
}


int decrypt_file(struct settings *s) 
{
    // Encrypts file with ceaser cipher
    
    FILE *f;
    int c, offset; 
    size_t filename_length;
    char* orig_filename, *temp, *filename;
    long file_length;

    filename = s->target;
    offset = s->scramble_offset;

    
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


int parse_arguments(int argc, char* argv[], struct settings* s)
{
    int status;
    char* indx;
    struct stat *st;
    int filename_length;

    // First check if valid number of argumnets
    if (argc != 3)
        return 1;

    // Check if valid flag
    if (strcmp(argv[1] , "-e") == 0) {
        s->encrypt = 1;
    } else if (strcmp(argv[1] , "-d") == 0) {
        s->encrypt = 0;
    } else {
        // Invalid flag
        return 3;
    }


    // Record parent directory 
    indx = rindex(argv[2] , '/');  
    s->dir = malloc(256);
    s->target = malloc(256);

    if (indx == NULL){
        getcwd(s->dir , 0);
        strcpy(s->target ,argv[2]);
    } else {
        filename_length = (indx - argv[2]);
        strncpy(s->dir , argv[2] , filename_length); 
        s->dir[filename_length + 1] = '\0';

        strcpy(s->target, argv[2] + filename_length + 1);
    } 

    // Then read file metadata
    st = malloc(sizeof(struct stat));
    status = stat(argv[2] , st);
   
    if (status == -1){
        // Target does not exist
        return 2;
    } 

    // Check if directory or regular file
    s->is_dir = ((st->st_mode & S_IFMT) == S_IFDIR); 
  
    s->keep_name = s->is_dir;

    // Set scramble offset 
    s->scramble_offset = SCRAMBLED_OFFSET;

    free(st);

    return 0; 

}

int main(int argc, char* argv[] ) 
{

    int status;
    time_t t;
    struct settings *s;
    

    // Allocate settings and parse argumnets
    s = malloc(sizeof(struct settings));
    status = parse_arguments(argc, argv, s);

    if (status == 1) {
        // Wrong number of arguments
        printf("Usage:\t%s <-e|-d> filename\n" , argv[0]);
        return 1;
    } else if ( status == 2) {
        // Target does not exist
        printf("Unable to open target\n");
        return 2;
    } else if (status == 3) {
        printf("Invalid flag:\t%s\n" , argv[1]);
        return 3;
    }


    // Seed random number generator
    srand( (unsigned) time(&t)); 
    
    if (s->is_dir){
        // TODO
         
    } else {
        
        // TODO - If already in current directory dont move 
        chdir(s->dir);

        if (s->encrypt) {
            status = encrypt_file(s);
        } else {
            status = decrypt_file(s);
        }
    }

    return 0;
}
