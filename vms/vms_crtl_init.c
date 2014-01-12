/* File: VMS_CRTL_INIT.C

 This file is common to a lot of projects.

 $Id: vms_crtl_init.c,v 1.1.1.1 2012/12/02 19:25:22 wb8tyw Exp $

 Module that provides a LIB$INITIALIZE routine for the GNV toolset that
 will turn on some CRTL features that are not enabled by default.

 The CRTL features can also be turned on via logical names, but that
 impacts all programs and some aren't ready, willing, or able to handle
 the settings that GNV needs.

 The original module was found linked with GPL V2 modules, and thus must
 be able to be distributed under the GPL V2 provisions.

 As this module or similar is needed for virtually all programs built to run
 under GNV or UNIX, it can be distributed with other licenses.

 Edit History

 1-001	John Reagan	Initial version using the old style interface
			but with the new version commented out.

 1-002	John Reagan	Switch to new API for setting features

 1-003	Steve Pitcher	Add DECC$RENAME_NO_INHERIT.

 1-004	Steve Pitcher	Quiet these, if the DECC feature doesn't exist.

 2-001	J. Malmberg	New GNV requirements:
			    Three variations of object modules:
				1. For use with shells, sets the logical
				   name GNV$UNIX_SHELL.

				2. For utilities, if the logical name
				   GNV$UNIX_SHELL is set, it means that the
				   settings should assume that they are
				   running under a UNIX like shell.

				3. A third setting is for utilities that
				   always should behave as if they are
				   running under a UNIX shell.

			    If GNV$GNU is defined, then locally define
			    SYS$POSIX_ROOT to it.  GNV$GNU can be set in
			    the SYSTEM table by the GNV setup.
			    SYS$POSIX_ROOT can not.

			    The logical name BIN also needs to be defined
			    here, otherwise the CRTL replaces it with
			    SYS$SYSTEM:

			    Never set the POSIX UID here, it will break
			    every reference to a GID/UID on systems that
			    do not have every VMS account mapped to a UID/GID
			    by TCPIP services.

			    Reformat text to fit 80 columns.

			    Remove all VAX C specific code.

			    Linker is probably using exact case, so public
			    symbols for LIB$* and SYS$* must be in upper case.

 2-002	J. Malmberg	Support for VAX builds.  OpenVMS/VAX does not have the
	17-Jun-2010	DECC$FEATURE routines.  At this time I will not
			be concerned if a feature setting exists on VAX,
			as all we are doing is setting a logical name.

 2-003	J. Malmberg	Add DECC$FILENAME_UNIX_NOVERSION as version numbers
			will usually mess up ported programs.

*/

#include <stdio.h>
#include <descrip.h>
#include <lnmdef.h>
#include <stsdef.h>
#include <string.h>

#pragma message disable pragma
#pragma message disable dollarid
#pragma message disable valuepres

#pragma member_alignment save
#pragma nomember_alignment longword
#pragma message save
#pragma message disable misalgndmem
struct itmlst_3 {
  unsigned short int buflen;
  unsigned short int itmcode;
  void *bufadr;
  unsigned short int *retlen;
};
#pragma message restore
#pragma member_alignment restore

#ifdef __VAX
#define ENABLE "ENABLE"
#define DISABLE "DISABLE"
#else

#define ENABLE TRUE
#define DISABLE 0
int   decc$feature_get_index (const char *name);
int   decc$feature_set_value (int index, int mode, int value);

#endif

int   SYS$TRNLNM(
	const unsigned long * attr,
	const struct dsc$descriptor_s * table_dsc,
	struct dsc$descriptor_s * name_dsc,
	const unsigned char * acmode,
	const struct itmlst_3 * item_list);
int   SYS$CRELNM(
	const unsigned long * attr,
	const struct dsc$descriptor_s * table_dsc,
	const struct dsc$descriptor_s * name_dsc,
	const unsigned char * acmode,
	const struct itmlst_3 * item_list);
int   LIB$SIGNAL(int);

