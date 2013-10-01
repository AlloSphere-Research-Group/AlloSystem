

FILENAME=`echo $1 |cut -d'.' -f1 | sed -e "s|/|_|g"`
TARGET=${FILENAME}_run
ALLOVSR_BUILD=0

if [[ $FILENAME == allovsr* ]]
    then
    ALLOVSR_BUILD=1
fi

if [ ! -f Makefile ]; then
    ./distclean
    cmake . -DBUILD_ALLOGLV=1 -DBUILD_GLV=1 -DBUILD_ALLOVSR=$ALLOVSR_BUILD &> cmake_output.txt
    if [ $? != 0 ]; then
	cat cmake_output.txt
	exit 1
    fi
fi 

echo "Building $1"
make ${FILENAME} -j4 $* &> make_output.txt


# If command failed, try again after running cmake (in case it is a new target)
if [ $? != "0" ]
    then
    TARGETS=`make -qp | awk -F':' '/^[a-zA-Z0-9][^$#\/\t=]*:([^=]|$)/ {split($1,A,/ /);for(i in A)print A[i]}'`
    case $TARGETS in
	*"$TARGET"*) cat make_output.txt 
		exit 1
		;;
	
	*) echo "Can't find target --------------- Running CMAKE again"
	    cmake . -DBUILD_ALLOGLV=1 -DBUILD_GLV=1  -DBUILD_ALLOVSR=$ALLOVSR_BUILD &> cmake_output.txt
	    if [ $? != 0 ]; then
		cat cmake_output.txt
		exit 1
	    fi
	    make ${FILENAME} -j4 $* &> make_output.txt
	    if [ $? != "0" ]
	    then
		cat make_output.txt
	    fi
    esac
fi

make ${TARGET}