CC = gcc
CFLAGS = -Wall -Wextra -Werror

all: welcome enter_text redirect

welcome: src/welcome.c
	$(CC) $(CFLAGS) $< -o welcome

enter_text: src/enter_text.c
	$(CC) $(CFLAGS) $< -o enter_text

redirect: src/redirect.c
	$(CC) $(CFLAGS) $< -o redirect

clean:
	rm -f welcome enter_text redirect
