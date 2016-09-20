@LANG make
   @define cb @# @
@LANG C
   @define cb @ *@
/*
@LANG F77
   @define cb @* @
@LANG !
@ifdef ! ver
   @define ver @3.11.40@
@endifdef
@ifdef ! cwauth
   @define cwauth @R. Clint Whaley@
@endifdef
@(cb) Automatically Tuned Linear Algebra Software v@(ver)
@ifdef cwdate
   @define nmln @ Copyright (C) @(cwdate)@
   @undef cwdate
   @whiledef cwdate
      @define II @@len@(nmln)@
      @iif II > 5
         @define ln2 @@(nmln), @(cwdate)@
      @endiif
      @iif II < 6
         @define ln2 @               @(cwdate)@
      @endiif
      @undef II
      @undef nmln
      @define nmln @@(ln2)@
      @undef ln2
      @define II @@len@(nmln)@
      @iif II > 70
         @define cwd2 @@(cwdate)@
         @undef cwdate
         @ifdef cwdate
@(cb)@(nmln),
         @endifdef
         @ifdef ! cwdate
@(cb)@(nmln)
         @endifdef
         @define cwdate @@(cwd2)@
         @undef cwd2
         @undef nmln
         @define nmln @ @
      @endiif
      @undef II
   @endwhile
   @define II @@len@(nmln)@
   @iif II > 5
      @iif II > 60
@(cb)@(nmln)
@(cb)               @(cwauth)
      @endiif
      @iif II < 61
@(cb)@(nmln) @(cwauth)
      @endiif
   @endiif
   @iif II < 6
@(cb)               @(cwauth)
   @endiif
@endifdef
@ifdef contrib
@(cb) Code contributers : @(cwauth), @(contrib)
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
@LANG C
@endskip
 */
@undef cwauth
@ifdef contrib
   @undef contrib
@endifdef
