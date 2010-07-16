#include "awk.h"

static NODE *
do_check_arg(tree)
NODE *tree;
{
        int ret = 0, argc;
        NODE *arg1, *arg2, *arg3;

        argc = get_curfunc_arg_count();
        printf("arg count: defined = %d, supplied = %d\n", tree->param_cnt, argc);

        arg1 = get_scalar_argument(tree, 0, FALSE);
        arg2 = get_array_argument(tree, 1, FALSE);
        arg3 = get_scalar_argument(tree, 2, TRUE);	/* optional */
        if (argc > 3) {	/* try to use an extra arg */
                NODE *arg4;
                arg4 = get_array_argument(tree, 3, TRUE);
        }
        if (arg3 != NULL)
                printf("3rd arg present\n\n");
        else
                printf("no 3rd arg\n\n");
        /* Set the return value */
        set_value(tmp_number((AWKNUM) ret));

        /* Just to make the interpreter happy */
        return tmp_number((AWKNUM) 0);
}

/* dlload --- load new builtins in this library */

NODE *
dlload(tree, dl)
NODE *tree;
void *dl;
{
        make_builtin("check_arg", do_check_arg, 3);
        return tmp_number((AWKNUM) 0);
}
