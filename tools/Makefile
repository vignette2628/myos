CC	    = gcc

targets = build_image

%.o : %.c 
	@echo Making $<
	@$(CC) -Wall $< -o $@


all: $(targets)

clean:
	rm -f $(targets)
	rm -f *.bak
