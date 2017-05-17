#include <iostream>
#include <vector>
#include <string>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <map>
#include <fstream>
#include <climits>
#include <sys/types.h>
#include <dirent.h>
#include <glob.h>
#include <algorithm>

#define scr_clear() printf("\033[H\033[J")


#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

#define CLEAR "\033[2J"  // clear screen escape code

using namespace std;

#define PATH_BUFSIZE 1024
#define COMMAND_BUFSIZE 1024
#define TOKEN_BUFSIZE 64
#define TOKEN_DELIMITERS " \t\r\n\a"
#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1
#define PIPELINE_EXECUTION 2

struct pgid_with_size{
    pid_t pgid;
    int size;
    string command;
    pgid_with_size(){
        pgid = 0;
        size = 0;
    }
};

map<pid_t, vector<pgid_with_size>::iterator > pid_to_it;
vector<pgid_with_size> back_pgids;

struct command_segment{
    vector<string> args;
    pid_t pid;
    pid_t pgid;
    command_segment(){
        args.clear();
        pid = 0;
        pgid = 0;
    }
    void clear(void){
        args.clear();
        pid = 0;
        pgid = 0;
    }
};

struct command{
    string raw;
    list<command_segment> segment;
    int mode;
    command(){
        segment.clear();
        mode = FOREGROUND_EXECUTION;
    }
    void clear(void){
        segment.clear();;
        mode = FOREGROUND_EXECUTION;
    }
    int size(void){
        return this->segment.size();
    }
};

int cmd_cd(string path){
    int rtn_cd = chdir(path.c_str());
    if(rtn_cd == -1){
        cout << strerror(errno) <<endl;
        return -1;
    }
    return 0;
}

int cmd_exit(void){
    //clean up
    for(int i=0;i<back_pgids.size();++i){
        kill(-back_pgids[i].pgid,SIGINT);
    }
    //say goodbye!
    cout << "Goodbye!" <<endl;
    exit(0);
}

int cmd_kill(string pid_s){
    int pid_to_kill = atoi(pid_s.c_str());
    kill(pid_to_kill,SIGINT);
    for(int i=0;i<back_pgids.size();++i){
        if(back_pgids[i].pgid == pid_to_kill){
            back_pgids.erase(back_pgids.begin()+i);
            break;
        }
    }
    return 0;
}

int cmd_fg(int pgid){
    int this_pgid = getpgid(0);
    int index_pgid = -1;
    int wait_status = 0;

    kill(pgid,SIGCONT);

    //find it in background_pigds
    for(int i=0;i<back_pgids.size();++i){
        if(back_pgids[i].pgid == pgid)
            index_pgid = i;
    }
    if(index_pgid == -1){
        cout << "pid/pgid not found" << endl;
        return -1;
    }

    //make it foreground
    if(tcsetpgrp(STDIN_FILENO,pgid) == -1){
        cerr << "tcsetpgrp error 0" <<endl;
        cerr << strerror(errno) <<endl;
    }
    //wait for pgid
    for(int i=0;i<back_pgids[index_pgid].size;++i){
        //cout << "i am waiting " << i <<endl;
        int rtn_waitpid = waitpid(-pgid,&wait_status,WUNTRACED) ;
        if( rtn_waitpid == -1 && errno != ECHILD){
            cerr << "waitpid wrong" <<endl;
            cerr << strerror(errno) <<endl;
        }
        if(rtn_waitpid != -1 && WIFEXITED(wait_status) == 1){
            back_pgids[index_pgid].size--;
        }
        //cout << "finished " << i <<endl;
    }

    if(tcsetpgrp(STDIN_FILENO,this_pgid) == -1){
        cerr << "tcsetpgrp error 1" <<endl;
        cerr << strerror(errno) <<endl;
    }

    return 0;
}

int cmd_bg(int pgid){
    kill(pgid,SIGCONT);
    return 0;
}

int cmd_bg(string pgid_s){
    int pgid = atoi(pgid_s.c_str());
    return cmd_bg(pgid);
}

int cmd_fg(string pgid_s){
    int pgid = atoi(pgid_s.c_str());
    return cmd_fg(pgid);
}

