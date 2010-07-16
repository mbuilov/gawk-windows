# From arnold@f7.net  Thu Dec 15 08:57:08 2005
# Return-Path: <arnold@f7.net>
# Received: from localhost (skeeve [127.0.0.1])
# 	by skeeve.com (8.12.11/8.12.11) with ESMTP id jBF6ubXX003003
# 	for <arnold@localhost>; Thu, 15 Dec 2005 08:57:08 +0200
# Received: from pop.012.net.il [84.95.5.221]
# 	by localhost with POP3 (fetchmail-6.2.5)
# 	for arnold@localhost (single-drop); Thu, 15 Dec 2005 08:57:08 +0200 (IST)
# Received: from mtain3.012.net.il ([10.220.5.7])
#  by i_mss3.012.net.il (HyperSendmail v2004.12)
#  with ESMTP id <0IRI007F9SHYRDF0@i_mss3.012.net.il> for arobbins@012.net.il;
#  Thu, 15 Dec 2005 05:32:22 +0200 (IST)
# Received: from i_mtain1.012.net.il ([10.220.20.4])
#  by i_mtain3.012.net.il (HyperSendmail v2004.12)
#  with ESMTP id <0IRI00CUNSHXU5D0@i_mtain3.012.net.il> for arobbins@012.net.il
#  (ORCPT arobbins@012.net.il); Thu, 15 Dec 2005 05:32:22 +0200 (IST)
# Received: from f7.net ([64.34.169.74])
#  by i_mtain1.012.net.il (HyperSendmail v2004.12)
#  with ESMTP id <0IRI005JEPNHIG00@i_mtain1.012.net.il> for arobbins@012.net.il;
#  Thu, 15 Dec 2005 04:30:54 +0200 (IST)
# Received: (from arnold@localhost)	by f7.net (8.11.7-20030920/8.11.7)
#  id jBF2TkQ18162	for arobbins@012.net.il; Wed, 14 Dec 2005 20:29:46 -0600
# Received: from fencepost.gnu.org (fencepost.gnu.org [199.232.76.164])
# 	by f7.net (8.11.7-20030920/8.11.7) with ESMTP id jBF2Tis18148	for
#  <arnold@skeeve.com>; Wed, 14 Dec 2005 20:29:44 -0600
# Received: from monty-python.gnu.org ([199.232.76.173])
# 	by fencepost.gnu.org with esmtp (Exim 4.34)
# 	id 1Emir1-0007i6-1e	for bug-gawk@gnu.org; Wed, 14 Dec 2005 21:28:23 -0500
# Received: from Debian-exim by monty-python.gnu.org with spam-scanned
#  (Exim 4.34)	id 1Emiv6-0006FV-9G	for bug-gawk@gnu.org; Wed,
#  14 Dec 2005 21:32:37 -0500
# Received: from [129.99.139.165] (helo=ece03.nas.nasa.gov)
# 	by monty-python.gnu.org with esmtp (TLS-1.0:DHE_RSA_3DES_EDE_CBC_SHA:24)
# 	(Exim 4.34)	id 1Emiv5-0006F0-UO	for bug-gawk@gnu.org; Wed,
#  14 Dec 2005 21:32:36 -0500
# Received: from ece03.nas.nasa.gov (localhost.localdomain [127.0.0.1])
# 	by ece03.nas.nasa.gov (8.12.11/8.12.11) with ESMTP id jBF2TcNa022308; Wed,
#  14 Dec 2005 18:29:38 -0800
# Received: (from ellswort@localhost)
# 	by ece03.nas.nasa.gov (8.12.11/8.12.11/Submit) id jBF2TcQC022307; Wed,
#  14 Dec 2005 18:29:38 -0800
# Date: Wed, 14 Dec 2005 18:29:38 -0800
# From: David Ellsworth <ellswort@nas.nasa.gov>
# Subject: gawk number to string bug
# To: bug-gawk@gnu.org
# Cc: ellswort@nas.nasa.gov
# Message-id: <200512150229.jBF2TcQC022307@ece03.nas.nasa.gov>
# Original-recipient: rfc822;arobbins@012.net.il
# X-Spam-Checker-Version: SpamAssassin 2.63 (2004-01-11) on skeeve.com
# X-Spam-Level: 
# X-Spam-Status: No, hits=-4.9 required=5.0 tests=BAYES_00 autolearn=ham 
# 	version=2.63
# 
# 
# I ran into a bug in how gawk converts numbers into strings.  
# 
# Running the program
   BEGIN { print 9223372036854775808 }
# prints
#   -9223372036854775808
# on a 64-bit machine.  On a 32-bit machine, the same program prints out
#   9.22337e+18
# which is correct.
# 
# Note that 9223372036854775808 == 2^63.  Printing out other powers of 2
# is OK.
# 
# Version details:
#   gawk version 3.1.5, freshly downloaded from gnu.org
#   gcc (GCC) 3.4.4 (Gentoo 3.4.4-r1, ssp-3.4.4-1.0, pie-8.7.8)
#   uname -a output:
#     Linux pixie 2.6.12-gentoo-r10 #2 SMP Tue Nov 1 03:42:22 PST 2005
#     x86_64 Dual Core AMD Opteron(tm) Processor 275 AuthenticAMD GNU/Linux
# 
# (the 32-bit example used gawk version 3.1.3)
# 
# - David
# 
# 
# #####################################################################################
# This Mail Was Scanned by 012.net AntiVirus Service4- Powered by TrendMicro Interscan
#
