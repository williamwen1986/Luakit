#-------------------------------------------------------------------
if [ -z "$CONFIG" ]
then
    	export CONFIG=Debug
fi

if [ -z "$OUTPUT_DIR" ]
then
	export	OUTPUT_DIR=../libs/linux-$CONFIG
fi


../bin/build-linux.sh
#--------------------------------------------------------------------
