#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

void my_handler(int signum)
{
    signal(SIGUSR1, my_handler);
}


int main() {
//    Define signal
    signal(SIGUSR1, my_handler);
//    Print the server process id
    printf("%ld\n", (long)getpid());

    while(1)
    {
        pause();
        // Receive CLIENT to SERVER
        if(fork() == 0)
        {
            FILE *fp = fopen("to_srv.txt", "r");
            if (fp == NULL)
            {
                perror("Cannot open the to_srv.txt file");
                exit(-1);
            }

//        Get the lines from the to_srv.txt file

            char *line = NULL;
            size_t len = 0;
            ssize_t line_size;

            char client_process_id[50], val1[50], val2[50], calc_operation[50];
            size_t client_process_id_len, val1_len, val2_len, calc_operation_len;

            memset(client_process_id, 0x00, sizeof(client_process_id));
            memset(val1, 0x00, sizeof(val1));
            memset(val2, 0x00, sizeof(val2));

            int lineNum = 0;
            while ((line_size = getline(&line, &len, fp)) != -1) {

                if (lineNum == 0) {
                    strcpy(client_process_id, line);
                    //no need '\n' in the end
                    client_process_id_len = line_size - 1;
                    client_process_id[client_process_id_len] = '\0';
                } else if (lineNum == 1) {
                    strcpy(val1, line);
                    val1_len = line_size - 1;
                    val1[val1_len] = '\0';
                } else if (lineNum == 2) {
                    strcpy(val2, line);
                    val2_len = line_size - 1;
                    val2[val2_len] = '\0';
                } else if (lineNum == 3) {
                    strcpy(calc_operation, line);
                    calc_operation_len = line_size - 1;
                    calc_operation[calc_operation_len] = '\0';
                }
                else {
                    perror("Invalid to_srv.txt file");
                    fclose(fp);
                    if (line != NULL || line != 0)
                        free(line);
                    exit(-1);
                }
                lineNum += 1;
            }
//      Deallocate memory
            if (line != NULL || line != 0)
                free(line);

//      Close the to_srv.txt file
            fclose(fp);

            // Delete the to_srv.txt file using rm command
            // After the deletion of to_srv.txt file, the server is able to receive another client
            char* rm_command = "rm";
            char* rm_command_argument_list[] = {rm_command, "to_srv.txt", NULL};
            if(fork()==0) {
                execvp(rm_command, rm_command_argument_list);
            } // Already checked the Invalidation of the file
            else {
                wait(NULL);
            }

//      Convert the lines to integers
            int client_process_id_integer = atoi(client_process_id);
            int val1_integer = atoi(val1);
            int val2_integer = atoi(val2);
            int calc_operation_integer = atoi(calc_operation);
            int calc_res;

//        addition (+)
            if(calc_operation_integer==1){
                calc_res = val1_integer + val2_integer;
//      subtraction (-)
            }else if(calc_operation_integer==2){
                calc_res = val1_integer - val2_integer;
//      multiplication (*)
            }else if(calc_operation_integer==3){
                calc_res = val1_integer * val2_integer;
//      division (/)
            }else if(calc_operation_integer==4){
                if (val2_integer == 0) //division by zero
                {
                    perror("Invalid values of operation, cannot divide a number by zero");
                    exit(-1);
                }
                calc_res = val1_integer / val2_integer;
            }else {
                perror("Invalid value of calculation operation");
                exit(-1);
            }

            char filename[50];
            memset(filename, '\0', sizeof(filename));

            sprintf(filename, "to_client_%d.txt", client_process_id_integer);


            char calc_res_str[50];
            memset(calc_res_str, '\0', sizeof(calc_res_str));
            sprintf(calc_res_str, "%d", calc_res);

//      Create to_client_xxxxx.txt file
            FILE * fp_client = fopen(filename, "w+");
//      Write  to_client_xxxxx.txt file the calculation result
            fwrite(calc_res_str, sizeof(char), strlen(calc_res_str), fp_client);
            fclose(fp_client);

            // After writing to the to_client.tx file, send a signal back to the client
            kill(client_process_id_integer, SIGUSR2);
        }
    }
    exit(0);
}








