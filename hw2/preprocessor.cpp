#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <sstream>
#include <unistd.h>

using namespace std;

char pre_defines[]= \
"#define _GNU_SOURCE // for RTLD_NEXT\n\n\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <stdarg.h>\n\
#include <dlfcn.h>\n\
#include <string.h>\n\
#include <dirent.h>\n\
#include <unistd.h>\n\
#include <sys/stat.h>\n\
\n\
char buf[300];\n\
char* get_fn(FILE * f){\n\
    int fno = fileno(f);\n\
    char proclnk[300];\n\
    sprintf(proclnk, \"/proc/self/fd/%d\", fno);\n\
    int r = readlink(proclnk, buf, 300);\n\
    if (r < 0)\n\
    {\n\
        printf(\"failed to readlink\\n\");\n\
        exit(1);\n\
    }\n\
    buf[r] = '\\0';\n\
    return buf;\n\
}\n";

struct func{
    string rtn_type;
    string name;
    vector<string> arg_type;
    bool has_valist;
    func(){
        this->has_valist = false;
    }
};

char* remove_leading_space(char* &p){
    for(;p[0]!='\0';++p){
        if(p[0] != ' ')
            break;
    }
    return p;
}

void remove_ending_space(char* &p){
    size_t index_last_space = -1;
    for(int i=strlen(p)-1; i>=0; --i){
        if(p[i] != ' ')
            break;
        index_last_space = i;
    }
    p[index_last_space] = '\0';
    return;
}

bool string_exist(string s, const char* target){
    return s.find(target) != string::npos;
}

string get_type(char * &p){ // get retrun type
    string rtn;
    bool last_pointer = false;
    size_t func_name_index = 0;
    size_t return_type_length = 0;
    for(size_t i=0;i<strlen(p);++i){
        if( p[i]==' ' || p[i]=='*' )
            func_name_index = return_type_length = i+1;
        if( p[i]=='*' )
            last_pointer = true;
    }
    if(last_pointer == false)
        return_type_length -= 1;
    rtn = string( p, 0, return_type_length);
    p += func_name_index;
    return rtn;
}

func get_func(char *func_line){
    func rtn;
    char *pfl = strtok(func_line, "(");
    rtn.rtn_type = get_type(pfl);
    rtn.name = string(pfl, strlen(pfl));
    pfl = strtok(NULL, ",)");
    while( pfl != NULL ){
        string type = get_type(pfl);
        if(string_exist(type, "void") && !string_exist(type, "*"))
            break;
        if(string_exist(type, "...")){
            rtn.has_valist = true;
            break;
        }
        rtn.arg_type.push_back( type );
        pfl = strtok(NULL, ",);");
    }

    return rtn;
}

vector<func> get_funcs(const char *addr_cl){
    char buffer[200];
    vector<func> funcs;
    fstream in_call_list(addr_cl, fstream::in);
    while( in_call_list.getline(buffer, 200) ){
        funcs.push_back(get_func(buffer));
    }
    in_call_list.close();
    return funcs;
}

string get_arg_string(func f){
    stringstream ss;
    ss << "(";
    for(size_t i=0, sa=f.arg_type.size(); i<sa; ++i){
        ss << f.arg_type[i] << " " << string(i+1, '_');
        if(i != sa-1)
            ss << ", ";
    }
    if(f.has_valist)
        ss << ", ...";
    ss << ")";
    return ss.str();
}

string get_arg_format(string arg){
    stringstream ss;
    if( string_exist(arg, "int")){
        if( string_exist(arg, "*") )
            ss << "%p";
        else if( string_exist(arg, "unsigned"))
            ss << "%u";
        else
            ss << "%d";
    }
    else if( string_exist(arg, "mode_t")){
        if( string_exist(arg, "*") )
            ss << "%p";
        else
            ss << "%u";
    }
    else if( string_exist(arg, "long") ){
        if( string_exist(arg, "*") )
            ss << "%p";
        else
            ss << "%ld";
    }
    else if( string_exist(arg, "size_t") || string_exist(arg, "off_t")
                || string_exist(arg, "ssize_t")){
        if( string_exist(arg, "*") )
            ss << "%p";
        else
            ss << "%lu";
    }
    else if( string_exist(arg, "char") ){
        if( string_exist(arg, "*") )
            ss << "\'%s\'";
        else
            ss << "%c";
    }
    else if(string_exist(arg, "uid_t")
                || string_exist(arg, "gid_t") || string_exist(arg, "pid_t")){
        if( string_exist(arg, "*") )
            ss << "%p";
        else
            ss << "%u";
    }
    else if( string_exist(arg, "FILE") && string_exist(arg, "*")){
        ss << "\'%s\'";
    }
    else{
        if( string_exist(arg, "*"))
            ss << "%p";
        else
            ss << "%d";
    }
    return ss.str();
}

