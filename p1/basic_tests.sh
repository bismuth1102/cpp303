# [303] This is not a comprehensive test suite, especially because it does not
#       test for how the server responds to an incorrect client.  However, this
#       should be a useful script for testing the basic functionality of your
#       server and client.

# Reset file system from any prior failures
rm -f rsa.pri rsa.pub company.dir # server makes these
rm -f localhost.pub               # client makes these

# rm -rf rsa.pri rsa.pub company.dir localhost.pub obj64/
# ./server.exe -p 9999 -k rsa -f company.dir > /dev/null

# Start server as a background process, wait a second for it to be ready
echo "Starting server... Expect error about company.dir not found"
./obj64/server.exe -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

# Register a user named alice, set her data, get her data, and make sure data
# matches
echo "Expect 3x OK"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -R
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -S client_args.h
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -G alice
diff alice.file.dat client_args.h
rm alice.file.dat

# Get all users, make sure it's just alice
echo "Expect OK"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -A allfile
echo -n "alice" > expect
diff allfile expect
rm allfile expect

# Stop the server
echo "Expect OK and Server terminated"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B

# Restart the server, wait a second
./obj64/server.exe -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

# Attempt to register alice should fail
echo "Expect ERR_USER_EXISTS"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -R

# Attempt to get data should still work
echo "Expect OK"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -G alice
diff alice.file.dat client_args.h
rm alice.file.dat

# Attempt to log in with bad password should fail
echo "Expect ERR_LOGIN"
obj64/client.exe -k localhost.pub -u alice -w bob_is_the_best -s localhost -p 9999 -G alice

# Attempt to log in invalid alice should fail
echo "Expect ERR_USER_EXISTS"
obj64/client.exe -k localhost.pub -u bob -w alice_is_awesome -s localhost -p 9999 -G alice

# Create a new alice.  It should be able to get alice's data
echo "Expect 2x OK"
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -R
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -G alice
diff alice.file.dat client_args.h
rm alice.file.dat

# New alice data shouldn't exist
echo "Expect ERR_NO_DATA"
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -G bob

# get all alices, and it should have two alices now :)
# NB: No guarantees on order from serer
echo "Expect OK"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -A allfile
echo "alice" > expect; echo "bob" >> expect
sort -u allfile > tmp
mv tmp allfile
diff expect allfile
rm expect allfile

# Stop the server
echo "Expect OK and Server terminated"
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B
sleep 1

# Reset file system
rm rsa.pri rsa.pub company.dir # server makes these
rm localhost.pub               # client makes these