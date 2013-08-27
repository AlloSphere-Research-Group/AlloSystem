

FILENAME=`echo $1 |cut -d'.' -f1 | sed -e "s|/|_|g"`
ALLOVSR_BUILD=0

if [[ $FILENAME == allovsr* ]]
    then
    ALLOVSR_BUILD=1
fi

if [ ! -f Makefile ]; then
    .distclean
    cmake . -DBUILD_ALLOGLV=1 -DBUILD_GLV=1 -DBUILD_ALLOVSR=$ALLOVSR_BUILD
    if [ $? != 0 ]; then
	exit 1
    fi
fi 

make `echo $1 |cut -d'.' -f1 | sed -e "s|/|_|g"`_run -j4 $*

# If command failed, try again after running cmake (in case it is a new target)
if [ $? != "0" ]
    then
    echo "Can't find target --------------- Running CMAKE again"
    cmake . -DBUILD_ALLOGLV=1 -DBUILD_GLV=1  -DBUILD_ALLOVSR=$ALLOVSR_BUILD
    if [ $? != 0 ]; then
	exit 1
    fi
    make `echo $1 |cut -d'.' -f1 | sed -e "s|/|_|g"`_run -j4 $*
fi