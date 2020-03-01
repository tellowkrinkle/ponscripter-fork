if [ -n "$STEAMLESS" ] && [ -z "$SSH_KEY" ]; then
	echo "Run with steamless but no SSH key, this would be a repeat of steamless with an SSH key, stopping."
	exit 0
elif [ -z "$STEAMLESS" ] && [ -n "$SSH_KEY" ]; then
	STEAM="-steam"
	if [ "$TRAVIS_OS_NAME" == "linux" ]; then
		export STEAM_RUNTIME_HOST_ARCH=$(dpkg --print-architecture)
		export STEAM_RUNTIME_ROOT="$(pwd)/steam/runtime/amd64"
		export STEAM_RUNTIME_TARGET_ARCH=amd64
		export PATH="$(pwd)/steam/bin:$PATH"
	fi
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
	./configure --unsupported-compiler --with-internal-libs $STEAM
	make
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
	if [ -n "$STEAM" ]; then
		# Freetype header search is slightly broken, "fix" it by soft linking it to the expected place
		sudo ln -s /usr/include/freetype2 /usr/include/freetype2/freetype
		run.sh ./configure --with-external-sdl-mixer $STEAM
		LDFLAGS="-Wl,-rpath,\\\$\$ORIGIN/lib64:." run.sh make
	else
		mkdir src/extlib/include src/extlib/lib
		# Program seems to break (testing with Ubuntu 18.04) with internal SDL, so using external SDL
		# On the other hand, GOG copy is missing libpng12 so on systems with a newer libpng the game doesn't work
		# Fix that by using internal sdlimage
		./configure --with-internal-sdl-image
		LDFLAGS="-Wl,-rpath,\\\$\$ORIGIN/lib64:." make
	fi
else
	echo -n
	# TODO: Windows build
fi

cd src
zip -9 ../ponscr.zip ponscr*
cd ..

echo -n "$TRAVIS_OS_NAME$STEAM" > buildinfo.txt