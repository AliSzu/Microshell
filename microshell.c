#define  _GNU_SOURCE
#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>

#define MAXSIZE 2075

char *command_auto_complete[]={"cd", "mkdir", "rm", "help", "echo", "pwd", "exit","cp", NULL};
char previous_cwd[MAXSIZE];
char answer_temp[MAXSIZE];
char *arguments[MAXSIZE];
char *mkdir_arguments[MAXSIZE];
char *mkdir_brackets[MAXSIZE];
char *arguments_rm[MAXSIZE];
char brackets_temp2[MAXSIZE];
char *answer;
char *brackets_temp;
char *history[100];
char *previous_path;
char *current_path;
char *mkdir_path[100];
char host[512];
char cwd[512];
char *linestop;
int history_count = 0;
int howmany;
int dlugosc;
int q;
int stop = 0;
int howmany2;
int remove_mkdir;

sigjmp_buf ctrlc_buf; /* zeby sygnaly dzialaly poprawnie */

int remove_from_directory(char *path)
{   
    struct dirent *d;
    char file_to_delete[MAXSIZE];
    int empty = 0;
    answer = answer_temp;
    char *answerpom;
    char str[MAXSIZE];

    if (path == NULL)
    {
        fprintf(stderr, "rm: missing operand\n");
    }
    else
    {
        howmany = howmany + 1;
        DIR *directory = opendir(path);
        if (directory)
        {

            while ((d = readdir(directory)) != NULL) /*sprawdza czy folder jest pusty czy nie */
            {
                if (strcmp(".", d->d_name) == 0 || (strcmp("..", d->d_name) == 0))
                {
                    continue;
                }
                else
                {
                    empty++;
                }
            }

            closedir(directory);

            if (empty == 0)
                {
                    if (rmdir(path) == -1)
                    {
                        fprintf(stderr, "rm: cannot remove '%s': No such file or directory\n", path);
                    }
                }
            else
                {
                    if (howmany == 1)
                    {
                    printf("rm: directory %s contains files, are you sure that you want to delete it? [Y/N]\n", arguments[2]);
                    answerpom = fgets(str, MAXSIZE, stdin);
                    strncpy(answer, answerpom, MAXSIZE);
                    }
                        if ((strcmp("y\n", answer) == 0) || (strcmp("Y\n", answer) == 0) || remove_mkdir > 0)
                        {
                            DIR *directory = opendir(path);

                            while(( d= readdir(directory)) != NULL)
                            {
                                if (strcmp(".", d->d_name) == 0 || (strcmp("..", d->d_name) == 0))
                                {
                                    continue;
                                }
                                else
                                { 
                                    sprintf(file_to_delete, "%s/%s",path, d->d_name);

                                    if (d->d_type == DT_REG)
                                    {
                                        unlink(file_to_delete);
                                    }
                                    else if (d->d_type == DT_DIR)
                                    {
                                        remove_from_directory(file_to_delete);
                                    }
                                }
                            }
                            closedir(directory); 
                        } 
                        else
                        {
                            printf("rm: directory %s will not be deleted\n", path);
                            howmany = 0;
                            return 0;
                        }
                        
                    } 
                rmdir(path);
            }
        else
        {
            fprintf(stderr, "rm: cannot remove '%s': No such file or directory\n", path);
        }
    }
      return 0;
}
/* Kod czy plik jest zwyklym plikiem czy folderem zaczerpiniety z https://stackoverflow.com/questions/44164436/check-if-item-on-directory-is-folder-in-c */

void REMOVE(char line[])
{
    howmany=0;
    if (line == NULL)
    {
        fprintf(stderr, "rm: missing operand\n");
    }
    else if (strcmp(line, "-r") == 0 )
    {
       remove_from_directory(arguments[2]);
     }
    else
    {
        int result = unlink(line);

        if (result != 0 )
        {
            fprintf(stderr, "rm: cannot remove '%s': No such file or directory\n", line);
        }
        
    }
    
}

