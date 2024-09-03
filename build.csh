
echo "*** Building r136 and supporting tools"
echo
cc -o r136 *.c -lcurses -ltermlib
cc -o tools/gendata tools/gendata.c lib.c

echo
echo "*** Removing and generating data files"
echo
rm -r data
tools/gendata

echo
echo "*** Creating tarball"
echo
tar cvf r136.tar r136 data

echo
echo "*** Done"