/* Take all the fun out of simply looking up a logical name */
static int sys_trnlnm
   (const char * logname,
    char * value,
    int value_len)
{
    const $DESCRIPTOR(table_dsc, "LNM$FILE_DEV");
    const unsigned long attr = LNM$M_CASE_BLIND;
    struct dsc$descriptor_s name_dsc;
    int status;
    unsigned short result;
    struct itmlst_3 itlst[2];

    itlst[0].buflen = value_len;
    itlst[0].itmcode = LNM$_STRING;
    itlst[0].bufadr = value;
    itlst[0].retlen = &result;

    itlst[1].buflen = 0;
    itlst[1].itmcode = 0;

    name_dsc.dsc$w_length = strlen(logname);
    name_dsc.dsc$a_pointer = (char *)logname;
    name_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    name_dsc.dsc$b_class = DSC$K_CLASS_S;

    status = SYS$TRNLNM(&attr, &table_dsc, &name_dsc, 0, itlst);

    if ($VMS_STATUS_SUCCESS(status)) {

	 /* Null terminate and return the string */
	/*--------------------------------------*/
	value[result] = '\0';
    }

    return status;
}

/* How to simply create a logical name */
static int sys_crelnm
   (const char * logname,
    const char * value)
{
    int ret_val;
    const char * proc_table = "LNM$PROCESS_TABLE";
    struct dsc$descriptor_s proc_table_dsc;
    struct dsc$descriptor_s logname_dsc;
    struct itmlst_3 item_list[2];

    proc_table_dsc.dsc$a_pointer = (char *) proc_table;
    proc_table_dsc.dsc$w_length = strlen(proc_table);
    proc_table_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    proc_table_dsc.dsc$b_class = DSC$K_CLASS_S;

    logname_dsc.dsc$a_pointer = (char *) logname;
    logname_dsc.dsc$w_length = strlen(logname);
    logname_dsc.dsc$b_dtype = DSC$K_DTYPE_T;
    logname_dsc.dsc$b_class = DSC$K_CLASS_S;

    item_list[0].buflen = strlen(value);
    item_list[0].itmcode = LNM$_STRING;
    item_list[0].bufadr = (char *)value;
    item_list[0].retlen = NULL;

    item_list[1].buflen = 0;
    item_list[1].itmcode = 0;

    ret_val = SYS$CRELNM(NULL, &proc_table_dsc, &logname_dsc, NULL, item_list);

    return ret_val;
}


 /* Start of DECC RTL Feature handling */

/*
** Sets default value for a feature
*/
#ifdef __VAX
static void set_feature_default(const char *name, const char *value)
{
    sys_crelnm(name, value);
}
#else
static void set_feature_default(const char *name, int value)
{
     int index;

     index = decc$feature_get_index(name);

     if (index > 0)
	decc$feature_set_value (index, 0, value);
}
#endif

