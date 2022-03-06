SHELL:=bash
lib:
	gcc aug.c -o aug.so -shared -fPIC -pthread -ldl
run:
	LD_PRELOAD=$(PWD)/aug.so $(SHELL)
