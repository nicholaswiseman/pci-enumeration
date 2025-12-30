enum.exe: ume.c
	gcc -g -O0 ume.c -o enum.exe -lsetupapi

clean:
	rm -f enum.exe