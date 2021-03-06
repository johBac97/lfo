#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<time.h>
#include<dirent.h>


#include<sys/stat.h>


// LIGHT FILE OBFUSCATOR

#define BUFFER_SIZE                 (4096)
#define SCRAMBLED_OFFSET            (99)
#define LENGTH_SCRAMBLED_FILENAME   (10)
#define NUMBER_ROUNDS               (30)

struct settings {
    char* target;
    short unsigned is_dir;
    short unsigned keep_name; 
    short unsigned scramble;
    short unsigned scramble_offset;
    unsigned n_rounds;
};


/*
 * Scrambles the contents of a file with a Ceasar cipher. Assumes file path is in current dir.
 */
int scramble_file(struct settings *s) 
{
     
    FILE *f;
    long file_size;
    unsigned n_rounds;
    char* temp;
    char* new_filename;
    char* filename, * buffer;
    size_t n_read , i;
    
    filename = s->target;

    f = fopen(filename, "r+");

    // Check if we were able to open the file
    if (f == NULL) {

        // Access denied
        if ( errno == EACCES) 
            printf("Permission denied:\t%s\n" , filename);
         
        errno = 0;
        return 1; // ERROR
    }

    // Get file size first. If smaller than 9 bytes obviously not scrambled
    if (fseek(f, 0 , SEEK_END) != 0)
        perror("Unable to set stream position to end of file.");
    file_size = ftell(f);

    if (file_size > 9) {
        temp = malloc(12);
       
        // At end of each scrambled file these sentry bytes are. 
        // If they are present the file is already scrambled 
        if ( fseek(f, -9 , SEEK_END) != 0)
            perror("Unable to set stream position");

        temp = fgets(temp , 10 , f);
        if (temp == NULL) 
            perror("Unable to read file");

        if (strcmp(temp , "scrambled") == 0) {
            // File is already scrambled return.
            free(temp);
            return 2;
        }
        free(temp);
    }

    // First add filename to end of file along with the length of the filename
    fseek(f , 0 , SEEK_END);
    fputs(filename , f);  

    fputc( (unsigned)strlen(filename) , f);
    fseek(f , 0 , SEEK_SET);

    buffer = malloc(BUFFER_SIZE);

    n_rounds = 0;

    do {
        // Read current contents
        n_read = fread(buffer, 1 , BUFFER_SIZE, f);
       
        // Alter buffer 
        for ( i = 0; i < n_read; i++)
            buffer[i] = (buffer[i] + s->scramble_offset ) % 256;

        // write back to file
        fseek(f , -n_read , SEEK_CUR);

        fwrite(buffer, 1, n_read, f);
        
        n_rounds++; 

    } while( n_read == BUFFER_SIZE && n_rounds < s->n_rounds);

    // In case the cursor has passed the EOF we reset it
    fseek(f, 0 , SEEK_END);

    // Now finally add 8 bytes identifying that this file is scrambled
    fputs("scrambled", f);

    // Close file
    fclose(f); 

    // If enabled scramble the filename as well
    if (!s->keep_name) {
        new_filename = malloc(LENGTH_SCRAMBLED_FILENAME + 1); 
        for (i = 0; i < LENGTH_SCRAMBLED_FILENAME; i++)
            new_filename[i] = rand() % 26 + 'a';
        new_filename[LENGTH_SCRAMBLED_FILENAME] = '\0';

        rename(filename , new_filename);
        free(new_filename);
    }

    free(buffer);

    return 0;
}


