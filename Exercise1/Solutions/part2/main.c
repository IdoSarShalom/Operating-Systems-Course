#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SUCCESS_GRADE 100
#define FAIL_GRADE 0

int main(int argc, char *argv[]) {

    //Open the configuration file
    FILE *fp_config = fopen(argv[1], "r");
    if (fp_config == NULL)
    {
        perror("Invalid configuration file");
        exit(-1);
    }


    char *line = NULL;
    size_t len = 0;
    ssize_t line_size;

    char usersPath[50], inputPath[50], outputPath[50];
    size_t users_len, input_len, output_len;

    memset(usersPath, 0x00, sizeof(usersPath));
    memset(inputPath, 0x00, sizeof(inputPath));
    memset(outputPath, 0x00, sizeof(outputPath));

    int lineNum = 0;
    while ((line_size = getline(&line, &len, fp_config)) != -1) {

        if (lineNum == 0) {
            strcpy(usersPath, line);
            //no need '\n' in the end
            users_len = line_size - 1;
            usersPath[users_len] = '\0';
        } else if (lineNum == 1) {
            strcpy(inputPath, line);
            input_len = line_size - 1;
            inputPath[input_len] = '\0';
        } else if (lineNum == 2) {
            strcpy(outputPath, line);
            output_len = line_size - 1;
            outputPath[output_len] = '\0';
        } else {
            perror("Invalid configuration file");
            fclose(fp_config);
            if (line != NULL || line != 0)
                free(line);
            exit(-1);
        }

        lineNum += 1;
    }

    if (line != NULL || line != 0)
        free(line);

    //  Run ls in order to get all the files names

    char* ls_command = "ls";
    char* ls_command_argument_list[] = {"ls", usersPath, NULL};

    // Create a file of user List which includes all the students
    FILE* userList_fptr = fopen("userList.txt", "w+");

    // Create result file which includes the score of the students
    FILE* result_fptr = fopen("result.csv", "w+");

    //  Run ls shell command in order to get all the students names

    if (fork() == 0) {
        // Newly spawned child Process. This will be taken over by "ls "
        close(1);
        int fd1 = dup(fileno(userList_fptr));
        close(fileno(userList_fptr));
        int ret_code = execvp(ls_command, ls_command_argument_list);
        if (ret_code == -1){
            perror("Invalid path of sub folders - students path");
            fclose(fp_config);
            fclose(userList_fptr);
            fclose(result_fptr);
            exit(-1);
        }
    } else
        wait(NULL);

    // Close the students file
    fclose(userList_fptr);

    // Open the user list file for reading, iterate over each user name
    FILE *file_user_list_ptr = fopen("userList.txt", "r+");

    if (file_user_list_ptr == NULL) {
        perror("Invalid path of sub folders - students path");
        fclose(fp_config);
        fclose(result_fptr);
        exit(-1);
    }

    char *user_list_current_line = NULL;
    size_t user_list_current_len = 0;
    ssize_t user_list_current_line_size;

    while ((user_list_current_line_size = getline(&user_list_current_line, &user_list_current_len, file_user_list_ptr)) !=-1) {

        user_list_current_line_size--;
        user_list_current_line[user_list_current_line_size] = '\0';

        // Run the main.exe compiled file of the current student
        char bash_command[512];
        memset(bash_command, 0x00, sizeof(bash_command));

        char* bash_command_argument_list[] = {bash_command, inputPath, NULL};

        int res = sprintf(bash_command, "%s/%s/main.exe", usersPath, user_list_current_line);
        // Invalid bash command
        if (res < 0) {
            // size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
            char temp[50];
            sprintf(temp, "%s,%d\n", user_list_current_line, FAIL_GRADE);
            fwrite(temp, sizeof(char), strlen(temp), result_fptr);
            // Next student !
            continue;
        }

        // Create the output file of the current student
        FILE* user_output_file_pointer = fopen("user_output.txt", "w+");

        // Run the main.exe compiled file of the current student
        if (fork() == 0) {
            // Newly spawned child Process.
            close(1);
            int fd = dup(fileno(user_output_file_pointer));
            close(fileno(user_output_file_pointer));
            int ret_code = execvp(bash_command, bash_command_argument_list);
            if (ret_code == -1) {
                // size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream );
                // cracked compiled file main.exe of the current student
                char temp[50];
                sprintf(temp, "%s,%d\n", user_list_current_line, FAIL_GRADE);
                fwrite(temp, sizeof(char), strlen(temp), result_fptr);
                // Next student !
                continue;
            }
        } else
            wait(NULL);

        fclose(user_output_file_pointer);

        // Use the comp program in order to compare the files
        int status_comp;
        char* comp_command = "./comp.out";
        char* comp_argument_list[] = {comp_command, "user_output.txt", outputPath, NULL};

        if (fork() == 0) {
            int ret_val = execvp(comp_command, comp_argument_list);
            if(ret_val==-1)
            {
                // Execvp failed
                char temp[50];
                sprintf(temp, "%s,%d\n", user_list_current_line, FAIL_GRADE);
                fwrite(temp, sizeof(char), strlen(temp), result_fptr);
                // Next student !
                continue;
            }
        } else
            wait(&status_comp);

        // Decode the status exit from fork()
        if (WEXITSTATUS(status_comp) == 2){
            char temp[50];
            sprintf(temp, "%s,%d\n", user_list_current_line, SUCCESS_GRADE);
            fwrite(temp, sizeof(char), strlen(temp), result_fptr);
            // Next student !
            continue;
        } else {
            // Student output.txt file is different from the expected output
            char temp[50];
            sprintf(temp, "%s,%d\n", user_list_current_line, FAIL_GRADE);
            fwrite(temp, sizeof(char), strlen(temp), result_fptr);
            // Next student !
            continue;
        }
    }

    // Delete the user_output.txt and userList.txt files using rm command
    char* rm_command = "rm";
    char* rm_command_argument_list[] = {rm_command, "user_output.txt", NULL};
    // Delete the user_output.txt file
    if(fork()==0)
        execvp(rm_command, rm_command_argument_list); // Already checked the Invalidation of the file
    else
        wait(NULL);

    // Delete the userList.txt file

    rm_command_argument_list[1] = "userList.txt" ;

    if(fork()==0)
    {
        execvp(rm_command, rm_command_argument_list);
        if (user_list_current_line != NULL || user_list_current_line != 0)
            free(user_list_current_line);
        // In case of an error
        perror("Invalid path of sub folders - students path");
        fclose(file_user_list_ptr);
        fclose(fp_config);
        fclose(result_fptr);
        exit(-1);
    }
    else
        wait(NULL);


    if (user_list_current_line != NULL || user_list_current_line != 0)
        free(user_list_current_line);
    fclose(file_user_list_ptr);
    fclose(fp_config);
    fclose(result_fptr);

    exit(0);
}