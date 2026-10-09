#include <iostream> 
#include <unistd.h>
#include <csignal> 
#include <sys/wait.h>
#include <sys/mman.h> 

using namespace std;



void handler(int signal){
     const char message[] = "NO Activity Detected!\n";

     write(1, message, sizeof(message) -1); 
} 


int main(int argc, char*argv[]) { 
    signal(SIGUSR1, handler); 


    int* activity = (int*) mmap(
        NULL,
        sizeof(int),
        PROT_READ | PROT_WRITE, 
        MAP_SHARED | MAP_ANONYMOUS, 
        -1, 
         0
    ); 

    if(activity == MAP_FAILED) {
      const char error[] = "mmap failed\n";
      write(2, error, sizeof(error) - 1);
      return 1;
    }

    *activity = 0; 

    pid_t pid = fork();
  
    if(pid < 0) {
      const char error[] = "fork failed\n";
      write(2, error, sizeof(error) - 1);
      munmap(activity, sizeof(int));
      return 1;
    }

    while (true) {


        if (pid == 0) {

           sleep(10);

           if (*activity == 0) {

              pid_t parent_pid = getppid(); 


              kill(parent_pid, SIGUSR1);
   
              sleep(1);


              kill(parent_pid, SIGTERM);
          }
          munmap(activity, sizeof(int));
      
          return 0;
      }

      else {

          char buffer[100];


          int bytes_read = read(0, buffer, 99); 

          if (bytes_read > 0) {
             if (buffer[bytes_read - 1] == '\n') {
           
                bytes_read--;
          }

          *activity = 1;


          write(1, "!", 1);
   

          write(1, buffer, bytes_read); 
   

          write(1, "!\n", 2);


          kill(pid, SIGTERM);


          wait(NULL);

          *activity = 0;

          pid = fork();

          if (pid < 0) {
              const char error[] = "fork failed\n";
              write(2, error, sizeof(error) -1);
              break;
          }
   
        }

     } 
   
   }
   munmap(activity, sizeof(int));
   return 0; 
}






