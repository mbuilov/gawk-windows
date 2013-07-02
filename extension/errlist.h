/*
 * errlist.h - List of errno values.
 */

/*
 * Copyright (C) 2013 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifdef E2BIG
init_errno (E2BIG, "E2BIG")
#endif
#ifdef EACCES
init_errno (EACCES, "EACCES")
#endif
#ifdef EADDRINUSE
init_errno (EADDRINUSE, "EADDRINUSE")
#endif
#ifdef EADDRNOTAVAIL
init_errno (EADDRNOTAVAIL, "EADDRNOTAVAIL")
#endif
#ifdef EADV
init_errno (EADV, "EADV")
#endif
#ifdef EAFNOSUPPORT
init_errno (EAFNOSUPPORT, "EAFNOSUPPORT")
#endif
#ifdef EAGAIN
init_errno (EAGAIN, "EAGAIN")
#endif
#ifdef EALREADY
init_errno (EALREADY, "EALREADY")
#endif
#ifdef EAUTH
init_errno (EAUTH, "EAUTH")
#endif
#ifdef EBADE
init_errno (EBADE, "EBADE")
#endif
#ifdef EBADF
init_errno (EBADF, "EBADF")
#endif
#ifdef EBADFD
init_errno (EBADFD, "EBADFD")
#endif
#ifdef EBADMSG
init_errno (EBADMSG, "EBADMSG")
#endif
#ifdef EBADR
init_errno (EBADR, "EBADR")
#endif
#ifdef EBADRPC
init_errno (EBADRPC, "EBADRPC")
#endif
#ifdef EBADRQC
init_errno (EBADRQC, "EBADRQC")
#endif
#ifdef EBADSLT
init_errno (EBADSLT, "EBADSLT")
#endif
#ifdef EBFONT
init_errno (EBFONT, "EBFONT")
#endif
#ifdef EBUSY
init_errno (EBUSY, "EBUSY")
#endif
#ifdef ECANCELED
init_errno (ECANCELED, "ECANCELED")
#endif
#ifdef ECHILD
init_errno (ECHILD, "ECHILD")
#endif
#ifdef ECHRNG
init_errno (ECHRNG, "ECHRNG")
#endif
#ifdef ECOMM
init_errno (ECOMM, "ECOMM")
#endif
#ifdef ECONNABORTED
init_errno (ECONNABORTED, "ECONNABORTED")
#endif
#ifdef ECONNREFUSED
init_errno (ECONNREFUSED, "ECONNREFUSED")
#endif
#ifdef ECONNRESET
init_errno (ECONNRESET, "ECONNRESET")
#endif
#ifdef EDEADLK
init_errno (EDEADLK, "EDEADLK")
#endif
#ifdef EDEADLOCK
#if !defined(EDEADLK) || (EDEADLK != EDEADLOCK)
init_errno (EDEADLOCK, "EDEADLOCK")
#endif
#endif
#ifdef EDESTADDRREQ
init_errno (EDESTADDRREQ, "EDESTADDRREQ")
#endif
#ifdef EDOM
init_errno (EDOM, "EDOM")
#endif
#ifdef EDOTDOT
init_errno (EDOTDOT, "EDOTDOT")
#endif
#ifdef EDQUOT
init_errno (EDQUOT, "EDQUOT")
#endif
#ifdef EEXIST
init_errno (EEXIST, "EEXIST")
#endif
#ifdef EFAULT
init_errno (EFAULT, "EFAULT")
#endif
#ifdef EFBIG
init_errno (EFBIG, "EFBIG")
#endif
#ifdef EFTYPE
init_errno (EFTYPE, "EFTYPE")
#endif
#ifdef EHOSTDOWN
init_errno (EHOSTDOWN, "EHOSTDOWN")
#endif
#ifdef EHOSTUNREACH
init_errno (EHOSTUNREACH, "EHOSTUNREACH")
#endif
#ifdef EIDRM
init_errno (EIDRM, "EIDRM")
#endif
#ifdef EILSEQ
init_errno (EILSEQ, "EILSEQ")
#endif
#ifdef EINPROGRESS
init_errno (EINPROGRESS, "EINPROGRESS")
#endif
#ifdef EINTR
init_errno (EINTR, "EINTR")
#endif
#ifdef EINVAL
init_errno (EINVAL, "EINVAL")
#endif
#ifdef EIO
init_errno (EIO, "EIO")
#endif
#ifdef EISCONN
init_errno (EISCONN, "EISCONN")
#endif
#ifdef EISDIR
init_errno (EISDIR, "EISDIR")
#endif
#ifdef EISNAM
init_errno (EISNAM, "EISNAM")
#endif
#ifdef EKEYEXPIRED
init_errno (EKEYEXPIRED, "EKEYEXPIRED")
#endif
#ifdef EKEYREJECTED
init_errno (EKEYREJECTED, "EKEYREJECTED")
#endif
#ifdef EKEYREVOKED
init_errno (EKEYREVOKED, "EKEYREVOKED")
#endif
#ifdef EL2HLT
init_errno (EL2HLT, "EL2HLT")
#endif
#ifdef EL2NSYNC
init_errno (EL2NSYNC, "EL2NSYNC")
#endif
#ifdef EL3HLT
init_errno (EL3HLT, "EL3HLT")
#endif
#ifdef EL3RST
init_errno (EL3RST, "EL3RST")
#endif
#ifdef ELAST
init_errno (ELAST, "ELAST")
#endif
#ifdef ELIBACC
init_errno (ELIBACC, "ELIBACC")
#endif
#ifdef ELIBBAD
init_errno (ELIBBAD, "ELIBBAD")
#endif
#ifdef ELIBEXEC
init_errno (ELIBEXEC, "ELIBEXEC")
#endif
#ifdef ELIBMAX
init_errno (ELIBMAX, "ELIBMAX")
#endif
#ifdef ELIBSCN
init_errno (ELIBSCN, "ELIBSCN")
#endif
#ifdef ELNRNG
init_errno (ELNRNG, "ELNRNG")
#endif
#ifdef ELOOP
init_errno (ELOOP, "ELOOP")
#endif
#ifdef EMEDIUMTYPE
init_errno (EMEDIUMTYPE, "EMEDIUMTYPE")
#endif
#ifdef EMFILE
init_errno (EMFILE, "EMFILE")
#endif
#ifdef EMLINK
init_errno (EMLINK, "EMLINK")
#endif
#ifdef EMSGSIZE
init_errno (EMSGSIZE, "EMSGSIZE")
#endif
#ifdef EMULTIHOP
init_errno (EMULTIHOP, "EMULTIHOP")
#endif
#ifdef ENAMETOOLONG
init_errno (ENAMETOOLONG, "ENAMETOOLONG")
#endif
#ifdef ENAVAIL
init_errno (ENAVAIL, "ENAVAIL")
#endif
#ifdef ENEEDAUTH
init_errno (ENEEDAUTH, "ENEEDAUTH")
#endif
#ifdef ENETDOWN
init_errno (ENETDOWN, "ENETDOWN")
#endif
#ifdef ENETRESET
init_errno (ENETRESET, "ENETRESET")
#endif
#ifdef ENETUNREACH
init_errno (ENETUNREACH, "ENETUNREACH")
#endif
#ifdef ENFILE
init_errno (ENFILE, "ENFILE")
#endif
#ifdef ENOANO
init_errno (ENOANO, "ENOANO")
#endif
#ifdef ENOBUFS
init_errno (ENOBUFS, "ENOBUFS")
#endif
#ifdef ENOCSI
init_errno (ENOCSI, "ENOCSI")
#endif
#ifdef ENODATA
init_errno (ENODATA, "ENODATA")
#endif
#ifdef ENODEV
init_errno (ENODEV, "ENODEV")
#endif
#ifdef ENOENT
init_errno (ENOENT, "ENOENT")
#endif
#ifdef ENOEXEC
init_errno (ENOEXEC, "ENOEXEC")
#endif
#ifdef ENOKEY
init_errno (ENOKEY, "ENOKEY")
#endif
#ifdef ENOLCK
init_errno (ENOLCK, "ENOLCK")
#endif
#ifdef ENOLINK
init_errno (ENOLINK, "ENOLINK")
#endif
#ifdef ENOMEDIUM
init_errno (ENOMEDIUM, "ENOMEDIUM")
#endif
#ifdef ENOMEM
init_errno (ENOMEM, "ENOMEM")
#endif
#ifdef ENOMSG
init_errno (ENOMSG, "ENOMSG")
#endif
#ifdef ENONET
init_errno (ENONET, "ENONET")
#endif
#ifdef ENOPKG
init_errno (ENOPKG, "ENOPKG")
#endif
#ifdef ENOPROTOOPT
init_errno (ENOPROTOOPT, "ENOPROTOOPT")
#endif
#ifdef ENOSPC
init_errno (ENOSPC, "ENOSPC")
#endif
#ifdef ENOSR
init_errno (ENOSR, "ENOSR")
#endif
#ifdef ENOSTR
init_errno (ENOSTR, "ENOSTR")
#endif
#ifdef ENOSYS
init_errno (ENOSYS, "ENOSYS")
#endif
#ifdef ENOTBLK
init_errno (ENOTBLK, "ENOTBLK")
#endif
#ifdef ENOTCONN
init_errno (ENOTCONN, "ENOTCONN")
#endif
#ifdef ENOTDIR
init_errno (ENOTDIR, "ENOTDIR")
#endif
#ifdef ENOTEMPTY
init_errno (ENOTEMPTY, "ENOTEMPTY")
#endif
#ifdef ENOTNAM
init_errno (ENOTNAM, "ENOTNAM")
#endif
#ifdef ENOTRECOVERABLE
init_errno (ENOTRECOVERABLE, "ENOTRECOVERABLE")
#endif
#ifdef ENOTSOCK
init_errno (ENOTSOCK, "ENOTSOCK")
#endif
#ifdef ENOTTY
init_errno (ENOTTY, "ENOTTY")
#endif
#ifdef ENOTUNIQ
init_errno (ENOTUNIQ, "ENOTUNIQ")
#endif
#ifdef ENXIO
init_errno (ENXIO, "ENXIO")
#endif
#ifdef EOPNOTSUPP
init_errno (EOPNOTSUPP, "EOPNOTSUPP")
#endif
#ifdef EOVERFLOW
init_errno (EOVERFLOW, "EOVERFLOW")
#endif
#ifdef EOWNERDEAD
init_errno (EOWNERDEAD, "EOWNERDEAD")
#endif
#ifdef EPERM
init_errno (EPERM, "EPERM")
#endif
#ifdef EPFNOSUPPORT
init_errno (EPFNOSUPPORT, "EPFNOSUPPORT")
#endif
#ifdef EPIPE
init_errno (EPIPE, "EPIPE")
#endif
#ifdef EPROCLIM
init_errno (EPROCLIM, "EPROCLIM")
#endif
#ifdef EPROCUNAVAIL
init_errno (EPROCUNAVAIL, "EPROCUNAVAIL")
#endif
#ifdef EPROGMISMATCH
init_errno (EPROGMISMATCH, "EPROGMISMATCH")
#endif
#ifdef EPROGUNAVAIL
init_errno (EPROGUNAVAIL, "EPROGUNAVAIL")
#endif
#ifdef EPROTO
init_errno (EPROTO, "EPROTO")
#endif
#ifdef EPROTONOSUPPORT
init_errno (EPROTONOSUPPORT, "EPROTONOSUPPORT")
#endif
#ifdef EPROTOTYPE
init_errno (EPROTOTYPE, "EPROTOTYPE")
#endif
#ifdef ERANGE
init_errno (ERANGE, "ERANGE")
#endif
#ifdef EREMCHG
init_errno (EREMCHG, "EREMCHG")
#endif
#ifdef EREMOTE
init_errno (EREMOTE, "EREMOTE")
#endif
#ifdef EREMOTEIO
init_errno (EREMOTEIO, "EREMOTEIO")
#endif
#ifdef ERESTART
init_errno (ERESTART, "ERESTART")
#endif
#ifdef ERFKILL
init_errno (ERFKILL, "ERFKILL")
#endif
#ifdef EROFS
init_errno (EROFS, "EROFS")
#endif
#ifdef ERPCMISMATCH
init_errno (ERPCMISMATCH, "ERPCMISMATCH")
#endif
#ifdef ESHUTDOWN
init_errno (ESHUTDOWN, "ESHUTDOWN")
#endif
#ifdef ESOCKTNOSUPPORT
init_errno (ESOCKTNOSUPPORT, "ESOCKTNOSUPPORT")
#endif
#ifdef ESPIPE
init_errno (ESPIPE, "ESPIPE")
#endif
#ifdef ESRCH
init_errno (ESRCH, "ESRCH")
#endif
#ifdef ESRMNT
init_errno (ESRMNT, "ESRMNT")
#endif
#ifdef ESTALE
init_errno (ESTALE, "ESTALE")
#endif
#ifdef ESTRPIPE
init_errno (ESTRPIPE, "ESTRPIPE")
#endif
#ifdef ETIME
init_errno (ETIME, "ETIME")
#endif
#ifdef ETIMEDOUT
init_errno (ETIMEDOUT, "ETIMEDOUT")
#endif
#ifdef ETOOMANYREFS
init_errno (ETOOMANYREFS, "ETOOMANYREFS")
#endif
#ifdef ETXTBSY
init_errno (ETXTBSY, "ETXTBSY")
#endif
#ifdef EUCLEAN
init_errno (EUCLEAN, "EUCLEAN")
#endif
#ifdef EUNATCH
init_errno (EUNATCH, "EUNATCH")
#endif
#ifdef EUSERS
init_errno (EUSERS, "EUSERS")
#endif
#ifdef EWOULDBLOCK
#if !defined(EAGAIN) || (EWOULDBLOCK != EAGAIN)
init_errno (EWOULDBLOCK, "EWOULDBLOCK")
#endif
#endif
#ifdef EXDEV
init_errno (EXDEV, "EXDEV")
#endif
#ifdef EXFULL
init_errno (EXFULL, "EXFULL")
#endif
