CFLAGS=-g -Wall -Werror
.PHONY: tests2

all: tests lib_tar.o intro

intro:
	@echo "LINFO1252 – Systèmes informatiques Projet 2 "
	@echo "Groupe 51(vendredi)"
	@echo ""
	@echo "Description du projet :"
	@echo "Manipulation et Gestion des Archives TAR"
	@echo ""
	@echo "Membres du groupe :"
	@echo "Nathan Spilette ,NOMA  :35612000"
	@echo "Bruno Ingrao    ,NOMA  :77242000"
	@echo ""
	@echo "faire make tests2 pour lancer les tests"
	@echo ""
	@echo "Vous pouvez lancer make tests3 mais il faut" 
	@echo "Changer manuellement path_file à "lib_tar.c""
	@echo "Dans le code tests.c"

lib_tar.o: lib_tar.c lib_tar.h

tests: tests.c lib_tar.o

tests2: 
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c fichier1.c bonjour/ lib_tar.c lib_tar.h > archive.tar
	./tests archive.tar

tests3: 
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c fichier1.c lib_tar.c bonjour lib_tar.h > archive.tar
	./tests archive.tar 


clean:
	rm -f lib_tar.o tests soumission.tar

submit: all
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c *.h *.c Makefile > soumission.tar