int COPY_FILES(char *from, char *to)
{
    int file[2];
    char buffer[MAXSIZE];
    char answer_temp2[MAXSIZE];
    ssize_t count;
    char *answer2;
    answer2 = answer_temp2;
    char *answerpom;
    char str[MAXSIZE];

    if ((file[0] = open(from, O_RDONLY)) == -1)
    {
        fprintf(stderr, "cp: cannot stat '%s': No such file or directory\n", from);
        return 0;
    }
    if ((file[1] = open(to, O_WRONLY | O_CREAT | S_IRUSR | S_IWUSR, 0666)) == -1 )
    {
        printf("WARNING! File '%s' already exists! Do you wish to overwrite it? [Y/N]\n", to);
        answerpom = fgets(str, MAXSIZE, stdin);
        strncpy(answer2, answerpom, MAXSIZE);
        if ((strcmp("y\n", answer2) == 0) || (strcmp("Y\n", answer2) == 0))
        {
            file[1] = open(to, O_RDONLY | O_WRONLY | O_TRUNC, 0666);
        }
        else
        {
            printf("cp: file %s will not be overwriten\n", to);
            return 0;
        }
            
    }
    while ((count = read(file[0], buffer, sizeof(buffer))) != 0)
    {
        write(file[1], buffer, count);
    }
    close(file[0]);
    close(file[1]);
    return 1;
}

int COPY_DIRECTORY(char *from, char *to)
{
    struct dirent *d;
    DIR* directory = opendir(from);
    if (directory)
    {
    char file[MAXSIZE];
    char cwd_from[MAXSIZE];
    char cwd_to[MAXSIZE];
    char answer_temp2[MAXSIZE];
    char *answer2;
    answer2 = answer_temp2;
    char *answerpom;
    char str[MAXSIZE];

    DIR* directory = opendir(to);
    if(directory)
    {
        printf("WARNING! Directory '%s' already exists! Do you want to overwrite it? [Y/N]\n", to);
        answerpom = fgets(str, MAXSIZE, stdin);
        strncpy(answer2, answerpom, MAXSIZE);
        if ((strcmp("y\n", answer2) == 0) || (strcmp("Y\n", answer2) == 0))
        {
            remove_mkdir = 1;
            howmany++;
            remove_from_directory(to);
            mkdir(to, 0777);
            howmany--;
            remove_mkdir = 0;
        }
        else
        {
            printf("cp: directory %s will not be overwriten\n", to);
            return 0;
        }
    }
    if (ENOENT == errno)
    {
        mkdir(to, 0777);
    }
    closedir(directory);

    directory = opendir(from);
    
    while ((d = readdir(directory)) != NULL)
    {
        if (strcmp(".", d->d_name) == 0 || (strcmp("..", d->d_name) == 0))
        {
            continue;
        }
        else
        {
            sprintf(file, "%s/%s",from, d->d_name);
            if(d->d_type == DT_DIR)
            {
                chdir(from);
                getcwd(cwd_from, sizeof(cwd_from)); /*zapisywanie sciezki pierwotnego folder*/
                strcat(cwd_from, "/");
                strcat(cwd_from, d->d_name);
                chdir(cwd);

                chdir(to);
                getcwd(cwd_to, sizeof(cwd_to)); /*zapisywanie sciezki folderu do ktorego ma zostac skopiowana zawartosc*/
                strcat(cwd_to, "/");
                strcat(cwd_to, d->d_name);
                COPY_DIRECTORY(cwd_from, cwd_to);
            }
            else
            {
                getcwd(cwd_from,sizeof(cwd_from));
                strcpy(cwd_from, from);
                strcat(cwd_from, "/");

                chdir(to);
                getcwd(cwd_to, sizeof(cwd_to));
                strcat(cwd_to, "/");

                chdir(cwd);
                strcat(cwd_to, d->d_name);
                strcat(cwd_from, d->d_name);
                COPY_FILES(cwd_from, cwd_to);
            }
        } 
    } 
    closedir(directory);
    }
    else
    {
        fprintf(stderr, "cp: cannot stat '%s': No such file or directory\n", from);
    }
    return 1;
    
}

void COPY()
{
    char first_cwd[MAXSIZE];
    if (strcmp(arguments[1], "-r") == 0)
    {
        if (arguments[2] == NULL || arguments[3] == NULL)
        {
            printf("cp: missing file operand\n");
        }
        else
        {
            getcwd(first_cwd, sizeof(first_cwd));
            COPY_DIRECTORY(arguments[2], arguments[3]);
            chdir(first_cwd);
        } 
    }
    else
    {
        COPY_FILES(arguments[1], arguments[2]);
    }
}

