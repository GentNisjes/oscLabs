//
// Created by stijn on 11/18/24.
//

//CHILD
/* the child is the logger that will be activated with the create_log_process
 * 1) needs to open the file to write in
 * 2) close the writing file descriptor pipe_fd[1]
 * 3) fprint to the file if message is not longer then the maximum input characters
 * 4) fclose the file
 * 5) close the reading file descriptor pipe_fd[0]
*/
void start_logger() {

}

//PARENT
int write_to_log_process(char *msg) {
    return -1;
}

//PARENT
int create_log_process() {
    return -1;
}

//PARENT
int end_log_process() {
    return -1;
}