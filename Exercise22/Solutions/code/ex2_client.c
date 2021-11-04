#include <stdio.h>
#include <stdlib.h>
//#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
//#include <ctype.h>
#include <time.h>
//Function declaration
int IsValidNumber(char* string);
int IsValidOperation(char* string);

int IsValidNumber(char* string)
{
    for(int i = 0; i < strlen( string ); i ++)
    {
        //ASCII value of 0 = 48, 9 = 57. So if value is outside of numeric range then fail
        if (string[i] < 48 || string[i] > 57)
            return 0;
    }
    return 1;
}

int IsValidOperation(char* string)
{
    if(strlen(string) == 1)
        if (string [0] == '1' || string [0] == '2' || string [0] == '3' || string [0] == '4')
            return 1;
    return 0;
}

void my_handler()
{
    signal(SIGUSR2, my_handler);
}


int main(int argc, char **argv) {
    if(!(argc == 5 && IsValidNumber(argv[1]) == 1 && IsValidNumber(argv[2])==1 && IsValidOperation(argv[3])==1 && IsValidNumber(argv[4])==1))
    {
        perror("Invalid arguments values");
        exit(-1);
    }
    if(strcmp(argv[3],"4")==0 && strcmp(argv[4],"0")==0) //division by zero
    {
        perror("Invalid values of operation, cannot divide a number by zero");
        exit(-1);
    }

    int pid_server = atoi(argv[1]);
    int val1 = atoi(argv[2]);
    int val2 = atoi(argv[4]);
    int calc_operation = atoi(argv[3]);

    // Check if the to_srv file already exist
    FILE * fp_check;
    // Use current time as seed for random generator
    srand(time(0));
    int flag = 0;
    for (int i = 0; i < 10; ++i) {
        fp_check = fopen("to_srv.txt", "r");
        if(fp_check != NULL){ // The file is already exist
            fclose(fp_check);
            //  randomize time between 1 sec to 5 sec
            int num = (rand() % 5) + 1;
            sleep(num); // sleep random time between 1 to 5 sec
        }else{
            flag = 1;
            break;
        }
    }

    if(flag == 0){ //The client did not succeed to create the to_srv.txt file
        perror("Cannot create the to_srv.txt file");
        exit(-1);
    }

//    Create the file to server
    FILE * fp = fopen("to_srv.txt", "w");

    pid_t client_process_id = getpid();

    char str_write_to_file[50];
    memset(str_write_to_file, '\0', sizeof(str_write_to_file));

    sprintf(str_write_to_file, "%d\n%d\n%d\n%d\n", client_process_id , val1, val2, calc_operation);

    fwrite(str_write_to_file, sizeof(char), strlen(str_write_to_file), fp);
    fclose(fp);

    // Send signal to the server
    kill(pid_server, SIGUSR1);

    signal(SIGUSR2, my_handler);
    pause();

    char filename[50];
    memset(filename, 0x00, sizeof(filename));

    sprintf(filename, "to_client_%d.txt", client_process_id);

    FILE * client_fp = fopen(filename, "r");

//  Get the calculation result from the to_client_xxxxxx.txt file

    char *line = NULL;
    size_t len = 0;
    ssize_t line_size;

    char calc_res[50];
    memset(calc_res, 0x00, sizeof(calc_res));
    size_t calc_res_len;


    int lineNum = 0;
    while ((line_size = getline(&line, &len, client_fp)) != -1) {
        if (lineNum == 0) {
            strcpy(calc_res, line);
            //no '\n' in the end
            calc_res_len = line_size;
        }else {
            perror("Invalid to_client_xxxxxx.txt file");
            fclose(client_fp);
            if (line != NULL || line != 0)
                free(line);
            exit(-1);
        }
        lineNum += 1;
    }
//      Deallocate memory
    if (line != NULL || line != 0)
        free(line);

//  Close the to_client_xxxxxx.txt file
    fclose(client_fp);

    // Delete the to_client_xxxxxx.txt file using rm command
    char* rm_command = "rm";
    char* rm_command_argument_list[] = {rm_command, filename, NULL};
    if(fork()==0)
        execvp(rm_command, rm_command_argument_list); // Already checked the Invalidation of the file
    else
        wait(NULL);

    int calc_res_integer = atoi(calc_res);

//  Print to STDOUT the calculation result
    printf("The calculation result: %d\n",calc_res_integer);

    return 0;
}