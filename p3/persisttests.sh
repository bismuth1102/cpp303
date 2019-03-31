# [303] This is not a comprehensive test suite, especially because it does not
#       test for how the server responds to an incorrect client.  However, this
#       should be a useful script for testing the basic functionality of your
#       persistent server and client.

# create_file will produce a file named $1, consisting of 2^$2 bytes of data
function create_file () {
    echo -n "x" > "$1"
    for i in $(seq $2)
    do
        cat "$1" "$1" >> "$1".tmp
        mv "$1".tmp $1
    done
}

# set up a few constants to help this run smoothly
u1=aaaa              # user1's name.  4 chars, to make things easier
u2=bbbb              # user2's name.  4 chars, to make things easier
u1p=aaaa_is_awesome  # user1's pass
u2p=bbbb_is_the_best # user2's pass
sf=company.dir       # file where server saves data
ck=localhost.pub     # file where client saves key
kb=rsa               # basename of server key files
ip=localhost         # ip of server
p=9999               # port of server
k1=k1k1k1k1          # a key for kv tests
k2=k2k2k2k2          # a key for kv tests
k3=k3k3k3k3          # a key for kv tests
k4=k4k4k4k4          # a key for kv tests

# Reset file system from any prior failures
echo -n "Clearing server and client files from prior executions...         "
rm -f $kb.pri $kb.pub $sf # server makes these
rm -f $ck                 # client makes these
echo "OK"

# Generate a few files so that we can have known sizes for content
echo -n "Creating temporary data files... "
create_file "64k.dat" 16
echo -n "64K "
create_file "32k.dat" 15
echo -n "32K "
create_file "4k.dat" 12
echo -n "4K "
create_file "1k.dat" 10
echo -n "1K "
echo "                   OK"

# create an empty data file, so that we don't get an error message on server
# startup
touch $sf

# start server, wait a second...
echo
echo "Starting server..."
./obj64/server.exe -p $p -k $kb -f $sf > /dev/null &
sleep 1

echo -n "Registering new user $u1. Expect OK                              "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -R