void stop_signal()
{
    if (strcmp(arguments[0], "sleep") == 0)
    {
        stop++;
        printf("\n");
        printf("[%d]+ Stopped           %s\n",stop, arguments[0]);
    }
    else
    {
        printf("\n");
    } 
}

void signals(int signo)
{
    if (signo == SIGINT)
    {
        printf("\n");
    }
    else if (signo == SIGTSTP)
    {
        stop_signal();
    }
    siglongjmp(ctrlc_buf, 1);
}

char *command_generator(const char *text, int state)
{
    static int command_index, lenght;
    char *name;

    if (state == 0)
    {
        command_index = 0;
        lenght = strlen(text);
    }
    while ((name = command_auto_complete[command_index]))
    {
        command_index++;
        if (strncmp(name,text,lenght) == 0)
        {
            return strdup(name);
        }
    }
    return NULL;
}

char **auto_complete(const char* text, int start, int end)
{
    return rl_completion_matches (text, command_generator);
}
/*Kod do autouzupełniania komend zaczerpnięty z dokumentacji readline, https://tiswww.case.edu/php/chet/readline/readline.html znajduje sie pod koniec, w przykladzie " Interface to Readline Completion " */

void make_directory_bracets(char mkdir_line[])
{   
    char korektor_mkdir[] = "{,}";
    char *schowek_mkdir;
    schowek_mkdir = strtok(mkdir_line, korektor_mkdir);
    q = 0;

    while ( schowek_mkdir != NULL )
    {
        mkdir_brackets[q] = schowek_mkdir;
        q = q + 1;

        schowek_mkdir = strtok(NULL, korektor_mkdir);
    }
}

void make_directory_arguments_p()
{
     /*Rozdzielanie katalogow*/
        char korektor_mkdir[] = "/";
        char *schowek_mkdir;
        schowek_mkdir = strtok(arguments[2], korektor_mkdir);
        int m = 0;
        int first_bracet = -1;
        int last_bracet = -1;
        int roznica;
        int ile_razy = 0;
        int where = 0;
        int inside=0;

        while ( schowek_mkdir != NULL )
        {
            mkdir_arguments[m] = schowek_mkdir;
            m = m + 1;

            schowek_mkdir = strtok(NULL, korektor_mkdir);
        }
        int i = 0;
        int p;
        while ( i < m ) /*przegadanie rozdzielonych wyrazow w celu znalezienia ewentualnego {,}*/
        {
            p = strlen(mkdir_arguments[i]);
            int j = 0;
            while ( j < p)
            {
                roznica = 0;
                if (mkdir_arguments[i][j] == '{') /* Obliczanie pozcji { */
                {
                    first_bracet = j;
                    where = i;
                }
                else if (mkdir_arguments[i][j] == '}') /*Obliczanie pozacji } */
                {
                    last_bracet = j;
                }

                roznica = last_bracet - first_bracet;
                if ( roznica > 1 && ile_razy == 0) /*Rozdzialnie wyrazow jednie w przypadku gdy istnieją katalogi pomiedzy { i } */                
                {
                    make_directory_bracets(mkdir_arguments[i]);

                    int pom = 0;
                    while ( pom < q)
                    {
                        if (mkdir(mkdir_brackets[pom], 0777) == -1 )
                        {
                            fprintf(stderr, "mkdir: cannot create directory '%s': File exists\n", mkdir_brackets[pom]);
                        }
                        
                        pom = pom + 1;
                    }

                    ile_razy = ile_razy + 1;
                }
                j = j + 1;

            }
            int pozycja = i - where;
            if (( pozycja == 1 && where != 0 )|| inside > 0)
            {   
                char brackets_path[MAXSIZE];
                char brackets_array[MAXSIZE][MAXSIZE];
                int pom2 = 0;
                if (inside == 0)
                {
                    while (pom2 < q)
                    {
                        char temp_cwd[512];
                        getcwd(temp_cwd, sizeof(temp_cwd));
                        chdir(mkdir_brackets[pom2]);
                        mkdir(mkdir_arguments[i], 0777);
                        chdir(mkdir_arguments[i]);
                        getcwd(brackets_path, sizeof(brackets_path));
                        strncpy(brackets_array[pom2], brackets_path, MAXSIZE);
                        chdir(temp_cwd);

                        pom2=pom2+1;
                    }
                    inside++;
                }
                else
                {
                    while (pom2 < q)
                    {
                        brackets_temp = brackets_temp2;
                        strncpy(brackets_temp2, brackets_array[pom2], MAXSIZE);
                        char temp_cwd[512];
                        getcwd(temp_cwd, sizeof(temp_cwd));
                        chdir(brackets_temp);
                        mkdir(mkdir_arguments[i], 0777);
                        chdir(mkdir_arguments[i]);
                        getcwd(brackets_path, sizeof(brackets_path));
                        strncpy(brackets_array[pom2], brackets_path, MAXSIZE);
                        chdir(temp_cwd);

                        pom2=pom2+1;
                         
                    }
                }
            }
            else
            {
                if (roznica > 1 && i!=where)
                {
                if( mkdir(mkdir_arguments[i], 0777) == -1)
                {
                    fprintf(stderr, "mkdir: cannot create directory '%s': File exists\n", mkdir_arguments[i]);
                }
                
                chdir(mkdir_arguments[i]);
                }
            else if ( roznica < 1 )
            {
                  if( mkdir(mkdir_arguments[i], 0777) == -1)
                {
                    fprintf(stderr, "mkdir: cannot create directory '%s': File exists\n", mkdir_arguments[i]);
                }
                
                chdir(mkdir_arguments[i]);
            }

            }
            i = i + 1;
        }
}

