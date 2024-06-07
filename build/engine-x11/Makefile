all:
	@echo 'Usage:'
	@echo '  make linux    ... for Linux'
	@echo '  make static   ... for Linux (statically-linked)'
	@echo '  make debug    ... for Linux (with debug symbols)'
	@echo '  make freebsd  ... for FreeBSD'
	@echo '  make netbsd   ... for NetBSD'
	@echo '  make openbsd  ... for OpenBSD'

linux:
	@make -f Makefile.linux

static:
	@make -f Makefile.linux-static

debug:
	@make -f Makefile.linux-debug

freebsd:
	@make -f Makefile.freebsd

netbsd:
	@make -f Makefile.netbsd

openbsd:
	@make -f Makefile.openbsd
