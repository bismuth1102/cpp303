# [303] This is not a comprehensive test suite, especially because it does not
#       test for how the server responds to an incorrect client.  However, this
#       should be a useful script for testing the basic functionality of your
#       server and client.

# Reset file system from any prior failures
rm -f rsa.pri rsa.pub company.dir # server makes these
rm -f localhost.pub               # client makes these

echo ""

echo -n "Starting server. Expect 'File not found: company.dir'      "
./obj64/server.exe -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

echo -n "Registering new user alice. Expect OK                      "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -R
echo -n "Setting alice's content.  Expect OK                        "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -S client_args.h
echo -n "Checking alice's content.  Expect OK                       "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -G alice
diff alice.file.dat client_args.h
rm alice.file.dat

echo -n "Getting all users to make sure it's just alice.  Expect OK "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -A allfile
echo -n "alice" > expect
diff allfile expect
rm allfile expect

echo -n "Instructing server to persist data.  Expect OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -V

echo "----------------------------------------------------------------"

echo "Stopping server.  Expect Server terminated and OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B

echo ""

echo "Re-starting server to check that data persisted correctly"
./obj64/server.exe -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

echo -n "Re-registering alice.  Expect ERR_USER_EXISTS              "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -R

echo -n "Checking alice's old data.  Expect OK                      "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -G alice
diff alice.file.dat client_args.h
rm alice.file.dat

echo -n "Attempting access with bad password.  Expect ERR_LOGIN     "
obj64/client.exe -k localhost.pub -u alice -w bob_is_the_best -s localhost -p 9999 -G alice

echo -n "Attempting access with bad user.  Expect ERR_USER_EXISTS   "
obj64/client.exe -k localhost.pub -u bob -w alice_is_awesome -s localhost -p 9999 -G alice

echo -n "Registering user bob.  Expect OK                           "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -R

echo -n "Attempting to access alice's data by bob.  Expect OK       "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -G alice
diff alice.file.dat client_args.h
rm alice.file.dat

echo -n "Getting bob's nonexistent data.  Expect ERR_NO_DATA        "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -G bob

echo -n "Getting all users to check for alice and bob.  Expect OK   "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -A allfile
echo "alice" > expect; echo "bob" >> expect
sort -u allfile > tmp
mv tmp allfile
diff expect allfile
rm expect allfile

echo -n "Instructing server to persist data.  Expect OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -V

echo "----------------------------------------------------------------"

echo "Stopping server.  Expect Server terminated and OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B
sleep 1

# Reset file system
rm rsa.pri rsa.pub company.dir # server makes these
rm localhost.pub               # client makes these