void MAKE_DIRECTORY(char line[])
{
    getcwd(cwd, sizeof(cwd));

    if (line == NULL)
    {
        fprintf(stderr, "mkdir: missing operand\n");
    }
   else if (strcmp("-p", line) == 0) 
    {
        make_directory_arguments_p();
        chdir(cwd);
    } 
    else
    {
       
            make_directory_bracets(line);
            int pom2 = 0 ;
            while ( pom2 < q)
            {
                if (mkdir(mkdir_brackets[pom2], 0777) == -1)
                {
                    fprintf(stderr, "mkdir: cannot create directory '%s': File exists\n", mkdir_brackets[pom2]);
                }
                pom2 = pom2 + 1;
            }

    }
}

void add_to_history(char line[])
{
    history[history_count] = strdup(line);
}

void HISTORY()
{
    int pom = 1;
    while ( pom <= history_count)
    {
        printf("%d  %s\n",pom, history[pom]);
        pom = pom + 1;
    }
}

void CD(char *path)
{
    char *home = getenv("HOME");
    current_path = cwd;
    previous_path = previous_cwd;

    if ( path == NULL )
    {
        chdir(home);
        strncpy(previous_path, current_path, MAXSIZE);
    }
    else
    {

        if (strcmp("~", path) == 0)
        {
            chdir(home);
            strncpy(previous_path, current_path, MAXSIZE);
    
        }
        else if (strcmp("-", path) == 0)
        {
            chdir(previous_path);
            strncpy(previous_path, current_path, MAXSIZE);
        }
    
        else 
        {  
            if (chdir(path) == - 1)
            {
                fprintf(stderr, "%s: No such file or directory\n", path);
            }
            else
            {
                strncpy(previous_path, current_path, MAXSIZE);
            }  
        }
    }
}

void HELP()
{
    printf("Witam w Microshellu!\n");
    printf("Autor: Alicja Szulecka\n");
    printf("Oferowane funkcje:\n");
    printf("-Podstawowe komendy zawarte w ścieżce PATH, wykonane za pomocą execvp i fork\n");
    printf("-help - pokazuje dostepne funkcje\n");
    printf("-exit - konczy program\n");
    printf("-history - pokazuje historie ostatnio uzytych polecen\n");
    printf("-cd:\n");
    printf(" -cd ~ - przenosi do katalogu domowego\n");
    printf(" -cd - - przenosi do poprzedniego katalogu\n");
    printf(" -cd .. - przenosi do nadrzednej lokalizacji\n");
    printf(" -cd . - biezacy katalog\n");
    printf("-mkdir:\n");
    printf(" -mkdir A - tworzy katalog o nazwie A\n");
    printf(" -mkdir {A,B} - tworzy katalogi o nazwie A i B\n");
    printf(" -mkdir -p A/B - tworzy katalog A a w nim podkatalog B\n");
    printf(" -mkdir -p A/{a,b} - tworzy katalog A a w nim podtakalogi a i b\n");
    printf(" -mkdir -p A/{a,b}/c - tworzy katalog A, a w nim podkatalogi a i b, oraz kazdym z nich podkatalog c\n");
    printf(" !!UWAGA, jest to uproszczona wersja programu, bardziej skomplikowane polecenia jak A/B/{a,b}/C/{a,b} nie beda dzialac poprawnie!!\n");
    printf("-rm - usuwanie plikow jak i folderow\n");
    printf(" -rm -r - usuwanie folderow\n");
    printf("cp:\n");
    printf(" -cp a b - kopiuje plik a jako plik b\n");
    printf(" -cp -r A B - kopiuje folder A jako folder B. Jeśli folder B istnieje to zostanie on nadpisany\n");
    printf("Oferowane dodatkowe 'bajery':\n");
    printf("-Przegladanie historii ostatnio uzytych komend strzalka w gore\n");
    printf("-Autouzupelnianie komend poprzez nacisniecie 'tab' na klawiaturze\n");
}

