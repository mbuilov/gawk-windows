#include "awk.h"

int plugin_is_GPL_compatible;

static NODE *
do_check_arg(int nargs)
{
	int ret = 0, argc;
	NODE *arg1, *arg2, *arg3;

	argc = get_curfunc_arg_count();
	printf("arg count: defined = %d, supplied = %d\n",
	               nargs, argc);

	arg1 = get_scalar_argument(0, FALSE);
	arg2 = get_array_argument(1, FALSE);
	arg3 = get_scalar_argument(2, TRUE);	/* optional */
	if (argc > 3) {
		/* try to use an extra arg */
		NODE *arg4;
		arg4 = get_array_argument(3, TRUE);
		printf("Shouldn't see this line\n");
	}
	if (arg3 != NULL) {
		printf("3rd arg present\n");
		if (arg3->type != Node_val)
			printf("3nd arg type = %s (*** NOT OK ***)\n", nodetype2str(arg3->type));
	} else
		printf("no 3rd arg\n");

	if (arg2 != NULL) {
		if (arg2->type != Node_var_array)
			printf("2nd arg type = %s (*** NOT OK ***)\n", nodetype2str(arg2->type));
	} else
		printf("2nd arg missing (NULL) (*** NOT OK ***)\n");

	if (arg1 != NULL) {
		if (arg1->type != Node_val)
			printf("1st arg type = %s (*** NOT OK ***)\n", nodetype2str(arg1->type));
	} else
		printf("1st arg missing (NULL) (*** NOT OK ***)\n");
	printf("\n");

	/* Set the return value */
	return make_number((AWKNUM) ret);
}

/* dlload --- load new builtins in this library */

NODE *
dlload(tree, dl)
NODE *tree;
void *dl;
{
        make_builtin("check_arg", do_check_arg, 3);
        return make_number((AWKNUM) 0);
}
