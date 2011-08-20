extern NODE *install_symbol(char *name, NODETYPE type);
extern NODE *lookup(const char *name);
extern void install_params(NODE *paramtab, int pcount);
extern void remove_params(NODE *paramtab, int pcount);
extern NODE *remove_symbol(char *name);
extern void destroy_symbol(char *name);
