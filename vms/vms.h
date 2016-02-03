/*
 * vms.h - miscellaneous definitions for use with VMS system services.
 *							Pat Rankin, Nov'89
 */

#if 1
#include <iodef.h>
#else
#define IO$_WRITEVBLK	48	/* write virtual block */
#define IO$V_CANCTRLO	6	/* cancel <ctrl/O> (ie, resume tty output) */
#define IO$M_CANCTRLO	(1 << IO$V_CANCTRLO)
#endif

#if 1
#include <clidef.h>
#include <cliverbdef.h>
#include <fscndef.h>
#else
#define CLI$K_GETCMD	1
#define CLI$K_VERB_MCR	33
#define CLI$K_VERB_RUN	36
#define FSCN$_FILESPEC	1
#endif

#if 1
#include <climsgdef.h>
#else
#define CLI$_INSFPRM	0x00038048	/* insufficient parameters */
#define CLI$_VALREQ	0x00038150	/* missing required value  */
#define CLI$_NEGATED	0x000381F8	/* explicitly negated */
#define CLI$_CONFLICT	0x00038258	/* conflicting qualifiers  */
#define CLI$_NOOPTPRS	0x00038840	/* no option present	   */
#endif
/* Missing in VAX/VMS 7.3 */
#ifndef CLI$_RUNUSED
#define CLI$_RUNUSED	0x00030000	/* value returned by $CLI for "RUN" */
#endif
/* Missing as of VMS 8.4 */
#ifndef CLI$_SYNTAX
#define CLI$_SYNTAX	0x000310FC	/* error signalled by CLI$DCL_PARSE */
#endif


#if 1
#include <psldef.h>
#else
#define PSL$C_USER	3	/* user mode */
#endif

/* note: `ulong' and `u_long' end up conflicting with various header files */
typedef unsigned long	U_Long;
typedef unsigned short	U_Short;

#include <descrip.h>
#include <stsdef.h>

#pragma member_alignment save
#pragma nomember_alignment longword
#pragma message save
#pragma message disable misalgndmem
typedef struct _itm { U_Short len, code; void *buffer; U_Short *retlen; } Itm;
#pragma message restore
#pragma member_alignment restore


#define vmswork(sts) $VMS_STATUS_SUCCESS(sts)
#define vmsfail(sts) (!$VMS_STATUS_SUCCESS(sts))
#define CondVal(sts) ((sts)&0x0FFFFFF8)     /* strip severity & msg inhibit */
#define Descrip(strdsc,strbuf) struct dsc$descriptor_s \
  strdsc = {sizeof strbuf - 1, DSC$K_DTYPE_T, DSC$K_CLASS_S, (char *)strbuf}

extern int    shell$is_shell(void);
extern U_Long LIB$FIND_FILE(const struct dsc$descriptor_s *,
                            struct dsc$descriptor_s *, void *, ...);
extern U_Long LIB$FIND_FILE_END(void *);
#ifndef NO_TTY_FWRITE
extern U_Long LIB$GET_EF(long *);
extern U_Long SYS$ASSIGN(const struct dsc$descriptor_s *, short *, long,
                         const struct dsc$descriptor_s *);
extern U_Long SYS$DASSGN(short);
extern U_Long SYS$QIO(U_Long, U_Long, U_Long, void *,
			 void (*)(U_Long), U_Long,
			 const char *, int, int, U_Long, int, int);
extern U_Long SYS$SYNCH(long, void *);
#endif	/*!NO_TTY_FWRITE*/
extern U_Long LIB$SPAWN(const struct dsc$descriptor_s *,
                        const struct dsc$descriptor_s *,
                        const struct dsc$descriptor_s *,
			const U_Long *,
                        const struct dsc$descriptor_s *,
                        U_Long *, U_Long * ,...);
 /* system services for logical name manipulation */
extern U_Long SYS$TRNLNM(const U_Long *,
                         const struct dsc$descriptor_s *,
                         const struct dsc$descriptor_s *,
			 const unsigned char *, Itm *);
extern U_Long SYS$CRELNM(const U_Long *,
                         const struct dsc$descriptor_s *,
                         const struct dsc$descriptor_s *,
			 const unsigned char *, const Itm *);
extern U_Long SYS$CRELOG(int,
                         const struct dsc$descriptor_s *,
                         const struct dsc$descriptor_s *,
                         unsigned char);
extern U_Long SYS$DELLNM(const struct dsc$descriptor_s *,
                         const struct dsc$descriptor_s *,
                         const unsigned char *);

extern void   v_add_arg(int, const char *);
extern void   vms_exit(int);
extern char  *vms_strerror(int);
extern char  *vms_strdup(const char *);
extern int    vms_devopen(const char *,int);
extern int    vms_execute(const char *, const char *, const char *);
extern int    vms_gawk(void);
extern U_Long Cli_Present(const char *);
extern U_Long Cli_Get_Value(const char *, char *, int);
extern U_Long Cli_Parse_Command(const void *, const char *);

