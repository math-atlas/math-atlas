@WHAT crsetup
@define usept @ATL_USEPTHREADS@
@define ap00 @-def cdate 2000 -def author "Antoine P. Petitet"@
@define ap99 @-def cdate 1999 -def author "Antoine P. Petitet"@
@define aw99 @-def cdate 1999 -def author "Antoine P. Petitet" -def contrib "R. Clint Whaley"@
@define aw04 @-def cdate 2004 -def author "Antoine P. Petitet" -def contrib "R. Clint Whaley"@
@define aw00 @-def cdate 2000 -def author "Antoine P. Petitet" -def contrib "R. Clint Whaley"@
@define rm09 @-def cdate 2009 -def author "Ryan Moon" -def contrib "R. Clint Whaley"@
@define cw97 @-def cdate 1997@
@define cw98 @-def cdate 1998@
@define cw99 @-def cdate 1999@
@define cw00 @-def cdate 2000@
@define cw01 @-def cdate 2001@
@define cw02 @-def cdate 2002@
@define cw03 @-def cdate 2003@
@define cw04 @-def cdate 2004@
@define cw05 @-def cdate 2005@
@define cw06 @-def cdate 2006@
@define cw07 @-def cdate 2007@
@define cw08 @-def cdate 2008@
@define cw09 @-def cdate 2009@
@define cw10 @-def cdate 2010@
@define cw11 @-def cdate 2011@
@define cp99 @-def cdate 1999 -def contrib "Antoine P. Petitet"@
@define cp00 @-def cdate 2000 -def contrib "Antoine P. Petitet"@
@define sw01 @-def cdate 2001 -def author "Peter Soendergaard" -def contrib "R. Clint Whaley"@
@define ps01 @-def cdate 2001 -def author "Peter Soendergaard"@
@define pw01 @-def cdate 2001 -def author "Peter Soendergaard" -def contrib "R. Clint Whaley"@
@ifdef ! ver
   @define ver @3.11.40@
@endifdef
@ifdef ! author
   @define author @R. Clint Whaley@
@endifdef
@ifdef ! cdate
   @define cdate @1997@
@endifdef
@WHAT f77lp
*
*  -- Modified by @(author) for ATLAS Fortran77 LAPACK interface,
*     @(cdate)
*
@WHAT acw acw!
   @define cb @! @
@WHAT acw
   @define cb @; @
@WHAT cw
@ifdef ! cb
   @define NEED_START @true@
   @define cb @ *@
@endifdef
@ifdef NEED_START
/*
@endifdef
@WHAT acw cw acw!
@define nmln @(C) Copyright @(cdate) @(author)@
@(cb) Automatically Tuned Linear Algebra Software v@(ver)
@(cb) @(nmln)
@ifdef contrib
@(cb) Code contributers : @(author), @(contrib)
@endifdef
@beginskip
@(cb)
@(cb) Redistribution and use in source and binary forms, with or without
@(cb) modification, are permitted provided that the following conditions
@(cb) are met:
@(cb)   1. Redistributions of source code must retain the above copyright
@(cb)      notice, this list of conditions and the following disclaimer.
@(cb)   2. Redistributions in binary form must reproduce the above copyright
@(cb)      notice, this list of conditions, and the following disclaimer in the
@(cb)      documentation and/or other materials provided with the distribution.
@(cb)   3. The name of the ATLAS group or the names of its contributers may
@(cb)      not be used to endorse or promote products derived from this
@(cb)      software without specific written permission.
@(cb)
@(cb) THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
@(cb) ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
@(cb) TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
@(cb) PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE ATLAS GROUP OR ITS CONTRIBUTORS
@(cb) BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
@(cb) CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
@(cb) SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
@(cb) INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
@(cb) CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
@(cb) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
@(cb) POSSIBILITY OF SUCH DAMAGE. 
@(cb)
@endskip
@WHAT cw
@ifdef NEED_START
 */
@endifdef
