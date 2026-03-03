#ifndef BUILTIN_HELPER_H_
#define BUILTIN_HELPER_H_


int builtin_fg(char **argv, int argc);
int builtin_bg(char **argv, int argc);
int builtin_jobs(char **argv, int argc);
int builtin_cd(char **argv, int argc);
int builtin_exit(char **argv, int argc);
int builtin_exec(char **argv, int argc);

int parse_args(char **argv, int argc);


#endif // BUILTIN_HELPER_H_