echo -n "Setting $u1's content.  Expect OK                                "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -S 1k.dat
echo -n "Getting $u1's content.  Expect OK                                "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -G $u1
echo -n "Checking $u1's content.  Expect OK                               "
if [ $(diff $u1.file.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u1.file.dat

echo -n "Checking server content length.  Expect 1084                    "
# NB: 1084 because (8 + 4 + 4 + 4 + 16 + 4) + (8 + 4 + 4 + 4 + 1024)
#     (DIRENTRY ULEN UNAME PLEN PASS_HASH CLEN) 
#   + (DIRCNTNT ULEN UNAME CLEN CONTENT)
echo $(stat -c %s $sf)

echo -n "Instructing server to persist data.  Expect OK                    "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -V

echo -n "Checking server content length.  Expect 1064                    "
# Now expect 1064, because we go straight from first CLEN to final CONTENT
echo $(stat -c %s $sf)

echo -n "Updating content for $u1. Expect OK                              "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -S 4k.dat
echo -n "Updating content for $u1. Expect OK                              "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -S 64k.dat
echo -n "Updating content for $u1. Expect OK                              "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -S 32k.dat

echo -n "Checking server content length.  Expect 103524                "
# NB: 1064 + (8 + 4 + 4 + 4 + 4K) 
#                       + (8 + 4 + 4 + 4 + 64K)
#                       + (8 + 4 + 4 + 4 + 32K)
echo $(stat -c %s $sf)

echo -n "Getting $u1's content.  Expect OK                                "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -G $u1
echo -n "Checking $u1's content.  Expect OK                               "
if [ $(diff $u1.file.dat 32k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u1.file.dat

echo -n "Stopping server.  Expect OK and 'Server terminated'               "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -B
sleep 2

# start server, wait a second...
echo "Starting server..."
./obj64/server.exe -p $p -k $kb -f $sf > /dev/null &
sleep 1

echo -n "Getting $u1's content.  Expect OK                                "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -G $u1
echo -n "Checking $u1's content.  Expect OK                               "
if [ $(diff $u1.file.dat 32k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u1.file.dat

echo -n "Instructing server to persist data.  Expect OK                    "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -V

echo -n "Checking server content length.  Expect 32808                  "
# Now expect 32808, because we go straight from first CLEN to 32K CONTENT
echo $(stat -c %s $sf)

echo -n "Getting $u1's content.  Expect OK                                "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -G $u1
echo -n "Checking $u1's content.  Expect OK                               "
if [ $(diff $u1.file.dat 32k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u1.file.dat

# Now let's make sure that key operations work

echo -n "Registering new user $u2. Expect OK                              "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -R

echo -n "Checking server content length.  Expect 32848                  "
# Now expect 32848, because u2 is (8 + 4 + 4 + 4 + 16 + 4)
echo $(stat -c %s $sf)


echo -n "Setting $u2.key1.  Expect OK                                     "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -I $k1 -v 1k.dat
echo -n "Setting $u2.key2.  Expect OK                                     "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -I $k2 -v 4k.dat
echo -n "Setting $u2.key3.  Expect OK                                     "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -I $k3 -v 32k.dat
echo -n "Setting $u2.key4.  Expect OK                                     "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -I $k4 -v 64k.dat

echo -n "Checking $u2.key1.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k1 -v $u2.$k1.dat
echo -n "Checking $u2.key1 content.  Expect OK                            "
if [ $(diff $u2.$k1.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k1.dat

echo -n "Checking $u2.key2.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k2 -v $u2.$k2.dat
echo -n "Checking $u2.key2 content.  Expect OK                            "
if [ $(diff $u2.$k2.dat 4k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k2.dat

echo -n "Checking $u2.key3.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k3 -v $u2.$k3.dat
echo -n "Checking $u2.key3 content.  Expect OK                            "
if [ $(diff $u2.$k3.dat 32k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k3.dat

echo -n "Checking $u2.key4.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k4 -v $u2.$k4.dat
echo -n "Checking $u2.key4 content.  Expect OK                            "
if [ $(diff $u2.$k4.dat 64k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k4.dat

echo -n "Checking server content length.  Expect 136400                "
# Now expect 136400, because each key is 8 bytes, so each is (8 + 4 + 4 + 4 + 8 + 4 + SIZE), so 32848+(32*4)+1K+4K+32K+64K
echo $(stat -c %s $sf)

echo -n "Updating $u2.key4.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -U $k4 -v 1k.dat
echo -n "Checking $u2.key4.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k4 -v $u2.$k4.dat
echo -n "Checking $u2.key4 content.  Expect OK                            "
if [ $(diff $u2.$k4.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k4.dat

echo -n "Deleting $u2.key3.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -D $k3
echo -n "Checking $u2.key4.  Expect ERR_BADKEY                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k3 -v $u2.$k3.dat

echo -n "Checking server content length.  Expect 137484                "
# Now expect 137484, because UPD is 8+4+4+4+8+4+1K and DEL is 8+4+4+4+8
echo $(stat -c %s $sf)

echo -n "Stopping server.  Expect OK and 'Server terminated'               "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -B
sleep 2

# start server, wait a second...
echo "Starting server..."
./obj64/server.exe -p $p -k $kb -f $sf > /dev/null &
sleep 1

echo -n "Checking $u2.key1.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k1 -v $u2.$k1.dat
echo -n "Checking $u2.key1 content.  Expect OK                            "
if [ $(diff $u2.$k1.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k1.dat

echo -n "Checking $u2.key2.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k2 -v $u2.$k2.dat
echo -n "Checking $u2.key2 content.  Expect OK                            "
if [ $(diff $u2.$k2.dat 4k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k2.dat

echo -n "Checking $u2.key3.  Expect ERR_BADKEY                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k3 -v $u2.$k3.dat

echo -n "Checking $u2.key4.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k4 -v $u2.$k4.dat
echo -n "Checking $u2.key4 content.  Expect OK                            "
if [ $(diff $u2.$k4.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k4.dat

echo -n "Instructing server to persist data.  Expect OK                    "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -V

echo -n "Checking server content length.  Expect 39064                  "
# NB: u1 (32808) + u1's keys 3*(8+4+8+4+{1k/4k/1k}) + u2 (8+4+4+4+16+4) + 
echo $(stat -c %s $sf)

echo -n "Checking $u2.key1.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k1 -v $u2.$k1.dat
echo -n "Checking $u2.key1 content.  Expect OK                            "
if [ $(diff $u2.$k1.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k1.dat

echo -n "Checking $u2.key2.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k2 -v $u2.$k2.dat
echo -n "Checking $u2.key2 content.  Expect OK                            "
if [ $(diff $u2.$k2.dat 4k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k2.dat

echo -n "Checking $u2.key3.  Expect ERR_BADKEY                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k3 -v $u2.$k3.dat

echo -n "Checking $u2.key4.  Expect OK                                    "
obj64/client.exe -k $ck -u $u2 -w $u2p -s $ip -p $p -T $k4 -v $u2.$k4.dat
echo -n "Checking $u2.key4 content.  Expect OK                            "
if [ $(diff $u2.$k4.dat 1k.dat | wc -c) == "0" ]; then echo OK; else echo ERR; fi
rm $u2.$k4.dat

echo -n "Stopping server.  Expect OK and 'Server terminated'               "
obj64/client.exe -k $ck -u $u1 -w $u1p -s $ip -p $p -B
sleep 2

# Reset file system
rm $kb.pri $kb.pub $sf           # server makes these
rm $ck                           # client makes these
rm 1k.dat 4k.dat 32k.dat 64k.dat # script makes these