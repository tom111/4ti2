EXTRA_DIST = 					\
	33.mat 33.ray.chk			\
	44.mat 44.ray.chk			\
	55.mat 55.ray.chk

check-local:
	@../check.template $(srcdir) "" src/groebner/rays -q -m 
	@../check.template $(srcdir) "" src/groebner/rays -q -s 

CLEANFILES = .num_failures
