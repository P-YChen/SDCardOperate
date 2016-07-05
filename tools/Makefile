TARGET=ParseTimeAndDate
OBJS=ParseTimeAndDate.o

%.o:%.c
	gcc -c $< -o $@

$(TARGET):$(OBJS)
	gcc $< -o $@

clean:
	rm -rf *.o $(TARGET)
