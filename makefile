build:
	gcc -g src/agent.c src/main.c src/allocators.c src/boardio.c -o konane.exe

submission:
	gcc -g src/agent.c src/main.c src/allocators.c src/boardio.c -o T2

	
