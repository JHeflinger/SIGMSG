./build.sh
if [ $? -ne 0 ]; then
	exit 1
fi
./build/bin.exe $1 $2 $3 $4 $5