string get_parm(func f){
    stringstream ss;
    for(size_t i=0, sa=f.arg_type.size(); i<sa; ++i){
        if(string_exist(f.arg_type[i], "FILE") && string_exist(f.arg_type[i], "*"))
            ss << ", get_fn(" << string(i+1, '_') << ")";
        else
            ss << ", " << string(i+1, '_');
    }
    return ss.str();
}

string get_monitor_string(func f){
    stringstream ss;
    //printf("[monitor] func_name(...)");
    ss << "\tprintf(\"[monitor] " << f.name << "(";
    for(vector<string>::iterator it=f.arg_type.begin();it!=f.arg_type.end();++it){
        string arg = *it;
        ss << get_arg_format(arg);
        if(it+1 != f.arg_type.end())
            ss << ", ";
    }
    ss << ")";
    if(!string_exist(f.rtn_type, "void") || string_exist(f.rtn_type, "*")){
        ss << " = " << get_arg_format( f.rtn_type );
    }
    ss << "\\n\" ";
    ss << get_parm(f);
    if( !string_exist(f.rtn_type, "void") || string_exist(f.rtn_type, "*")){
        if(string_exist(f.rtn_type, "FILE") && string_exist(f.rtn_type, "*"))
            ss << ", get_fn(rtn)";
        else
            ss << ", rtn";
    }
    ss << ");";

    return ss.str();
}

void output_func(fstream &out, func f){

    /*
    int open(const char * pathname, int flags, mode_t mode){
        int (*_open)(const char * pathname, int flags, mode_t mode) =\
            (int(*)(const char * pathname, int flags, mode_t mode))dlsym(RTLD_NEXT, "open");
        int rtn = _open(pathname, flags, mode);
        printf("[monitor] open(%s, %d, %u) = %d\n", pathname, flags, mode, rtn);
        return rtn;
    }*/

    //int open(const char * pathname, int flags, mode_t mode){
    out << endl;
    out << f.rtn_type << " " << f.name;
    out << get_arg_string(f) << "{" <<endl;

    //int (*_open)(const char * pathname, int flags, mode_t mode) =
    out << "\t" << f.rtn_type << " (*_func)" << get_arg_string(f) << "=\\" <<endl;
        //(int(*)(const char * pathname, int flags, mode_t mode))dlsym(RTLD_NEXT, "open");
    out << "\t\t(" << f.rtn_type << "(*)" << get_arg_string(f) << ")dlsym(RTLD_NEXT, \"" << f.name << "\");" <<endl;

    if(string_exist(f.rtn_type, "void") && !string_exist(f.rtn_type, "*"))
        out << get_monitor_string(f) <<endl;

    if(f.has_valist){
        out << "\tva_list args;" <<endl;
        out << "\tva_start(args, " << string(f.arg_type.size(), '_') << ");" <<endl;
    }

    //int rtn = _open(pathname, flags, mode);
    if(!string_exist(f.rtn_type, "void") || string_exist(f.rtn_type, "*"))
        out << "\t" << f.rtn_type << " rtn = _func(";
    else
        out << "\t_func(";
    for(size_t i=0, sa=f.arg_type.size();i<sa;++i){
        out << string(i+1, '_');
        if(i != sa-1)
            out << ", ";
    }
    if(f.has_valist)
        out << ", args";
    out << ");" <<endl;
    if(f.has_valist)
        out << "\tva_end(args);" <<endl;

    //printf("[monitor] open(%s, %d, %u) = %d\n", pathname, flags, mode, rtn);
    if(!(string_exist(f.rtn_type, "void") && !string_exist(f.rtn_type, "*")))
        out << get_monitor_string(f) <<endl;

    //return rtn; }
    if(!string_exist(f.rtn_type, "void") || string_exist(f.rtn_type, "*"))
        out << "\treturn rtn;" <<endl;
    else
        out << "\treturn;" <<endl;
    out << "}" <<endl;

    return;
}

void output_c(const char* addr_c, vector<func> funcs){
    fstream out_c(addr_c, fstream::out);

    out_c << pre_defines <<endl;

    for(size_t i=0,sf=funcs.size();i<sf;++i){
        output_func(out_c, funcs[i]);
    }

    out_c.close();
    return;
}

int main(int argc, char* argv[]){
    vector<func> funcs = get_funcs("calls.c");
    if(argc > 1)
        output_c(argv[1], funcs);
    return 0;
}
