#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<string.h>



int main(int argc, char* argv[]) 
{
    int n_runs, status_sc, i, status_un;
    time_t t; 
    double tot_sc_time , tot_un_time;

    if (argc == 2) {
        // Set number of runs
        // Check that argument is entirly digits   
        if (strspn(argv[1] , "0123456789") == strlen(argv[1])){
            n_runs = atoi( argv[1] );
        } else {
            printf("Invalid integer argument.\n");
            return 1;
        }
    } else {
        n_runs = 10;
    }
    // TODO - Should propbably check that we are in correct dir
    
    // Make clean
    system("make -s clean"); 

    // make copy
    system("make -s copy");

    // TODO - Check that output is correct 
    // Do intitial run to check that program works correctly 
    status_sc = system("./lfo -e data2");
    status_un = system("./lfo -d data2");

    if ( system("diff -r data data2") == 1) {
       printf("Error, lfo has alterd contents of the directory data2\n"); 
       return 2;
    }

    if ( status_sc != 0 || status_un != 0) {
        printf("Error occurred during execution of lfo");
        return 3;
    }

    printf("Measuring execution time of lfo during %d runs...\n", n_runs);

    tot_sc_time = 0;
    tot_un_time = 0;
   
    // Run scramble and unscramble n_runs times
    for(i = 0; i < n_runs; i++) {
       
        // Count time of execution
        t = time(NULL);

        system("./lfo -e data2");
       
        t = time(NULL) - t;
        
        tot_sc_time += (double)t;
        
        t = time(NULL);

        system("./lfo -d data2");

        t = time(NULL) - t;

        tot_un_time += (double)t;

    }

    // Count mean execution time
    printf("Mean scramble execution time:\t%2.4f\n", tot_sc_time / ( (double) n_runs));
    printf("Mean unscramble execution time:\t%2.4f\n", tot_un_time / ( (double) n_runs));
    
    return 0;
}
