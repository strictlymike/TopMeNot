TARGET = topnot.exe

CC = cl.exe
CFLAGS = $(CFLAGS) /Zi
LIBS =
RM = DEL /Q
CLEANFILES = $(TARGET) *.exp *.obj *.pdb *.ilk

all: $(TARGET)

.cpp.exe:
	$(CC) $** $(LIBS) $(CFLAGS)

.c.exe:
	$(CC) $** $(LIBS) $(CFLAGS)

.cpp.dll:
	$(CC) /LD $** $(LIBS) $(CFLAGS)

.c.dll:
	$(CC) /LD $** $(LIBS) $(CFLAGS)

clean:
	$(RM) $(CLEANFILES) > NUL 2>&1
