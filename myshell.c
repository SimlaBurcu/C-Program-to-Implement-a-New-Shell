#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include <errno.h>
void  baslat(char **argv);
void beklet();
void  yurut(char **argv);
void durdur(char **argv);
void surdur(char **argv);
void oldur(char **argv);
int isDirected(char *argv[]);
void  parse(char *line, char **argv);
/*baslat cagrisinin gerceklendigi fonksiyon.*/
void  baslat(char **argv){
	pid_t pid;
	int status;

	if ((pid = fork()) < 0) {   /* fork a child process*/
		printf("*** ERROR: forking child process failed\n");
		exit(1);
	}
	else if (pid == 0) {   /* for the child process:*/
		printf("%i No'lu is baslatildi.\n",getpid());
		int dirpos;
		/*C'de sistem cagrisi yaparken output redirection(>) execvp ile dogrudan calistirilamamaktadir.
		 *isDirected(argv) fonksiyonunda verilen komutta redirection isareti > olup olmadigi kontrol edilir.
		 *komutta redirection isareti varsa if'in icine girer, burada >'dan sonraki kisim dosya ismini belirttiginden,
belirtilen isimli dosya acilir,mevcut degilse gerekli izinlerle olusturulur. 
		 *>'dan onceki kisim execvp() ile calistirilir, acilmis olan dosyanin icine fonksiyonun ciktisi yazilir.
		 *(Gerekli hata kontrolleri yapilmistir.)*/
		if((dirpos=(isDirected(argv)))!=0){
			int fd;
			argv[dirpos]='\0';
			fd = open( argv[dirpos+1], O_WRONLY | O_CREAT, 0777 );
			if( fd == -1 ) {
				printf("*** ERROR: open file failed: %s\n",strerror(errno));
				exit(0);
			}
			dup2( fd, 1);
			if (execvp(*argv, argv) < 0) {    
				printf("*** ERROR: exec failed: %s\n",strerror(errno));
				exit(1);
			}
			close(fd);
		}
		/*Output redirection yoksa execvp dogrudan verilen argumanlarla calistirilir.*/
		else if (execvp(*argv, argv) < 0) {    
			printf("*** ERROR: exec failed: %s\n",strerror(errno));
			exit(1);
		}	
	}
	else {   /* for the parent:*/
		sleep(1);   /*baslat komutu verildikten sonra, ekranda yenishell> yazisinin dogru anda görünmesi icin gerekli sure*/
	}
}
/*beklet cagrisinin gerceklendigi fonksiyon.Odev metninde belirtilen hata kontolleri yapilmistir.*/
void beklet(){
	pid_t pid=NULL;
	int status;
	pid = wait(&status);
	if(pid == -1){
		printf("*** ERROR: waiting error: %s, exit code: %i\n",strerror(errno),status >> 8);
	}
	else{
           if (WIFSIGNALED(status) != 0)
              printf("Child process %d sinyali nedeniyle sonlandi.\n",WTERMSIG(status));/**/
           else if (WIFEXITED(status) != 0)
              printf("Child process basariyle sonlandi: status = %d.\n",WEXITSTATUS(status));
           else
              printf("Child process basarisiz oldu, exit code: %i\n",status >> 8);	
	}
}
/*yurut cagrisinin gerceklendigi fonksiyon. Odev metninde belirtildigi gibi baslat ve beklet'in birlesimi gibidir. Ancak burada waitpid() kullanilmistir. yurut komutu girildikten sonra komut bitene kadar baska hicbir islem yapilmamaktadir. Komut gerceklesirken eger gecerli bir komut girilirse(yurut sleep 10'dan hemen sonra baslat ls gibi) yurut tamamlandiktan sonra gecerli komut calistirilir. baslat'tan farki budur, baslat process basladiktan sonra gelen komutlari aninda calistirir.*/
void  yurut(char **argv){
	pid_t pid;
	pid_t childpid;
	int status;

	if ((pid = fork()) < 0) {   /* fork a child process */
		printf("*** ERROR: forking child process failed: %s\n",strerror(errno));
		exit(1);
	}
	else if (pid == 0) {   /* for the child process: */
		childpid=getpid();
		printf("%i No'lu is baslatildi.\n",childpid);
		int dirpos;
		if((dirpos=(isDirected(argv)))!=0){
			int fd;
			argv[dirpos]='\0';
			fd = open( argv[dirpos+1], O_WRONLY | O_CREAT, 0777 );
			if( fd == -1 ) {
				exit(0);
			}
			dup2( fd, 1);
			if (execvp(*argv, argv) < 0) {    
				printf("*** ERROR: exec failed: %s\n",strerror(errno));
				exit(1);
			}
			close(fd);
		}
		else if (execvp(*argv, argv) < 0) {    
			printf("*** ERROR: exec failed: %s\n",strerror(errno));
			exit(1);
		}
	}
	else {   /* for the parent: */
		pid_t retwpid=waitpid(pid,&status,0);
		while (retwpid != pid);
		if(retwpid==-1)
			printf("*** ERROR: %s",strerror(errno));
		
	}
}

void durdur(char **argv){
	pid_t pid;
	pid=atoi(*argv);
	kill(pid,SIGSTOP);
}

void surdur(char **argv){
	pid_t pid;
	pid=atoi(*argv);
	kill(pid,SIGCONT);
}

void oldur(char **argv){
	pid_t pid;
	pid=atoi(*argv);
	kill(pid,SIGKILL);
}
/*Output redirection olup olmadigina bakar, varsa > sembolunun indexini return eder.*/
int isDirected(char *argv[]){
	char *s = ">";
	int i = 0;
	while(argv[i]) {
		if(strcmp(argv[i], s) == 0) {
			return i;
			break;
		}
		i++;
	}
	return 0;
}

void  parse(char *line, char **argv){
	/*while (*line != '\0') {       
		while (*line == ' ' || *line == '\t' || *line == '\n')
			*line++ = '\0';    
	*argv++ = line;          
	while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') 
		line++;            
	}
	*argv = '\0';
	 */
	const char s[2] = " ";
	char *token;
	int index=0;
	token = strtok(line, s);
	while( token != NULL ) {
		argv[index]=token;
		index++;  
		token = strtok(NULL, s);
	}
	argv[index]=NULL;             
}

void  main(void)
{
	char  line[1024];            
	char  *argv[10];             
	while (1) {                 
		printf("yenishell> ");   
		fgets(line,sizeof(line),stdin); 
		if (line[0] == '\n' || line == 0) {
			continue;
		} else {
    
		int len = strlen(line);
		line[len-1] = 0;
		}
		parse(line, argv);    
		char **komut=&(argv[1]);
		if (strcmp(argv[0], "kapat") == 0)  
			exit(0);           
		else if(strcmp(argv[0], "baslat") == 0){
			/*C'de sistem cagrisi yaparken >'da oldugu gibi gcc komutu da terminale girdigimiz sekliyle calismamaktadir. Bu sorunu cozmek icin asagidaki gibi bir degisim yapilmitir.*/
			if(strcmp(argv[1],"gcc")==0){
				argv[1]="/usr/bin/cc\0"	;	
			}
			baslat(komut);	
		}
		else if(strcmp(argv[0], "beklet") == 0){
			beklet();
		}
		else if(strcmp(argv[0], "yurut") == 0){
			yurut(komut);
		}
		else if(strcmp(argv[0], "durdur") == 0){
			durdur(komut);
		}
		else if(strcmp(argv[0], "surdur") == 0){
			surdur(komut);
		}
		else if(strcmp(argv[0], "oldur") == 0){
			oldur(komut);
		}
		
	}
}