int cmd_jobs(){
    char buffer[200];
    for(unsigned int i=0;i<back_pgids.size();++i){
        cout << back_pgids[i].pgid << "\t" ;

        // get status
        char addr_proc[200];
        sprintf(addr_proc, "/proc/%d/status", back_pgids[i].pgid);
        FILE *file_proc = fopen(addr_proc, "r");
        if( file_proc == NULL )
            cout << "exited" <<"\t";
        else{
            while(fgets(buffer, 200, file_proc)){
                if(!strncmp(buffer, "State:", 6)){
                    buffer[strlen(buffer) -1] = '\0';
                    cout << buffer+7 << "\t";
                    fclose(file_proc);
                    break;
                }
            }
        }
        // print out command
        cout << back_pgids[i].command <<endl;
    }
    return 0;
}

int cmd_export(string arg){
    char envname[500];
    char *envval;
    strcpy(envname, arg.c_str());
    envval = strstr(envname, "=");
    if(envval == NULL){
        cout << "format error!" <<endl;
        return -1;
    }
    *envval = '\0';
    envval += 1;

    setenv(envname, envval, 1);
    return 0;
}

int cmd_unset(string arg){
    unsetenv(arg.c_str());
    return 0;
}

void string_remove_space(string &input){
    string::iterator end_pos = remove(input.begin(), input.end(), ' ');
    input.erase(end_pos, input.end());
    return;
}

void shell_command_parser(string input,command &command_line){
    command_line.raw = input;
    //deal with > and <
    int lt_location = input.find("<"); // less than
    if( lt_location != string::npos ){
        bool find_first_letter = false;
        for(unsigned int i=lt_location+1;i<input.size();++i){
            if( !isspace(input[i]) )
                find_first_letter = true;
            if( (isspace(input[i]) && find_first_letter) || i == input.size()-1){
                int size_filein = i - lt_location;
                string filein = input.substr(lt_location+1, size_filein);
                string_remove_space(filein);
                input.erase(lt_location, size_filein+1);
                input = "cat " + filein + " | " + input;
                break;
            }
        }
    }
    int gt_location = input.find(">"); // greater than
    if( gt_location != string::npos ){
        bool find_first_letter = false;
        for(unsigned int i=gt_location+1;i<input.size();++i){
            if( !isspace(input[i]) )
                find_first_letter = true;
            if( (isspace(input[i]) && find_first_letter) || i == input.size()-1){
                int size_filein = i - gt_location;
                string fileout = input.substr(gt_location+1, size_filein);
                string_remove_space(fileout);
                input.erase(gt_location, size_filein+1);
                input = input + " | dd of=" + fileout + " status=none";
                break;
            }
        }
    }

    istringstream iss(input);
    string buffer;

    command_segment tmp_seg;
    while(iss >> buffer){
        int l_location = buffer.find("|");
        if( buffer[buffer.size()-1] == '|' ){
            if(buffer.size() != 1){
                string buffer_without_l(buffer.c_str(),buffer.size()-1);
                tmp_seg.args.push_back(buffer_without_l);
            }
            command_line.segment.push_back(tmp_seg);
            tmp_seg.clear();
        }
        else if( buffer[buffer.size()-1] == '&'){
            if(buffer.size() != 1){
                string buffer_without_and(buffer.c_str(),buffer.size()-1);
                tmp_seg.args.push_back(buffer_without_and);
            }
            command_line.segment.push_back(tmp_seg);
            tmp_seg.clear();
            command_line.mode = BACKGROUND_EXECUTION;
        }
        else if(buffer[0] == '|'){
            command_line.segment.push_back(tmp_seg);
            tmp_seg.clear();

            string buffer_without_l(buffer.begin()+1,buffer.end());
            tmp_seg.args.push_back(buffer_without_l);
        }
        else if(l_location != string::npos){
            string front(buffer.begin(),buffer.begin()+l_location);
            string back(buffer.begin()+l_location+1,buffer.end());
            tmp_seg.args.push_back(front);
            command_line.segment.push_back(tmp_seg);
            tmp_seg.args.clear();
            tmp_seg.args.push_back(back);
        }
        else{
            tmp_seg.args.push_back(buffer);
        }
    }
    if(!tmp_seg.args.empty())
        command_line.segment.push_back(tmp_seg);
    return;
}


int shell_exec_builtin(command_segment &segment){
    string &arg0 = segment.args[0];
    if(arg0 == "cd")
        cmd_cd(segment.args[1]);
    else if(arg0 == "exit")
        cmd_exit();
    else if(arg0 == "kill")
        cmd_kill(segment.args[1]);
    else if(arg0 == "fg")
        cmd_fg(segment.args[1]);
    else if(arg0 == "bg")
        cmd_bg(segment.args[1]);
    else if(arg0 == "jobs")
        cmd_jobs();
    else if(arg0 == "export")
        cmd_export(segment.args[1]);
    else if(arg0 == "unset")
        cmd_unset(segment.args[1]);
    else
        return 0; // it's not a builtin-function
    return 1;
}

