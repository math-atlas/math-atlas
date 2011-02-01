@define comdir      @@(BASEdir)/comments/@
@define docdir      @@(BASEdir)/docs/@
@define incdir      @@(BASEdir)/include/@
@define makdir      @@(BASEdir)/make/@
@define srcdir      @@(BASEdir)/@
@skip
@skip
@extract -addkeys -b @(incdir)type.inc
@extract -addkeys -b @(incdir)macros.inc
