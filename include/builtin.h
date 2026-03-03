#ifndef BUILTIN_H_
#define BUILTIN_H_


typedef enum Builtin
{
    BUILTIN_FG, BUILTIN_BG, BUILTIN_JOBS,
    BUILTIN_CD,
    BUILTIN_EXIT,
    BUILTIN_EXEC,

    BUILTIN_NONE,
} Builtin;

Builtin match_builtin(char **argv);
int exec_builtin(Builtin builtin, char **argv, int argc, int infile, int outfile);


#endif // BUILTIN_H_