bool is_buildin(command_segment &segment){
    string &arg0 = segment.args[0];
    if(arg0 == "cd");
    else if(arg0 == "exit");
    else if(arg0 == "kill");
    else if(arg0 == "fg");
    else if(arg0 == "bg");
    else if(arg0 == "jobs");
    else if(arg0 == "export");
    else if(arg0 == "unset");
    else
        return false; // it's not a builtin-function
    return true;
}

void expand_segment(command_segment &segment){
    for(unsigned int i=0;i<segment.args.size();++i){
        if(segment.args[i].find("*") != string::npos || segment.args[i].find("?") != string::npos){ // expand *
            glob_t glob_buffer;
            glob(segment.args[i].c_str(), GLOB_NOSORT, NULL, &glob_buffer);

            for(unsigned int j=0;j<glob_buffer.gl_pathc;++j){
                segment.args.push_back(string(glob_buffer.gl_pathv[j]));
            }
            segment.args.erase(segment.args.begin()+i); // remove args contains star or question mark
        }
    }
    return;
}

int shell_exec_segment(command_segment &segment, int in_fd, int out_fd, int mode, int pgid , vector<vector<int> > &fds){

    if(shell_exec_builtin(segment))
        return 0;

    expand_segment(segment);

    char** args = new char*[segment.args.size()+1];
    for(int i=0;i<segment.args.size();++i){
        args[i] = new char[segment.args[i].size()+1];
        strcpy(args[i],segment.args[i].c_str());
    }
    args[ segment.args.size() ] = NULL;

    pid_t childpid = fork();
    if(childpid == -1){
        cout << "wrong at fork" <<endl;
        exit(0);
    }

    if(childpid == 0){//child
        //cout << "in_fd : " << in_fd << "\tout_fd : " << out_fd <<endl;

        if(in_fd != STDIN_FILENO && in_fd != -1){
            if(dup2(in_fd,STDIN_FILENO) == -1){
                cerr << strerror(errno) <<endl;
                exit(errno);
            }
        }

        if(out_fd != STDOUT_FILENO && out_fd != -1){
            if(dup2(out_fd,STDOUT_FILENO) == -1){
                cerr << strerror(errno) <<endl;
                exit(errno);
            }
        }

        //close all the fds
        for(int i=0;i<fds.size();++i){
            close(fds[i][0]);
            close(fds[i][1]);
        }

        if(execvp(segment.args[0].c_str(),args) == -1){
            cerr << strerror(errno) <<endl;
        }

        exit(errno);
    }
    else{//parent
        if(pgid == 0){//it's first segment
            if(setpgid(childpid,childpid) == -1){
                cout << "setpgid error" <<endl;
                cout << strerror(errno) <<endl;
            }
            segment.pgid = childpid;
            //cout << "pgid = " << childpid <<endl;
        }
        else{
            if(setpgid(childpid,pgid) == -1){
                cout << "setpgid error" <<endl;
                cout << strerror(errno) <<endl;
            }
            segment.pgid = pgid;
            //cout << "pgid = " << pgid <<endl;
        }
        segment.pid = childpid;
        for(int i=0;i<segment.args.size();++i)
            delete [] args[i];
        delete [] args;
        return 0;
    }

    return 0;
}

