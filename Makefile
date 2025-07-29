CFLAGS = -Wall -Iinclude -MMD -MP `pkg-config --cflags gtk4`
LDFLAGS = `pkg-config --libs gtk4` -lm -lX11
SRC = $(wildcard src/*.c lib/*.c)

OBJ = $(patsubst %.c,build/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
TARGET = build/sticky-notes-app

WIN_CC = x86_64-w64-mingw32-gcc
WIN_CFLAGS = -Wall -Iinclude -MMD -MP
WIN_PKG_CONFIG_ENV = PKG_CONFIG_DEFINE_PREFIX=1 PKG_CONFIG_SYSROOT_DIR=/opt/gtk-win64 PKG_CONFIG_LIBDIR=/opt/gtk-win64/lib/pkgconfig PKG_CONFIG_PATH=/opt/gtk-win64/lib/pkgconfig
WIN_PKG_CONFIG = x86_64-w64-mingw32-pkg-config
# Manually override include paths since pkg-config has wrong paths
WIN_GTK_INCLUDES = -I/opt/gtk-win64/include/gtk-4.0 -I/opt/gtk-win64/include/pango-1.0 -I/opt/gtk-win64/include -I/opt/gtk-win64/include/glib-2.0 -I/opt/gtk-win64/lib/glib-2.0/include -I/opt/gtk-win64/include/harfbuzz -I/opt/gtk-win64/include/freetype2 -I/opt/gtk-win64/include/libpng16 -I/opt/gtk-win64/include/fribidi -I/opt/gtk-win64/include/cairo -I/opt/gtk-win64/include/pixman-1 -I/opt/gtk-win64/include/gdk-pixbuf-2.0 -I/opt/gtk-win64/include/webp -I/opt/gtk-win64/include/graphene-1.0 -I/opt/gtk-win64/lib/graphene-1.0/include -mfpmath=sse -msse -msse2 -DLIBDEFLATE_DLL
WIN_GTK_LIBS = -L/opt/gtk-win64/lib -lgtk-4 -lpangowin32-1.0 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lvulkan-1 -lgraphene-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl
WIN_LDFLAGS = -mwindows $(WIN_GTK_LIBS) -lm
WIN_CFLAGS_FULL = $(WIN_CFLAGS) $(WIN_GTK_INCLUDES)
WIN_TARGET = build-win/sticky-notes-app.exe
WIN_OBJ = $(patsubst %.c,build-win/%.o,$(SRC))
WIN_DEP = $(WIN_OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ) | build/
	gcc -o $@ $^ $(LDFLAGS)

win: $(WIN_TARGET)

$(WIN_TARGET): $(WIN_OBJ) | build-win/
	$(WIN_CC) -o $@ $^ $(WIN_LDFLAGS)

build/:
	mkdir -p build

build-win/:
	mkdir -p build-win

-include $(DEP)
-include $(WIN_DEP)

dist-win: win
	@echo "Creating Windows distribution..."
	@mkdir -p dist
	@cp build-win/sticky-notes-app.exe dist/
	@cp -r res/ dist/
	@for dll in libgtk-4-1.dll libglib-2.0-0.dll libgobject-2.0-0.dll libgio-2.0-0.dll libpango-1.0-0.dll libpangocairo-1.0-0.dll libpangowin32-1.0-0.dll libcairo-2.dll libcairo-gobject-2.dll libharfbuzz-0.dll libgraphene-1.0-0.dll libgdk_pixbuf-2.0-0.dll libintl-8.dll libffi-8.dll libpcre2-8-0.dll zlib1.dll libpng16-16.dll libfreetype-6.dll; do \
		if [ -f "/opt/gtk-win64/bin/$$dll" ]; then \
			cp "/opt/gtk-win64/bin/$$dll" dist/; \
			echo "Added: $$dll"; \
		fi; \
	done
	@echo "Windows distribution ready in dist/"

build/%.o: %.c | build/
	mkdir -p $(dir $@)
	gcc $(CFLAGS) -c $< -o $@

build-win/%.o: %.c | build-win/
	mkdir -p $(dir $@)
	$(WIN_CC) $(WIN_CFLAGS_FULL) -c $< -o $@

clean:
	rm -f $(OBJ) $(DEP) $(TARGET)
	rm -f $(WIN_OBJ) $(WIN_DEP) $(WIN_TARGET)
	rm -rf build build-win

.PHONY: all clean win
