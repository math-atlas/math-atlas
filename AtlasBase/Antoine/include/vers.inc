@define atlas       @ATLAS@
@define version     @Version 3.9.24@
@ifdef ! year
   @define year        @2000@
   @define date        @December 25, @(year)@
@endifdef
@ifdef ! date
   @define date @@(year)@
@endifdef
@define antoine     @Antoine P. Petitet@
@define clint       @R. Clint Whaley@
@define jeff        @Jeff Horner@
@ifdef ! author
@define author      @@(antoine)@
@endifdef
@lan F
*
*  -- Automatically Tuned Linear Algebra Software (@(atlas))
*     (C) Copyright @(year) All Rights Reserved
*
@inc AtlasTstDrv
*  -- @(atlas) testing driver -- @(version) -- @(date)
@inc AtlasTstRout
*  -- @(atlas) testing routine -- @(version) -- @(date)
@inc AtlasTimDrv
*  -- @(atlas) timing driver -- @(version) -- @(date)
@inc AtlasTimRout
*  -- @(atlas) timing routine -- @(version) -- @(date)
@inc AtlasSrcRout
*  -- @(atlas) routine -- @(version) -- @(date)
@inc AtlasAuxRout
*  -- @(atlas) auxiliary routine -- @(version) -- @(date)
@inc AtlasF77Rout
*  -- @(atlas) routine -- F77 Interface -- @(version) -- @(date)
@inc !
@skip *
@skip *  -- Suggestions,  comments,  bugs reports should be reported to the ATLAS
@skip *     support tracker at : 
@skip *        http://sourceforge.net/tracker/?group_id=23725&atid=379483
*
@ifdef author
*  Author         : @(author)
@endifdef
@ifdef contributor
*  Contributor(s) : @(contributor)
@endifdef
@iif year < 2002
*  Originally developed at the University of Tennessee,
*  Innovative Computing Laboratory,  Knoxville TN, 37996-1301, USA. 
@endiif
*
*  ---------------------------------------------------------------------
*
*  -- Copyright notice and Licensing terms:
*
*  Redistribution  and  use in  source and binary forms, with or without
*  modification, are  permitted provided  that the following  conditions
*  are met:
*
*  1. Redistributions  of  source  code  must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce  the above copyright
*     notice,  this list of conditions, and the  following disclaimer in
*     the documentation and/or other materials provided with the distri-
*     bution.
*  3. The name of the University,  the ATLAS group,  or the names of its
*     contributors  may not be used to endorse or promote products deri-
*     ved from this software without specific written permission.
*     
*  -- Disclaimer:
*
*  THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
*  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPE-
*  CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
*  TO,  PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
*  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEO-
*  RY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (IN-
*  CLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
*  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*  
*  ---------------------------------------------------------------------
*
@lan C
/* ---------------------------------------------------------------------
 *
 * -- Automatically Tuned Linear Algebra Software (@(atlas))
 *    (C) Copyright @(year) All Rights Reserved
 *
@inc AtlasTstDrv
 * -- @(atlas) testing driver -- @(version) -- @(date)
@inc AtlasTstRout
 * -- @(atlas) test routine -- @(version) -- @(date)
@inc AtlasTimDrv
 * -- @(atlas) timing driver -- @(version) -- @(date)
@inc AtlasTimRout
 * -- @(atlas) test routine -- @(version) -- @(date)
@inc AtlasSrcRout
 * -- @(atlas) routine -- @(version) -- @(date)
@inc AtlasAuxRout
 * -- @(atlas) auxiliary routine -- @(version) -- @(date)
@inc AtlasF77Rout
 * -- @(atlas) routine -- F77 Interface -- @(version) -- @(date)
@inc !
 *
@skip * -- Suggestions,  comments,  bugs reports should be sent to the follo-
@skip *    wing e-mail address: atlas@cs.utk.edu
@skip *
@ifdef author
 * Author         : @(author)
@endifdef
@ifdef contributor
 * Contributor(s) : @(contributor)
@endifdef
 * Originally developed at the University of Tennessee,
 * Innovative Computing Laboratory, Knoxville TN, 37996-1301, USA. 
 *
 * ---------------------------------------------------------------------
 *
 * -- Copyright notice and Licensing terms:
 *
 *  Redistribution  and  use in  source and binary forms, with or without
 *  modification, are  permitted provided  that the following  conditions
 *  are met:
 *
 * 1. Redistributions  of  source  code  must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce  the above copyright
 *    notice,  this list of conditions, and the  following disclaimer in
 *    the documentation and/or other materials provided with the distri-
 *    bution.
 * 3. The name of the University,  the ATLAS group,  or the names of its
 *    contributors  may not be used to endorse or promote products deri-
 *    ved from this software without specific written permission.
 *
 * -- Disclaimer:
 *
 * THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO,  PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEO-
 * RY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (IN-
 * CLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ---------------------------------------------------------------------
 */
@lan M
########################################################################
#  
#  -- Automatically Tuned Linear Algebra Software (@(atlas))
@skip #     University of Tennessee - Innovative Computing Laboratory
@skip #     Knoxville TN, 37996-1301 USA.
#     (C) Copyright @(year) All Rights Reserved
#
@skip #  -- Suggestions,  comments,  bugs reports should be sent to the follo-
@skip #     wing e-mail address: atlas@cs.utk.edu
@skip #  
########################################################################
#
#  -- Copyright notice and Licensing terms:
#
#  Redistribution  and  use in  source and binary forms, with or without
#  modification, are  permitted provided  that the following  conditions
#  are met:
#
#  1. Redistributions  of  source  code  must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce  the above copyright
#     notice,  this list of conditions, and the  following disclaimer in
#     the documentation and/or other materials provided with the distri-
#     bution.
#  3. The name of the University,  the ATLAS group,  or the names of its
#     contributors  may not be used to endorse or promote products deri-
#     ved from this software without specific written permission.
#
#  -- Disclaimer:
#
#  THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
#  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,  INDIRECT, INCIDENTAL, SPE-
#  CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO,  PROCUREMENT  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
#  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEO-
#  RY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT  (IN-
#  CLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
#  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#  
########################################################################
@lan M F C
@lan !
