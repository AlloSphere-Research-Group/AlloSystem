

FILENAME=`echo $1 |cut -d'.' -f1 | sed -e "s|/|_|g"`
TARGET=${FILENAME}_run
ALLOVSR_BUILD=0

if [[ $FILENAME == allovsr* ]]
    then
    ALLOVSR_BUILD=1
fi

if [ ! -f Makefile ]; then
    ./distclean
    cmake . -DBUILD_ALLOGLV=1 -DBUILD_GLV=1 -DBUILD_ALLOVSR=$ALLOVSR_BUILD
    if [ $? != 0 ]; then
	exit 1
    fi
fi 

make $TARGET -j4 $*

# If command failed, try again after running cmake (in case it is a new target)
if [ $? != "0" ]
    then
    TARGETS=`make -qp | awk -F':' '/^[a-zA-Z0-9][^$#\/\t=]*:([^=]|$)/ {split($1,A,/ /);for(i in A)print A[i]}'`
    case $TARGETS in
	*"$TARGET"*) echo "Target exists." ;;
	*) echo "Can't find target --------------- Running CMAKE again"
	    cmake . -DBUILD_ALLOGLV=1 -DBUILD_GLV=1  -DBUILD_ALLOVSR=$ALLOVSR_BUILD
	    if [ $? != 0 ]; then
		exit 1
	    fi
	    make `echo $1 |cut -d'.' -f1 | sed -e "s|/|_|g"`_run -j4 $* ;;
    esac
fi