int unscramble_file(struct settings *s) 
{
    // Encrypts file with ceaser cipher
    
    FILE *f;
    unsigned n_rounds;
    size_t filename_length, n_read , i;
    char* orig_filename, *temp, *filename;
    long file_size;
    char* buffer;

    filename = s->target;
    
    f = fopen(filename, "r+");

    // Check if we were able to open the file
    if (f == NULL) {

        // Access denied
        if (errno == EACCES) 
            printf("%s:\tPermission denied\n", filename);

        errno = 0;
        return 1;  // ERROR: unable to open file
    }

    // Program supports filenames up to 255 characters long
    orig_filename = malloc(255);

    temp = malloc(12);
    
    // First check to see that file is really scrambled before trying to decrypt
    fseek(f, -9 , SEEK_END);
    temp = fgets(temp , 10 , f);
    if (temp == NULL) 
        perror("Unable to read from file");

    if (strcmp(temp , "scrambled") != 0) {
        // File is not scrambled return.
        return 2;
    }

    // Also get file length while we are here
    file_size = ftell(f);
    
    // reset cursor  
    fseek(f , 0 , SEEK_SET);

    buffer = malloc(BUFFER_SIZE);
    
    n_rounds = 0;

    do {
        // Read current contents
        n_read = fread(buffer, 1 , BUFFER_SIZE, f);
        
        // Alter buffer 
        for ( i = 0; i < n_read; i++)
            buffer[i] = (buffer[i] - s->scramble_offset + 256) % 256;

        // write back to file
        fseek(f , -n_read , SEEK_CUR);

        fwrite(buffer, 1, n_read, f);

        n_rounds++;

    } while( n_read == BUFFER_SIZE && n_rounds < s->n_rounds);

    fseek(f , -10 , SEEK_END);
   
    // Read this byte which is the lengt of the filename
    filename_length = fgetc(f);
    
    // Move cursor to beginning of filename and read
    fseek(f , -(filename_length + 10) , SEEK_END);
    orig_filename = fgets(orig_filename , filename_length + 1, f); 
    if (orig_filename == NULL) 
        perror("Unable to read filename from file");
    
    fclose(f); 

    // Truncate file
    if ( truncate(filename , file_size - (filename_length + 10)) != 0) 
        perror("Unable to truncate");

    // And rename file to its original name
    rename(filename, orig_filename);

    free(orig_filename);
    free(temp);
    free(buffer);

    return 0;
}

int scramble_dir(struct settings *s)
{
    int i;
    DIR *d;
    struct dirent *ep; 
    struct settings *s_next; 
    char* new_dir_name , *original_dir_name ;
    FILE *f;
    size_t n_read;

    // Get contents of directory
    d = opendir(s->target);

    if (d == NULL) {
        // Unable to read contents of directory
        return 1;
    }
    
    // Move to new dir
    if (chdir(s->target) != 0)
        perror("Unable to change directory");

    // Allocate settings for next target
    s_next = malloc(sizeof(struct settings));
    s_next->target = malloc(PATH_MAX);

    // Loop over all directory entries and scramble them
    // Do it recursivly for directoris
    while (( ep = readdir(d)) ) {

        // Set file type and check if skip
        if ((ep->d_type == DT_LNK) || (strcmp(ep->d_name, "..") == 0) || (strcmp(ep->d_name,".") == 0)) {
            // Don't follow sym links and skip special directories
            continue;
        } else if ( ep->d_type == DT_REG) {
            s_next->is_dir = 0;
        } else if ( ep->d_type == DT_DIR) {
            s_next->is_dir = 1;
        } else {
            printf("encounterd unknown file:\t%d\n" , ep->d_type);
            exit(1);
        }

        // Set the filename   
        s_next->target = strcpy(s_next->target, ep->d_name);
        if (s_next->target == NULL)
            perror("Unable to set target name");

        s_next->scramble = s->scramble; 

        s_next->scramble_offset = s->scramble_offset;

        s_next->keep_name = 0;

        s_next->n_rounds = s->n_rounds;
    
        // If it is a directory continue recursivly otherwise scramble/unscramble file
        if (s_next->is_dir) {
            scramble_dir(s_next);
        } else {
            if ( s_next->scramble)
                scramble_file(s_next);
            else 
                unscramble_file(s_next);
        }
    }
  
    // When finished close directory
    if (closedir(d) != 0)
        perror("Unable to close directory");

    // Check if we should rename the directory
    // If the ".original_dir_name" file is present then don't scramble dir name
    if (s->scramble && !s->keep_name && access(".original_dir_name" , F_OK) != 0) {
        
        // Access sets errno to 2
        errno = 0;

        // first we need to store original name somewhere 
        f = fopen(".original_dir_name" , "w");
        fputs(s->target , f); //Original name
        fclose(f);

        // now scramble that file
        strcpy(s_next->target, ".original_dir_name");
        s_next->keep_name = 1;

        scramble_file(s_next);

        new_dir_name = malloc(LENGTH_SCRAMBLED_FILENAME + 1); 
        for (i = 0; i < LENGTH_SCRAMBLED_FILENAME; i++)
            new_dir_name[i] = rand() % 26 + 'a';
        new_dir_name[LENGTH_SCRAMBLED_FILENAME] = '\0';

        // Move up
        if ( chdir("..") != 0) {
            perror("Unable to change directory");
        }

        rename(s->target ,  new_dir_name);

        free(new_dir_name);
    } else if (!s->scramble) {
        // Unscramble dir name

        original_dir_name = malloc(PATH_MAX);

        // Look for file containing original directory name
        f = fopen(".original_dir_name", "r");
        if (f == NULL) {
            // Unable to find file, this directgory was not name scrambled
            if ( chdir("..") != 0) {
                perror("Unable to change directory");
            }
        } else {
            n_read = fread(original_dir_name , 1, PATH_MAX, f);
            fclose(f);
            
            // Add null byte 
            original_dir_name[n_read] = '\0';

            // remove excess file
            remove(".original_dir_name");

            if ( chdir("..") != 0) {
                perror("Unable to change directory");
            }
            
            // rename it back to original
            rename(s->target , original_dir_name);
        }

        free(original_dir_name);
    } else {

        if ( chdir("..") != 0) {
            perror("Unable to change directory");
        }

    }
    // Free up memory 
    free(s_next->target);
    free(s_next); 

    return 0;
}



