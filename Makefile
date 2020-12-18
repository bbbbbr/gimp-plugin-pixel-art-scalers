# Predefined constants
CC      = gcc
TARGET  = plugin-pixel-art-scalers
TARGETLIB = libpas.a
SRC_DIR = src
OBJ_DIR = obj
GTKCFLAGS  = $(shell pkg-config --cflags gtk+-2.0) \
             $(shell pkg-config --cflags gimp-2.0)
GTKLFLAGS  = $(shell pkg-config --libs glib-2.0) \
          $(shell pkg-config --libs gtk+-2.0) \
          $(shell pkg-config --libs gimp-2.0) \
          $(shell pkg-config --libs gimpui-2.0)

# File definitions
LIB_SRC_FILES=$(wildcard $(SRC_DIR)/lib/*.c)
GTK_SRC_FILES=$(wildcard $(SRC_DIR)/gtk2/*.c)
LIB_OBJ_FILES=$(LIB_SRC_FILES:$(SRC_DIR)/lib/%.c=$(OBJ_DIR)/lib/%.o)
GTK_OBJ_FILES=$(GTK_SRC_FILES:$(SRC_DIR)/gtk2/%.c=$(OBJ_DIR)/gtk2/%.o)

$(TARGET): $(OBJ_DIR) $(TARGETLIB) $(GTK_OBJ_FILES)
	$(CC) $(GTK_OBJ_FILES) $(TARGETLIB) -o $(TARGET) $(GTKLFLAGS)

$(TARGETLIB): $(OBJ_DIR) $(LIB_OBJ_FILES)
	$(AR) rcs $@ $(LIB_OBJ_FILES)

$(OBJ_DIR)/lib/%.o: $(SRC_DIR)/lib/%.c
	$(CC) -c $^ -o $@ $(CFLAGS)

$(OBJ_DIR)/gtk2/%.o: $(SRC_DIR)/gtk2/%.c
	$(CC) -c $^ -o $@ $(GTKCFLAGS)

$(OBJ_DIR):
	test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
	test -d $(OBJ_DIR)/lib || mkdir -p $(OBJ_DIR)/lib
	test -d $(OBJ_DIR)/gtk2 || mkdir -p $(OBJ_DIR)/gtk2

clean:
	rm -rf $(OBJ_DIR)
	rm $(TARGET) $(TARGETLIB)

install:
	mkdir -p $(DESTDIR)$(exec_prefix)/lib/gimp/2.0/plug-ins
	cp $(TARGET) $(DESTDIR)$(exec_prefix)/lib/gimp/2.0/plug-ins

uninstall:
	rm $(DESTDIR)$(exec_prefix)/lib/gimp/2.0/plug-ins/$(TARGET)

.PHONY: clean install uninstall