int shell_exec_command(command &command_line){
    int rtn_exec = 0;
    vector< vector<int> > fds;
    list<command_segment>::iterator it = command_line.segment.begin();

    //open a lot of pipe
    for(int i=0;i<command_line.size()-1;++i){
        vector<int> tmp_pipe(2,0);
        int tmp_fd[2];
        if(pipe(tmp_fd) == -1){
            cerr << "wrong at pipe()" <<endl;
            return -1;
        }
        tmp_pipe[0] = tmp_fd[0];
        tmp_pipe[1] = tmp_fd[1];
        fds.push_back(tmp_pipe);
    }
    //pass pipe and segment to exec_seg
    for(int i=0;i<command_line.size();++i){
        int in_fd = STDIN_FILENO;
        int out_fd = STDOUT_FILENO;
        if(i != 0)
            in_fd = fds[i-1][0];
        if(i != command_line.size()-1)
            out_fd = fds[i][1];

        rtn_exec = shell_exec_segment(*it,in_fd,out_fd,command_line.mode,command_line.segment.front().pgid,fds);
        if(rtn_exec != 0){
            cerr << "rtn_exec = " << rtn_exec <<endl;
            break;
        }
        else{
            it++;
        }
    }
    //close all pipe in parent
    for(int i=0;i<fds.size();++i){
        close(fds[i][0]);
        close(fds[i][1]);
    }

    //push group into process
    pgid_with_size tmp_pgid_w_size;
    tmp_pgid_w_size.pgid = command_line.segment.front().pgid;
    tmp_pgid_w_size.size = command_line.segment.size();
    tmp_pgid_w_size.command = command_line.raw;
    if(tmp_pgid_w_size.pgid != 0){
        back_pgids.push_back(tmp_pgid_w_size);
        list<command_segment>::iterator tmp_it = command_line.segment.begin();
        for(int i=0;i<command_line.size();++i){
            pid_to_it[tmp_it->pid] = back_pgids.end()-1;
            tmp_it++;
        }
    }

    //wait for this group if mode is foreground
    if(command_line.mode == FOREGROUND_EXECUTION){
        pid_t child_pgid = command_line.segment.front().pgid;
        if(child_pgid != 0)
            cmd_fg(child_pgid);
    }

    command_line.clear();
    fds.clear();
    return 0;
}

void shell_print_promt(){
    char buffer[200];
    char username[200];
    getlogin_r(username,200);
    cout << BOLDYELLOW << "shell-prompt$ " << WHITE;
    return;
}


int shell_clean_up(void){
    vector< vector<pgid_with_size>::iterator > it_need_removed;
    vector<pgid_with_size>::iterator it = back_pgids.begin();
    while(it!=back_pgids.end()){
        if(it->size == 0)
            it_need_removed.push_back(it);
        it++;
    }
    for(int i=0;i<it_need_removed.size();++i){
        back_pgids.erase(it_need_removed[i]);
    }
    return 0;
}

void shell_loop(){
    string input_command;
    command command_line;

    int status = 1;

    while(status >= 0){
        shell_clean_up();
        shell_print_promt();
        getline(cin,input_command);
        if(input_command.size() == 0)
            continue;
        shell_command_parser(input_command,command_line);
        status = shell_exec_command(command_line);
        input_command.clear();
        command_line.clear();
    }
    return;
}

void gotoxy(int x,int y){
    printf("%c[%d;%df",0x1B,y,x);
}

void shell_welcome(){
    scr_clear();
    return;
}

void kill_foreground(int signum){
    fflush(stdin);
    shell_print_promt();
    return;
}
void do_nothing(int signum){
    return;
}
void make_shell_forground(int signum){
    cout << "what are you doing" <<endl;
    int this_pgid = getpgid(0);
    //set parent to foreground
    if(tcsetpgrp(STDIN_FILENO,this_pgid) == -1){
        cerr << "tcsetpgrp error" <<endl;
        cerr << strerror(errno) <<endl;
    }
    return;
}

void deal_with_zombie(int signum){
    //cout << "SIGCHILD handling" <<endl;
    int status = 0;
    int zombie_pid = waitpid(-1,&status,WNOHANG);
    if(zombie_pid == -1 ){
        //cout << "error occures when dealing with zombie" <<endl;
        //cout << strerror(errno) <<endl;
        return ;
    }
    if(zombie_pid == 0)
        return;
    for(int i=0;i<back_pgids.size();++i){
        if(back_pgids[i].pgid == zombie_pid){
            back_pgids.erase(back_pgids.begin() + i);
            break;
        }
    }
    pid_to_it[zombie_pid]->size--;
    //pid_to_it.erase(zombie_pid);
    //cout << "finished" <<endl;
    return;
}

void shell_init(){
    //set this pgid
    setpgid(0,0);
    //set ctrl+C
    signal(SIGINT,&kill_foreground);
    //set ctrl+Z
    signal(SIGTSTP,&make_shell_forground);
    //set SIGCHLD for zombie
    signal(SIGCHLD,&deal_with_zombie);

    signal(SIGTTIN,SIG_IGN);
    signal(SIGTTOU,SIG_IGN);

    return;
}

int main()
{
    shell_init();
    shell_welcome();
    shell_loop();

    return 0;
}