static void set_coe ( void )
{

    char gnv_posix_root[4096];
    char unix_shell_name[255];
    int use_unix_settings = 0;
    int status;
    int gnv_posix_root_found = 0;

    /* If this is compiled for use with a UNIX shell, then the logical
     * name GNV$UNIX_SHELL will be set to that shell name.
     *
     * Else, if the GNV$UNIX_SHELL logical name is set, then this application
     * is running under some UNIX like shell, so it should modify it's
     * behavior to be UNIX like.
     *
     * If the above logical name is not set, then the application should
     * expect that it is running under DCL, and should expect VMS filenames
     * on input, and may need to output filenames in VMS format.
     *
     * This can be overriden at compile time with GNV_UNIX_TOOL being
     * defined.
     *
     * So this means that there will be multiple object modules from this
     * source module.  One for each shell, one for programs that can function
     * in both DCL and UNIX environments, and one for programs that require
     * a UNIX environment.
     */

#ifdef GNV_UNIX_SHELL
    use_unix_settings = 1;

    status = sys_crelnm("GNV$UNIX_SHELL", GNV_UNIX_SHELL);
    if (!$VMS_STATUS_SUCCESS(status)) {
	/* We have a big problem */
	LIB$SIGNAL(status);
    }
#else

#ifdef GNV_UNIX_TOOL
    use_unix_settings = 1;
#else
    status = sys_trnlnm("GNV$UNIX_SHELL",
			unix_shell_name, sizeof
			unix_shell_name -1);
    if (!$VMS_STATUS_SUCCESS(status)) {
	unix_shell_name[0] = 0;
	use_unix_settings = 0;
    }
#endif /* GNV_UNIX_TOOL */

#endif /* GNV_UNIX_SHELL */

    /* New style interface that works only on very recent
       (Apr 2001 and beyond) CRTLs */

    /*
     * Only setting defaults allows logical names to
     * override these settings.
     */

    /* Always set */

    /* ACCESS should check ACLs or it is lying. */
    set_feature_default("DECC$ACL_ACCESS_CHECK"		, ENABLE);

    /* We always want the new parse style */
    set_feature_default ("DECC$ARGV_PARSE_STYLE"	, ENABLE);

    /* Unless we are in POSIX compliant mode, we want the old POSIX root
     * enabled.
     */
    set_feature_default("DECC$DISABLE_POSIX_ROOT", DISABLE);

    /* EFS charset, means UTF-8 support */
    /* VTF-7 support is controlled by a feature setting called UTF8 */
    set_feature_default ("DECC$EFS_CHARSET" 		, ENABLE);
    set_feature_default ("DECC$EFS_CASE_PRESERVE"	, ENABLE);


    /* Support timestamps when available */
    set_feature_default ("DECC$EFS_FILE_TIMESTAMPS"	, ENABLE);

    /* Cache environment varibles - performance improvements */
    set_feature_default ("DECC$ENABLE_GETENV_CACHE"	, ENABLE);

    /* Start out with new file attribute inheritance */
#ifdef __VAX
    set_feature_default ("DECC$EXEC_FILEATTR_INHERITANCE", "2");
#else
    set_feature_default ("DECC$EXEC_FILEATTR_INHERITANCE", 2);
#endif

    /* Don't display trailing dot after files without type */
    set_feature_default ("DECC$READDIR_DROPDOTNOTYPE"	, ENABLE);

    /* For standard output channels buffer output until terminator */
    /* Gets rid of output logs with single character lines in them. */
    set_feature_default ("DECC$STDIO_CTX_EOL"		, ENABLE);

    /* Fix mv aa.bb aa						*/
    set_feature_default ("DECC$RENAME_NO_INHERIT"  	, ENABLE);

    if (use_unix_settings) {

	/* POSIX requires that open files be able to be removed */
	set_feature_default ("DECC$ALLOW_REMOVE_OPEN_FILES", ENABLE);

	set_feature_default ("DECC$FILENAME_UNIX_ONLY"	, ENABLE);
		/* FILENAME_UNIX_ONLY Implicitly sets
		  decc$disable_to_vms_logname_translation */

	set_feature_default ("DECC$FILE_PERMISSION_UNIX", ENABLE);

	/* For now this only with UNIX mode, applications can override
	 * with out using a LIB$INITIALIZE setting.
	 * This should be an application specific setting only enabled
	 * if the application requires it.
	 * Left here for now for backwards compatibility
         */
	set_feature_default ("DECC$FILE_SHARING"	, ENABLE);

        set_feature_default ("DECC$FILE_OWNER_UNIX"  	, ENABLE);
        set_feature_default ("DECC$POSIX_SEEK_STREAM_FILE", ENABLE);

    } else {
	 set_feature_default("DECC$FILENAME_UNIX_REPORT", ENABLE);
    }

    /* When reporting UNIX filenames, glob the same way */
    set_feature_default ("DECC$GLOB_UNIX_STYLE"	, ENABLE);

    /* The VMS version numbers on Unix filenames is incompatible with most */
    /* ported packages. */
    set_feature_default("DECC$FILENAME_UNIX_NO_VERSION", ENABLE);

    /* The VMS version numbers on Unix filenames is incompatible with most */
    /* ported packages. */
    set_feature_default("DECC$UNIX_PATH_BEFORE_LOGNAME", ENABLE);

    /* Set strtol to proper behavior */
    set_feature_default("DECC$STRTOL_ERANGE", ENABLE);

    /*  Pipe feature settings are longer needed with virtual memory pipe
	code.  Programs that use  pipe need to be converted to use the
	virtual memory pipe code, which effectively removes the hangs and
	left over temporary files.

        Comment left here to prevent regressions, as the larger pipe size
        actually hurts memory usage with the new algorithm.
     */
    /* do_not_set_default ("DECC$PIPE_BUFFER_SIZE"	, 8192); */


    /* Rather than remove this completely, a comment is left here to warn
     * someone from putting this bug back in.
     *
     * POSIX style UIDs require that the system administrator have set the
     * system up to use POSIX style UIDs and GIDs.  And if they have done
     * so, then they should set the DECC$POSIX_STYLE_UID as a system wide
     * logical name.
     *
     * Setting them in a program will break all routines that expect GID/UID
     * stuff to work on systems set up by default with out mappings.
     *
     * Most utilities do not reference GID/UID values, so it took a while for
     * this bug to surface.
     */
     /*  do_not_set_default ("DECC$POSIX_STYLE_UID"	, TRUE); */



    /* GNV depends on SYS$POSIX_ROOT to be properly set.  Since SYS$POSIX_ROOT
     * globally affects all C applications, SYS$POSIX_ROOT can not be set
     * anywhere that can be seen by other applications.
     *
     * So GNV$GNU is used instead, and SYS$POSIX_ROOT will be set in
     * in the process table in user mode to that value.
     *
     * Restriction: The system manager should not point GNV$GNU at
     * SYS$POSIX_ROOT, or anything that resolves to SYS$POSIX_ROOT.
     *
     */

    status = sys_trnlnm("GNV$GNU",
			gnv_posix_root,
			sizeof gnv_posix_root - 1);
    if ($VMS_STATUS_SUCCESS(status)) {
	status = sys_crelnm("SYS$POSIX_ROOT", "GNV$GNU:");
	gnv_posix_root_found = 1;
    }

    /* GNV depends on BIN being set to GNV$GNU:[bin].  Since BIN
     * is not prefixed, and it affects everything globally, it needs to
     * be set here if it is not defined already.
     * If it is set already, assume that it is correct, rather than
     * trying to second guess the user.
     * If GNV$GNU is not defined, then define bin to be SYS$POSIX_ROOT.
     */

    status = sys_trnlnm("BIN",
			gnv_posix_root,
			sizeof gnv_posix_root - 1);
    if (!$VMS_STATUS_SUCCESS(status)) {
	if (gnv_posix_root_found) {
	    status = sys_crelnm("BIN", "GNV$GNU:[BIN]");
	} else {
	    status = sys_crelnm("BIN", "SYS$POSIX_ROOT:[BIN]");
	}
    }

}

#pragma nostandard
#pragma extern_model save
#ifdef __VAX
#pragma extern_model strict_refdef "LIB$INITIALIZE" nowrt, long, nopic
#else
#pragma extern_model strict_refdef "LIB$INITIALIZE" nowrt, long
#    if __INITIAL_POINTER_SIZE
#        pragma __pointer_size __save
#        pragma __pointer_size 32
#    else
#        pragma __required_pointer_size __save
#        pragma __required_pointer_size 32
#    endif
#endif
/* Set our contribution to the LIB$INITIALIZE array */
void (* const iniarray[])(void) = {set_coe, } ;
#ifndef __VAX
#    if __INITIAL_POINTER_SIZE
#        pragma __pointer_size __restore
#    else
#        pragma __required_pointer_size __restore
#    endif
#endif


/*
** Force a reference to LIB$INITIALIZE to ensure it
** exists in the image.
*/
int LIB$INITIALIZE(void);
#ifdef __DECC
#pragma extern_model strict_refdef
#endif
    int lib_init_ref = (int) LIB$INITIALIZE;
#ifdef __DECC
#pragma extern_model restore
#pragma standard
#endif
