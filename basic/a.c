#include<stdio.h>
#include<signal.h>
#include<unistd.h>
int waite;
void sig_handler(int signo)
{
	  if (signo == SIGINT){
		      printf("received SIGINT\n");
			waite=0;
	  
	  }
}

int main(void)
{
	  /*if (signal(SIGINT, sig_handler) == SIG_ERR){
		    printf("\ncan't catch SIGINT\n");
	  }
	    // A long long wait so that we can easily issue a signal to this process
		   while(1) 
			sleep(1);*/
	
	 waite=1;
	while(waite){
		signal(SIGINT,sig_handler);
	}
		    return 0;
}