void arguments_function(char line[])
{
    char korektor[]=" \t\n";
    char *schowek;
    schowek = strtok(line,korektor);
    int j = 0;
    while ( j<dlugosc)
        {
            arguments[j] = schowek; /*Tworzenie tablicy skladajacej sie z rozbitego inputu, o dlugosci ilosci slow*/
            j = j + 1;

            schowek = strtok(NULL, korektor);
        }

        arguments[dlugosc] = NULL;
        /* !! Implementacja strtok przy pomocy https://cpp0x.pl/dokumentacja/standard-C/strtok/442 !! */
}

void line_lenght(char line[]) /*Obliczanie ilosci slow z inputu, potrzebne do strtok*/
{
    int i = 0;
    dlugosc = 1;
    while ( line[i] != '\0')    
        {
            if (line[i] == ' ' || line[i] == '\t')
                {
                    dlugosc = dlugosc + 1;
                }
            i = i + 1;
        }
}

int main()
{
    char str[MAXSIZE]="";
    char *line = NULL;
    pid_t id;
    char *name;
    name = getlogin();
    gethostname(host, 512);

    signal(SIGINT, signals);
    signal(SIGTSTP, signals);
    rl_attempted_completion_function = auto_complete;

    while(1)
    {
        while ( sigsetjmp( ctrlc_buf, 1 ) != 0 ); /*zeby sygnaly dzialaly poprawnie*/

        getcwd(cwd,sizeof(cwd));
        sprintf(str, "[\e[1;95m%s@%s\x1B[0m:\e[1;34m%s\x1B[0m]$ ",name, host, cwd);
        line = readline(str);
        if (strcmp("\n", line) == 0 || (strcmp("\0", line) == 0))
        {
            continue;
        }
        add_history(line);
        history_count = history_count + 1;
        add_to_history(line);
        line_lenght(line);
        arguments_function(line);
            if (strcmp("exit", arguments[0]) == 0)
            {
                exit(0);
            }
            else if (strcmp("mkdir", arguments[0]) == 0)
            {
                if (dlugosc > 2 && strcmp("-p", arguments[1]) != 0)
                {
                    int i = 1;
                    while (dlugosc != 1)
                    {
                        MAKE_DIRECTORY(arguments[i]);
                        i = i + 1;
                        dlugosc--;
                    }
                }
                else
                {
                    MAKE_DIRECTORY(arguments[1]);
                }
            }
            else if (strcmp("help", arguments[0]) == 0)
            {
                HELP();
            }
            else if (strcmp("history", arguments[0]) == 0)
            {
                HISTORY();
            }
            else if (strcmp("cd", arguments[0]) == 0)
            {
                CD(arguments[1]);
            }
            else if (strcmp("rm", arguments[0]) == 0)
            {
                REMOVE(arguments[1]);
            }
            else if (strcmp("cp", arguments[0]) == 0)
            {
                COPY();
            }
            else
            {      
                id = fork();
                if ( id == 0)
                {
                    if ( execvp(arguments[0], arguments) < 0)
                    {
                        fprintf(stderr, "%s: command not found\nType 'help' to see available commands\n", arguments[0]);
                        exit(0);
                    } 
                }
                else 
                {
                    wait(NULL);
                }   
            } 

        free(line);
    }
    return 0;

    }