int parse_arguments(int argc, char* argv[], struct settings* s)
{
    int status;
    struct stat *st;
    char *dir , *indx;
    size_t target_name_len;  

    // First check if valid number of argumnets
    if (argc != 3)
        return 1;

    // Check if valid flag
    if (strcmp(argv[1] , "-e") == 0) {
        s->scramble = 1;
    } else if (strcmp(argv[1] , "-d") == 0) {
        s->scramble = 0;
    } else {
        // Invalid flag
        return 3;
    }

    s->target = malloc(PATH_MAX);
  
    // If an additional '/' at end of target remove it 
    target_name_len = strlen( argv[2]);
    if ( argv[2][target_name_len - 1] == '/') 
        argv[2][target_name_len -1] = '\0';
        


    // Move to directory and store relative filename
    indx = rindex(argv[2] , '/');
    if (indx != NULL) {
        // move to directory
        dir = malloc(PATH_MAX);
        dir = strncpy(dir, argv[2], (indx - argv[2]));
        if (chdir(dir) != 0) {
            perror("Unable to change directory");
            errno = 0;
        }
        free(dir);
        // Store file name
        s->target = strcpy(s->target, indx + 1);
    } else {
        // target is in same folder just copy name
        s->target = strcpy(s->target, argv[2]);

    }
       
    

    // Then read file metadata
    st = malloc(sizeof(struct stat));
    status = stat(s->target , st);
   
    if (status == -1){
        
        if (errno == ENOENT){
            // Target does not exist
            free(st);
            free(s->target);
            free(s);
            errno = 0;
            return 2;
        } else if (errno == EACCES){
            // Search permission denied 
            free(st);
            free(s->target);
            free(s);
            return 4;
        } else {
            perror("Unknown error:\t");
            return 5;
        }
        
    } 

    // Check if directory or regular file
    s->is_dir = ((st->st_mode & S_IFMT) == S_IFDIR); 
  
    s->keep_name = s->is_dir;

    // Set scramble offset 
    s->scramble_offset = SCRAMBLED_OFFSET;

    // Set number of rounds
    s->n_rounds = NUMBER_ROUNDS;

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
        printf("Usage:\t%s [ -e | -d ] target\n" , argv[0]);
        free(s);
        return 1;
    } else if ( status == 2) {
        // Target does not exist
        printf("Target does not exist:\t%s\n", argv[2]);
        return 2;
    } else if (status == 3) {
        printf("Invalid flag:\t%s\n" , argv[1]);
        free(s);
        return 3;
    } else if (status == 4) {
        printf("Access denied!\n");
        return 4;
    }

    // Seed random number generator
    srand( (unsigned) time(&t)); 
    
    if (s->is_dir){
        status = scramble_dir(s);
    } else {

        if (s->scramble) {
            status = scramble_file(s);
        } else {
            status = unscramble_file(s);
        }
    }
    
    free(s->target);
    free(s);

    return status;
}
