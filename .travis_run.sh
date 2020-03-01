set -e

if [ -z "$STEAMLESS" ] && [ -z "$SSH_KEY" ]; then
	echo "Can't get steam API without SSH key, skipping"
	exit 0
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
	./configure --unsupported-compiler --with-internal-libs $STEAM
	make
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
	if [ -n "$STEAM" ]; then
		# Freetype header search is slightly broken, "fix" it by soft linking it to the expected place
		sudo ln -s /usr/include/freetype2 /usr/include/freetype2/freetype
		run.sh ./configure --with-external-sdl-mixer $STEAM
		LDFLAGS="-Wl,-rpath,XORIGIN/lib64:." run.sh make
	else
		mkdir src/extlib/include src/extlib/lib
		# Program seems to break (testing with Ubuntu 18.04) with internal SDL, so using external SDL
		# On the other hand, GOG copy is missing libpng12 so on systems with a newer libpng the game doesn't work
		# Fix that by using internal sdlimage
		# For sdlmixer, ciconia ships with a broken libvorbis, so this should get around that
		./configure --with-internal-sdl-image --with-internal-sdl-mixer
		LDFLAGS="-Wl,-rpath,XORIGIN/lib64:." make
	fi
	chrpath -r "\$ORIGIN/lib64:." src/ponscr
else
	# Windows build
	$mingw32 ./configure $STEAM
	$mingw32 make
fi

cd src
if [ "$TRAVIS_OS_NAME" == "windows" ]; then
	zip -9 ../ponscr.zip ponscr.exe
else
	zip -9 ../ponscr.zip ponscr
fi
cd ..

echo -n "$TRAVIS_OS_NAME$STEAM" > buildinfo.txt